#include "game/tileGrid.h"

TileGrid::TileGrid(glm::uvec2 _tile_size, glm::uvec2 _chunk_size, unsigned _chunk_slots) : tile_size(_tile_size), chunk_size(chunk_size) {
	initBuffers(_chunk_slots);
}

TileGrid::TileGrid(std::string path, glm::uvec2 _tile_size, glm::uvec2 _chunk_size, unsigned _chunk_slots) : tile_size(_tile_size), chunk_size(chunk_size) {
	openFile(path);
	initBuffers(_chunk_slots);
}

unsigned TileGrid::addTexMap(TexMap map) {
	unsigned offset = 0;
	// The offset for this texmap is going to be at the end of the last loaded texmap, if there is one
	if(!textures.empty()) {
		auto last = textures.rbegin();
		offset = last->first + last->second.columns * last->second.rows;
	}

	for(auto chunk : chunks) {
		chunk.tiles_per_texmap.emplace(std::pair(offset, 0));
	}

	textures.emplace(offset, map);
	return offset;
}

std::vector<TileGrid::Chunk>::iterator TileGrid::addChunk(glm::ivec2 position, std::multimap<unsigned, glm::ivec2> tiles, unsigned file_pos, unsigned file_size) {
	// Not sure what this was for
	// if(!tileIDOffset) {
	// 	for(std::pair<unsigned, glm::vec2> tile : tiles) {
	// 		tile.first = tile.first + tileIDOffset;
	// 	}
	// }

	std::map<unsigned, unsigned> tptm;
	for(auto tmap : textures) {
		tptm.emplace(tmap.first, 0);
	}
	
	chunks.push_back({
		position,
		tiles,
		{},
		file_pos
	});

	return chunks.end()--;
}

TileGrid::Chunk* TileGrid::getChunk(glm::vec2 position) {
	// Calculate the chunk position from the absolute position
	glm::ivec2 chunk_pos = glm::ivec2(
		position.x / chunk_size.x + ((position.x < 0) ? -1 : 0),
		position.y / chunk_size.y + ((position.y < 0) ? -1 : 0)
	);

	auto chunk_it = std::find_if(chunks.begin(), chunks.end(), [chunk_pos](auto c){
		return c.pos == chunk_pos;
	});

	if(chunk_it == chunks.end()) {
		std::multimap<unsigned, glm::ivec2> tiles;
		chunk_it = addChunk(chunk_pos, tiles);
	}

	return &(*chunk_it);
}

void TileGrid::addTileToGrid(glm::vec2 pos, unsigned tileID) {
	Chunk* chunk = getChunk(pos);

	glm::ivec2 tile_pos = glm::ivec2(std::round(pos.x), std::round(pos.y)); // Absolute position in the grid
	tile_pos.x %= chunk_size.x * ((tile_pos.x < 0) ? -1 : 1);
	tile_pos.y %= chunk_size.y * ((tile_pos.y < 0) ? -1 : 1);

	addTileToChunk(tile_pos, tileID, chunk);
}

void TileGrid::addTileToChunk(glm::ivec2 position, unsigned tileID, Chunk* chunk) {
	// Find any tiles with the same position in this chunk
	auto it = std::find_if(chunk->tiles.begin(), chunk->tiles.end(), [position](auto pair) {
		return position == pair.second;
	});

	if(it != chunk->tiles.end()) {
		// If a tile is found, erase it first
		chunk->tiles.erase(it);
	} 
	
	// Insert the tile
	glm::ivec2 grid_pos = glm::ivec2(
		position.x + chunk->pos.x * chunk_size.x,
		position.y + chunk->pos.y * chunk_size.y
	);
	chunk->tiles.emplace(std::pair(tileID, grid_pos));
}

void TileGrid::initBuffers(unsigned slot_count) {
	// Create buffers
	glGenVertexArrays(1, &VAO); // Vertex array object
	glGenBuffers(1, &EBO); // Element buffer object

	unsigned vbos[slot_count];
	glGenBuffers(slot_count, vbos);
	for(unsigned i=0; i < slot_count; i++) {
		chunk_slots.emplace(vbos[i], nullptr);
	}

	glBindVertexArray(VAO); // Start using this array
	
	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2d), (void*)0);

	// The texture coordinates of this tile
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex2d),
		(void*)offsetof(Vertex2d, tex)
	);

	// Set tile indices for all possible tiles in a chunk
	// We only need 1 EBO because any possible combination of tiles will
	// use the same indicies
	std::vector<unsigned> indices;
	for(unsigned i = 0; i < chunk_size.x * chunk_size.y; i+=4) {
		std::vector<unsigned> rect_indices = Primitive::rect(0, 0, 0, 0, i).indices;
		indices.insert(indices.end(), rect_indices.begin(), rect_indices.end());
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		indices.size() * sizeof(unsigned),
		&indices[0],
		GL_STATIC_DRAW
	);

	glBindVertexArray(0); // Clear binds
}


