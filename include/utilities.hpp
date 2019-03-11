#pragma once
#include <glm/glm.hpp>
#include <iostream>

bool usingOpenGLPreview = true;

static void print_vec(glm::vec3 V) { std::cerr << "[" << V.x << ", " << V.y << ", " << V.z << "]"; }
static void print_vec(glm::vec4 V) { std::cerr << "[" << V.x << ", " << V.y << ", " << V.z << ", " << V.w << "]"; }
