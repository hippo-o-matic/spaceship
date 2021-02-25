#pragma once

#include "game/tileGrid.h"
#include "input.h"
#include "camera2d.h"
#include "window.h"

#include "imgui.h"
#include "imgui_stdlib.h"

namespace GridEditor {
	extern Input input_map;
	extern Input framebuffer_input;
	extern glm::ivec2 frame_size;
	extern glm::vec2 screen_to_frame; // Conversion factor from window size to framebuffer size, used for hidpi screens

	extern Camera2d camera;
	extern TileGrid* grid;

	extern unsigned selected; // The tile to place when placeTile is called
	extern unsigned current_attribs; // The attributes to apply to this tile

	const float place_threshold = 0.05; // When the mouse moves past this threshold, dont register a tile placement
	extern glm::vec2 drag_last; // Used to calculate where the camera should move when dragging
	extern bool dont_place;

	void init();
	void show_gui();
	void updateFramebuffer(GLFWwindow* window, int w, int h);

	glm::vec2 mouseToWorld(Camera2d& cam);

	void scrollChangeTile(GLFWwindow* window, double x, double y);
	void scrollZoom(GLFWwindow* window, double x, double y);
	void placeTile(glm::vec2 pos, unsigned tileID);
	void fillChunk();
}