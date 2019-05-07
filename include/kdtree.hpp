#ifndef KDTREE_H
#define KDTREE_H
#include "mesh.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Model;
class Mesh;
class Vertex;

struct Triangle {
    id_t fst;
    id_t snd;
    id_t trd;
};

class KDTree {
  public:
    KDTree(Model &model);
    KDTree(std::vector<Mesh> &meshes);
    bool intersectRay(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition, Triangle &triangle);
    bool intersectShadowRay(const glm::vec3 &origin, const glm::vec3 &dir);
    std::vector<Vertex> vertices;
    std::vector<Mesh *> materials;
    glm::vec3 minCoords;
    glm::vec3 maxCoords;
    struct KDNode {
        // leaf
        std::vector<Triangle> triangles;

        // node
        id_t child;

        struct Split {
            id_t axis;
            float position;
        } split;

        bool isLeaf;
    };

  private:
    bool intersectRayTriangle(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition,
                              const Triangle &tri);

    bool intersectRayNode(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition, Triangle &triangle,
                          KDTree::KDNode &node, float tmin, float tmax);
    KDNode build(std::vector<Triangle> &tris, glm::vec3 &max, glm::vec3 &min);
    KDNode::Split findSplit(std::vector<Triangle> &tris, glm::vec3 &max, glm::vec3 &min);
    float triMin(Triangle &t, id_t axis);
    float triMax(Triangle &t, id_t axis);
    bool inRight(Triangle &t, float min, id_t axis);
    bool inLeft(Triangle &t, float max, id_t axis);
    std::vector<KDNode> nodes;
};

#endif // KDTREE_H