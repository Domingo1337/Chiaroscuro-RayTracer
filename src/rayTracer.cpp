#include "rayTracer.hpp"
#include "prng.hpp"

#include <FreeImage.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/io.hpp>
#include <omp.h>

#include <chrono>
#include <iostream>

RayTracer::RayTracer(Model &_model, Scene &_scene)
    : scene(_scene), pixels(_scene.yres, std::vector<glm::vec3>(_scene.xres)), data(scene.yres * scene.xres * 3),
      kdtree(_model, _scene) {}

void RayTracer::rayTrace(glm::vec3 eye, glm::vec3 center, glm::vec3 up = {0.f, 1.f, 0.f}, float yview = 1.f) {
    static unsigned layers = 0;
    static glm::vec3 lastEye = glm::vec3(FLT_MAX);
    static glm::vec3 lastCenter = glm::vec3(FLT_MAX);
    static glm::vec3 lastUp = glm::vec3(FLT_MAX);
    static float lastYview = -1;

    const bool newLayer = (eye == lastEye) && (center == lastCenter) && (lastUp == lastUp) && (yview == lastYview);
    if (newLayer)
        layers++;
    else {
        layers = 1;
        lastEye = eye;
        lastCenter = center;
        lastUp = up;
        lastYview = yview;
    }

    std::cerr << "Camera at " << eye << " facing: " << center << " with up: " << up << " and yview: " << yview
              << "\nRendering image of size " << scene.xres << "x" << scene.yres << " with " << layers * scene.samples
              << " samples, using " << omp_get_max_threads() << " threads...\t";

    auto beginTime = std::chrono::high_resolution_clock::now();

    float z = 1.f;
    float y = z * 0.5f * yview;
    float x = y * ((float)scene.xres / (float)scene.yres);

    /* rotate the corners of screen to look from VP to LA */
    auto rotate = glm::inverse(glm::mat3(glm::lookAt(eye, center, up)));
    const glm::vec3 dy = (1.f / scene.yres) * rotate * glm::vec3(0.f, -2.f * y, 0.f);
    const glm::vec3 dx = (1.f / scene.xres) * rotate * glm::vec3(2.f * x, 0.f, 0.f);
    const glm::vec3 leftUpper = rotate * glm::vec3(-x, y, -z);

    maxVal = 0.f;
    const float invSamples = 1.f / scene.samples;

    PRNG::setSeed();
#pragma omp parallel for
    for (unsigned y = 0; y < scene.yres; y++) {
        for (unsigned x = 0; x < scene.xres; x++) {
            glm::vec3 temp;
            for (unsigned s = 0; s < scene.samples; s++)
                temp += sendRay(
                    eye, leftUpper + (x + PRNG::uniformFloat(0.f, 1.f)) * dx + (y + PRNG::uniformFloat(0.f, 1.f)) * dy,
                    1);

            pixels[y][x] = (pixels[y][x] * float(layers - 1) + (temp * invSamples)) / float(layers);

            maxVal = maxVal > pixels[y][x].r ? maxVal : pixels[y][x].r;
            maxVal = maxVal > pixels[y][x].g ? maxVal : pixels[y][x].g;
            maxVal = maxVal > pixels[y][x].b ? maxVal : pixels[y][x].b;
        }
    }

    auto finishedTime = std::chrono::high_resolution_clock::now();
    std::cerr << "took " << (finishedTime - beginTime).count() * 0.000000001f << " seconds.\a\n";
}

glm::vec3 RayTracer::sendRay(const glm::vec3 &origin, const glm::vec3 dir, const int k) {
    glm::vec3 intersection;
    glm::vec3 normal;
    BRDF *material;
    if (intersectRayKDTree(origin, dir, intersection, normal, material)) {
        // inverse direction
        const glm::vec3 wo = glm::normalize(origin - intersection);

        // nonzero only when primary ray had hit the light surface
        glm::vec3 direct = (k > 1) ? glm::vec3(0.f) : material->radiance() * std::max(0.f, glm::dot(wo, normal));

        // calculate direct lightning
        // choose random point on surface lights
        if (scene.lightTriangles.size()) {

            auto &light = scene.randomLight();
            const Triangle &lightSurface = kdtree.triangles[light.id];
            const Material &lightMat = kdtree.materials[light.id];

            // uniform barycentric coordinates
            const float v0 = PRNG::uniformFloat(0.f, 1.f);
            const float v1 = PRNG::uniformFloat(0.f, 1.f - v0);
            const glm::vec3 lightPoint =
                v0 * lightSurface.posFst + v1 * lightSurface.posSnd + (1.f - v0 - v1) * lightSurface.posTrd;

            const float distance = glm::distance(intersection, lightPoint);
            const glm::vec3 wl = glm::normalize(lightPoint - intersection);

            if (!kdtree.intersectShadowRay(intersection + (0.001f * normal), wl, distance, light.id)) {
                const float geometric =
                    std::max(0.f, glm::dot(normal, wl) * glm::dot(-wl, lightMat.normal) / (1.f + distance * distance));

                direct += lightMat.Ke * (geometric * light.surface * scene.lightTriangles.size()) *
                          material->f(wl, wo, normal);
            }
        }

        if (k == scene.k) {
            delete material;
            return direct;
        }

        // calculate indirect light
        glm::vec3 wi;
        float pdf;
        const glm::vec3 f = material->sample_wi(wi, wo, normal, pdf);
        delete material;

        // Roussian roulette termination
        const float Kmax = std::max(std::max(f.r, f.g), f.b);
        if (pdf == 0.f || PRNG::uniformFloat(0.f, 1.f) > Kmax)
            return direct;

        const float cosine = std::abs(glm::dot(normal, wi));
        const glm::vec3 indirect = (f * cosine / (pdf * Kmax)) * sendRay(intersection + 0.001f * normal, wi, k + 1);

        return direct + indirect;
    }
    return scene.background;
}

