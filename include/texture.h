#pragma once

#include "logs.h"

#include "glm/glm.hpp"
#include "glad/glad.h"
#include "stb_image.h"

#include <string>
#include <map>
#include <vector>
#include <algorithm>

struct Texture {
	unsigned int glID; // The id OpenGL has assigned to this texture
	std::string type; // The name of the texture passed to the shader
	std::string path; // The path to the texture, stored to prevent duplicates from being loaded
	
	int width, height; // Total width and height of this texture
	// The texcoords that specify the top left and bottom right corners of the region that should be used as a texture
	std::vector<glm::vec2> basis = { glm::vec2(0), glm::vec2(1) };

	static std::vector<Texture> loaded; // Keeps track of all the textures we've loaded
};

// Loads a texture from a file
Texture loadTexture(std::string path);

class TexMap {
public:
	TexMap(std::string path, int tile_width, int tile_height);

	// Information about the texture map
	int tile_width;
	int tile_height;
	unsigned columns;
	unsigned rows;

	unsigned getGLID();
	std::string getPath();
	std::vector<glm::vec2> getTileCoords(unsigned index);
	Texture getTileTexture(unsigned index);

	Texture texture;
};