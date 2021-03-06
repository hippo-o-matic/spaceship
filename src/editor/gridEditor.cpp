#include "editor/gridEditor.h"

// TODO: jank
Input GridEditor::input_map;
Input GridEditor::framebuffer_input;
glm::ivec2 GridEditor::frame_size;
glm::vec2 GridEditor::screen_to_frame;
Camera2d GridEditor::camera("editor_camera", &SCR_WIDTH ,&SCR_HEIGHT);
TileGrid* GridEditor::grid;
unsigned GridEditor::selected;
unsigned GridEditor::current_attribs;
glm::vec2 GridEditor::drag_last = glm::vec2(0);
bool GridEditor::dont_place = true;

void GridEditor::init() {
	framebuffer_input.setFramebufferCallback(updateFramebuffer);
	framebuffer_input.activate();

	input_map.setScrollCallback(scrollZoom);

	input_map.addBind("drag", [](){
		glm::vec2 pos = mouseToWorld(GridEditor::camera) - GridEditor::camera.getPos(); // Find the position of the cursor independent of the camera position
		glm::vec2 offset = pos - drag_last; // The amount to move the camera
		if(offset.x > place_threshold || offset.x < -place_threshold || offset.y > place_threshold || offset.y < -place_threshold) {
			dont_place = true;
		}
		
		GridEditor::camera.setPos(camera.getPos() - offset);
		drag_last = pos;
	}, GLFW_MOUSE_BUTTON_1, GLFW_PRESS, MOUSE_B);

	input_map.addBind("placeTile", [](){
		if(!dont_place)
			grid->updateChunk(grid->addTileToGrid(mouseToWorld(GridEditor::camera), selected, current_attribs));
		dont_place = false;
	}, GLFW_MOUSE_BUTTON_1, INPUT_ONCE_RELEASE, MOUSE_B);

	// input_map.addBind("fillChunk", [](){
	// 	fillChunk();
	// }, GLFW_MOUSE_BUTTON_2, INPUT_ONCE_RELEASE, MOUSE_B);

	input_map.addBind("eraseTile", [](){
		grid->updateChunk(grid->removeTileFromGrid(mouseToWorld(GridEditor::camera)));
	}, GLFW_MOUSE_BUTTON_2, INPUT_ONCE_RELEASE, MOUSE_B);

	input_map.addBind("drag_release", [](){
		drag_last = mouseToWorld(GridEditor::camera) - GridEditor::camera.getPos();
	}, GLFW_MOUSE_BUTTON_3, GLFW_RELEASE, MOUSE_B);

	input_map.addBind("nextTile", [](){
		selected++;
	}, GLFW_KEY_1, INPUT_ONCE);

	input_map.addBind("prevTile", [](){
		if(selected != 0)
			selected--;
	}, GLFW_KEY_2, INPUT_ONCE);
}

void GridEditor::show_gui() {
	ImGui::Begin("Grid Editor");

	static int buffer;
	ImGui::InputInt("Tile id", &buffer, 1);
	if(buffer < 0)
		buffer = 0;
	selected = buffer;

	static int sel_rotation = 0;
	ImGui::Combo("Tile Rotation", &sel_rotation, " 0\0 90\0 180\0 270\0\0");

	static bool toggle_mirror_h = false;
	ImGui::Checkbox("Mirror Horizontally", &toggle_mirror_h);
	static bool toggle_mirror_v = false;
	ImGui::Checkbox("Mirror Vertically", &toggle_mirror_v);

	static std::string input_path;
	ImGui::InputText("Map file path", &input_path);
	if(ImGui::Button("Save"))
		grid->saveFile(input_path);
	ImGui::SameLine();
	if(ImGui::Button("Load"))
		grid->loadFile(input_path);

	static bool edit_mode = false;
	ImGui::Checkbox("Edit mode", &edit_mode);
	
	// Note: this is janky and could result in a segfault, but because this is supposed to be a dev tool it likely doesn't matter too much
	// TODO[4]: hold on to this camera and make sure it isn't destroyed, or just redo the camera system
	static Camera2d* last_cam = Camera2d::main_camera;
	static bool change_cam = true;
	if(edit_mode && change_cam) {
		input_map.activate();
		last_cam = Camera2d::main_camera;
		Camera2d::main_camera = &GridEditor::camera;
		change_cam = false;
		drag_last = GridEditor::camera.getPos();
	} else if(!edit_mode && !change_cam) {
		input_map.deactivate();
		Camera2d::main_camera = last_cam;
		change_cam = true;
	}

	switch (sel_rotation) {
		case 0:
			current_attribs = 0;
			break;
		case 1:
			current_attribs = ROTATE_90;
			break;
		case 2:
			current_attribs = ROTATE_180;
			break;
		case 3:
			current_attribs = ROTATE_180 | ROTATE_90;
			break;
	}

	if(toggle_mirror_h)
		current_attribs |= REFLECT_H;
	if(toggle_mirror_v)
		current_attribs |= REFLECT_V;

	ImGui::End();
}

