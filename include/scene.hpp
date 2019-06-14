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
    LightTriangle(id_t id, float surface);
    id_t id;
    float surface;
};

class Scene {
  public:
    Scene(int argc, char **argv);

    // params in rtc file or command line args

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
    bool usingOpenGLPreview;
    unsigned int previewHeight;
    size_t kdtreeLeafSize;
    glm::vec3 background;
    unsigned int samples;

    // computed by reading model
    std::vector<LightTriangle> lightTriangles;

    std::vector<std::string> params;

    // chosen by uniform distribution, pdf is 1 / lightTriangles.size()
    const LightTriangle &randomLight();

  private:
    Scene(std::string filename);
};

#endif
