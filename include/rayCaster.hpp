#ifndef RAY_CASTER_H
#define RAY_CASTER_H

#include "scene.hpp"

#include <glm/glm.hpp>
#include <vector>

class Model;
class Color;

class RayCaster {
  public:
    RayCaster(Model &_model, Scene &_scene);

    /* Fill pixels with rays shot on screen centered between eye and center */
    void rayTrace(glm::vec3 eye, glm::vec3 center, glm::vec3 up, float yview);
    /* Get RGB (24 bits per pixel) image location */
    uint8_t *getData();
    /* Export image to file using FreeImage library. Default format is png. 24 bits per pixel */
    void exportImage(const char *filename, const char *format);

  private:
    /* Intersect ray specified by origin and direction with every triangle in the model, storing the hitpoint in params: cross, normal, color */
    bool intersectRayModel(const glm::vec3 &origin, const glm::vec3 &direction, glm::vec3 &cross, glm::vec3 &normal,
                           Color &color, bool shadowRay);

    Model &model;
    Scene &scene;
    std::vector<std::vector<glm::vec3>> pixels;
    std::vector<uint8_t> data;
};
#endif