void TileGrid::updateVBO(unsigned VBO) {
	std::vector<float> chunk_verts;
	Chunk* chunk = chunk_slots.at(VBO);
	if(chunk == nullptr) // No assigned chunk, skip
		return;

	// Fill final_tiles' tiles with the correct texture coordinates depending on their texture map
	auto tex = textures.begin(); // Iterator for texture maps
	auto tile = chunk->tiles.begin(); // Iterator for tiles

	while(tile != chunk->tiles.end()) {
		// If the next map offset is less than the current tile, go to that texmap
		// Also make sure the next one isnt beyond the end of the texmaps
		if(std::next(tex) != textures.end() && std::next(tex)->first < tile->first) {
			tex++;
			chunk->tiles_per_texmap.at(tex->first) = 0;
			continue;
		}

		const Polygon tile_mesh = Primitive::rect(tile_size.x, tile_size.y, tile->second.x, tile->second.y);
		std::vector<Vertex2d> tile_verts = joinCoords(tile_mesh.vertices, tex->second.getTileCoords(tile->first - tex->first));

		chunk_verts.insert(chunk_verts.end(), tile_verts.begin(), tile_verts.end());

		tile++; // Success, next tile
		chunk->tiles_per_texmap.at(tex->first)++; // Add 1 to the number of tiles using this texture
	}

	glBindVertexArray(VAO);
	// Set tile vertices
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		chunk_verts.size() * sizeof(Vertex2d), 
		&chunk_verts[0], 
		GL_STATIC_DRAW
	);

	glBindVertexArray(0);
}

void TileGrid::updateChunk(Chunk* chunk) {
	auto it = std::find_if(chunk_slots.begin(), chunk_slots.end(), [chunk](auto pair) {
		return chunk == pair.second;
	});

	if(it == chunk_slots.end()) {
		return;
	}

	updateVBO(it->first);
}

// Marking a chunk places it in a slot, giving it a VBO that it can update so that it will be drawn next drawChunks()
void TileGrid::markChunk(Chunk* chunk) {
	static auto current_slot = chunk_slots.begin();
	current_slot->second = chunk;
	updateVBO(current_slot->first);
	current_slot++;
}

// void TileGrid::updateBuffer() {
// 	std::multimap<unsigned, glm::ivec2> visible_tiles;
// 	chunk_verts.clear();
// 	visible_indices.clear();
// 	// auto loaded_range = chunks.equal_range(true);

// 	for(auto chunk : chunks) {
// 		if(chunk.visible) {
// 			for(auto t : chunk.tiles) {
// 				glm::ivec2 offset = glm::ivec2(
// 					t.second.x + chunk.pos.x * chunk_size.x,
// 					t.second.y + chunk.pos.y * chunk_size.y
// 				);
// 				visible_tiles.emplace(t.first, offset);
// 			}
// 		}
// 	}

// 	// Fill final_tiles' tiles with the correct texture coordinates depending on their texture map
// 	unsigned index_offset = 0;
// 	auto tex = textures.begin(); // Iterator for texture maps
// 	auto tile = visible_tiles.begin(); // Iterator for tiles
// 	tiles_per_texmap.at(tex->first) = 0;

// 	while(tile != visible_tiles.end()) {
// 		// If the next map offset is less than the current tile, go to that texmap
// 		// Also make sure the next one isnt beyond the end of the texmaps
// 		if(std::next(tex) != textures.end() && std::next(tex)->first < tile->first) {
// 			tex++;
// 			tiles_per_texmap.at(tex->first) = 0;
// 			continue;
// 		}

// 		const Polygon tile_mesh = Primitive::rect(tile_size.x, tile_size.y, tile->second.x, tile->second.y, index_offset);
// 		std::vector<Vertex2d> tile_verts = joinCoords(tile_mesh.vertices, tex->second.getTileCoords(tile->first - tex->first));

// 		chunk_verts.insert(chunk_verts.end(), tile_verts.begin(), tile_verts.end());
// 		visible_indices.insert(visible_indices.end(), tile_mesh.indices.begin(), tile_mesh.indices.end());

// 		tile++; // Success, next tile
// 		index_offset += 4; // Number of vertexes in a rectangle
// 		tiles_per_texmap.at(tex->first)++; // Add 1 to the number of tiles using this texture
// 	}
// 	glBindVertexArray(VAO);
// 	// Set tile vertices
// 	glBindBuffer(GL_ARRAY_BUFFER, VBO);
// 	glBufferData(
// 		GL_ARRAY_BUFFER,
// 		chunk_verts.size() * sizeof(Vertex2d), 
// 		&chunk_verts[0], 
// 		GL_STATIC_DRAW
// 	);

// 	glBindVertexArray(0);
// }

void TileGrid::drawChunks(Shader &shader) {
	shader.set("transform", glm::mat4(1));

	// Bind the grid's VAO and EBO
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	for(auto slot : chunk_slots) {
		const unsigned& VBO = slot.first;
		Chunk* chunk = slot.second;
		if(chunk == nullptr)
			continue; // No assigned chunk data, skip

		glBindBuffer(GL_ARRAY_BUFFER, VBO); // There is a seperate VBO for each loaded slot, bind it here

		unsigned i_offset = 0;
		for(auto tex : textures) { // For each texmap, draw the number of tiles that use this map
			TexMap& map = tex.second;
			unsigned index_count = chunk->tiles_per_texmap.at(tex.first) * 6;

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, map.getGLID());
			shader.set("sprite", (int)GL_TEXTURE0);

			glDrawRangeElements(
				GL_TRIANGLES, // Mode
				i_offset, // Start of range
				i_offset + index_count, // End of range
				index_count, // Number of elements
				GL_UNSIGNED_INT, // Type of indices in EBO
				(void*)0 // Unused pointer
			);

			i_offset += index_count;
		}
	}

	glBindVertexArray(0);
}


