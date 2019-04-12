#include "kdtree.hpp"
#include "model.hpp"

#include <glm/gtx/intersect.hpp>

#include <algorithm>
#include <iostream>

Triangle::Triangle(const Vertex &_v1, const Vertex &_v2, const Vertex &_v3, Mesh *_mesh)
    : v1(_v1), v2(_v2), v3(_v3), midpoint((_v1.Position + _v2.Position + _v3.Position) / 3.f), mesh(_mesh){};

KDTree::KDTree(Model &model) {
    for (auto &mesh : model.meshes) {
        auto &indices = mesh.indices;
        auto &vertices = mesh.vertices;
        for (unsigned i = 0; i < indices.size(); i += 3) {
            Vertex &v1 = vertices[indices[i]];
            Vertex &v2 = vertices[indices[i + 1]];
            Vertex &v3 = vertices[indices[i + 2]];
            triangles.push_back(Triangle(v1, v2, v3, &mesh));
        }
    }
    root = KDNode(triangles.data(), triangles.size(), 0);
}

KDTree::KDNode::KDNode() {}

KDTree::KDNode::KDNode(Triangle *triangles, size_t size, int axis) {
    if (size == 1) {
        left = nullptr;
        right = nullptr;
        triangle = triangles;
        bbox = BoundingBox(triangle);
        isLeaf = true;
    } else {
        std::sort(triangles, triangles + size,
                  [axis](Triangle &t1, Triangle &t2) { return t1.midpoint[axis] < t2.midpoint[axis]; });
        left = std::make_unique<KDNode>(triangles, size / 2, (axis + 1) % 3);
        right = std::make_unique<KDNode>(triangles + size / 2, size - size / 2, (axis + 1) % 3);

        bbox = BoundingBox(left->bbox, right->bbox);
        triangle = nullptr;
        isLeaf = false;
    }
}

KDTree::KDNode::BoundingBox::BoundingBox(){};

KDTree::KDNode::BoundingBox::BoundingBox(const BoundingBox &b1, const BoundingBox &b2) {
    for (unsigned i = 0; i < 3; i++) {
        minCorner[i] = b1.minCorner[i] < b2.minCorner[i] ? b1.minCorner[i] : b2.minCorner[i];
        maxCorner[i] = b1.maxCorner[i] > b2.maxCorner[i] ? b1.maxCorner[i] : b2.maxCorner[i];
    };
}

KDTree::KDNode::BoundingBox::BoundingBox(const Triangle *triangle) {
    for (unsigned i = 0; i < 3; i++) {
        auto corners = std::minmax_element(&triangle->v1, &triangle->v1 + 3, [i](const Vertex &v1, const Vertex &v2) {
            return v1.Position[i] < v2.Position[i];
        });
        minCorner[i] = corners.first->Position[i];
        maxCorner[i] = corners.second->Position[i];
    }
};

Triangle *KDTree::intersectRay(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition) {
    return root.intersectRay(origin, dir, baryPosition);
}

Triangle *KDTree::KDNode::intersectRay(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition) {
    if (isLeaf)
        return triangle->intersectRay(origin, dir, baryPosition) ? triangle : nullptr;
    if (!bbox.intersectRay(origin, dir) || (!left.get() && !right.get())) {
        return nullptr;
    }

    if (!left.get())
        return right->intersectRay(origin, dir, baryPosition);
    if (!right.get())
        return left->intersectRay(origin, dir, baryPosition);

    glm::vec3 lBaryPos, rBaryPos;
    Triangle *lTriangle = left->intersectRay(origin, dir, lBaryPos);
    Triangle *rTriangle = right->intersectRay(origin, dir, rBaryPos);
    if (!lTriangle) {
        baryPosition = rBaryPos;
        return rTriangle;
    }
    if (!rTriangle) {
        baryPosition = lBaryPos;
        return lTriangle;
    }
    if (lBaryPos.z < rBaryPos.z) {
        baryPosition = lBaryPos;
        return lTriangle;
    } else {
        baryPosition = rBaryPos;
        return rTriangle;
    }
}

bool Triangle::intersectRay(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition) {
    return glm::intersectRayTriangle(origin, dir, v1.Position, v2.Position, v3.Position, baryPosition);
}

bool KDTree::KDNode::BoundingBox::intersectRay(const glm::vec3 &origin, const glm::vec3 &dir) {
    // calculate time of intersection with min.x and max.x
    float txmin = (minCorner.x - origin.x) / dir.x;
    float txmax = (maxCorner.x - origin.x) / dir.x;
    if (txmin > txmax)
        std::swap(txmin, txmax);

    // the same for y
    float tymin = (minCorner.y - origin.y) / dir.y;
    float tymax = (maxCorner.y - origin.y) / dir.y;
    if (tymin > tymax)
        std::swap(tymin, tymax);

    // intersection of any min.axis must happen before any other max.axis
    if ((txmin > tymax) || (tymin > txmax))
        return false;

    // check the same for z
    float tzmin = (minCorner.z - origin.z) / dir.z;
    float tzmax = (maxCorner.z - origin.z) / dir.z;
    if (tzmin > tzmax)
        std::swap(tzmin, tzmax);

    if ((std::max(txmin, tymin) > tzmax) || (tzmin > std::min(txmax, tymax)))
        return false;
    return true;
}

bool KDTree::intersectShadowRay(const glm::vec3 &origin, const glm::vec3 &dir) {
    glm::vec3 temp;
    return root.intersectRay(origin, dir, temp);
}

bool KDTree::KDNode::intersectShadowRay(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPos) {
    if (isLeaf)
        return triangle->intersectRay(origin, dir, baryPos);
    if (!bbox.intersectRay(origin, dir) || (!left.get() && !right.get()))
        return false;
    if (!left.get())
        return right->intersectShadowRay(origin, dir, baryPos);
    if (!right.get())
        return left->intersectShadowRay(origin, dir, baryPos);
    return left->intersectShadowRay(origin, dir, baryPos) || right->intersectShadowRay(origin, dir, baryPos);
}
