#include "scene.hpp"
#include "prng.hpp"
#include <omp.h>

#include "glm/gtc/random.hpp"
#include <random>

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

Scene::Scene(int argc, char **argv) : Scene(argc > 1 ? argv[1] : "cornell.rtc") {
    for (int i = 2; i < argc; i++)
        params.emplace_back(argv[i]);

    for (unsigned i = 0; i < params.size(); i++) {
        if (params[i][0] == '#')
            continue;
        else if (params[i] == "no-preview")
            this->usingOpenGLPreview = false;
        else if (params[i] == "input")
            this->objPath = params[++i];
        else if (params[i] == "output")
            this->renderPath = params[++i];
        else if (params[i] == "k")
            this->k = std::stoi(params[++i]);
        else if (params[i] == "xres")
            this->xres = std::stoi(params[++i]);
        else if (params[i] == "yres")
            this->yres = std::stoi(params[++i]);
        else if (params[i] == "VP") {
            const float x = std::stof(params[++i]);
            const float y = std::stof(params[++i]);
            const float z = std::stof(params[++i]);
            this->VP = glm::vec3(x, y, z);
        } else if (params[i] == "LA") {
            const float x = std::stof(params[++i]);
            const float y = std::stof(params[++i]);
            const float z = std::stof(params[++i]);
            this->LA = glm::vec3(x, y, z);
        } else if (params[i] == "UP") {
            const float x = std::stof(params[++i]);
            const float y = std::stof(params[++i]);
            const float z = std::stof(params[++i]);
            this->UP = glm::vec3(x, y, z);
        } else if (params[i] == "yview")
            yview = std::stof(params[++i]);
        else if (params[i] == "preview-height")
            previewHeight = std::stoi(params[++i]);
        else if (params[i] == "samples")
            samples = std::stoi(params[++i]);
        else if (params[i] == "exposure")
            exposure = std::stof(params[++i]);
        else if (params[i] == "kdtree-leaf-size")
            kdtreeLeafSize = std::stoi(params[++i]);
        else
            std::cerr << "Invalid argument \"" << params[i] << "\"\n";
    }
}

// set default values and parse input from file
Scene::Scene(std::string filename)
    : renderPath("renders/output.exr"), k(3), xres(400), yres(300), VP(0, 0, 2), LA(0, 0, 0), UP(0, 1, 0), yview(1),
      usingOpenGLPreview(true), previewHeight(900), kdtreeLeafSize(8), background(0), samples(100), exposure(5) {
    std::ifstream file(filename);
    std::string input;
    while (std::getline(file, input)) {
        if (input.length() > 0)
            params.push_back(input);
    }
}

LightPoint::LightPoint(glm::vec3 _color, glm::vec3 _position, float _intensity)
    : color(_color), position(_position), intensity(_intensity) {}

LightTriangle::LightTriangle(id_t i, float s) : id(i), surface(s){};

const LightTriangle &Scene::randomLight() {
    return lightTriangles[std::uniform_int_distribution<>(0,
                                                          lightTriangles.size() - 1)(PRNG::rng[omp_get_thread_num()])];
}
