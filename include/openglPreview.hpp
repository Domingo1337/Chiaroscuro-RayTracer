#ifndef OPENGLPREVIEW_H
#define OPENGLPREVIEW_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <camera.hpp>
#include <scene.hpp>
// #include <viewer.hpp>

class OpenGLPreview {
  private:
    Scene *scene;
    Shader shader;
    Model *ourModel;

  public:
    unsigned RTx, RTy;

    OpenGLPreview() {}

    OpenGLPreview(Scene *scene) {
        viewerInit(1600, 900);
        Shader tempShader("shader/simple_vs.glsl", "shader/simple_fs.glsl");
        this->shader = tempShader;
        this->scene = scene;

        camera.Position = scene->VP;
        camera.Front = scene->LA - scene->VP;
    };

    void setModel(Model *model) { this->ourModel = model; }

    void loop() {
        while (!glfwWindowShouldClose(window)) {
            // per-frame time logic
            // --------------------
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // input
            // -----
            processInput(window);

            // render
            // ------
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 model, view, projection;
            view = camera.GetViewMatrix();
            projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                          camera.NearPlane, 100.0f);
            // model = glm::rotate(model, rotateY, glm::vec3(0.0f, 1.0f, 0.0f));
            // model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
            // model = glm::scale(model, glm::vec3(0.2f));

            shader.use();
            shader.setMat4("model", model);
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);

            ourModel->Draw(shader);

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
            // etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        // glDeleteVertexArrays(vertices.size(), VAO);
        // glDeleteBuffers(vertices.size(), VBO);
        // glDeleteBuffers(10, EBO);
        glfwTerminate();
    }
};
#endif
