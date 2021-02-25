#pragma once

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "input.h"
#include "logs.h"
#include "shader.h"

extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;

GLFWwindow* init_main_window();
ImGuiIO* init_imGui(GLFWwindow* window);
void cleanup(GLFWwindow* window);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
extern Input window_input;
extern Input imgui_input;

