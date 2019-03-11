#ifndef VIEWER_H
#define VIEWER_H

//#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <camera.hpp>
#include <string>
#include <iostream>

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
bool createWindow(unsigned, unsigned);
int intify(std::string s);

// settings
const unsigned int SCR_WIDTH = 1300;
const unsigned int SCR_HEIGHT = 700;

// mouse
bool firstMouse = true;
float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;

// timing
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// window
GLFWwindow *window;

#endif