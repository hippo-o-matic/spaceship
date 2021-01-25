#pragma once

#include "object2d.h"
#include "texture.h"
#include "mesh2d.h"
#include "shader.h"

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "tileGrid.pb.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <forward_list>

class TileGrid {
public:
	struct Tile {
		unsigned tileID;
		glm::ivec2 pos;
		unsigned int attribs = NONE;
	};
	struct TileLess {
		bool operator()(const Tile& a, const Tile& b) const {
			return a.tileID < b.tileID;
		}
	};
	typedef std::multiset<Tile, TileLess> TileSet;

	// A collection of tiles, used for extracting from files
	struct Chunk {
		glm::ivec2 pos;
		TileSet tiles;
		std::map<unsigned, unsigned> tiles_per_texmap = {}; // Stores the amount of tiles that use a texmap <same key from textures, tile_num>
		unsigned tile_count = 0;
		unsigned tile_index = 0; // The chunk's position in the grid file
	};

	TileGrid(glm::uvec2 tile_size = glm::uvec2(1), glm::uvec2 chunk_size = glm::uvec2(32), unsigned chunk_slots = 9);
	TileGrid(std::string path, unsigned chunk_slots = 9);

	// This grids chunks, declared by a map file and loaded in when needed
	std::map<unsigned, Chunk*> chunk_slots;

	glm::uvec2 getChunkSize();
	glm::uvec2 getTileSize();

	Chunk* addChunk(glm::ivec2 position, TileSet tiles, unsigned file_pos = 0, unsigned file_size = 0);
	Chunk* getChunk(glm::vec2 position);
	Chunk* getChunkFromGridPos(glm::ivec2 chunk_pos);
	glm::ivec2 calcChunkPos(glm::vec2 world_pos);

	Chunk* addTileToGrid(glm::vec2 position, unsigned tileID, unsigned attribs = NONE);
	void addTileToChunk(Chunk* chunk, glm::ivec2 position, unsigned tileID, unsigned attribs = NONE);

	unsigned addTexMap(TexMap);
	unsigned addTexMap(TexMap map, unsigned offset);
 
	void updateVBO(unsigned);
	void updateChunk(Chunk* chunk);
	void markChunk(Chunk* chunk);
	void drawChunks(Shader& shader); // Draws tiles from the currently loaded chunks using data in the IBO (from updateTiles())

	bool loadFile(std::string); // Reads texture data and chunk positions into the grid
	void saveFile(std::string); // Saves all chunks and texture data to a readable map file

private:
	const unsigned format_version = 0;

	unsigned VAO, EBO;
	glm::uvec2 tile_size; // The amount of world units the tiles should be
	glm::uvec2 chunk_size = glm::uvec2(32); // Columns and rows in a chunk
	std::string path;
	std::forward_list<Chunk> chunks;

	std::map<unsigned, TexMap> textures; 

	void initBuffers(unsigned slot_count); // Called once to initalize buffers, setting up `slot_count` slots
};