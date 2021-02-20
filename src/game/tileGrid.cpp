#include "game/tileGrid.h"

// Static Initialization /////////////////////////////////////////////////////////

const char* TileGrid::default_shader_path_vert = "tests/shader/sprite.vs";
const char* TileGrid::default_shader_path_frag = "tests/shader/sprite.fs";

// TileGrid Member Functions /////////////////////////////////////////////////////

TileGrid::TileGrid(glm::uvec2 tile_size, glm::uvec2 chunk_size, unsigned chunk_slots, int layer) : 
	Renderable([this](Shader& shader) {
		shader.set("layer", getLayer());
		this->drawChunks(shader);
	}, defaultShader(), layer),
	tile_size(tile_size), chunk_size(chunk_size)
{
	initBuffers(chunk_slots);
}

TileGrid::TileGrid(std::string path, unsigned chunk_slots, int layer) : 
	Renderable([this](Shader& shader) {
		shader.set("layer", getLayer());
		this->drawChunks(shader);
	}, defaultShader(), layer)
{
	if(!loadFile(path)) {
		tile_size = glm::uvec2(0);
		chunk_size = glm::uvec2(32);
	}
	initBuffers(chunk_slots);
}

Shader* TileGrid::defaultShader() {
	static Shader shader(default_shader_path_vert, default_shader_path_frag);
	return register_shader(&shader);
}

glm::uvec2 TileGrid::getChunkSize() {
	return chunk_size;
}

glm::uvec2 TileGrid::getTileSize() {
	return tile_size;
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

unsigned TileGrid::addTexMap(TexMap map, unsigned offset) {
	textures.emplace(offset, map);

	for(auto chunk : chunks) {
		chunk.tiles_per_texmap.emplace(std::pair(offset, 0));
	}

	return offset;
}

TileGrid::Chunk* TileGrid::addChunk(glm::ivec2 position, TileSet tiles, unsigned file_pos, unsigned file_size) {
	// Not sure what this was for
	// if(!tileIDOffset) {
	// 	for(std::pair<unsigned, glm::vec2> tile : tiles) {
	// 		tile.tileID = tile.tileID + tileIDOffset;
	// 	}
	// }

	auto chunk = std::find_if(chunks.begin(), chunks.end(), [position](Chunk& c){
		return c.pos == position;
	});

	if(chunk == chunks.end()) {
		std::map<unsigned, unsigned> tptm;
		for(auto tmap : textures) {
			tptm.emplace(tmap.first, 0);
		}
		
		chunks.push_front({
			position,
			tiles,
			tptm,
			file_pos,
			file_size
		});

		return &chunks.front();
	} else {
		chunk->tiles = tiles;
		chunk->tile_index = file_pos;
		chunk->tile_count = file_size;
		return &(*chunk);
	}
}

TileGrid::Chunk* TileGrid::getChunk(glm::vec2 position) {
	// Calculate the chunk position from the absolute position
	glm::ivec2 chunk_pos = calcChunkPos(position);

	auto chunk_it = std::find_if(chunks.begin(), chunks.end(), [chunk_pos](auto c){
		return c.pos == chunk_pos;
	});

	if(chunk_it == chunks.end()) {
		return addChunk(chunk_pos, {});
	}

	return &(*chunk_it);
}

TileGrid::Chunk* TileGrid::getChunkFromGridPos(glm::ivec2 chunk_pos) {
	auto chunk_it = std::find_if(chunks.begin(), chunks.end(), [chunk_pos](auto c){
		return c.pos == chunk_pos;
	});

	if(chunk_it == chunks.end()) {
		return addChunk(chunk_pos, {});
	}

	return &(*chunk_it);
}

TileGrid::Chunk* TileGrid::addTileToGrid(glm::vec2 pos, unsigned tileID, unsigned attribs) {
	Chunk* chunk = getChunk(pos);

	glm::ivec2 tile_pos = glm::ivec2(std::round(pos.x), std::round(pos.y)); // Absolute position in the grid
	tile_pos.x %= chunk_size.x * ((tile_pos.x < 0) ? -1 : 1);
	tile_pos.y %= chunk_size.y * ((tile_pos.y < 0) ? -1 : 1);

	addTileToChunk(chunk, tile_pos, tileID, attribs);
	return chunk;
}

TileGrid::Chunk* TileGrid::removeTileFromGrid(glm::vec2 pos) {
	Chunk* chunk = getChunk(pos);

	glm::ivec2 tile_pos = glm::ivec2(std::round(pos.x), std::round(pos.y)); // Absolute position in the grid
	tile_pos.x %= chunk_size.x * ((tile_pos.x < 0) ? -1 : 1);
	tile_pos.y %= chunk_size.y * ((tile_pos.y < 0) ? -1 : 1);

	removeTileFromChunk(chunk, tile_pos);
	return chunk;
}

void TileGrid::addTileToChunk(Chunk* chunk, glm::ivec2 position, unsigned tileID, unsigned attribs) {
	// Find any tiles with the same position in this chunk
	removeTileFromChunk(chunk, position);
	
	// Insert the tile
	glm::ivec2 pos_in_chunk = glm::ivec2(
		position.x % chunk_size.x,
		position.y % chunk_size.y
	);
	chunk->tiles.insert({ tileID, pos_in_chunk, attribs });
}

void TileGrid::removeTileFromChunk(Chunk* chunk, glm::ivec2 position) {
	// Find any tiles with the same position in this chunk
	auto it = std::find_if(chunk->tiles.begin(), chunk->tiles.end(), [position](auto tile) {
		return position == tile.pos;
	});

	if(it != chunk->tiles.end()) {
		// If a tile is found, erase it first
		chunk->tiles.erase(it);
	} 
}

glm::ivec2 TileGrid::calcChunkPos(glm::vec2 world_pos) {
	glm::ivec2 chunk_pos = glm::ivec2(
		floor((world_pos.x + tile_size.x / 2.f) / chunk_size.x),
		floor((world_pos.y + tile_size.y / 2.f) / chunk_size.y)
	);
	return chunk_pos;
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

	// Set tile indices for all possible tiles in a chunk
	// We only need 1 EBO because any possible combination of tiles will
	// use the same indicies
	std::vector<unsigned> indices;
	for(unsigned i = 0; i < chunk_size.x * chunk_size.y * 4; i+=4) {
		std::vector<unsigned> rect_indices = Primitive::rect(glm::vec2(0), glm::vec2(0)).setIndexOffset(i).indices;
		indices.insert(indices.end(), rect_indices.begin(), rect_indices.end());
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		indices.size() * sizeof(unsigned),
		&indices[0],
		GL_STATIC_DRAW
	);

	glEnableVertexAttribArray(0);
	glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex2d, pos));
	glVertexAttribBinding(0, 0);
	
	glEnableVertexAttribArray(1);
	glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex2d, tex));
	glVertexAttribBinding(1, 0);
	
	glBindVertexArray(0); // Clear VAO bind
}


