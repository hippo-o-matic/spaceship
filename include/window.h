#pragma once

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "input.h"
#include "logs.h"

extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;

GLFWwindow* init_main_window();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
extern Input window_input;