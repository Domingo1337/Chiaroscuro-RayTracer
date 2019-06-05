#include "scene.hpp"

#include "glm/gtc/random.hpp"

#include <cstring>
#include <fstream>
#include <iostream>

Scene::Scene(int argc, char **argv) : Scene(argc > 1 ? argv[1] : "view_test.rtc") {
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--no-preview") == 0)
            usingOpenGLPreview = false;
        else if (strcmp(argv[i], "--preview-height") == 0)
            previewHeight = std::stoi(argv[++i]);
        else if (strcmp(argv[i], "--kdtree-leaf-size") == 0)
            kdtreeLeafSize = std::stoi(argv[++i]);
        else
            std::cerr << "Invalid argument" << argv[i] << "\n";
    }
}

Scene::Scene(std::string filename) {
    this->usingOpenGLPreview = true;
    this->previewHeight = 900;
    this->kdtreeLeafSize = 8;
    this->background = {0.5f, 0.5f, 0.5f};
    this->samples = 100;

    std::ifstream input(filename);
    char c;
    while (input >> c && c == '#') {
        input.ignore(256, '\n');
    }
    input.unget();

    input >> objPath >> renderPath >> k >> xres >> yres;
    input >> VP.x >> VP.y >> VP.z;
    input >> LA.x >> LA.y >> LA.z;
    input >> UP.x >> UP.y >> UP.z;
    input >> yview;

    while (input >> c) {
        if (c != 'L') {
            input.unget();
            break;
        }
        glm::vec3 color, position;
        input >> position.x >> position.y >> position.z;
        input >> color.r >> color.g >> color.b;

        float intensity;
        input >> intensity;

        color /= 255.f;

        lightPoints.push_back(LightPoint(color, position, intensity));
    }
    input.close();
}

LightPoint::LightPoint(glm::vec3 _color, glm::vec3 _position, float _intensity)
    : color(_color), position(_position), intensity(_intensity) {}

LightTriangle::LightTriangle(id_t _id, float _invSurface) : id(_id), invSurface(_invSurface){};
