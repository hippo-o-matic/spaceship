#pragma once

#include "render.h"
#include "object2d.h"
#include "shader.h"
#include "mesh2d.h"
#include "texture.h"

#include "glm/glm.hpp"
#include "stb_image.h" // For loading textures

#include <string>
#include <filesystem> // For loading textures

class Sprite : public Renderable, public Object2d {
public:
	Sprite(
		std::string id,
		std::string path,
		glm::vec2 _pos = glm::vec2(0),
		double _rot = 0.0,
		glm::vec2 _scl = glm::vec2(1),
		int layer = 1
	);

	Sprite(
		std::string id,
		Texture tex,
		glm::vec2 _pos = glm::vec2(0),
		double _rot = 0.0,
		glm::vec2 _scl = glm::vec2(1),
		int layer = 1
	);

	Polygon mesh = Primitive::rect();
	Texture texture;

	unsigned int VAO;

	void draw(Shader &shader);
	void updateMesh();

	static std::vector<Sprite> sprites;
	static Shader* defaultShader();

protected:
	unsigned int VBO, EBO;
	
	// initializes all the buffer objects/arrays
	void init_buffers();
	
	static const char* default_shader_path_vert;
	static const char* default_shader_path_frag;
};

