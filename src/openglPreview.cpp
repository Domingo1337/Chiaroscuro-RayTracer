#include "openglPreview.hpp"
#include "model.hpp"
#include "rayTracer.hpp"
#include "scene.hpp"

#include <glad/glad.h>

#include "utilities.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

OpenGLPreview::OpenGLPreview(Scene *_scene)
    : scene(_scene), previewHeight(_scene->previewHeight),
      previewWidth(((double)_scene->xres / _scene->yres) * _scene->previewHeight),
      camera(_scene->VP, _scene->LA, _scene->UP), showRender(false) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (!scene->usingOpenGLPreview)
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    window = glfwCreateWindow(previewWidth, previewHeight, "Chiaroscuro", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(1);
    }

    if (scene->usingOpenGLPreview) {
        setCallbacks();
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        shaderTextured = Shader("shader/simple_vs.glsl", "shader/simple_fs.glsl");
        shaderMaterial = Shader("shader/material.vs", "shader/material.fs");
        camera.Zoom = glm::degrees(2.f * atanf(0.5f * scene->yview));
        screen = Screen(scene->xres, scene->yres);
    }
}

void OpenGLPreview::setModel(Model *_model) { model = _model; }

void OpenGLPreview::setRenderer(RayTracer *renderer) { screen.renderer = renderer; }

void OpenGLPreview::loop() {
    float deltaTime = 0.0f; // Time between left_upper frame and last frame
    float lastFrame = 0.0f; // Time of last frame
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInputs(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (showRender) {
            screen.draw();
        } else {
            glEnable(GL_DEPTH_TEST);

            glm::mat4 _model, view, projection;
            view = camera.GetViewMatrix();
            projection = glm::perspective(glm::radians(camera.Zoom), (float)scene->xres / scene->yres, 0.1f, 2000.f);

            shaderTextured.setMat4("model", _model);
            shaderTextured.setMat4("view", view);
            shaderTextured.setMat4("projection", projection);

            shaderMaterial.setMat4("model", _model);
            shaderMaterial.setMat4("view", view);
            shaderMaterial.setMat4("projection", projection);

            // care only about first light, it's just a preview
            shaderMaterial.setVec3("light.position", scene->lightPoints[0].position);
            shaderMaterial.setVec3("viewPos", camera.Position);
            shaderMaterial.setVec3("light.ambient", scene->lightPoints[0].color);
            shaderMaterial.setVec3("light.diffuse", scene->lightPoints[0].color);
            shaderMaterial.setVec3("light.specular", scene->lightPoints[0].color);

            model->Draw(shaderTextured, shaderMaterial);
        }
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse
        // moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // glDeleteVertexArrays(vertices.size(), VAO);
    // glDeleteBuffers(vertices.size(), VBO);
    // glDeleteBuffers(10, EBO);
    glfwTerminate();
}

void OpenGLPreview::setCallbacks() {
    glfwSetWindowUserPointer(window, &camera);
    glfwSetFramebufferSizeCallback(window,
                                   [](GLFWwindow *window, int width, int height) { glViewport(0, 0, width, height); });
    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
        static bool firstMouse = true;
        static float lastX = 0.f;
        static float lastY = 0.f;
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

        static_cast<Camera *>(glfwGetWindowUserPointer(window))->ProcessMouseMovement(xoffset, yoffset);
    });
    glfwSetScrollCallback(window, [](GLFWwindow *window, double xoffset, double yoffset) {
        static_cast<Camera *>(glfwGetWindowUserPointer(window))->ProcessMouseScroll(yoffset);
    });
}

void OpenGLPreview::processInputs(float deltaTime) {
    static bool shouldRequestRender = true;
    static bool shouldSwitch = true;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        showRender = true;
        if (shouldRequestRender) {
            screen.requestRender(&camera);
            shouldRequestRender = false;
        }
    } else {
        shouldRequestRender = true;
    }

    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (shouldSwitch)
            showRender = !showRender;
        shouldSwitch = false;
    } else {
        shouldSwitch = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        print_vec(camera.Position);
        std::cerr << "\n";
    }

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
        camera.MovementSpeed = 30.f;
    else
        camera.MovementSpeed = 2.5f;
}

OpenGLPreview::Screen::Screen() {}

OpenGLPreview::Screen::Screen(int texWidth, int texHeight)
    : height(texHeight), width(texWidth), shader("shader/screen.vs", "shader/screen.fs") {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
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
    // all upcoming GL_TEXTURE_2D operations now have effect on this texture
    // object set the texture wrapping parameters
    glBindTexture(GL_TEXTURE_2D, texture);
    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void OpenGLPreview::Screen::draw() {
    glBindTexture(GL_TEXTURE_2D, texture);
    shader.use();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void OpenGLPreview::Screen::requestRender(Camera *camera) {
    renderer->rayTrace(camera->Position, camera->Front + camera->Position, camera->Up,
                       2 * tan(camera->Zoom * M_PI / 360.));
    renderer->normalizeImage();
    updateScreen();
}

void OpenGLPreview::Screen::updateScreen() {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, renderer->getData());
    glGenerateMipmap(GL_TEXTURE_2D);
}
