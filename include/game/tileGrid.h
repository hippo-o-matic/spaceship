#pragma once

#include "object2d.h"
#include "texture.h"
#include "mesh2d.h"
#include "shader.h"

#include "glad/glad.h"
#include "glm/glm.hpp"

#include <map>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <forward_list>

class TileGrid {
public:
	// A collection of tiles, used for extracting from files
	struct Chunk {
		glm::ivec2 pos;
		std::multimap<unsigned, glm::ivec2> tiles;
		std::map<unsigned, unsigned> tiles_per_texmap = {}; // Stores the amount of tiles that use a texmap <same key from textures, tile_num>
		unsigned file_pos = 0; // The chunk's position in the grid file
		unsigned file_size = 0;
	};

	TileGrid(glm::uvec2 _tile_size = glm::uvec2(1), glm::uvec2 _chunk_size = glm::uvec2(32), unsigned _chunk_slots = 9);
	TileGrid(std::string path, glm::uvec2 _tile_size = glm::uvec2(1), glm::uvec2 _chunk_size = glm::uvec2(32), unsigned _chunk_slots = 4);

	// This grids chunks, declared by a map file and loaded in when needed
	std::map<unsigned, Chunk*> chunk_slots;

	glm::uvec2 getChunkSize();

	Chunk* addChunk(glm::ivec2 position, std::multimap<unsigned, glm::ivec2> tiles, unsigned file_pos = 0, unsigned file_size = 0);
	Chunk* getChunk(glm::vec2 position);
	Chunk* getChunkFromGridPos(glm::ivec2 chunk_pos);

	Chunk* addTileToGrid(glm::vec2 position, unsigned tileID);
	void addTileToChunk(glm::ivec2 position, unsigned tileID, Chunk* chunk);

	unsigned addTexMap(TexMap);
 
	void updateVBO(unsigned);
	void updateChunk(Chunk* chunk);
	void markChunk(Chunk* chunk);
	void drawChunks(Shader& shader); // Draws tiles from the currently loaded chunks using data in the IBO (from updateTiles())

	bool openFile(std::string); // Reads texture data and chunk positions into the grid
	void loadChunk(Chunk* chunk);
	void loadChunksFromPos(std::vector<glm::ivec2> positions); // Reads tiles from a chunk in the file opened by openFile
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