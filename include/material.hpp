/* Based off http://www.cse.chalmers.se/edu/year/2018/course/TDA362/tutorials/pathtracer.html */

#pragma once

#include <glm/glm.hpp>

// The interface for any BRDF.
class BRDF {
  public:
    // Return the value of the brdf for specific directions
    virtual glm::vec3 f(const glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &n) = 0;

    // Sample a suitable direction and return the brdf in that direction as
    // well as the pdf (~probability) that the direction was chosen.
    virtual glm::vec3 sample_wi(glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &n, float &pdf) = 0;

    virtual glm::vec3 radiance();
};

// A Lambertian (diffuse) material
class Diffuse : public BRDF {
  public:
    glm::vec3 color;
    Diffuse(glm::vec3 c) : color(c) {}
    virtual glm::vec3 f(const glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &n) override;
    virtual glm::vec3 sample_wi(glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &n, float &pdf) override;
};

// Emissive material
class Emissive : public Diffuse {
  public:
    glm::vec3 radianceColor;
    Emissive(glm::vec3 c, glm::vec3 r) : Diffuse(c), radianceColor(r) {}
    virtual glm::vec3 radiance() override;
};