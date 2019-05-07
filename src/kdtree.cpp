#include "kdtree.hpp"
#include "model.hpp"

#include <glm/gtc/random.hpp>
#include <glm/gtx/intersect.hpp>

#include <glm/gtx/io.hpp>

#include <algorithm>
#include <iostream>

float KDTree::triMax(Triangle &t, id_t axis) {
    float max = vertices[t.fst].Position[axis];
    max = vertices[t.snd].Position[axis] > max ? vertices[t.snd].Position[axis] : max;
    return vertices[t.trd].Position[axis] > max ? vertices[t.trd].Position[axis] : max;
}

float KDTree::triMin(Triangle &t, id_t axis) {
    float min = vertices[t.fst].Position[axis];
    min = vertices[t.snd].Position[axis] < min ? vertices[t.snd].Position[axis] : min;
    return vertices[t.trd].Position[axis] < min ? vertices[t.trd].Position[axis] : min;
}

bool KDTree::inLeft(Triangle &t, float max, id_t axis) {
    return vertices[t.fst].Position[axis] <= max || vertices[t.snd].Position[axis] <= max ||
           vertices[t.trd].Position[axis] <= max;
}
bool KDTree::inRight(Triangle &t, float min, id_t axis) {
    return vertices[t.fst].Position[axis] >= min || vertices[t.snd].Position[axis] >= min ||
           vertices[t.trd].Position[axis] >= min;
}
bool lessThan(glm::vec3 &compare, glm::vec3 &to) { return compare.x < to.x || compare.y < to.y || compare.z < to.z; }
bool greaterThan(glm::vec3 &compare, glm::vec3 &to) { return compare.x > to.x || compare.y > to.y || compare.z > to.z; }

KDTree::KDTree(Model &model) : KDTree(model.meshes) {}

KDTree::KDTree(std::vector<Mesh> &meshes) : minCoords(FLT_MAX, FLT_MAX, FLT_MAX), maxCoords(FLT_MIN, FLT_MIN, FLT_MIN) {
    size_t verticesCount = 0;
    size_t indicesCount = 0;

    for (auto &mesh : meshes) {
        verticesCount += mesh.vertices.size();
        indicesCount += mesh.indices.size();
    }

    vertices.resize(verticesCount);
    materials.resize(verticesCount);
    std::vector<Triangle> triangles(indicesCount / 3);

    indicesCount = 0;
    verticesCount = 0;
    for (auto &mesh : meshes) {
        auto &indices = mesh.indices;
        for (unsigned i = 0; i < indices.size(); i += 3, indicesCount++) {
            triangles[indicesCount].fst = indices[i + 0] + verticesCount;
            triangles[indicesCount].snd = indices[i + 1] + verticesCount;
            triangles[indicesCount].trd = indices[i + 2] + verticesCount;
        }
        for (unsigned i = 0; i < mesh.vertices.size(); i++) {
            materials[verticesCount] = &mesh;
            vertices[verticesCount++] = mesh.vertices[i];
            for (unsigned j = 0; j < 3; j++) {
                minCoords[j] =
                    mesh.vertices[i].Position[j] < minCoords[j] ? mesh.vertices[i].Position[j] : minCoords[j];
                maxCoords[j] =
                    mesh.vertices[i].Position[j] > maxCoords[j] ? mesh.vertices[i].Position[j] : maxCoords[j];
            }
        }
    }
    triangles.resize(indicesCount);

    nodes.push_back(KDTree::KDNode());
    nodes[0] = build(triangles, maxCoords, minCoords);
    std::cout << "Triangles in scenem: " << triangles.size() << "\n";
}

std::pair<id_t, float> KDTree::findSplit(std::vector<Triangle> &tris, glm::vec3 &max, glm::vec3 &min) {
    id_t bestAxis = 3; // if 3 is returned then there's no point in splitting
    float bestSplit = 0.f;
    float bestCost = float(tris.size());

    for (id_t axis = 0; axis < 3; axis++) {
        for (float ratio = 0.01f; ratio < 1.0f; ratio += 0.01f) {
            float split = min[axis] + ratio * (max[axis] - min[axis]);
            float cost = 0.f;
            size_t leftCount = 0;
            size_t rightCount = 0;

            for (auto &tri : tris) {
                if (inLeft(tri, split, axis)) {
                    cost += ratio;
                    leftCount++;
                }
                if (inRight(tri, split, axis)) {
                    cost += (1.f - ratio);
                    rightCount++;
                }
            }
            if (leftCount < tris.size() && rightCount < tris.size() && cost < bestCost) {
                bestAxis = axis;
                bestSplit = split;
                bestCost = cost;
            }
        }
    }

    return {bestAxis, bestSplit};
}

