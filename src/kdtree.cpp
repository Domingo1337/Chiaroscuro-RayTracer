#include "kdtree.hpp"
#include "model.hpp"
#include "scene.hpp"

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

KDTree::KDTree(Model &model, Scene &scene)
    : leafSize(scene.kdtreeLeafSize), minCoords(FLT_MAX, FLT_MAX, FLT_MAX), maxCoords(FLT_MIN, FLT_MIN, FLT_MIN) {
    size_t verticesCount = 0;
    size_t indicesCount = 0;

    for (auto &mesh : model.meshes) {
        verticesCount += mesh.vertices.size();
        indicesCount += mesh.indices.size();
    }

    vertices.resize(verticesCount);
    materials.resize(verticesCount);
    std::vector<Triangle> triangles(indicesCount / 3);

    indicesCount = 0;
    verticesCount = 0;
    for (auto &mesh : model.meshes) {
        const bool isLight = mesh.materialColor.emissive.r > 0.f || mesh.materialColor.emissive.g > 0.f ||
                             mesh.materialColor.emissive.b > 0.f;
        auto &indices = mesh.indices;
        for (unsigned i = 0; i < indices.size(); i += 3, indicesCount++) {
            triangles[indicesCount].fst = indices[i + 0] + verticesCount;
            triangles[indicesCount].snd = indices[i + 1] + verticesCount;
            triangles[indicesCount].trd = indices[i + 2] + verticesCount;
            if (isLight)
                scene.triangleLights.push_back(triangles[indicesCount]);
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
    std::cout << "Triangles in scene: " << triangles.size() << "\n";
    std::cerr << "Surface Lights in scene:";
    for (auto &tri : scene.triangleLights) {
        std::cerr << "\nTriangle {" << vertices[tri.fst].Position << vertices[tri.snd].Position
                  << vertices[tri.trd].Position << "} of color emissive " << materials[tri.fst]->materialColor.emissive;
    }
    std::cerr << (scene.triangleLights.size() == 0 ? " None.\n" : "\n");
    std::cerr << "Point Lights in scene:";
    for (auto &light : scene.lights) {
        std::cerr << "\nPosition " << light.position << " of color " << light.color << " and intesity "
                  << light.intensity;
    }
    std::cerr << (scene.lights.size() == 0 ? " None.\n" : "\n");

    minCoords -= 0.0001f;
    maxCoords += 0.0001f;
}

KDTree::KDNode::Split KDTree::findSplit(std::vector<Triangle> &tris, glm::vec3 &max, glm::vec3 &min) {
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
    if (tris.size() <= leafSize || (node.split = findSplit(tris, max, min)).axis == 3) {
        node.isLeaf = true;
        node.triangles = tris;
        return node;
    }
    node.isLeaf = false;
    node.child = nodes.size();
    nodes.resize(nodes.size() + 2);
    {
        glm::vec3 min(FLT_MAX, FLT_MAX, FLT_MAX), max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        std::vector<Triangle> childTriangles;
        for (auto &tri : tris) {
            if (inLeft(tri, node.split.position, node.split.axis)) {
                childTriangles.push_back(tri);

                min[node.split.axis] = std::min(min[node.split.axis], triMin(tri, node.split.axis));
                max[node.split.axis] = std::max(max[node.split.axis], triMax(tri, node.split.axis));
                id_t ax = (node.split.axis + 1) % 3;
                min[ax] = std::min(min[ax], triMin(tri, ax));
                max[ax] = std::max(max[ax], triMax(tri, ax));
                ax = (ax + 1) % 3;
                min[ax] = std::min(min[ax], triMin(tri, ax));
                max[ax] = std::max(max[ax], triMax(tri, ax));
            }
        }

        nodes[node.child] = build(childTriangles, max, min);
    }
    {
        glm::vec3 min(FLT_MAX, FLT_MAX, FLT_MAX), max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        std::vector<Triangle> childTriangles;
        for (auto &tri : tris) {
            if (inRight(tri, node.split.position, node.split.axis)) {
                childTriangles.push_back(tri);

                min[node.split.axis] = std::min(min[node.split.axis], triMin(tri, node.split.axis));
                max[node.split.axis] = std::max(max[node.split.axis], triMax(tri, node.split.axis));
                id_t ax = (node.split.axis + 1) % 3;
                min[ax] = std::min(min[ax], triMin(tri, ax));
                max[ax] = std::max(max[ax], triMax(tri, ax));
                ax = (ax + 1) % 3;
                min[ax] = std::min(min[ax], triMin(tri, ax));
                max[ax] = std::max(max[ax], triMax(tri, ax));
            }
        }
        nodes[node.child + 1] = build(childTriangles, max, min);
    }

    return node;
}

std::pair<float, float> intersectRayBox(const glm::vec3 &origin, const glm::vec3 &dir, glm::vec3 &max, glm::vec3 &min) {
    const float dirinvy = 1.f / dir.y;
    const float dirinvx = 1.f / dir.x;
    const float dirinvz = 1.f / dir.z;
    const float txmin = (min.x - origin.x) * dirinvx;
    const float txmax = (max.x - origin.x) * dirinvx;
    const float tymin = (min.y - origin.y) * dirinvy;
    const float tymax = (max.y - origin.y) * dirinvy;
    const float tzmin = (min.z - origin.z) * dirinvz;
    const float tzmax = (max.z - origin.z) * dirinvz;
    return {std::max(std::max(std::min(txmin, txmax), std::min(tymin, tymax)), std::min(tzmin, tzmax)),
            std::min(std::min(std::max(txmin, txmax), std::max(tymin, tymax)), std::max(tzmin, tzmax))};
}

bool KDTree::intersectRay(const glm::vec3 &origin, const glm::vec3 &dir, Triangle &triangle, glm::vec2 &baryPosition,
                          float &distance) {
    auto intersect = intersectRayBox(origin, dir, maxCoords, minCoords);
    if (intersect.second < 0 || intersect.second < intersect.first)
        return false;
    return intersectRayNode(origin, dir, triangle, baryPosition, distance, nodes[0], intersect.first, intersect.second);
}

/* based off original Möller–Trumbore algorithm */
bool KDTree::intersectRayTriangle(const glm::vec3 &origin, const glm::vec3 &dir, const Triangle &tri,
                                  glm::vec2 &baryPosition, float &distance) {
    const glm::vec3 v0 = vertices[tri.fst].Position;
    const glm::vec3 e1 = vertices[tri.snd].Position - v0;
    const glm::vec3 e2 = vertices[tri.trd].Position - v0;

    const glm::vec3 p = glm::cross(dir, e2);

    const float a = glm::dot(e1, p);

    const float Epsilon = std::numeric_limits<float>::epsilon();
    if (a < Epsilon && a > -Epsilon)
        return false;

    const float f = 1.f / a;

    const glm::vec3 s = origin - v0;
    baryPosition.x = f * glm::dot(s, p);
    if (baryPosition.x < 0.f || baryPosition.x > 1.f)
        return false;

    const glm::vec3 q = glm::cross(s, e1);
    baryPosition.y = f * glm::dot(dir, q);
    if (baryPosition.y < 0.f || baryPosition.y + baryPosition.x > 1.f)
        return false;

    return (distance = f * glm::dot(e2, q)) >= 0.f;
}

bool KDTree::intersectRayNode(const glm::vec3 &origin, const glm::vec3 &dir, Triangle &triangle,
                              glm::vec2 &baryPosition, float &distance, KDTree::KDNode &node, float tmin, float tmax) {
    if (node.isLeaf) {
        glm::vec2 bPos;
        bool ret = false;
        float dist;
        for (auto &tri : node.triangles) {
            if (intersectRayTriangle(origin, dir, tri, bPos, dist) && dist < tmax) {
                baryPosition = bPos;
                tmax = dist;
                triangle = tri;
                ret = true;
            }
        }
        distance = tmax;
        return ret;
    }

    const float tsplit = (node.split.position - origin[node.split.axis]) / dir[node.split.axis];
    const id_t belowFirst = (origin[node.split.axis] < node.split.position) ||
                            (origin[node.split.axis] == node.split.position && dir[node.split.axis] <= 0);

    if (tsplit >= tmax || tsplit < 0)
        return intersectRayNode(origin, dir, triangle, baryPosition, distance, nodes[node.child + (1 - belowFirst)],
                                tmin, tmax);
    else if (tsplit <= tmin)
        return intersectRayNode(origin, dir, triangle, baryPosition, distance, nodes[node.child + belowFirst], tmin,
                                tmax);
    else
        return intersectRayNode(origin, dir, triangle, baryPosition, distance, nodes[node.child + (1 - belowFirst)],
                                tmin, tsplit) ||
               intersectRayNode(origin, dir, triangle, baryPosition, distance, nodes[node.child + belowFirst], tsplit,
                                tmax);
}

bool KDTree::intersectShadowRay(const glm::vec3 &origin, const glm::vec3 &dir, const float distance,
                                const Triangle &lightTriangle) {
    auto intersect = intersectRayBox(origin, dir, maxCoords, minCoords);
    if (intersect.second < 0 || intersect.second < intersect.first || intersect.first > distance)
        return false;
    return intersectShadowRayNode(origin, dir, lightTriangle, nodes[0], intersect.first,
                                  std::min(intersect.second, distance));
}

// pretty much the same as intersectRayTriangle
bool KDTree::intersectShadowRayTriangle(const glm::vec3 &origin, const glm::vec3 &dir, const Triangle &tri,
                                        const float tmax) {
    const glm::vec3 v0 = vertices[tri.fst].Position;
    const glm::vec3 e1 = vertices[tri.snd].Position - v0;
    const glm::vec3 e2 = vertices[tri.trd].Position - v0;

    const glm::vec3 p = glm::cross(dir, e2);

    const float a = glm::dot(e1, p);

    const float Epsilon = std::numeric_limits<float>::epsilon();
    if (a < Epsilon && a > -Epsilon)
        return false;

    const float f = 1.f / a;

    const glm::vec3 s = origin - v0;
    const float baryPositionx = f * glm::dot(s, p);
    if (baryPositionx < 0.f || baryPositionx > 1.f)
        return false;

    const glm::vec3 q = glm::cross(s, e1);
    const float baryPositiony = f * glm::dot(dir, q);
    if (baryPositiony < 0.f || baryPositiony + baryPositionx > 1.f)
        return false;
    const float t = (f * glm::dot(e2, q));
    return t >= 0.f && t < tmax;
}

bool KDTree::intersectShadowRayNode(const glm::vec3 &origin, const glm::vec3 &dir, const Triangle &lightTriangle,
                                    KDTree::KDNode &node, float tmin, float tmax) {
    if (node.isLeaf) {
        for (auto &tri : node.triangles) {
            // todo: unique triangle id or something
            if (tri.fst != lightTriangle.fst && tri.snd != lightTriangle.snd && tri.trd != lightTriangle.trd &&
                intersectShadowRayTriangle(origin, dir, tri, tmax)) {
                return true;
            }
        }
        return false;
    }

    const float tsplit = (node.split.position - origin[node.split.axis]) / dir[node.split.axis];
    const id_t belowFirst = (origin[node.split.axis] < node.split.position) ||
                            (origin[node.split.axis] == node.split.position && dir[node.split.axis] <= 0);

    if (tsplit >= tmax || tsplit < 0)
        return intersectShadowRayNode(origin, dir, lightTriangle, nodes[node.child + (1 - belowFirst)], tmin, tmax);
    else if (tsplit <= tmin)
        return intersectShadowRayNode(origin, dir, lightTriangle, nodes[node.child + belowFirst], tmin, tmax);
    else
        return intersectShadowRayNode(origin, dir, lightTriangle, nodes[node.child + (1 - belowFirst)], tmin, tsplit) ||
               intersectShadowRayNode(origin, dir, lightTriangle, nodes[node.child + belowFirst], tsplit, tmax);
}
