#ifndef SCENE_H
#define SCENE_H

#include "kdtree.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>

struct LightPoint {
    LightPoint(glm::vec3 color, glm::vec3 position, float intensity);
    glm::vec3 color;
    glm::vec3 position;
    float intensity;
};

struct LightTriangle {
    LightTriangle(id_t id, float invSurface);
    id_t id;
    float invSurface;
};

class Scene {
  public:
    Scene();
    Scene(std::string filename);
    Scene(int argc, char **argv);

    // given in .rtc file
    std::string objPath;
    std::string renderPath;
    int k;
    unsigned xres;
    unsigned yres;
    glm::vec3 VP;
    glm::vec3 LA;
    glm::vec3 UP;
    float yview;
    std::vector<LightPoint> lightPoints;

    // command line args
    bool usingOpenGLPreview;
    unsigned int previewHeight;
    size_t kdtreeLeafSize;
    glm::vec3 background;
    unsigned int samples;

    std::vector<LightTriangle> lightTriangles;
};

#endif
