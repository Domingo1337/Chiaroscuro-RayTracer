#include "rayCaster.hpp"
#include "model.hpp"

#include <FreeImage.h>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

RayCaster::RayCaster(Model &_model, Scene &_scene)
    : model(_model), scene(_scene), pixels(scene.yres, std::vector<glm::vec3>(scene.xres)),
      data(scene.yres * scene.xres * 3) {}

uint8_t *RayCaster::getData() { return data.data(); }

bool RayCaster::intersectRayModel(const glm::vec3 &origin, const glm::vec3 &direction, glm::vec3 &cross,
                                  glm::vec3 &normal, Color &color, bool shadowRay = false) {
    float minDist = FLT_MAX;
    Mesh *closestMesh = NULL;
    glm::vec2 closestPos;

    for (auto &mesh : model.meshes) {
        auto &indices = mesh.indices;
        auto &vertices = mesh.vertices;
        /* triangles loop */
        for (unsigned i = 0; i < indices.size(); i += 3) {

            auto &A = vertices[indices[i]];
            auto &B = vertices[indices[i + 1]];
            auto &C = vertices[indices[i + 2]];

            glm::vec3 baryPosition;
            if (glm::intersectRayTriangle(origin, direction, A.Position, B.Position, C.Position, baryPosition) &&
                baryPosition.z < minDist) {
                if (shadowRay) {
                    return true;
                }
                minDist = baryPosition.z;
                baryPosition.z = 1.f - baryPosition.x - baryPosition.y;
                closestMesh = &mesh;
                closestPos = A.TexCoords * baryPosition.z + B.TexCoords * baryPosition.x + C.TexCoords * baryPosition.y;
                normal = A.Normal * baryPosition.z + B.Normal * baryPosition.x + C.Normal * baryPosition.y;
            }
        }
    }
    if (closestMesh) {
        cross = origin + minDist * direction;
        color = closestMesh->getColorAt(closestPos);
        return true;
    }
    return false;
}

void RayCaster::rayTrace(glm::vec3 eye, glm::vec3 center, glm::vec3 up = {0.f, 1.f, 0.f}, float yview = 1.f) {
    float z = 1.f;
    float y = z * 0.5f * yview;
    float x = y * ((float)scene.xres / (float)scene.yres);

    glm::vec3 leftUpper = {-x, y, -z};
    glm::vec3 dy = {0.f, -2.f * y, 0.f};
    glm::vec3 dx = {2.f * x, 0.f, 0.f};

    /* rotate the corners of screen to look from VP to LA */
    auto rotate = glm::inverse(glm::mat3(glm::lookAt(eye, center, up)));
    leftUpper = rotate * leftUpper;
    dy = (1.f / scene.yres) * rotate * dy;
    dx = (1.f / scene.xres) * rotate * dx;

    glm::vec3 current = leftUpper + 0.5f * (dy + dx);
    for (unsigned y = 0; y < scene.yres; y++, current += dy) {
        glm::vec3 currentRay = current;
        for (unsigned x = 0; x < scene.xres; x++, currentRay += dx) {
            pixels[y][x] = {0.f, 0.f, 0.f};
            glm::vec3 cross;
            glm::vec3 normal;
            Color color;
            if (intersectRayModel(eye, currentRay, cross, normal, color)) {
                if (scene.k == 0) {
                    pixels[y][x] = color.diffuse;
                } else {
                    pixels[y][x] = scene.ambientLight * color.ambient;
                    for (auto &light : scene.lights) {
                        /* render lights */
                        if (glm::areCollinear(currentRay, light.position - eye, 0.005f)) {
                            pixels[y][x] = light.color;
                            break;
                        }
                        glm::vec3 tempCross, tempNormal;
                        Color tempColor;
                        if (!intersectRayModel(cross + (0.0001f * (light.position - cross)), (light.position - cross),
                                               tempCross, tempNormal, tempColor)) {
                            /* Phong's model */
                            glm::vec3 V = glm::normalize(eye - cross);
                            glm::vec3 N = glm::normalize(normal);
                            glm::vec3 L = glm::normalize(light.position - cross);
                            glm::vec3 R = glm::normalize(2.f * (glm::dot(L, N)) * N - L);
                            float distance = glm::distance(cross, light.position);
                            float attentuation = (1.f / (1.f + distance * distance));

                            glm::vec3 phong = color.diffuse * glm::max(glm::dot(L, N), 0.f) +
                                              color.specular * glm::pow(glm::max(glm::dot(R, V), 0.f), color.shininess);
                            phong.r = std::max(phong.r * light.color.r, 0.f);
                            phong.g = std::max(phong.g * light.color.g, 0.f);
                            phong.b = std::max(phong.b * light.color.b, 0.f);

                            // some meaningless number here, just so the render looks somewhat
                            // decent
                            pixels[y][x] += phong * attentuation * light.intensity * 0.05f;
                        }
                    }
                }
            }
            glm::clamp(pixels[y][x], 0.f, 1.f);
            int i = 3 * ((scene.yres - y - 1) * scene.xres + x);
            data[i] = (uint8_t)(255.f * pixels[y][x].r);
            data[i + 1] = (uint8_t)(255.f * pixels[y][x].g);
            data[i + 2] = (uint8_t)(255.f * pixels[y][x].b);
        }
    }
}

void RayCaster::exportImage(const char *filename, const char *format) {
    FreeImage_Initialise();

    FREE_IMAGE_FORMAT fileformat = FIF_PNG;
    if (!strcasecmp(format, "jpg") || !strcasecmp(format, "jpeg"))
        fileformat = FIF_JPEG;

    FIBITMAP *bitmap = FreeImage_Allocate(scene.xres, scene.yres, 24);
    if (!bitmap) {
        std::cerr << "FreeImage export failed.\n";
        return FreeImage_DeInitialise();
    }

    RGBQUAD color;
    for (unsigned y = 0; y < scene.yres; y++) {
        for (unsigned x = 0; x < scene.xres; x++) {
            color.rgbRed = data[3 * (scene.xres * y + x)];
            color.rgbGreen = data[3 * (scene.xres * y + x) + 1];
            color.rgbBlue = data[3 * (scene.xres * y + x) + 2];
            FreeImage_SetPixelColor(bitmap, x, y, &color);
        }
    }
    if (FreeImage_Save(fileformat, bitmap, filename, 0))
        std::cerr << "Render succesfully saved to file " << filename << "\n";
    else
        std::cerr << "FreeImage export failed.\n";

    FreeImage_DeInitialise();
}
