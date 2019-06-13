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

// Generate uniform points on a disc
void concentricSampleDisk(float *dx, float *dy) {
    // Uniform random point on square [-1,1]x[-1,1]
    const float sx = PRNG::uniformFloat(-1.f, 1.f);
    const float sy = PRNG::uniformFloat(-1.f, 1.f);

    // Map square to (r, theta)
    // Handle degeneracy at the origin
    if (sx == 0.0 && sy == 0.0) {
        *dx = 0.0;
        *dy = 0.0;
        return;
    }
    float r, theta;
    if (sx >= -sy) {
        if (sx > sy) { // Handle first region of disk
            r = sx;
            if (sy > 0.0)
                theta = sy / r;
            else
                theta = 8.0f + sy / r;
        } else { // Handle second region of disk
            r = sy;
            theta = 2.0f - sx / r;
        }
    } else {
        if (sx <= sy) { // Handle third region of disk
            r = -sx;
            theta = 4.0f - sy / r;
        } else { // Handle fourth region of disk
            r = -sy;
            theta = 6.0f + sx / r;
        }
    }
    theta *= M_PI / 4.f;
    *dx = r * cosf(theta);
    *dy = r * sinf(theta);
}

// Generate points with a cosine distribution on the hemisphere
glm::vec3 cosineSampleHemisphere() {
    glm::vec3 ret;
    concentricSampleDisk(&ret.x, &ret.y);
    ret.z = sqrt(std::max(0.f, 1.f - ret.x * ret.x - ret.y * ret.y));
    return ret;
}

// Generic BRDF
glm::vec3 BRDF::radiance() { return glm::vec3(0.f); };

BRDF::~BRDF(){};

// A Lambertian (diffuse) material
glm::vec3 Diffuse::f(const glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &n) { return float(M_1_PI) * color; }

glm::vec3 Diffuse::sample_wi(glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &n, float &pdf) {
    glm::vec3 tangent = normalize(perpendicular(n));
    glm::vec3 bitangent = normalize(cross(tangent, n));
    glm::vec3 sample = cosineSampleHemisphere();
    wi = glm::normalize(sample.x * tangent + sample.y * bitangent + sample.z * n);
    pdf = glm::max(0.0f, dot(n, wi)) * M_1_PI;
    return f(wi, wo, n);
}

Diffuse::~Diffuse(){};

// Emissive material
glm::vec3 Emissive::radiance() { return radianceColor; }

Emissive::~Emissive(){};