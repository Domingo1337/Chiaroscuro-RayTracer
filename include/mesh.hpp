#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader.hpp>
#include <utilities.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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
};

struct Color {
    Color() : ambient({0.2f, 0.2f, 0.2f}), diffuse({0.5f, 0.5f, 0.5f}), specular({0.7f, 0.7f, 0.7f}){};
    Color(glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec) : ambient(ambi), diffuse(diff), specular(spec){};
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

class Mesh {
  public:
    // Mesh Data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    Color color;
    Texture *textureNormal = NULL;
    Texture *textureHeight = NULL;
    Texture *textureDiffuse = NULL;
    Texture *textureSpecular = NULL;
    bool hasTexture() { return textureNormal || textureHeight || textureDiffuse || textureSpecular; }
    Color getColorAt(glm::vec2 coords) {
        Color _color = this->color;
        int x, y;
        while (coords.x > 1.f) coords.x -= 1.f;
        while (coords.x < 0.f) coords.x += 1.f;
        while (coords.y > 1.f) coords.y -= 1.f;
        while (coords.y < 0.f) coords.y += 1.f;

        if (textureDiffuse) {
            x = coords.x * textureDiffuse->width;
            y = coords.y * textureDiffuse->height;
            unsigned char *pixel =
                &textureDiffuse->image[(y * textureDiffuse->width + x) * textureDiffuse->nrComponents];
            _color.diffuse.r = (float)(*pixel) / 255.f;
            _color.diffuse.g = (float)(*(pixel + 1)) / 255.f;
            _color.diffuse.b = (float)(*(pixel + 2)) / 255.f;
        }
        if (textureSpecular) {
            x = coords.x * textureSpecular->width;
            y = coords.y * textureSpecular->height;
            unsigned char *pixel =
                &textureSpecular->image[(y * textureSpecular->width + x) * textureSpecular->nrComponents];
            _color.diffuse.r = (float)(*pixel) / 255.f;
            _color.diffuse.g = (float)(*(pixel + 1)) / 255.f;
            _color.diffuse.b = (float)(*(pixel + 2)) / 255.f;
        }

        return _color;
    }
    // Functions
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Color color);
    void Draw(Shader shader);

  private:
    // Render data
    unsigned int VAO, VBO, EBO;
    // Functions
    void setupMesh();
};

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures,
           Color color) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    this->color = color;
    if (usingOpenGLPreview) {
        setupMesh();
    }
}

void Mesh::setupMesh() {

    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Mesh::Draw(Shader shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse") {
            number = std::to_string(diffuseNr++);
            textureDiffuse = &textures[i];
        } else if (name == "texture_specular") {
            number = std::to_string(specularNr++); // transfer unsigned int to stream
            textureSpecular = &textures[i];

        } else if (name == "texture_normal") {
            number = std::to_string(normalNr++); // transfer unsigned int to stream

            textureNormal = &textures[i];
        } else if (name == "texture_height") {
            number = std::to_string(heightNr++); // transfer unsigned int to stream
            textureHeight = &textures[i];
        }
        glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
        shader.setFloat(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}

#endif