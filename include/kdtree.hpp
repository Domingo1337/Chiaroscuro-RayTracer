#ifndef KDTREE_H
#define KDTREE_H
#include "mesh.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Model;

struct Triangle {
    Triangle(const Vertex &v1, const Vertex &v2, const Vertex &v3, Mesh *mesh);
    bool intersectRay(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition);
    Vertex v1;
    Vertex v2;
    Vertex v3;
    glm::vec3 midpoint;
    Mesh *mesh;
};

class KDTree {
  public:
    KDTree(Model &model);
    Triangle *intersectRay(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition);
    bool intersectShadowRay(const glm::vec3 &origin, const glm::vec3 &dir);

  private:
    struct KDNode {
        KDNode();
        KDNode(Triangle *triangles, size_t size, int axis);
        // leaf
        Triangle *triangle;

        // node
        std::unique_ptr<KDNode> left;
        std::unique_ptr<KDNode> right;

        // shared
        struct BoundingBox {
            BoundingBox();
            BoundingBox(const BoundingBox &b1, const BoundingBox &b2);
            BoundingBox(const Triangle *triangle);
            bool intersectRay(const glm::vec3 &origin, const glm::vec3 &dir);
            glm::vec3 minCorner;
            glm::vec3 maxCorner;
        } bbox;
        bool isLeaf;

        Triangle *intersectRay(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition);
        bool intersectShadowRay(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition);
    } root;
    std::vector<Triangle> triangles;
};

#endif // KDTREE_H