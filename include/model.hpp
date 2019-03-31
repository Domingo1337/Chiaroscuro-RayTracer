/* Based off LearnOpenGL's
 * https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/model.h
 */

#ifndef MODEL_H
#define MODEL_H

#define STB_IMAGE_IMPLEMENTATION

#include "mesh.hpp"

#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Shader;

class Model {
  public:
    Model(std::string path);
    void Draw(Shader shaderTexture, Shader shaderMaterial);
    std::vector<Mesh> meshes;

  private:
    std::string directory;
    std::vector<Texture> textures_loaded;
    void loadModel(std::string path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};

#endif