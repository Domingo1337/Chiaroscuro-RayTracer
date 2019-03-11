#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>

#include <camera.hpp>
#include <model.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Light {
    glm::vec3 position;
    glm::vec3 color;
    float &r = color.r;
    float &g = color.g;
    float &b = color.b;
    float &x = position.x;
    float &y = position.y;
    float &z = position.z;
    float intensity;
};

class Scene {
  public:
    Scene(std::string filename);
    Scene();
    void transform(Model &model);
    std::string comment;
    std::string objFile;
    std::string pngFile;
    int k;
    unsigned xres;
    unsigned yres;
    glm::vec3 VP;
    glm::vec3 LA;
    glm::vec3 UP;
    float yview;
    std::vector<Light> lights;
};

Scene::Scene(std::string filename) {
    std::ifstream input(filename);
    getline(input, comment);
    input >> objFile >> pngFile >> k >> xres >> yres;
    input >> VP.x >> VP.y >> VP.z;
    input >> LA.x >> LA.y >> LA.z;
    input >> UP.x >> UP.y >> UP.z;
    input >> yview;
    char c;
    while (input >> c) {
        if (c != 'L') {
            input.unget();
            break;
        }
        Light temp;
        input >> temp.position.x >> temp.position.y >> temp.position.z;
        input >> temp.color.r >> temp.color.g >> temp.color.b;
        input >> temp.intensity;
        lights.push_back(temp);
    }
    input.close();
}

void Scene::transform(Model &model) {
    auto lookAtMtx = glm::lookAt(VP, LA, UP);
    for (auto &light : lights) {
        glm::vec4 temp(light.position.x, light.position.y, light.position.z, 1.f);
        light.position = lookAtMtx * temp;
    }
    for (auto &mesh : model.meshes) {
        for (auto &vertex : mesh.vertices) {
            glm::vec4 temp(vertex.Position.x, vertex.Position.y, vertex.Position.z, 1.f);
            vertex.Position = lookAtMtx * temp;
        }
    }
}

#endif
