#ifndef OPENGLPREVIEW_H
#define OPENGLPREVIEW_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <camera.hpp>
#include <rayCaster.hpp>
#include <scene.hpp>

class OpenGLPreview {
  private:
    Scene *scene;
    Shader shaderSimple;
    Shader shaderScreen;
    Model *ourModel;
    Camera camera;
    bool rayTraceee = false;
    GLFWwindow *window;
    RayCaster *rendererPtr;
    bool shouldRender = true;
    bool shouldSwitch = true;

  public:
    unsigned RTx, RTy;
    unsigned xres, yres;
    float &yfov;
    void setRenderer(RayCaster *rendererPtr) { this->rendererPtr = rendererPtr; }
    OpenGLPreview(Scene *scene) : yfov(scene->yview), camera(scene->VP, glm::normalize(scene->LA - scene->VP)) {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(((double)scene->xres / scene->yres) * previewHeight, previewHeight, "OpenGL peek",
                                  NULL, NULL);
        if (window == NULL) {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            exit(1);
        }
        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            exit(1);
        }

        glfwSetWindowUserPointer(window, &camera);
        glfwSetFramebufferSizeCallback(
            window, [](GLFWwindow *window, int width, int height) { glViewport(0, 0, width, height); });
        glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
            Camera *camera = static_cast<Camera *>(glfwGetWindowUserPointer(window));
            static bool firstMouse = true;
            static float lastX = 100.f;
            static float lastY = 100.f;
            if (firstMouse) // this bool variable is initially set to true
            {
                lastX = xpos;
                lastY = ypos;
                firstMouse = false;
            }

            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
            lastX = xpos;
            lastY = ypos;

            camera->ProcessMouseMovement(xoffset, yoffset);
        });
        glfwSetScrollCallback(window, [](GLFWwindow *window, double xoffset, double yoffset) {
            Camera *camera = static_cast<Camera *>(glfwGetWindowUserPointer(window));
            camera->ProcessMouseScroll(yoffset);
        });

        glClearColor(0.5f, 0.3f, 0.3f, 1.0f);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        this->shaderSimple = Shader("shader/simple_vs.glsl", "shader/simple_fs.glsl");
        this->shaderScreen = Shader("shader/screen.vs", "shader/screen.fs");
        this->scene = scene;
        this->xres = scene->xres;
        this->yres = scene->yres;
        this->yfov = scene->yview;
        camera.Zoom = glm::degrees(2.f * atanf(0.5f * scene->yview));
    }

    void setModel(Model *model) { this->ourModel = model; }

    void loop(uint8_t *data = NULL, int texWidth = 0, int texHeight = 0) {
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // build and compile our shader zprogram
        // ------------------------------------
        unsigned int VBO, VAO, EBO;
        unsigned int texture;
        if (data) {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            // set up vertex data (and buffer(s)) and configure vertex attributes
            // ------------------------------------------------------------------
            float vertices[] = {
                // positions          // colors           // texture coords
                1.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
                1.0f,  -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
                -1.0f, 1.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
            };
            unsigned int indices[] = {
                0, 1, 3, // first triangle
                1, 2, 3  // second triangle
            };
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);
            // color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            // texture coord attribute
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            // load and create a texture
            // -------------------------
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D,
                          texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
            // set the texture wrapping parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                            GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // set texture filtering parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            //  create texture and generate mipmaps
            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            } else {
                std::cout << "Failed to load texture" << std::endl;
            }
        }
        float deltaTime = 0.0f; // Time between left_upper frame and last frame
        float lastFrame = 0.0f; // Time of last frame
        while (!glfwWindowShouldClose(window)) {
            // per-frame time logic
            // --------------------

            if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
                rayTraceee = true;
                if (rendererPtr && shouldRender) {
                    rendererPtr->rayTrace(camera.Position, camera.Front + camera.Position, camera.Up,
                                          2 * tan(camera.Zoom * M_PI / 360.));
                    shouldRender = false;
                }
            } else {
                shouldRender = true;
            }

            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                if (shouldSwitch)
                    rayTraceee = !rayTraceee;
                shouldSwitch = false;
            } else {
                shouldSwitch = true;
            }

            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                camera.ProcessKeyboard(FORWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                camera.ProcessKeyboard(BACKWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                camera.ProcessKeyboard(LEFT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                camera.ProcessKeyboard(RIGHT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
                camera.ProcessKeyboard(UPWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
                camera.ProcessKeyboard(DOWNWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                camera.MovementSpeed = 12.5f;
            else
                camera.MovementSpeed = 2.5f;
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (rayTraceee) {
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
                glClear(GL_COLOR_BUFFER_BIT);
                glBindTexture(GL_TEXTURE_2D, texture);
                shaderScreen.use();
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            } else {
                glEnable(GL_DEPTH_TEST);

                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glm::mat4 model, view, projection;
                view = camera.GetViewMatrix();
                projection = glm::perspective(glm::radians(camera.Zoom), (float)xres / (float)yres, 0.1f, 100.0f);
                // model = glm::rotate(model, rotateY, glm::vec3(0.0f, 1.0f, 0.0f));
                // model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
                // model = glm::scale(model, glm::vec3(0.2f));

                shaderSimple.use();
                shaderSimple.setMat4("model", model);
                shaderSimple.setMat4("view", view);
                shaderSimple.setMat4("projection", projection);

                ourModel->Draw(shaderSimple);
            }
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
