#include "main.h"

int main() {
	GLFWwindow* window = init_main_window();
	ImGuiIO* imgui_io = init_imGui(window);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int appstate = 1;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	// Testing zone /////////////////////////////////////////
	Editor::init();
	Editor::input_map.activate();

	Camera2d cam("main_camera");
	cam.display_height = &SCR_HEIGHT;
	cam.display_width = &SCR_WIDTH;

	Shader shade("tests/shader/sprite.vs", "tests/shader/sprite.fs");
	Shader bg("tests/shader/bg.vs", "tests/shader/sprite.fs");

	// Sprite background("tests/img/space.png", glm::vec2(0), 0, glm::vec2(5));

	Sprite test("test", "tests/img/tex.png");
	Sprite vest("vest", "tests/img/gex.png", glm::vec2(1, 1));
	vest.mesh = Primitive::rect(glm::vec2(1), glm::vec2(0), ROTATE_90 | ROTATE_180);
	vest.updateMesh();
	vest.layer = 1;

	TileGrid grid = gridTest();
	Editor::grid = &grid;
	Editor::camera = &cam;
	cam.move(std::make_unique<ChunkLoader>("chunk_loader", &grid, 1));
	
	//////////////////////////////////////////////

	Input control;
	control.activate();
	float speed = 5;
	control.addBind("speen", 
		[&cam, &deltaTime, &speed](){cam.rotate(20 * deltaTime);},
		GLFW_KEY_F
	);
	control.addBind("down", 
		[&cam, &deltaTime, speed](){cam.position -= speed * cam.up() * deltaTime;},
		GLFW_KEY_DOWN
	);
	control.addBind("up", 
		[&cam, &deltaTime, speed](){cam.position += speed * cam.up() * deltaTime;},
		GLFW_KEY_UP
	);
	control.addBind("left", 
		[&cam, &deltaTime, speed](){cam.position -= speed * cam.right() * deltaTime;},
		GLFW_KEY_LEFT
	);
	control.addBind("right", 
		[&cam, &deltaTime, speed](){cam.position += speed * cam.right() * deltaTime;},
		GLFW_KEY_RIGHT
	);

	bool imgui_window_state = true;
	// Main loop
	while(appstate) {
		if(glfwWindowShouldClose(window)) {
			appstate = 0;
			imgui_window_state = false;
		}

		// Time calculation
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Calculate input
		// See if imgui needs inputs
		if(imgui_io->WantCaptureMouse || imgui_io->WantCaptureKeyboard)
			imgui_input.activate_solo();
		else
			imgui_input.undo_solo();
		
		// Input::block_keys = imgui_io->WantCaptureKeyboard;
		// Input::block_mouse = imgui_io->WantCaptureMouse;
		// Input::block_mouse_buttons = imgui_io->WantCaptureMouse;
		// Input::block_scroll = imgui_io->WantCaptureMouse;
		
		glfwPollEvents();
		Input::processActive(window);

		ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		if (imgui_window_state)
            ImGui::ShowDemoWindow(&imgui_window_state);

		Editor::show_gui();

		shade.set("view", cam.getViewMatrix());
		shade.set("projection", cam.getProjectionMatrix());
	
		bg.set("view", cam.getViewMatrix());
		bg.set("projection", cam.getProjectionMatrix());

		cam["chunk_loader"]->as<ChunkLoader>()->loadChunksSquare();

		// background.draw(bg);

		test.draw(shade);
		vest.draw(shade);
		grid.drawChunks(shade);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	// Cleanup
	cleanup(window);
	return 0;
}

TileGrid gridTest() {
	// tiles.insert(std::pair(1, glm::vec2(-1, 0)));
	// tiles.insert(std::pair(1, glm::vec2(1,0)));
	// tiles.insert(std::pair(0, glm::vec2(-2, -2)));

	TileGrid grid(glm::uvec2(1), glm::uvec2(36), 9);
	TexMap map("tests/textures/demo.png", 24, 24);
	grid.addTexMap(map);
	grid.addChunk(glm::vec2(0), {});

	// TileGrid grid("tests/gridtest.mp", 9);

	return grid;
}