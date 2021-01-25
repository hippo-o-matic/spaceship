#pragma once

#include "object2d.h"
#include "game/tileGrid.h"

class ChunkLoader : public Object2d {
public:
	TileGrid* grid;
	int radius;

	ChunkLoader(std::string id, TileGrid* grid, unsigned radius);

	void loadChunksSquare();

private:
	std::vector<glm::ivec2> calcChunkCoords(glm::ivec2 offset);
};