void TileGrid::updateVBO(unsigned VBO) {
	std::vector<Vertex2d> chunk_verts;
	Chunk* chunk = chunk_slots.at(VBO);
	if(chunk == nullptr) // No assigned chunk, skip
		return;

	if(chunk->tiles.empty()) {
		glBindVertexArray(VAO);
		// Set tile vertices
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(
			GL_ARRAY_BUFFER,
			0, 
			nullptr,
			GL_STATIC_DRAW
		);

		glBindVertexArray(0);
		return;
	}
		
	// Fill final_tiles' tiles with the correct texture coordinates depending on their texture map
	auto tex = textures.begin(); // Iterator for texture maps
	auto tile = chunk->tiles.begin(); // Iterator for tiles
	chunk->tiles_per_texmap.at(tex->first) = 0;
	while(tile != chunk->tiles.end()) {
		// If the next map offset is less than the current tile, go to that texmap
		// Also make sure the next one isnt beyond the end of the texmaps
		if(std::next(tex) != textures.end() && std::next(tex)->first < tile->tileID) {
			tex++;
			chunk->tiles_per_texmap.at(tex->first) = 0;
			continue;
		}

		auto basis = tex->second.getTileBasis(tile->tileID - tex->first);
		Polygon tile_mesh = Primitive::rect(tile_size, tile->pos)
			.setOptions(tile->attribs)
			.setBasis(basis[0], basis[1])
			.regenTexCoords();

		for(Vertex2d& vert : tile_mesh.vertices) {
			vert.pos.x += chunk->pos.x * (int)chunk_size.x;
			vert.pos.y += chunk->pos.y * (int)chunk_size.y;
		}

		chunk_verts.insert(chunk_verts.end(), tile_mesh.vertices.begin(), tile_mesh.vertices.end());

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
	shader.set("layer", getLayer());

	// Bind the grid's VAO and EBO
	glBindVertexArray(VAO);

	for(auto slot : chunk_slots) {
		Chunk* chunk = slot.second;
		if(chunk == nullptr)
			continue; // No assigned chunk data, skip

		glBindVertexBuffer(0, slot.first, 0, sizeof(Vertex2d)); // There is a seperate VBO for each loaded slot, bind it here

		unsigned i_offset = 0;
		for(auto tex : textures) { // For each texmap, draw the number of tiles that use this map
			TexMap& map = tex.second;
			unsigned index_count = chunk->tiles_per_texmap.at(tex.first) * 6;
			if(index_count == 0)
				continue;

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


bool TileGrid::loadFile(std::string path) {
	std::ifstream file(path);
	if(!file) {
		log("Unable to read map file \"" + path + "\"", ERR);
		return false;
	}

	tileGridProto::Grid pb;
	if(!pb.ParseFromIstream(&file)) {
		log("Failed to load map file \"" + path + "\"", ERR);
		return false;
	}

	this->path = path; // The file was successfully read, make its path the active path

	// If we've succeeded so far, clear any data the grid might have
	textures.clear();

	for(auto tex : pb.textures()) {
		unsigned offset = tex.offset();
		int width = tex.tile_width();
		int height = tex.tile_height();
		std::string path = tex.path();

		addTexMap(TexMap(path, width, height), offset);
	}

	tile_size.x = pb.grid_width();
	tile_size.y = pb.grid_height();
	chunk_size.x = pb.chunk_width();
	chunk_size.y = pb.chunk_height();
	
	// Extract chunk header, only includes positions and the position in the file
	for(auto c : pb.chunks()) {
		int x = c.x();
		int y = c.y();
		unsigned count = c.tile_count();
		unsigned index = c.tile_index();

		TileSet tiles;
		for(unsigned i = index; i < index + count; i++) {
			auto& pb_tile = pb.tiles()[i];
			Tile t;
			t.tileID = pb_tile.tileid();
			t.pos.x = pb_tile.x();
			t.pos.y = pb_tile.y();
			t.attribs = pb_tile.attribs();

			tiles.insert(t);
		}

		addChunk(glm::ivec2(x,y), tiles, count, index);
	}

	// Report any errors that occured
	if(file.bad()) {
		log("File IO error occured while reading \"" + path +"\"", ERR);	
		return false;
	}

	for(auto pair : chunk_slots) {
		updateVBO(pair.first);
	}

	return true;
}

void TileGrid::saveFile(std::string path) {
	std::ofstream file(path, std::ios::binary | std::ofstream::trunc);
	if(!file) {
		log("Unable to open map file \"" + path + "\"", ERR);
		return;
	}

	tileGridProto::Grid pb;
	pb.set_id("none");

	// Write texture data
	auto pb_texs = pb.textures();
	for(auto t : textures) {
		TexMap& map = t.second;
		auto tex_entry = pb.add_textures();
		tex_entry->set_offset(t.first);
		tex_entry->set_tile_width(map.tile_width);
		tex_entry->set_tile_height(map.tile_height);
		tex_entry->set_path(map.getPath());
	}
	
	// Set grid attributes
	pb.set_grid_width(tile_size.x);
	pb.set_grid_height(tile_size.y);
	pb.set_chunk_width(chunk_size.x);
	pb.set_chunk_height(chunk_size.y);

	unsigned tile_index = 0;
	for(auto chunk : chunks) {
		auto c_entry = pb.add_chunks();
		c_entry->set_x(chunk.pos.x);
		c_entry->set_y(chunk.pos.y);
		c_entry->set_tile_count(chunk.tiles.size());
		c_entry->set_tile_index(tile_index);

		tile_index += chunk.tiles.size();

		for(auto tile : chunk.tiles) {
			auto t_entry = pb.add_tiles();
			t_entry->set_tileid(tile.tileID);
			t_entry->set_x(tile.pos.x);
			t_entry->set_y(tile.pos.y);
			t_entry->set_attribs(tile.attribs);
		}
	}

	// Serialize and report any errors
	if(!pb.SerializeToOstream(&file)) {
		log("Failed to save map file \"" + path + "\"", ERR);
	}

	if(file.bad()) {
		log("File IO error occured while writing to \"" + path +"\"", ERR);	
	}
}