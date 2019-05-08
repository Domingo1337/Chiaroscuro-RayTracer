#include "scene.hpp"

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
    std::ifstream input(filename);
    char c;
    while (input >> c && c == '#') {
        input.ignore(256, '\n');
    }
    input.unget();

    input >> objFile >> pngFile >> k >> xres >> yres;
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
        ambientLight += color;

        lights.push_back(Light(color, position, intensity));
    }
    input.close();

    // some meaningless number here
    ambientLight /= 10 * lights.size();
}

Light::Light(glm::vec3 _color, glm::vec3 _position, float _intensity)
    : color(_color), position(_position), intensity(_intensity) {}
