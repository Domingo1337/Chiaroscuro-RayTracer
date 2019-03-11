#pragma once
#include <camera.hpp>
#include <fstream>
#include <glm/gtx/intersect.hpp>
#include <iostream>
#include <model.hpp>
#include <scene.hpp>
#include <vector>

class RayCaster {
  public:
    Model &model;
    Scene &scene;
    std::vector<std::vector<glm::vec3>> pixels;

    /* Fill pixels with rays shot on screen centered between eye and center */
    void rayTrace(glm::vec3 eye, glm::vec3 center, glm::vec3 up);

    /* Print render in PPM format to specified stream (default: std::cout) */
    void printPPM(std::ostream &ostream);

    /* Intersect ray specified by origin and direction with every triangle in the model,
       storing the hitpoint's position in cross and color in pixel */
    bool intersectRayModel(const glm::vec3 &origin, const glm::vec3 &direction, glm::vec3 &pixel, glm::vec3 &cross);

    RayCaster(Model &_model, Scene &_scene)
        : model(_model), scene(_scene), pixels(scene.yres, std::vector<glm::vec3>(scene.xres)) {}
};

bool RayCaster::intersectRayModel(const glm::vec3 &origin, const glm::vec3 &direction, glm::vec3 &pixel,
                                  glm::vec3 &cross) {
    for (auto &mesh : model.meshes) {
        auto &indices = mesh.indices;
        auto &vertices = mesh.vertices;
        /* triangles loop */
        for (int i = 0; i < indices.size(); i += 3) {

            auto &A = vertices[indices[i]].Position;
            auto &B = vertices[indices[i + 1]].Position;
            auto &C = vertices[indices[i + 2]].Position;

            glm::vec3 baryPosition;
            if (glm::intersectRayTriangle(origin, direction, A, B, C, baryPosition)) {
                cross = origin + baryPosition.z * direction;
                pixel = mesh.color.diffuse;
                return true;
            }
        }
    }
    pixel = {0.f, 0.f, 0.f};
    return false;
}

void RayCaster::rayTrace(glm::vec3 eye, glm::vec3 center, glm::vec3 up = {0.f, 0.f, 0.f}) {
    float z = 100.f;
    float y = z * 0.5f * scene.yview;
    float x = y * ((float)scene.xres / (float)scene.yres);

    glm::vec3 leftUpper = {-x, y, -z};
    glm::vec3 leftLower = {-x, -y, -z};
    glm::vec3 rightUpper = {x, y, -z};

    /* rotate the corners of screen to look from VP to LA */
    auto rotate = glm::inverse(glm::mat3(glm::lookAt(eye, center, up)));
    leftUpper = rotate * leftUpper;
    leftLower = rotate * leftLower;
    rightUpper = rotate * rightUpper;

    glm::vec3 dx = (1.f / scene.xres) * (rightUpper - leftUpper);
    glm::vec3 dy = (1.f / scene.yres) * (leftLower - leftUpper);

    glm::vec3 current = leftUpper + eye + 0.5f * (dy + dx);

    for (int y = 0; y < scene.yres; y++, current += dy) {
        glm::vec3 currentRay = current;
        for (int x = 0; x < scene.xres; x++, currentRay += dx) {
            glm::vec3 cross;
            if (intersectRayModel(eye, currentRay, pixels[y][x], cross) && scene.k) {
                /* tbh I have only implemented the algorithm for k == 0 */
                /*  check for shadow rays */
                for (auto &light : scene.lights) {
                    glm::vec3 temp_pixel, temp_cross;
                    if (intersectRayModel(cross + (0.0001f * (light.position - cross)), light.position, temp_pixel, temp_cross)) {
                        // just to show which mesh blocked the light we set the pixel color to mesh's (darker) color
                        pixels[y][x] = 0.5f * temp_pixel;
                    }
                }
            }
        }
    }
}

void RayCaster::printPPM(std::ostream &ostream = std::cout) {
    if (pixels.size() < scene.yres)
        return;
    ostream << "P3\n";
    ostream << scene.xres << " " << scene.yres << " \n255\n";
    for (int y = 0; y < scene.yres; y++) {
        for (int x = 0; x < scene.xres; x++) {
            ostream << (unsigned)(255.f * pixels[y][x].x) << " " << (unsigned)(255.f * pixels[y][x].y) << " "
                    << (unsigned)(255.f * pixels[y][x].z) << " ";
        }
        ostream << "\n";
    }
}
