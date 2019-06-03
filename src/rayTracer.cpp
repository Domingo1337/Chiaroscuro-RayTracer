#include "rayTracer.hpp"

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
                    scene.k);

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
    Color color;
    if (intersectRayKDTree(origin, dir, cross, normal, color)) {
        // inverse direction
        glm::vec3 viewer = glm::normalize(origin - cross);

        glm::vec3 direct(0.f, 0.f, 0.f);

        for (auto &light : scene.lights) {
            glm::vec3 lightdir = glm::normalize(light.position - cross);
            float distance = glm::distance(cross, light.position);

            Triangle placeholder;
            placeholder.fst = placeholder.snd = placeholder.trd = -1;
            if (!kdtree.intersectShadowRay(cross + (0.0001f * lightdir), lightdir, distance, placeholder)) {
                float attentuation = (1.f / (1.f + distance * distance));
                direct += light.color * attentuation * light.intensity;
                // make this more physically corect or get rid of it
            }
        }

        for (auto &light : scene.triangleLights) {
            // choose random point on light triangle
            const float v0 = glm::linearRand(0.f, 1.f);
            const float v1 = glm::linearRand(0.f, 1.f - v0);
            glm::vec3 lightPoint = v0 * kdtree.vertices[light.fst].Position + v1 * kdtree.vertices[light.snd].Position +
                                   (1.f - v0 - v1) * kdtree.vertices[light.trd].Position;

            float distance = glm::distance(cross, lightPoint);
            glm::vec3 lightdir = glm::normalize(lightPoint - cross);
            if (!kdtree.intersectShadowRay(cross + (0.0001f * lightdir), lightdir, distance, light)) {
                float attentuation = (1.f / (1.f + distance * distance));
                direct += kdtree.materials[light.fst]->materialColor.emissive * attentuation;
                // TODO:
                //  divided by light surface
                //  throw in some cosines
            }
        }

        if (k == 0)
            return direct * color.diffuse + color.emissive;

        // calculate reflected ray:
        // rotation to normal matrix
        glm::mat3 rotate;
        // if normal is the same as 'up' vector we cant use glm::inverse
        if (std::abs(normal.y) >= 0.99f) {
            float sgn = normal.y > 0.f ? 1.f : -1.f;
            rotate[0] = {1.f, 0.f, 0.f};
            rotate[1] = {0.f, 0.f, sgn};
            rotate[1] = {0.f, sgn, 0.f};
        } else
            rotate = glm::inverse(glm::mat3(glm::lookAt({0.f, 0.f, 0.f}, normal, {0.f, 1.f, 0.f})));
        const glm::vec3 reflectedDir = glm::normalize(rotate * hemisphereRand());

        const float indrctCoeff = fabs(glm::dot(viewer, reflectedDir)) * M_PI * 1.3f;
        const glm::vec3 indirect = indrctCoeff * sendRay(cross + 0.0001f * reflectedDir, reflectedDir, k - 1);

        // right now there's just diffuse colour
        return (direct + indirect) * float(M_1_PI) * color.diffuse;
    }
    return scene.background;
}

bool RayTracer::intersectRayKDTree(const glm::vec3 &origin, const glm::vec3 &direction, glm::vec3 &cross,
                                   glm::vec3 &normal, Color &color) {
    glm::vec2 baryPos;
    float distance;
    Triangle triangle;
    if (!kdtree.intersectRay(origin, direction, triangle, baryPos, distance))
        return false;

    Vertex &fst = kdtree.vertices[triangle.fst];
    Vertex &snd = kdtree.vertices[triangle.snd];
    Vertex &trd = kdtree.vertices[triangle.trd];
    float baryPosz = (1.f - baryPos.x - baryPos.y);

    normal = fst.Normal * baryPosz + snd.Normal * baryPos.x + trd.Normal * baryPos.y;
    cross = fst.Position * baryPosz + snd.Position * baryPos.x + trd.Position * baryPos.y;
    color = kdtree.materials[triangle.fst]->getColorAt(fst.TexCoords * baryPosz + snd.TexCoords * baryPos.x +
                                                       trd.TexCoords * baryPos.y);
    return true;
}

uint8_t *RayTracer::getData() { return data.data(); }

void RayTracer::normalizeImage() { normalizeImage(maxVal); }

void RayTracer::normalizeImage(float max) {
    if (max == 0.f)
        return;
    float inversedMax = 1 / max;
    for (unsigned y = 0; y < scene.yres; y++) {
        for (unsigned x = 0; x < scene.xres; x++) {
            glm::vec3 pixel = glm::clamp(pixels[y][x] * inversedMax, 0.f, 1.f);
            int i = 3 * ((scene.yres - y - 1) * scene.xres + x);
            data[i++] = (uint8_t)(255.f * pixel.r);
            data[i++] = (uint8_t)(255.f * pixel.g);
            data[i++] = (uint8_t)(255.f * pixel.b);
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
