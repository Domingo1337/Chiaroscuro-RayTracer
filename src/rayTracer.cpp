#include "rayTracer.hpp"
#include "model.hpp"

#include <FreeImage.h>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <iostream>

RayTracer::RayTracer(Model &_model, Scene &_scene)
    : model(_model), scene(_scene), pixels(scene.yres, std::vector<glm::vec3>(scene.xres)),
      data(scene.yres * scene.xres * 3), kdtree(_model, scene.kdtreeLeafSize) {}

void RayTracer::rayTrace(glm::vec3 eye, glm::vec3 center, glm::vec3 up = {0.f, 1.f, 0.f}, float yview = 1.f) {
    std::cerr << "Rendering image of size " << scene.xres << "x" << scene.yres << "...\t";
    auto beginTime = std::chrono::high_resolution_clock::now();

    float z = 1.f;
    float y = z * 0.5f * yview;
    float x = y * ((float)scene.xres / (float)scene.yres);

    glm::vec3 leftUpper = {-x, y, -z};
    glm::vec3 dy = {0.f, -2.f * y, 0.f};
    glm::vec3 dx = {2.f * x, 0.f, 0.f};

    /* rotate the corners of screen to look from VP to LA */
    auto rotate = glm::inverse(glm::mat3(glm::lookAt(eye, center, up)));
    leftUpper = rotate * leftUpper;
    dy = (1.f / scene.yres) * rotate * dy;
    dx = (1.f / scene.xres) * rotate * dx;

    maxVal = 0.f;

    glm::vec3 current = leftUpper + 0.5f * (dy + dx);
    for (unsigned y = 0; y < scene.yres; y++, current += dy) {
        // std::cerr << y << "/" << scene.yres << "... ";

        glm::vec3 currentRay = current;
        for (unsigned x = 0; x < scene.xres; x++, currentRay += dx) {
            pixels[y][x] = sendRay(eye, currentRay, scene.k);

            maxVal = maxVal > pixels[y][x].r ? maxVal : pixels[y][x].r;
            maxVal = maxVal > pixels[y][x].g ? maxVal : pixels[y][x].g;
            maxVal = maxVal > pixels[y][x].b ? maxVal : pixels[y][x].b;

            glm::vec3 pixel = glm::clamp(pixels[y][x], 0.f, 1.f);
            int i = 3 * ((scene.yres - y - 1) * scene.xres + x);
            data[i++] = (uint8_t)(255.f * pixel.r);
            data[i++] = (uint8_t)(255.f * pixel.g);
            data[i++] = (uint8_t)(255.f * pixel.b);
        }
    }

    auto finishedTime = std::chrono::high_resolution_clock::now();
    std::cerr << "took " << std::chrono::duration_cast<std::chrono::milliseconds>(finishedTime - beginTime).count()
              << " miliseconds.\n";
}

glm::vec3 RayTracer::sendRay(glm::vec3 origin, glm::vec3 dir, int k) {
    glm::vec3 pixel = {0.f, 0.f, 0.f};
    glm::vec3 cross;
    glm::vec3 normal;
    Color color;
    if (intersectRayKDTree(origin, dir, cross, normal, color)) {
        if (k == 0) {
            pixel = color.diffuse;
        } else {
            glm::vec3 V = glm::normalize(origin - cross); // vector from cross to origin
            glm::vec3 N = glm::normalize(normal);         // normal in point of cross

            glm::vec3 diffuse = {0.f, 0.f, 0.f};
            glm::vec3 specular = {0.f, 0.f, 0.f};
            for (auto &light : scene.lights) {
                /* render lights */
                // if (glm::areCollinear(dir, light.position - origin, 0.005f)) {
                //     pixel = light.color * light.intensity;
                //     break;
                // }

                glm::vec3 tempCross, tempNormal;
                Color tempColor;
                glm::vec3 lightdir = glm::normalize(light.position - cross);
                if (!kdtree.intersectShadowRay(cross + (0.0001f * lightdir), lightdir)) {

                    /* Phong's model */
                    glm::vec3 L = lightdir;
                    glm::vec3 R = glm::normalize(2.f * (glm::dot(L, N)) * N - L);

                    float distance = glm::distance(cross, light.position);
                    float attentuation = 1.f; //(1.f / (1.f + distance * distance));
                    glm::vec3 lightColor = light.color * light.intensity * attentuation;

                    diffuse += glm::max(glm::dot(L, N), 0.f) * lightColor;
                    specular += glm::pow(glm::max(glm::dot(R, V), 0.f), color.shininess) * lightColor;
                }
            }

            pixel += color.ambient * scene.ambientLight + color.diffuse * diffuse + color.specular * specular;

            glm::vec3 reflectedDir = glm::normalize(2.f * glm::dot(V, N) * N - V);
            if (k > 1)
                pixel += (float)M_1_PI * color.diffuse * sendRay(cross + 0.0001f * reflectedDir, reflectedDir, k - 1);
        }
    }
    return pixel;
}

bool RayTracer::intersectRayKDTree(const glm::vec3 &origin, const glm::vec3 &direction, glm::vec3 &cross,
                                   glm::vec3 &normal, Color &color) {
    glm::vec2 baryPos;
    float distance;
    Triangle triangle;
    if (!kdtree.intersectRay(origin, direction, triangle, baryPos, distance))
        return false;
    normal = kdtree.vertices[triangle.fst].Normal * (1.f - baryPos.x - baryPos.y) +
             kdtree.vertices[triangle.snd].Normal * baryPos.x + kdtree.vertices[triangle.trd].Normal * baryPos.y;
    if (kdtree.materials[triangle.fst])
        color = kdtree.materials[triangle.fst]->getColorAt(
            kdtree.vertices[triangle.fst].TexCoords * (1.f - baryPos.x - baryPos.y) +
            kdtree.vertices[triangle.snd].TexCoords * baryPos.x + kdtree.vertices[triangle.trd].TexCoords * baryPos.y);
    else
        color = Color(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f), 1.f);
    cross = origin + distance * direction;
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
