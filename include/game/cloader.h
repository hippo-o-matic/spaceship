#pragma once

#include "object2d.h"
#include "game/tileGrid.h"

class ChunkLoader : public Object2d {
	TileGrid* grid;

	ChunkLoader(TileGrid* grid);

	
}
