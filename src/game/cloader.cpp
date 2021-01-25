#include "game/cloader.h"

ChunkLoader::ChunkLoader(std::string id, TileGrid* grid, unsigned radius) : Object2d(id), grid(grid), radius(radius) {}

std::vector<glm::ivec2> ChunkLoader::calcChunkCoords(glm::ivec2 offset) {
	std::vector<glm::ivec2> chunk_coords;
	for(int i = -radius; i <= radius; i++) { // Row
		for(int j = -radius; j <= radius; j++) { // Column
			chunk_coords.push_back(glm::ivec2(i, j) + offset);
		}
	}

	return chunk_coords;
}

void ChunkLoader::loadChunksSquare() {
	glm::vec2 world_pos = getWorldPos(); // Get the position of this chunkloader
	glm::uvec2 c_size = grid->getChunkSize(); // Get the chunk size of the grid we load
	glm::uvec2 t_size = grid->getTileSize();

	// Get the chunk coordinate that this loader is in
	glm::ivec2 now_pos = glm::ivec2(
		floor((world_pos.x + t_size.x / 2.f) / c_size.x),
		floor((world_pos.y + t_size.y / 2.f) / c_size.y)
	);

	// Remember what the last chunk we were in was
	static glm::ivec2 last_pos = glm::ivec2(world_pos.x + radius * 2 + 1, 0); // Initialized so we start by loading everything once
	
	if(now_pos == last_pos)
		return; // We haven't moved from this chunk, so we dont need to update
	
	std::vector<glm::ivec2> now_chunks = calcChunkCoords(now_pos); // Get the coordinates of each chunk that should be loaded now
	// Same, but for the last coordinates. Only calculates once, as it is static we simply store the now_chunks in it afterwards
	static std::vector<glm::ivec2> last_chunks = calcChunkCoords(last_pos);
	std::vector<glm::ivec2> to_load;
	std::vector<glm::ivec2>& to_unload = last_chunks;

	// Find the differences between the last and current chunks, so we dont try to load the same ones twice
	// TODO: find out if this is slower than just finding all now_chunks

	for(auto chunk : now_chunks) {
		auto f = std::find(last_chunks.begin(), last_chunks.end(), chunk);
		if(f == last_chunks.end()) {
			to_load.push_back(std::move(chunk)); // Chunk not already loaded, add to load
		} else {
			last_chunks.erase(f); // Chunk already loaded, erase from here, the remaining chunks will need to be unloaded
		}
	}
	// The chunks remaining in last_chunks are ones not found in now_chunks, as such they will be replaced

	// Replace each to_unload chunk with one that needs to be loaded
	auto load_coord = to_load.begin();
	for(auto replace_coord : to_unload) {
		// Find the chunk to replace in the slots
		auto to_replace = std::find_if(grid->chunk_slots.begin(), grid->chunk_slots.end(), [replace_coord](auto pair){
			if(pair.second == nullptr) return true; // Empty slot, just fill it
			return pair.second->pos == replace_coord;
		});
		to_replace->second = grid->getChunkFromGridPos(*load_coord); // Replace it with one that needs to be loaded
		grid->updateVBO(to_replace->first);

		load_coord++;
	}

	// Remember these for next call
	last_pos = now_pos;
	last_chunks = now_chunks;
}