#include "scene.hpp"

#include <fstream>
#include <iostream>

Scene::Scene(std::string filename) {
  std::ifstream input(filename);
  getline(input, comment);
  input >> objFile >> pngFile >> k >> xres >> yres;
  input >> VP.x >> VP.y >> VP.z;
  input >> LA.x >> LA.y >> LA.z;
  input >> UP.x >> UP.y >> UP.z;
  input >> yview;

  char c;
  while (input >> c) {
    if (c != 'L') {
      input.unget();
      break;
    }
    glm::vec3 color, position;
    input >> position.x >> position.y >> position.z;
    input >> color.r >> color.g >> color.b;

    float intensity;
    input >> intensity;

    color /= 255.f;
    ambientLight += color;

    lights.push_back(Light(color, position));
  }
  input.close();
  ambientLight /= 10 * lights.size();
}

Light::Light(glm::vec3 _color, glm::vec3 _position)
    : color(_color), r(color.r), g(color.g), b(color.b), position(_position),
      x(position.x), y(position.y), z(position.z) {}