void GridEditor::fillChunk() {
	TileGrid::TileSet tiles;
	for(unsigned i = 0; i < grid->getChunkSize().x; i++) {
		for(unsigned j = 0; j < grid->getChunkSize().y; j++) {
			tiles.insert({ selected, glm::ivec2(i, j), NONE });
		}
	}
	TileGrid::Chunk* c = grid->getChunk(mouseToWorld(camera));
	c->tiles = tiles;
	grid->updateChunk(c);
}

void GridEditor::scrollChangeTile(GLFWwindow* window, double x, double y) {
	if((selected + y) >= 0) // If the scroll doesnt put us below 0
		selected += y;
}

void GridEditor::scrollZoom(GLFWwindow* window, double x, double y) {
	camera.fov += y * -0.5;
	if(camera.fov > 30)
		camera.fov = 30;
	if(camera.fov < .5)
		camera.fov = .5;
}

glm::vec2 GridEditor::mouseToWorld(Camera2d& cam) {
	glm::vec2 screen = glm::vec2(Input::mouse_x, Input::mouse_y);
	
	// Screen to framebuffer
	glm::vec2 cursor = screen * screen_to_frame;
	
	// Framebuffer to clip space
	cursor.x = ((cursor.x + .5f) / frame_size.x) * 2 - 1;
	cursor.y = 1 - ((cursor.y + .5f) / frame_size.y) * 2;
	
	// Clip space to camera space
	glm::mat4 to_world = glm::inverse(cam.getProjectionMatrix() * cam.getViewMatrix());
	cursor = to_world * glm::vec4(cursor, 0, 1);
	return cursor;
}

void GridEditor::updateFramebuffer(GLFWwindow* window, int w, int h) {
	glm::ivec2 screen;
	glfwGetWindowSize(window, &screen.x, &screen.y); 
	glfwGetFramebufferSize(window, &frame_size.x, &frame_size.y);
	screen_to_frame = glm::vec2(frame_size) / glm::vec2(screen);
}

// void GridEditor::placeTile(glm::vec2 pos, unsigned tileID) {
// 	glm::ivec2 tile_pos = glm::ivec2(std::round(pos.x), std::round(pos.y)); // Absolute position in the grid

// 	// Calculate the chunk position from the absolute position
// 	glm::ivec2 chunk_pos = glm::ivec2(
// 		(float)tile_pos.x / grid->chunk_size.x + ((tile_pos.x < 0) ? -1 : 0),
// 		(float)tile_pos.y / grid->chunk_size.y + ((tile_pos.y < 0) ? -1 : 0)
// 	);

// 	// Make the tile coordinates relative to the chunk coordinates
// 	tile_pos.x %= grid->chunk_size.x * ((tile_pos.x < 0) ? -1 : 1);
// 	tile_pos.y %= grid->chunk_size.y * ((tile_pos.y < 0) ? -1 : 1);
	
// 	auto chunk_it = std::find_if(grid->chunks.begin(), grid->chunks.end(), [chunk_pos](auto c){
// 		return c.pos == chunk_pos;
// 	});

// 	if(chunk_it == grid->chunks.end()) {
// 		std::multimap<unsigned, glm::ivec2> tiles = {std::pair(tileID, tile_pos)};
// 		grid->addChunk(chunk_pos, tiles);
// 	} else {
// 		grid->addTile(tile_pos, tileID, &*chunk_it);
// 	}
// }