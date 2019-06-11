/* Based off http://www.cse.chalmers.se/edu/year/2018/course/TDA362/tutorials/pathtracer.html */

#include "brdf.hpp"
#include "prng.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

glm::vec3 perpendicular(const glm::vec3 &v) {
    if (fabsf(v.x) < fabsf(v.y)) {
        return glm::vec3(0.0f, -v.z, v.y);
    }
    return glm::vec3(-v.z, 0.0f, v.x);
}

// generate random point on hemisphere defined by normal with Phong exponent's shininess
// based off https://blog.thomaspoulet.fr/uniform-sampling-on-unit-hemisphere/
glm::vec3 hemisphereRandom(glm::vec3 normal, float shininess = 1.f) {

    // calculate reflected ray:
    // rotation to normal matrix
    glm::mat3 rotate;
    // if normal is the same as 'up' vector we cant use glm::lookAt
    if (std::abs(normal.y) >= 0.99f) {
        float sgn = normal.y > 0.f ? 1.f : -1.f;
        rotate[0] = {1.f, 0.f, 0.f};
        rotate[1] = {0.f, 0.f, sgn};
        rotate[1] = {0.f, sgn, 0.f};
    } else
        rotate = glm::inverse(glm::mat3(glm::lookAt({0.f, 0.f, 0.f}, normal, {0.f, 1.f, 0.f})));

    const float theta = acosf(powf(PRNG::uniformFloat(0.f, 1.f), 1.f / (1.f + shininess)));
    const float phi = M_PI * PRNG::uniformFloat(0.f, 2.f);

    const float x = sin(theta) * cos(phi);
    const float y = sin(theta) * sin(phi);
    return rotate * glm::vec3(x, y, -glm::sqrt(1.f - x * x - y * y));
}

// Generic BRDF
glm::vec3 BRDF::radiance() { return glm::vec3(0.f); };

BRDF::~BRDF(){};

// A Lambertian (diffuse) material
glm::vec3 Diffuse::f(const glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &n) { return float(M_1_PI) * color; }

glm::vec3 Diffuse::sample_wi(glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &n, float &pdf) {
    glm::vec3 sample = hemisphereRandom(n);
    wi = sample;
    pdf = glm::max(0.0f, dot(n, wi)) * M_1_PI;
    return f(wi, wo, n);
}

Diffuse::~Diffuse(){};

// Emissive material
glm::vec3 Emissive::radiance() { return radianceColor; }

Emissive::~Emissive(){};