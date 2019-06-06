#ifndef KDTREE_H
#define KDTREE_H
#include "mesh.hpp"
#include "material.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Model;
class Mesh;
class Scene;
class Vertex;

struct Triangle {
    Vertex fst;
    Vertex snd;
    Vertex trd;
    Mesh * mat;
    std::unique_ptr<BRDF> brdf;
};

class KDTree {
  public:
    const size_t leafSize;
    KDTree(Model &model, Scene &scene);
    bool intersectRay(const glm::vec3 &origin, const glm::vec3 &dir, id_t &triangle, glm::vec2 &baryPosition,
                      float &distance);
    bool intersectShadowRay(const glm::vec3 &origin, const glm::vec3 &dir, const float distance,
                            const id_t lightTriangle);
    std::vector<Triangle> triangles;
    glm::vec3 minCoords;
    glm::vec3 maxCoords;
    struct KDNode {
        // leaf
        std::vector<id_t> trianglesids;

        // node
        id_t child;

        struct Split {
            id_t axis;
            float position;
        } split;

        bool isLeaf;
    };

  private:
    bool intersectRayTriangle(const glm::vec3 &origin, const glm::vec3 &dir, const id_t tri, glm::vec2 &baryPosition,
                              float &distance);
    bool intersectRayNode(const glm::vec3 &origin, const glm::vec3 &dir, id_t &triangle, glm::vec2 &baryPosition,
                          float &distance, KDTree::KDNode &node, float tmin, float tmax);
    bool intersectShadowRayTriangle(const glm::vec3 &origin, const glm::vec3 &dir, const id_t tri, float dist);
    bool intersectShadowRayNode(const glm::vec3 &origin, const glm::vec3 &dir, const id_t lightTriangle,
                                KDTree::KDNode &node, float tmin, float tmax);
    KDNode build(std::vector<id_t> &tris, glm::vec3 &max, glm::vec3 &min);
    KDNode::Split findSplit(std::vector<id_t> &tris, glm::vec3 &max, glm::vec3 &min);
    float triMin(id_t t, id_t axis);
    float triMax(id_t t, id_t axis);
    bool inRight(id_t t, float min, id_t axis);
    bool inLeft(id_t t, float max, id_t axis);
    std::vector<KDNode> nodes;
};

#endif // KDTREE_H