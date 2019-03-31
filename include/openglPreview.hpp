#ifndef OPENGLPREVIEW_H
#define OPENGLPREVIEW_H

#include "camera.hpp"
#include "shader.hpp"

class RayCaster;
class GLFWwindow;
class Scene;
class Model;

class OpenGLPreview {
  public:
    OpenGLPreview(Scene *_scene, unsigned int _previewHeight);

    void setModel(Model *model);
    void setRenderer(RayCaster *renderer);
    void loop();

  private:
    struct Screen {
        Screen();
        Screen(int texWidth, int texHeight);

        void draw();
        void requestRender(Camera *camera);
        void updateScreen();

        float vertices[32] = {
            // positions        // colors         // texture coords
            1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
            1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
            -1.0f, 1.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
        };
        unsigned int indices[6] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };
        unsigned int height;
        unsigned int width;
        unsigned int VBO, VAO, EBO;
        unsigned int texture;
        Shader shader;
        RayCaster *renderer;
    };

    void setCallbacks();
    void processInputs(float deltaTime);

    Scene *scene;
    Screen screen;
    Shader shaderTextured;
    Shader shaderMaterial;
    Model *model;
    GLFWwindow *window;
    unsigned int previewHeight;
    unsigned int previewWidth;
    Camera camera;
    bool showRender;
};
#endif
