#ifndef RAY_CASTER_H
#define RAY_CASTER_H

#include "scene.hpp"
#include "kdtree.hpp"

#include <glm/glm.hpp>
#include <vector>

class Model;
class Color;

class RayTracer {
  public:
    RayTracer(Model &_model, Scene &_scene);

    /* Fill pixels with rays shot on screen centered between eye and center */
    void rayTrace(glm::vec3 eye, glm::vec3 center, glm::vec3 up, float yview);
    /* Get RGB (24 bits per pixel) image location */
    uint8_t *getData();

    /* The biggest single pixel color generated in last rayTrace()*/
    float maxVal;
    /* Divide every pixel's color by maxVal */
    void normalizeImage();
    /* Divide every pixel's color by specified max */
    void normalizeImage(float max);

    /* Export image to file using FreeImage library. 24 bits per pixel */
    void exportImage(const char *filename);

  private:
    /* Recursive procedure used by rayTrace method */
    glm::vec3 sendRay(glm::vec3 origin, glm::vec3 dir, int k);

    /* Intersect ray specified by origin and direction with kd-tree, storing the hitpoint in params: cross, normal, color */
    bool intersectRayKDTree(const glm::vec3 &origin, const glm::vec3 &direction, glm::vec3 &cross, glm::vec3 &normal, Color &color);

    Model &model;
    Scene &scene;
    std::vector<std::vector<glm::vec3>> pixels;
    std::vector<uint8_t> data;
    KDTree kdtree;
};
#endif