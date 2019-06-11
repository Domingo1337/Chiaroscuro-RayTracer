/* Based off LearnOpenGL's
 * https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/mesh.h
 */

#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

class Shader;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
    unsigned char *image;
    int width;
    int height;
    int nrComponents;
    glm::vec3 getColorAt(glm::vec2 coords);
};

struct Color {
    Color();
    Color(glm::vec3 ambient, glm::vec3 diffse, glm::vec3 specular, glm::vec3 emissive, float shininess);
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 emissive;
    float shininess;
};

class Mesh {
  public:
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Color color);

    bool hasTexture();
    Color getColorAt(glm::vec2 coords);
    void Draw(Shader shaderTexture, Shader shaderMaterial);

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    Color materialColor;
    Texture *textureNormal = NULL;
    Texture *textureHeight = NULL;
    Texture *textureDiffuse = NULL;
    Texture *textureSpecular = NULL;

  private:
    void setupMesh();

    unsigned int VAO, VBO, EBO;
};

#endif