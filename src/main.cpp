#include "main.h"

bool show_debug_menu = true;

int main() {
	GLFWwindow* window = init_main_window();
	ImGuiIO* imgui_io = init_imGui(window);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int appstate = 1;
	bool show_debug_menu = true;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	// Testing zone /////////////////////////////////////////

	Editor::init();

	Object::ptr cam_temp = newObj<Camera2d>("player_camera", &SCR_WIDTH, &SCR_HEIGHT);
	Camera2d* cam = cam_temp->as<Camera2d>();
	Camera2d::main_camera = cam;

	cam->prevent_inherit_rot = true;
	cam->fov = 5;

	Shader bg("tests/shader/bg.vs", "tests/shader/sprite.fs");
	Renderable::register_shader(&bg);

	TiledSprite background("bg", "tests/img/space.png", glm::vec2(1, 1), glm::ivec2(3), -1);
	background.scl(glm::vec2(10));
	background.render_shader = &bg;

	Sprite test("test", "tests/img/gex.png");
	
	AnimSprite anim_test("aieee", "tests/textures/anim_test.png", 32, 10, 2);
	anim_test.pos(glm::vec2(3, 3));

	TileGrid grid = gridTest();
	Editor::grid = &grid;

	Ship::ship_classes.push_back({
		"testclass",
		"tests/textures/ship.png",
		5,
		3,
		100,
		0
	});

	Ship player("player", "testclass");
	player.take(newObj<ChunkLoader>("chunk_loader", &grid, 1));
	player.takeFromRef(cam_temp);
	
	//////////////////////////////////////////////

	Input control;
	control.activate();
	control.addBind("debug", 
		[&show_debug_menu](){show_debug_menu = !show_debug_menu; },
		GLFW_KEY_GRAVE_ACCENT, INPUT_ONCE
	);
	control.addBind("down", 
		[&player](){player.thrust += -player.up();},
		GLFW_KEY_S
	);
	control.addBind("up", 
		[&player](){player.thrust += player.up();},
		GLFW_KEY_W
	);
	control.addBind("left", 
		[&player](){player.angular_thrust = 1;},
		GLFW_KEY_A
	);
	control.addBind("right", 
		[&player](){player.angular_thrust = -1;},
		GLFW_KEY_D
	);
	control.addBind("boost", 
		[&player](){player.ship_class.thrust_power += 3;},
		GLFW_KEY_LEFT_SHIFT, INPUT_ONCE
	);
	control.addBind("unboost", 
		[&player](){player.ship_class.thrust_power -= 3;},
		GLFW_KEY_LEFT_SHIFT, INPUT_ONCE_RELEASE
	);
	control.addBind("drift", 
		[&player](){player.drift = player.right() * glm::dot(player.velocity, player.right());},
		GLFW_KEY_SPACE
	);
	control.addBind("undrift", 
		[&player](){player.drift = glm::vec2(0);},
		GLFW_KEY_SPACE, INPUT_RELEASE
	);

	control.addBind("test", [&anim_test](){
		anim_test.nextFrame();
	}, GLFW_KEY_P, INPUT_ONCE);

	////////////////////////////////////////////////

	bool imgui_window_state = false;
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
		
		glfwPollEvents();
		Input::processActive(window);

		ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		if (imgui_window_state)
            ImGui::ShowDemoWindow(&imgui_window_state);

		if(show_debug_menu) {
			Editor::show_gui();

			ImGui::Begin("Player");
			ImGui::Text(("X: " + std::to_string(player.position.x)).c_str());
			ImGui::Text(("Y: " + std::to_string(player.position.y)).c_str());
			ImGui::Text(("VX: " + std::to_string(player.velocity.x)).c_str());
			ImGui::Text(("VY: " + std::to_string(player.velocity.y)).c_str());
			ImGui::Text(("AV: " + std::to_string(player.angular_velocity)).c_str());
			ImGui::SliderFloat("Mass", &player.ship_class.mass, 0.1, 50);
			ImGui::SliderFloat("Power", &player.ship_class.thrust_power, 0, 10);
			ImGui::End();
		}

		player["chunk_loader"]->as<ChunkLoader>()->loadChunksSquare();
		anim_test.animate(deltaTime, 0.5);

		// background.draw(bg);
		Renderable::draw_all();

		player.update(deltaTime);

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