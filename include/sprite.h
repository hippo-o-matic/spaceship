#pragma once

#include "render.h"
#include "object2d.h"
#include "shader.h"
#include "mesh2d.h"
#include "texture.h"

#include "glm/glm.hpp"

#include <string>

class Sprite : public Renderable, public Object2d {
public:
	Sprite(std::string id, std::string path, int layer = 1);
	Sprite(std::string id, Texture tex, int layer = 1);

	Polygon mesh = Primitive::rect();
	Texture texture;

	unsigned int VAO;

	virtual void draw(Shader &shader);
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

class TiledSprite : public Sprite {
public:
	TiledSprite(
		std::string id,
		std::string path,
		glm::vec2 size,
		glm::ivec2 range,
		int layer = 1
	);

	TiledSprite(
		std::string id,
		Texture tex,
		glm::vec2 size,
		glm::ivec2 range,
		int layer = 1
	);
	
	void updateOffsets(glm::vec2 offset_size, glm::ivec2 tiling_range);
	void draw(Shader& shader) override;
	static Shader* tilingShader();

private:
	unsigned IBO;
	glm::vec2 offset_size;
	glm::ivec2 tiling_range;

	void init_IBO(glm::vec2 offset_size, glm::ivec2 tiling_range);

	static const char* tiled_shader_path_vert;
};

class AnimSprite : public Sprite {
public:
	AnimSprite(std::string id, std::string path, unsigned frame_height, unsigned frame_count, int layer = 1);
	AnimSprite(std::string id, Texture tex, unsigned frame_height, unsigned frame_count, int layer = 1);

	unsigned frame_height;
	unsigned start_frame = 0;
	unsigned end_frame;

	unsigned setFrame(unsigned frame);
	unsigned nextFrame(int frames = 1);

	void animate(float deltaTime, float fps);

private:
	unsigned frame = 0;

	void updateTexture();
};