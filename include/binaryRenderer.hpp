#include <camera.hpp>
#include <fstream>
#include <glm/gtx/intersect.hpp>
#include <iostream>
#include <model.hpp>
#include <vector>

class BinaryRenderer {
public:
  Camera camera;
  Model model;
  unsigned screenWidth;
  unsigned screenHeight;
  std::vector<std::vector<glm::vec3>> rays;

  BinaryRenderer(Model model, Camera camera, unsigned screenWidth,
                 unsigned screenHeight) {
    this->model = model;
    this->camera = camera;
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
  }

  void rayTrace() {
    rays.resize(screenHeight);
    glm::vec3 origin = camera.Position;
    glm::vec3 leftUpper;

    float screenSize = 10000.;
    float dx = 2. * screenSize / (float)screenWidth;
    float dy = dx * (float)screenHeight / (float)(screenWidth);

    glm::vec3 left_upper;
    left_upper.x = origin.x - screenSize + 0.5 * dx;
    left_upper.y = origin.y + screenSize - 0.5 * dy;
    left_upper.z = origin.z - screenSize;

    glm::vec3 current = left_upper;

    for (int y = 0; y < screenHeight; y++) {
      rays[y].resize(screenWidth);
      current.x = left_upper.x;
      for (int x = 0; x < screenWidth; x++) {
        bool traced = true;
        for (auto &mesh : model.meshes) {
          auto &indices = mesh.indices;
          auto &vertices = mesh.vertices;

          /* triangles loop */
          for (int i = 0; i < indices.size() && traced; i += 3) {
            auto &A = vertices[indices[i]].Position;
            auto &B = vertices[indices[i + 1]].Position;
            auto &C = vertices[indices[i + 2]].Position;

            glm::vec3 cross;

            if (glm::intersectRayTriangle(origin, current, A, B, C, cross)) {
              rays[y][x] = {1., 1., 1.};
              traced = false;
            }
          }
        }
        if (traced) {
          rays[y][x] = {0., 0., 0.};
        }

        current.x += dx;
      }
      current.y -= dy;
    }
  }

  void ppm_print(std::ostream &ostream = std::cout) {
    if (rays.size() < screenHeight)
      return;
    ostream << "P1\n";
    ostream << screenWidth << " " << screenHeight << "\n";
    for (int y = 0; y < screenHeight; y++) {
      for (int x = 0; x < screenWidth; x++) {
        ostream << rays[y][x].x << " ";
      }
      ostream << "\n";
    }
  }
};
