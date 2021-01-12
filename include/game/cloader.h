#pragma once

#include "object2d.h"
#include "game/tileGrid.h"

class ChunkLoader : public Object2d {
public:
	TileGrid* grid;
	unsigned radius;

	ChunkLoader(TileGrid* grid, unsigned radius);
	ChunkLoader(Object2d* parent, TileGrid* grid, unsigned radius);

	void loadChunksSquare();

private:
	std::vector<glm::ivec2> calcChunkCoords(glm::ivec2 offset);
};
