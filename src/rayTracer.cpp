#include "rayTracer.hpp"
#include "material.hpp"

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
    std::cerr << "Rendering image of size " << scene.xres << "x" << scene.yres << " with " << scene.samples
              << " samples, using " << omp_get_max_threads() << " threads...\t";

    auto beginTime = std::chrono::high_resolution_clock::now();

    float z = 1.f;
    float y = z * 0.5f * yview;
    float x = y * ((float)scene.xres / (float)scene.yres);

    /* rotate the corners of screen to look from VP to LA */
    auto rotate = glm::inverse(glm::mat3(glm::lookAt(eye, center, up)));
    const glm::vec3 dy = (1.f / scene.yres) * rotate * glm::vec3(0.f, -2.f * y, 0.f);
    const glm::vec3 dx = (1.f / scene.xres) * rotate * glm::vec3(2.f * x, 0.f, 0.f);
    const glm::vec3 leftUpper = rotate * glm::vec3(-x, y, -z) + 0.5f * (dx + dy);

    maxVal = 0.f;

#pragma omp parallel for
    for (unsigned y = 0; y < scene.yres; y++) {
        for (unsigned x = 0; x < scene.xres; x++) {
            pixels[y][x] = {0.f, 0.f, 0.f};
            for (unsigned s = 0; s < scene.samples; s++)
                pixels[y][x] += sendRay(
                    eye, leftUpper + (x + glm::linearRand(-0.5f, 0.5f)) * dx + (y + glm::linearRand(-0.5f, 0.5f)) * dy,
                    1);

            maxVal = maxVal > pixels[y][x].r ? maxVal : pixels[y][x].r;
            maxVal = maxVal > pixels[y][x].g ? maxVal : pixels[y][x].g;
            maxVal = maxVal > pixels[y][x].b ? maxVal : pixels[y][x].b;
        }
    }

    auto finishedTime = std::chrono::high_resolution_clock::now();
    std::cerr << "took " << (finishedTime - beginTime).count() * 0.000000001f << " seconds.\n";
}

// generate random point on hemisphere defined by normal with Phong exponent's shininess
// based off https://blog.thomaspoulet.fr/uniform-sampling-on-unit-hemisphere/
glm::vec3 hemisphereRand(float shininess = 1.f) {
    const float theta = acosf(powf(glm::linearRand(0.f, 1.f), 1.f / (1.f + shininess)));
    const float phi = M_PI * glm::linearRand(0.f, 2.f);

    const float x = sin(theta) * cos(phi);
    const float y = sin(theta) * sin(phi);
    return glm::vec3(x, y, -glm::sqrt(1.f - x * x - y * y));
}

glm::vec3 RayTracer::sendRay(const glm::vec3 &origin, const glm::vec3 dir, const int k) {
    glm::vec3 cross;
    glm::vec3 normal;
    BRDF *material;
    if (intersectRayKDTree(origin, dir, cross, normal, material)) {
        // inverse direction
        const glm::vec3 wo = glm::normalize(origin - cross);

        // nonzero only when primary ray had hit the light surface
        glm::vec3 direct = (k > 1) ? glm::vec3(0.f) : material->radiance() * std::max(0.f, glm::dot(wo, normal));

        // calculate direct light
        for (auto &light : scene.lightTriangles) { // TODO: change this to random light instead

            // choose random point on light triangle
            const float v0 = glm::linearRand(0.f, 1.f);
            const float v1 = glm::linearRand(0.f, 1.f - v0);
            const Triangle &lightTriangle = kdtree.triangles[light.id];
            const glm::vec3 lightPoint = v0 * lightTriangle.fst.Position + v1 * lightTriangle.snd.Position +
                                         (1.f - v0 - v1) * lightTriangle.trd.Position;

            const float distance = glm::distance(cross, lightPoint);
            const glm::vec3 wl = glm::normalize(lightPoint - cross);

            if (!kdtree.intersectShadowRay(cross + (0.001f * normal), wl, distance, light.id)) {
                const float geometric = std::max(0.f, glm::dot(normal, wl) * glm::dot(-wl, lightTriangle.fst.Normal) /
                                                          (1.f + distance * distance));
                direct += lightTriangle.brdf->radiance() * (geometric * light.invSurface) // incoming light
                          * material->f(wl, wo, normal);                                  // surface color
            }
        }
        if (k == scene.k)
            return direct;

        glm::vec3 wi;
        float pdf;
        const glm::vec3 brdf = material->sample_wi(wi, wo, normal, pdf);

        if (pdf == 0.f)
            return direct;

        const float cosine = std::abs(glm::dot(normal, wi));
        const glm::vec3 indirect = (brdf * cosine / pdf) * sendRay(cross + 0.001f * normal, wi, k + 1);

        return direct + indirect;
    }
    return scene.background;
}

bool RayTracer::intersectRayKDTree(const glm::vec3 &origin, const glm::vec3 &direction, glm::vec3 &cross,
                                   glm::vec3 &normal, BRDF *&brdf) {
    glm::vec2 baryPos;
    float distance;
    id_t triangle;
    if (!kdtree.intersectRay(origin, direction, triangle, baryPos, distance))
        return false;

    Vertex &fst = kdtree.triangles[triangle].fst;
    Vertex &snd = kdtree.triangles[triangle].snd;
    Vertex &trd = kdtree.triangles[triangle].trd;
    float baryPosz = (1.f - baryPos.x - baryPos.y);

    normal = fst.Normal * baryPosz + snd.Normal * baryPos.x + trd.Normal * baryPos.y;
    cross = fst.Position * baryPosz + snd.Position * baryPos.x + trd.Position * baryPos.y;
    brdf = kdtree.triangles[triangle].brdf.get();
    return true;
}

uint8_t *RayTracer::getData() { return data.data(); }

void RayTracer::normalizeImage() { normalizeImage(maxVal); }

void RayTracer::normalizeImage(float max) {
    if (max == 0.f)
        return;
    float inversedMax = 1.f / max;
    for (unsigned y = 0; y < scene.yres; y++) {
        for (unsigned x = 0; x < scene.xres; x++) {
            int i = 3 * ((scene.yres - y - 1) * scene.xres + x);
            data[i++] = (uint8_t)(255.f * glm::clamp((pixels[y][x].r) * inversedMax, 0.f, 1.f));
            data[i++] = (uint8_t)(255.f * glm::clamp((pixels[y][x].g) * inversedMax, 0.f, 1.f));
            data[i++] = (uint8_t)(255.f * glm::clamp((pixels[y][x].b) * inversedMax, 0.f, 1.f));
        }
    }
}

void RayTracer::exportImage(const char *filename) {
    FreeImage_Initialise();

    FIBITMAP *bitmap = FreeImage_Allocate(scene.xres, scene.yres, 24);
    if (!bitmap) {
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
    if (FreeImage_Save(FreeImage_GetFIFFromFilename(filename), bitmap, filename, 0))
        std::cerr << "Render succesfully saved to file " << filename << "\n";
    else
        std::cerr << "FreeImage export failed.\n";

    FreeImage_DeInitialise();
}
