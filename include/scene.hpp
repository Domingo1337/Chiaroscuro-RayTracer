#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>

#include <string>
#include <vector>

struct Light {
    Light(glm::vec3 _color, glm::vec3 _position, float _intensity);
    union {
        glm::vec3 color;
        struct {
            float r;
            float g;
            float b;
        };
    };
    union {
        glm::vec3 position;
        struct {
            float x;
            float y;
            float z;
        };
    };
    float intensity;
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
    glm::vec3 ambientLight;
    std::vector<Light> lights;

    // command line args and their default values
    bool usingOpenGLPreview = true;
    unsigned int previewHeight = 900;
    size_t kdtreeLeafSize = 8;
};

#endif
