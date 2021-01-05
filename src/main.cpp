#include "main.h"

int main() {
	GLFWwindow* window = init_main_window();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int appstate = 1;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	// Testing zone
	Editor::init();
	Editor::input_map.activate();

	Camera2d cam;
	cam.display_height = &SCR_HEIGHT;
	cam.display_width = &SCR_WIDTH;

	Shader shade("tests/shader/sprite.vs", "tests/shader/sprite.fs");
	Shader bg("tests/shader/bg.vs", "tests/shader/sprite.fs");

	Sprite background("tests/img/space.png", glm::vec2(0), 0, glm::vec2(5));

	Sprite test("tests/img/tex.png");
	Sprite vest("tests/img/gex.png", glm::vec2(.5, .75));
	vest.layer = 1;

	TileGrid grid = gridTest();
	Editor::grid = &grid;
	Editor::camera = &cam;

	TexMap map("tests/textures/colorgrid.png", 24, 24);
	Sprite aiee(map.getTileTexture(0), glm::vec2(-1,-1));
	
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

	// Main loop
	while(appstate) {
		if(glfwWindowShouldClose(window))
			appstate = 0;

		// Time calculation
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Calculate input
		Input::processActive(window);

		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shade.set("view", cam.getViewMatrix());
		shade.set("projection", cam.getProjectionMatrix());
	
		bg.set("view", cam.getViewMatrix());
		bg.set("projection", cam.getProjectionMatrix());

		// background.draw(bg);

		test.draw(shade);
		aiee.draw(shade);
		// vest.draw(shade);
		grid.drawChunks(shade);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

TileGrid gridTest() {
	std::multimap<unsigned, glm::ivec2> tiles;
	// tiles.insert(std::pair(1, glm::vec2(-1, 0)));
	// tiles.insert(std::pair(1, glm::vec2(1,0)));
	// tiles.insert(std::pair(0, glm::vec2(-2, -2)));

	TileGrid grid;
	TexMap map("tests/textures/colorgrid.png", 24, 24);
	grid.addTexMap(map);
	grid.addChunk(glm::vec2(0), tiles);

	grid.updateBuffer();

	return grid;
}