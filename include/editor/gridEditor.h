#pragma once

#include "game/tileGrid.h"
#include "input.h"
#include "camera2d.h"

namespace Editor {
	extern Input input_map;
	extern glm::ivec2 frame_size;
	extern glm::vec2 screen_to_frame; // Conversion factor from window size to framebuffer size, used for hidpi screens

	extern Camera2d* camera;
	extern TileGrid* grid;

	extern unsigned selected; // The tile to place when placeTile is called
	extern glm::vec2 drag_last; // Used to calculate where the camera should move when dragging

	void init();
	void updateFramebuffer(GLFWwindow* window, int w, int h);

	void placeTile(glm::vec2 pos, unsigned tileID);
	glm::vec2 mouseToWorld(Camera2d* cam);

	void scrollChangeTile(GLFWwindow* window, double x, double y);
	void scrollZoom(GLFWwindow* window, double x, double y);
	void mouseButton(GLFWwindow* win, int button, int action);
}