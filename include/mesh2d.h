#pragma once

#include "logs.h"

#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <algorithm>

struct Vertex2d {
	glm::vec2 pos; // Position
	float layer = 0;
	glm::vec2 tex; // Texture coordinates
};

struct Polygon {
	std::vector<Vertex2d> vertices;
	std::vector<unsigned> indices;
};

// Automatically generates texture coordinates for a vector of 2d vertices
// Also takes 2 vec2's, representing a subsection of the texture to be used
// basis[0] being the top left texcoord, and basis[1] being the botom right
const std::vector<Vertex2d> genTexCoords2d(std::vector<float> v_positions, std::vector<glm::vec2> = { glm::vec2(0), glm::vec2(1) });
// Takes seperate vectors of position and texture coordinates and joins them to a vector of Vertex2d
const std::vector<Vertex2d> joinCoords(std::vector<float> pos, std::vector<float> tex, float layer = 0);
// Takes seperate vectors of position and texture coordinates and joins them to a vector of Vertex2d
const std::vector<Vertex2d> joinCoords(std::vector<glm::vec2> pos, std::vector<glm::vec2> tex, float layer = 0);
// Overwrites the texcoords in the vector of Vertex2d provided
const std::vector<Vertex2d> joinCoords(std::vector<Vertex2d> pos, std::vector<glm::vec2> tex, float layer = 0);
// Overwrites the positions in the vector of Vertex2d provided
const std::vector<Vertex2d> joinCoords(std::vector<glm::vec2> pos, std::vector<Vertex2d> tex, float layer = 0);

namespace Primitive {
	const Polygon rect(
		float width = 1,
		float height = 1,
		float offsetX = 0,
		float offsetY = 0,
		unsigned index_offset = 0,
		std::vector<glm::vec2> tex_basis = { glm::vec2(0), glm::vec2(1) }
	);
}