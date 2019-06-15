#ifndef RAY_CASTER_H
#define RAY_CASTER_H

#include "kdtree.hpp"
#include "scene.hpp"

#include <glm/glm.hpp>
#include <vector>

class RayTracer {
  public:
    RayTracer(Model &_model, Scene &_scene);

    /* Fill pixels with rays shot on screen centered between eye and center. */
    void rayTrace(glm::vec3 eye, glm::vec3 center, glm::vec3 up, float yview);
    /* Get RGB (24 bits per pixel) image location. */
    uint8_t *getData();

    /* The biggest single pixel color generated in last rayTrace() call. */
    float maxVal;

    /* Normalize image so png and preview look somehow alike to exr output. */
    void normalizeImage(float exposure = 7.f, float defog = 0.f, float kneeLow = 0.f, float kneeHigh = 5.f,
                        float gamma = 2.2f);

    /* Export image to file using FreeImage library. */
    void exportImage(const char *filename);

  private:
    /* Recursive procedure used by rayTrace method */
    glm::vec3 sendRay(const glm::vec3 &origin, const glm::vec3 dir, const int k);

    /* Ray-model intersection accelerated by kd-tree. Stores result in params: intersection, normal, color, brdf. */
    bool intersectRayKDTree(const glm::vec3 &origin, const glm::vec3 &direction, glm::vec3 &intersection,
                            glm::vec3 &normal, BRDF *&brdf);

    Scene &scene;
    std::vector<std::vector<glm::vec3>> pixels;
    std::vector<uint8_t> data;
    KDTree kdtree;
};
#endif