bool RayTracer::intersectRayKDTree(const glm::vec3 &origin, const glm::vec3 &direction, glm::vec3 &intersection,
                                   glm::vec3 &normal, BRDF *&brdf) {
    glm::vec2 baryPos;
    float distance;
    id_t triangleID;
    if (!kdtree.intersectRay(origin, direction, triangleID, baryPos, distance))
        return false;

    const Triangle &triangle = kdtree.triangles[triangleID];
    const Material &material = kdtree.materials[triangleID];

    normal = material.normal;

    const float baryPosz = (1.f - baryPos.x - baryPos.y);
    intersection = triangle.posFst * baryPosz + triangle.posSnd * baryPos.x + triangle.posTrd * baryPos.y;

    const glm::vec3 Kd =
        material.texDiffuse && material.texDiffuse->image
            ? material.texDiffuse->getColorAt(material.texFst * baryPosz + material.texSnd * baryPos.x +
                                              material.texTrd * baryPos.y)
            : material.Kd;

    switch (material.BRDFtype) {
    case BRDFT::Diffuse:
        brdf = new Diffuse(Kd);
        break;
    case BRDFT::Emissive:
        brdf = new Emissive(Kd, material.Ke);
        break;
    }

    return true;
}

uint8_t *RayTracer::getData() { return data.data(); }

static inline float knee(double x, double f) { return logf(x * f + 1) / f; }

static float findKneeF(float x, float y) {
    float f0 = 0;
    float f1 = 1;

    while (knee(x, f1) > y) {
        f0 = f1;
        f1 = f1 * 2;
    }

    for (int i = 0; i < 30; ++i) {
        float f2 = (f0 + f1) / 2;
        float y2 = knee(x, f2);

        if (y2 < y)
            f1 = f2;
        else
            f0 = f2;
    }

    return (f0 + f1) / 2;
}

// As in exrdisplay implementation
void RayTracer::normalizeImage(float exposure, float defog, float kneeLow, float kneeHigh, float gamma) {
    if (exposure == FLT_MAX)
        exposure = scene.exposure;

    const float m = powf(2.f, exposure + 2.47393f);
    const float s = 255.f * powf(2.f, -3.5f * gamma);
    const float kl = powf(2.f, kneeLow);
    const float f = findKneeF(powf(2.f, kneeHigh), powf(2.f, 3.5) - kl);

    auto transform = [=](float x) {
        x = std::max(0.f, x - defog);
        x *= m;
        if (x > kl)
            x = kl + knee(x - kl, f);
        return glm::clamp(powf(x, gamma) * s, 0.f, 255.f);
    };

    for (unsigned y = 0; y < scene.yres; y++) {
        for (unsigned x = 0; x < scene.xres; x++) {
            int i = 3 * ((scene.yres - y - 1) * scene.xres + x);
            data[i++] = (uint8_t)transform(pixels[y][x].r);
            data[i++] = (uint8_t)transform(pixels[y][x].g);
            data[i++] = (uint8_t)transform(pixels[y][x].b);
        }
    }
}

void RayTracer::exportImage(const char *filename) {
    FreeImage_Initialise();
    FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(filename);
    FIBITMAP *bitmap;

    if (format == FIF_EXR || format == FIF_HDR) {
        // export in high dynamic range
        bitmap = FreeImage_AllocateT(FIT_RGBF, scene.xres, scene.yres);
        if (!bitmap) {
            std::cerr << "Couldn't allocate the image.\n";
            std::cerr << "FreeImage export failed.\n";
            return FreeImage_DeInitialise();
        }

        unsigned offset = FreeImage_GetPitch(bitmap);
        auto bits = FreeImage_GetBits(bitmap);

        for (unsigned y = scene.yres; y-- > 0;) {
            float *pixel = reinterpret_cast<float *>(bits);
            for (unsigned x = 0; x < scene.xres; x++) {
                *(pixel++) = pixels[y][x].r;
                *(pixel++) = pixels[y][x].g;
                *(pixel++) = pixels[y][x].b;
            }
            bits += offset;
        }

    } else {
        // export to normal image, so apply some kind of transformation
        normalizeImage();
        bitmap = FreeImage_Allocate(scene.xres, scene.yres, 24);
        if (!bitmap) {
            std::cerr << "Couldn't allocate the image.\n";
            std::cerr << "FreeImage export failed.\n";
            return FreeImage_DeInitialise();
        }
        RGBQUAD color;
        for (unsigned y = 0, i = 0; y < scene.yres; y++) {
            for (unsigned x = 0; x < scene.xres; x++) {
                color.rgbRed = data[i++];
                color.rgbGreen = data[i++];
                color.rgbBlue = data[i++];
                FreeImage_SetPixelColor(bitmap, x, y, &color);
            }
        }
    }

    if (FreeImage_Save(format, bitmap, filename, 0))
        std::cerr << "Render succesfully saved to file " << filename << "\n";
    else {
        std::cerr << "Couldn't save the image.\n";
        std::cerr << "FreeImage export failed.\n";
    }
    FreeImage_DeInitialise();
}
