#include "main.h"
#include "rigidbody2d.h"
#include "collider.h"

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

	GridEditor::init();

	Object::ptr cam_temp = newObj<Camera2d>("player_camera", &SCR_WIDTH, &SCR_HEIGHT);
	Camera2d* cam = cam_temp->as<Camera2d>();
	Camera2d::main_camera = cam;

	cam->prevent_inherit_rot = true;
	cam->fov = 5;

	Shader bg("tests/shader/bg.vs", "tests/shader/sprite.fs");
	Renderable::register_shader(&bg);

	TiledSprite background("bg", "tests/img/space.png", glm::vec2(1, 1), glm::ivec2(3), -1);
	background.setScl(glm::vec2(10));
	background.render_shader = &bg;

	Sprite test("test", "tests/img/gex.png");
	test.setPos(glm::vec2(1, 1));
	test.take(newObj<Rigidbody2d>("AAA", extractStructField(test.mesh.vertices, &Vertex2d::pos), 1));
	Rigidbody2d& a = *test["AAA"]->as<Rigidbody2d>();

	Sprite test2("test2", "tests/img/gex.png");
	test2.setPos(glm::vec2(0, 2));
	test2.take(newObj<Rigidbody2d>("AAA2", extractStructField(test2.mesh.vertices, &Vertex2d::pos), 1));
	Rigidbody2d& a2 = *test2["AAA2"]->as<Rigidbody2d>();

	AnimSprite anim_test("aieee", "tests/textures/anim_test.png", 32, 10, 2);
	anim_test.setPos(glm::vec2(3, 3));


	Line x_axis(glm::vec2(0, 0), glm::vec2(1, 0), glm::vec3(1, 0, 0));
	Line y_axis(glm::vec2(0), glm::vec2(0,1), glm::vec3(0,1,0));
	Line ship_v(glm::vec2(0), glm::vec2(0), glm::vec3(0, 0, 1));

	TileGrid grid = gridTest();
	GridEditor::grid = &grid;

	Ship::ship_classes.push_back({
		"testclass",
		"tests/textures/ship.png",
		5,
		150,
		500,
		0
	});

	PlayerShip player;
	player.take(newObj<ChunkLoader>("chunk_loader", &grid, 1));
	player.takeFromRef(cam_temp);
	
	//////////////////////////////////////////////
	Input control;
	control.activate();
	control.addBind("debug", 
		[&show_debug_menu](){show_debug_menu = !show_debug_menu; },
		GLFW_KEY_GRAVE_ACCENT, INPUT_ONCE
	);
	// control.addBind("drift", 
	// 	[&player](){player.drift = player.right() * glm::dot(player.velocity, player.right());},
	// 	GLFW_KEY_SPACE
	// );
	// control.addBind("undrift", 
	// 	[&player](){player.drift = glm::vec2(0);},
	// 	GLFW_KEY_SPACE, INPUT_RELEASE
	// );

	control.addBind("test", [&anim_test](){
		anim_test.nextFrame();
	}, GLFW_KEY_P, INPUT_ONCE);

	control.addBind("push", [&test](){
		test["AAA"]->as<Rigidbody2d>()->applyForce(glm::vec2(0,1));
	}, GLFW_KEY_UP);
	control.addBind("push2", [&test](){
		test["AAA"]->as<Rigidbody2d>()->applyForce(glm::vec2(0,-1));
	}, GLFW_KEY_DOWN);
	control.addBind("push3", [&test](){
		test["AAA"]->as<Rigidbody2d>()->applyForce(glm::vec2(1,0));
	}, GLFW_KEY_RIGHT);
	control.addBind("push4", [&test](){
		test["AAA"]->as<Rigidbody2d>()->applyForce(glm::vec2(-1,0));
	}, GLFW_KEY_LEFT);

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
			GridEditor::show_gui();

			ImGui::Begin("Player");
			ImGui::Text(("X: " + std::to_string(player.getPos().x)).c_str());
			ImGui::Text(("Y: " + std::to_string(player.getPos().y)).c_str());
			ImGui::Text(("VX: " + std::to_string(player.get<Rigidbody2d>("rigidbody").velocity.x)).c_str());
			ImGui::Text(("VY: " + std::to_string(player.get<Rigidbody2d>("rigidbody").velocity.y)).c_str());
			ImGui::Text(("AV: " + std::to_string(player.get<Rigidbody2d>("rigidbody").angular_velocity)).c_str());
			ImGui::SliderFloat("Mass", &player.ship_class.mass, 0.1, 50);
			ImGui::SliderFloat("Power", &player.ship_class.thrust_power, 0, 10);
			ImGui::End();
			
			ImGui::Begin("Rigidbody test");
			ImGui::Text(("X: " + std::to_string(test.getPos().x)).c_str());
			ImGui::Text(("Y: " + std::to_string(test.getPos().y)).c_str());
			ImGui::Text(("VX: " + std::to_string(a.velocity.x)).c_str());
			ImGui::Text(("VY: " + std::to_string(a.velocity.y)).c_str());
			ImGui::Text(("AV: " + std::to_string(a.angular_velocity)).c_str());
			ImGui::Text(("FX: " + std::to_string(a.getNetForce().x)).c_str());
			ImGui::Text(("FY: " + std::to_string(a.getNetForce().y)).c_str());
			ImGui::End();
		}

		player["chunk_loader"]->as<ChunkLoader>()->loadChunksSquare();
		// ship_v.setPoints(player.getPos(), player.getPos() + player.rigidbody.velocity / 10.f);

		anim_test.animate(deltaTime, 0.5);

		// background.draw(bg);
		Renderable::draw_all();

		player.update(deltaTime);
		Rigidbody2d::updateAll(deltaTime);
		Collider::checkAll(deltaTime);

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