bool TileGrid::openFile(std::string path) {
	std::ifstream file(path, std::ios::binary);
	if(!file) {
		log("Unable to read map file \"" + path + "\"", ERR);
		return false;
	}
	this->path = path;

	unsigned file_ver;
	file >> file_ver;
	if(file_ver != 0) {
		log("Unsupported map version", ERR);
		return false;
	}

	// If we've succeeded so far, clear any data the grid might have
	textures.clear();
	chunks.clear();

	unsigned count;
	file >> count;
	for(; count > 0; count--) {
		unsigned offset;
		int width, height;
		std::string path;

		file >> offset >> width >> height >> path;
		textures.emplace(offset, TexMap(path, width, height));
	}

	// Extract chunk info
	file >> tile_size.x >> tile_size.y >> chunk_size.x  >> chunk_size.y >> count;
	// Extract chunk header, only includes positions and the position in the file
	for(; count > 0; count--) {
		int x, y;
		unsigned fpos, size;

		file >> x >> y >> fpos >> size;

		addChunk(glm::ivec2(x,y), {}, fpos, size);
	}

	// Report any errors that occured
	if(file.bad()) {
		log("File IO error occured while reading \"" + path +"\"", ERR);	
		return false;
	}

	return true;
}

void TileGrid::loadChunk(Chunk* chunk) {
	if(path.empty()) {
		log("TileGrid does not have an open file to load from", ERR);
		return;
	}

	std::ifstream file(path, std::ios::binary);
	if(!file) {
		log("Unable to read map file \"" + path + "\"", ERR);
		return;
	}

	file.seekg(chunk->file_pos);
	unsigned tile_count = chunk->file_size / (sizeof(unsigned) + sizeof(float) * 2); // Find the number of tiles this chunk has
	for(; tile_count > 0; tile_count--) {
		unsigned tileID;
		int x, y;
		file >> tileID >> x >> y;

		chunk->tiles.emplace(tileID, glm::vec2(x, y));
	}
}

void TileGrid::loadChunksFromPos(std::vector<glm::ivec2> positions) {
	if(path.empty()) {
		log("TileGrid does not have an open file to load from", ERR);
		return;
	}

	std::ifstream file(path, std::ios::binary);
	if(!file) {
		log("Unable to read map file \"" + path + "\"", ERR);
		return;
	}

	for(auto p : positions) {
		auto chunk = std::find_if(chunks.begin(), chunks.end(), [&p](Chunk& c) {
			return c.pos == p;
		});
		auto next = chunk + 1; // Get an iterator to the next chunk

		file.seekg(chunk->file_pos);
		unsigned tile_count = (next->file_pos - chunk->file_pos) / (sizeof(unsigned) + sizeof(float) * 2); // Find the number of tiles this chunk has
		for(; tile_count > 0; tile_count--) {
			unsigned tileID;
			int x, y;
			file >> tileID >> x >> y;

			chunk->tiles.emplace(tileID, glm::vec2(x, y));
		}

	}
}

void TileGrid::saveFile(std::string path) {
	std::ofstream file(path, std::ios::binary);
	if(!file) {
		log("Unable to open map file \"" + path + "\"", ERR);
		return;
	}

	file << format_version;

	// Write texture data
	file << textures.size();
	for(auto t : textures) {
		TexMap& map = t.second;
		// Write in the texture offset, the tile height and width, and the path to the image inside the textures folder 
		file << t.first << map.tile_width << map.tile_height << map.getPath(); 
	}
	
	file << tile_size.x << tile_size.y;
	file << chunk_size.x << chunk_size.y; // Specify width and height
	file << chunks.size(); // Number of chunks

	size_t file_pos = file.tellp();
	for(auto chunk : chunks) {
		// Set the position in the file for this chunk's data
		file_pos += sizeof(float) * 2 + sizeof(unsigned) * 2; // Add the size of this chunk entry
		unsigned chunk_size = chunk.tiles.size() * (sizeof(float) * 2 + sizeof(unsigned));

		file << chunk.pos.x;
		file << chunk.pos.y;
		file << file_pos;
		file << chunk_size;

		// Advance the file_pos for the next chunk
		file_pos += chunk_size; // Add the size of the chunk itself
	}

	// Start writing chunk data
	for(auto chunk : chunks) {
		for(auto tile : chunk.tiles) {
			file << tile.first;
			file << tile.second.x;
			file << tile.second.y;
		}
	}

	// Report any errors that occured
	if(file.bad()) {
		log("File IO error occured while writing to \"" + path +"\"", ERR);	
	}
}