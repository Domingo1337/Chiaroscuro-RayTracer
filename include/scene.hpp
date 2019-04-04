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
    glm::vec3 ambientLight;
    std::vector<Light> lights;
};

#endif
