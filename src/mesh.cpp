/* Based off LearnOpenGL's
 * https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/mesh.h
 */

#include "mesh.hpp"
#include "shader.hpp"

#include <glad/glad.h>


Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures,
           Color materialColor) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    this->materialColor = materialColor;
    setupMesh();
}

bool Mesh::hasTexture() { return textureNormal || textureHeight || textureDiffuse || textureSpecular; }

Color Mesh::getColorAt(glm::vec2 coords) {
    Color color = this->materialColor;

    // assuming the texture is reapeated
    while (coords.x > 1.f) coords.x -= 1.f;
    while (coords.x < 0.f) coords.x += 1.f;
    while (coords.y > 1.f) coords.y -= 1.f;
    while (coords.y < 0.f) coords.y += 1.f;

    if (textureDiffuse && textureDiffuse->image) {
        int x = coords.x * textureDiffuse->width;
        int y = coords.y * textureDiffuse->height;
        unsigned char *pixel = &textureDiffuse->image[(y * textureDiffuse->width + x) * textureDiffuse->nrComponents];
        color.diffuse.r = (float)(*pixel) / 255.f;
        color.diffuse.g = (float)(*(pixel + 1)) / 255.f;
        color.diffuse.b = (float)(*(pixel + 2)) / 255.f;
    }
    if (textureSpecular && textureSpecular->image) {
        int x = coords.x * textureSpecular->width;
        int y = coords.y * textureSpecular->height;
        unsigned char *pixel =
            &textureSpecular->image[(y * textureSpecular->width + x) * textureSpecular->nrComponents];
        color.specular.r = (float)(*pixel) / 255.f;
        color.specular.g = (float)(*(pixel + 1)) / 255.f;
        color.specular.b = (float)(*(pixel + 2)) / 255.f;
    }

    return color;
}

void Mesh::Draw(Shader shaderTexture, Shader shaderMaterial) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    unsigned int textSize = textures.size();
    if (textSize > 0) {
        shaderTexture.use();
        for (unsigned int i = 0; i < textSize; i++) {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            std::string number;
            std::string name = textures[i].type;
            if (name == "texture_diffuse") {
                number = std::to_string(diffuseNr++);
            } else if (name == "texture_specular") {
                number = std::to_string(specularNr++); // transfer unsigned int to stream
            } else if (name == "texture_normal") {
                number = std::to_string(normalNr++); // transfer unsigned int to stream
            } else if (name == "texture_height") {
                number = std::to_string(heightNr++); // transfer unsigned int to stream
            }
            glUniform1i(glGetUniformLocation(shaderTexture.ID, (name + number).c_str()), i);
            shaderTexture.setFloat(("material." + name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

    } else {
        shaderMaterial.use();
        shaderMaterial.setVec3("material.ambient", materialColor.ambient);
        shaderMaterial.setVec3("material.diffuse", materialColor.diffuse);
        shaderMaterial.setVec3("material.specular", materialColor.specular);
        shaderMaterial.setFloat("material.shininess", materialColor.shininess);
    }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
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

    for (unsigned int i = 0; i < textures.size(); i++) {
        std::string name = textures[i].type;
        if (name == "texture_diffuse")
            textureDiffuse = &textures[i];
        else if (name == "texture_specular")
            textureSpecular = &textures[i];
        else if (name == "texture_normal")
            textureNormal = &textures[i];
        else if (name == "texture_height")
            textureHeight = &textures[i];
    }
}

Color::Color()
    : ambient({0.0f, 0.0f, 0.0f}), diffuse({0.5f, 0.5f, 0.5f}), specular({0.0f, 0.0f, 0.0f}), shininess(1.f){};

Color::Color(glm::vec3 ambi, glm::vec3 diff, glm::vec3 spec, float shin)
    : ambient(ambi), diffuse(diff), specular(spec), shininess(shin){};
