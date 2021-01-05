#include "window.h"

unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;
Input window_input;

GLFWwindow* init_main_window() {
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	 // glfw window creation
 	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "hippotest", NULL, NULL);
	if (window == NULL) {
		log("Failed to create GLFW window", CRIT);
		std::cin.get();
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(window);

	// Set glfw callbacks
	window_input.setFramebufferCallback(framebuffer_size_callback);
	window_input.activate();
	Input::assignCallbacks(window);

	// glad: load all OpenGL function pointers

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		log("Failed to initialize GLAD", CRIT);
		return nullptr;
	}
	
	return window;
}

// executed when the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	SCR_HEIGHT = height;
	SCR_WIDTH = width;
}