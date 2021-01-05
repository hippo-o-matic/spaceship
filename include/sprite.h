#pragma once

#include "object2d.h"
#include "shader.h"
#include "mesh2d.h"
#include "texture.h"

#include "glm/glm.hpp"
#include "stb_image.h" // For loading textures

#include <string>
#include <filesystem> // For loading textures

class Sprite : public Object2d {
public:
	Sprite(std::string,
		glm::vec2 _pos = glm::vec2(0),
		double _rot = 0.0,
		glm::vec2 _scl = glm::vec2(1)
	);

	Sprite(
		Texture tex,
		glm::vec2 _pos = glm::vec2(0),
		double _rot = 0.0,
		glm::vec2 _scl = glm::vec2(1)
	);

	Polygon mesh;
	Texture texture;

	unsigned int VAO;

	void draw(Shader &shader);
	void updateMesh();

	static std::vector<Sprite> sprites;

protected:
	unsigned int VBO, EBO;
	// initializes all the buffer objects/arrays
	void init_buffers();
};