KDTree::KDNode KDTree::build(std::vector<Triangle> &tris, glm::vec3 &max, glm::vec3 &min) {
    KDTree::KDNode node;
    if (tris.size() <= 64) {
        node.isLeaf = true;
        node.triangles = tris;
        return node;
    } else {
        auto split = findSplit(tris, max, min);
        if (split.first == 3) {
            node.isLeaf = true;
            node.triangles = tris;
            return node;
        }

        node.axis = split.first;
        node.split = split.second;
        node.isLeaf = false;
        node.child = nodes.size();
        nodes.resize(nodes.size() + 2);
        size_t rightchildSize = 0;
        size_t leftchildSize = 0;
        {
            glm::vec3 min(FLT_MAX, FLT_MAX, FLT_MAX), max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
            std::vector<Triangle> childTriangles;
            for (auto &tri : tris) {
                if (inLeft(tri, node.split, node.axis)) {

                    childTriangles.push_back(tri);

                    // find min max for chdil build
                    min[node.axis] = std::min(min[node.axis], triMin(tri, node.axis));
                    max[node.axis] = std::max(max[node.axis], triMax(tri, node.axis));
                    id_t ax = (node.axis + 1) % 3;
                    min[ax] = std::min(min[ax], triMin(tri, ax));
                    max[ax] = std::max(max[ax], triMax(tri, ax));
                    ax = (ax + 1) % 3;
                    min[ax] = std::min(min[ax], triMin(tri, ax));
                    max[ax] = std::max(max[ax], triMax(tri, ax));
                }
            }
            leftchildSize += childTriangles.size();
            nodes[node.child] = build(childTriangles, max, min);
        }
        {
            glm::vec3 min(FLT_MAX, FLT_MAX, FLT_MAX), max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
            std::vector<Triangle> childTriangles;
            for (auto &tri : tris) {
                if (inRight(tri, node.split, node.axis)) {

                    // find min max for chdil build
                    childTriangles.push_back(tri);
                    min[node.axis] = std::min(min[node.axis], triMin(tri, node.axis));
                    max[node.axis] = std::max(max[node.axis], triMax(tri, node.axis));
                    id_t ax = (node.axis + 1) % 3;
                    min[ax] = std::min(min[ax], triMin(tri, ax));
                    max[ax] = std::max(max[ax], triMax(tri, ax));
                    ax = (ax + 1) % 3;
                    min[ax] = std::min(min[ax], triMin(tri, ax));
                    max[ax] = std::max(max[ax], triMax(tri, ax));
                }
            }
            rightchildSize += childTriangles.size();
            nodes[node.child + 1] = build(childTriangles, max, min);
        }
    }
    return node;
}
bool KDTree::intersectRay(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition, Triangle &triangle) {
    float distance = FLT_MAX;
    return intersectRayNode(origin, dir, baryPosition, triangle, distance, nodes[0], maxCoords, minCoords);
}
bool KDTree::intersectRayTriangle(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition,
                                  const Triangle &tri) {
    return glm::intersectRayTriangle(origin, dir, vertices[tri.fst].Position, vertices[tri.snd].Position,
                                     vertices[tri.trd].Position, baryPosition);
}

static float intersectRayBox(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &max, glm::vec3 &min) {
    // calculate time of intersection with min.x and max.x
    float txmin = (min.x - origin.x) / dir.x;
    float txmax = (max.x - origin.x) / dir.x;
    if (txmin > txmax)
        std::swap(txmin, txmax);

    // the same for y
    float tymin = (min.y - origin.y) / dir.y;
    float tymax = (max.y - origin.y) / dir.y;
    if (tymin > tymax)
        std::swap(tymin, tymax);

    // intersection of any min.axis must happen before any other max.axis
    if ((txmin > tymax) || (tymin > txmax))
        return FLT_MAX;

    // check the same for z
    float tzmin = (min.z - origin.z) / dir.z;
    float tzmax = (max.z - origin.z) / dir.z;
    if (tzmin > tzmax)
        std::swap(tzmin, tzmax);

    if ((std::max(txmin, tymin) > tzmax) || (tzmin > std::min(txmax, tymax)))
        return FLT_MAX;
    return std::min({txmin, tymin, tzmin});
}

bool KDTree::intersectRayNode(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &baryPosition,
                              Triangle &triangle, float &distance, KDTree::KDNode &node, glm::vec3 &max,
                              glm::vec3 &min) {

    if (intersectRayBox(origin, dir, max, min) >= distance) {
        return false;
    }

    if (node.isLeaf) {
        glm::vec3 bPos;
        bool ret = false;
        int i = 0;
        for (auto &tri : node.triangles) {
            if (intersectRayTriangle(origin, dir, bPos, tri) && bPos.z < distance) {
                baryPosition = bPos;
                distance = bPos.z;
                triangle = tri;
                ret = true;
            }
        }
        return ret;
    }

    glm::vec3 maxLeft = max;
    maxLeft[node.axis] = node.split;
    glm::vec3 minRight = min;
    minRight[node.axis] = node.split;

    float dist;
    if (dir[node.axis] == 0.f || (dist = (node.split - origin[node.axis]) / dir[node.axis]) < 0 ||
        dist >= distance) {                  // ray doesnt cross the intersection
        if (origin[node.axis] <= node.split) // origin is in left son
            return intersectRayNode(origin, dir, baryPosition, triangle, distance, nodes[node.child], maxLeft, min);

        return intersectRayNode(origin, dir, baryPosition, triangle, distance, nodes[node.child + 1], max, minRight);
    }

    if (origin[node.axis] <= node.split) // ray comes from below intersection, so first left, then right
        return intersectRayNode(origin, dir, baryPosition, triangle, distance, nodes[node.child], maxLeft, min) ||
               intersectRayNode(origin, dir, baryPosition, triangle, distance, nodes[node.child + 1], max, minRight);
    // otherwise
    return intersectRayNode(origin, dir, baryPosition, triangle, distance, nodes[node.child + 1], max, minRight) ||
           intersectRayNode(origin, dir, baryPosition, triangle, distance, nodes[node.child], maxLeft, min);
}

// dummy function
bool KDTree::intersectShadowRay(const glm::vec3 &origin, const glm::vec3 &dir) {
    glm::vec3 baryPos;
    Triangle triangle;
    return intersectRay(origin, dir, baryPos, triangle);
}
