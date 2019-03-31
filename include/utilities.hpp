#ifndef UTILITIES_H
#define UTILITIES_H

#include <glm/glm.hpp>
#include <iostream>

void print_vec(glm::vec3 V) { std::cerr << "[" << V.x << ", " << V.y << ", " << V.z << "]"; }
void print_vec(glm::vec4 V) { std::cerr << "[" << V.x << ", " << V.y << ", " << V.z << ", " << V.w << "]"; }

#endif