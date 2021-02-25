#include "sprite.h"

const char* Sprite::default_shader_path_vert = "tests/shader/sprite.vs";
const char* Sprite::default_shader_path_frag = "tests/shader/sprite.fs";
const char* TiledSprite::tiled_shader_path_vert = "tests/shader/tile.vs";

Sprite::Sprite(std::string id, std::string image_path, int layer) : 
	Renderable(defaultShader(), layer),
	Object2d(id)
{

	texture = loadTexture(image_path);
	init_buffers();
}

Sprite::Sprite(std::string id, Texture tex, int layer) : 
	Renderable(defaultShader(), layer),
	Object2d(id)
{
	texture = tex;
	mesh = Primitive::rect(glm::vec2(1), glm::vec2(0)).setBasis(tex.basis_lower_left, tex.basis_upper_right);

	init_buffers();
}

Shader* Sprite::defaultShader() {
	static Shader shader(default_shader_path_vert, default_shader_path_frag);
	return register_shader(&shader);
}

void Sprite::init_buffers() {
	// Create buffers
	glGenVertexArrays(1, &VAO); // Vertex array object
	glGenBuffers(1, &VBO); // Vertex buffer object
	glGenBuffers(1, &EBO); // Element buffer object

	updateMesh(); // Send vertices and indices
	glBindVertexArray(VAO); // Start using this array

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2d), (void*)0);

	// vertex texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex2d),
		(void*)offsetof(Vertex2d, tex)
	);

	glBindVertexArray(0); // Clear binds
}

void Sprite::updateMesh() {
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		mesh.vertices.size() * sizeof(Vertex2d), 
		&mesh.vertices[0], 
		GL_STATIC_DRAW
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		mesh.indices.size() * sizeof(unsigned int),
		&mesh.indices[0],
		GL_STATIC_DRAW
	);

	glBindVertexArray(0); // Clear binds
}

// render the mesh
void Sprite::draw(Shader& shader){
	unsigned tex_unit = GL_TEXTURE0;
	glActiveTexture(tex_unit);
	glBindTexture(GL_TEXTURE_2D, texture.glID);
	shader.set("sprite", (int)tex_unit);

	obj_layer = getLayer();
	shader.set("transform", getWorldTransform());
	shader.set("layer", getLayer());

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
	
	// Cleanup
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

// TiledSprite ////////////////////////////

TiledSprite::TiledSprite(std::string id, std::string path, glm::vec2 size, glm::ivec2 range, int layer) : 
	Sprite(id, path, layer)
{
	init_IBO(size, range);
	render_shader = tilingShader();
}

Shader* TiledSprite::tilingShader() {
	static Shader shader(tiled_shader_path_vert, default_shader_path_frag);
	return register_shader(&shader);
}

void TiledSprite::init_IBO(glm::vec2 offset_size, glm::ivec2 tiling_range) {
	glBindVertexArray(VAO);
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ARRAY_BUFFER, IBO);

	// Vertex attrib for offsets
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
	glVertexAttribPointer(2, 2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(glm::vec2),
		(void*)0
	);

	updateOffsets(offset_size, tiling_range);
}

void TiledSprite::updateOffsets(glm::vec2 offset_size, glm::ivec2 tiling_range) {
	this->offset_size = offset_size;
	this->tiling_range = tiling_range;
	std::vector<glm::vec2> offsets;
	
	// If a range dimension is even, we need to offset the tiles by half their size in that direction
	glm::vec2 add_offset = glm::vec2(0);
	if(tiling_range.x % 2 == 0)
		add_offset.x = offset_size.x / 2;
	if(tiling_range.y % 2 == 0)
		add_offset.y = offset_size.y / 2;

	tiling_range /= 2; // Divide by 2 so we can center the range on the middle sprite
	for(int i = -tiling_range.x; i <= tiling_range.x; i++) { // Column, left to right
		for(int j = -tiling_range.y; j <= tiling_range.y; j++) { // Row, bottom to top
			offsets.push_back(glm::vec2(i, j) * offset_size + add_offset);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, IBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &offsets[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TiledSprite::draw(Shader& shader) {
	unsigned tex_unit = GL_TEXTURE0;
	glActiveTexture(tex_unit);
	glBindTexture(GL_TEXTURE_2D, texture.glID);
	shader.set("sprite", (int)tex_unit);

	obj_layer = getLayer();
	shader.set("transform", getWorldTransform());
	shader.set("layer", getLayer());

	int tile_count = tiling_range.x * tiling_range.y;

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElementsInstanced(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr, tile_count);

	// Cleanup
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

// AnimSprite ////////////////////////////////////////////////

AnimSprite::AnimSprite(std::string id, std::string path, unsigned frame_height, unsigned frame_count, int layer) : 
	Sprite(id, path, layer),
	frame_height(frame_height),
	end_frame(frame_count)
{
	updateTexture();
}

AnimSprite::AnimSprite(std::string id, Texture tex, unsigned frame_height, unsigned frame_count, int layer) :
	Sprite(id, tex, layer),
	frame_height(frame_height),
	end_frame(frame_count)
{
	updateTexture();
}

unsigned AnimSprite::setFrame(unsigned frame) {
	frame = frame % (end_frame - start_frame) + start_frame;
	updateTexture();
	return frame;
}

unsigned AnimSprite::nextFrame(int frames) {
	frame += frames;
	frame = frame % (end_frame - start_frame) + start_frame;
	updateTexture();
	return frame;
}

// Calculate the texture coordinates for this frame
void AnimSprite::updateTexture() {
	static unsigned last_frame = 0;
	// TODO: Ughhhhh fix unmergeBasis
	// First, undo the basis from the last frame change
	// Don't do this the first time this function is called, or we'll end up undoing something that didn't happen
	static bool first_update = true;
	if(!first_update) {
		// Undoes the last update. Read the next part to see what this does
		glm::vec2 lower_left(0, (last_frame * frame_height) / (float)texture.height);
		glm::vec2 upper_right(1, ((last_frame + 1) * frame_height) / (float)texture.height); // Add 1 to the frame to get the top basis
		mesh.unmergeBasis(lower_left, upper_right); 
	 	last_frame = frame;
	} else {
		first_update = false; // Not a fan of this branch personally, but idk a cooler way to do this
	}

	// Find the basis for the frame
	// Animated textures have their frames arranged vertically, so the horizontal on the basis will just be 0 to 1
	// The lower basis is the frame # (starting at 0), times the frames height, divided by the total texture height to scale it between 0 and 1
	glm::vec2 lower_left(0, (frame * frame_height) / (float)texture.height);
	glm::vec2 upper_right(1, ((frame + 1) * frame_height) / (float)texture.height); // Add 1 to the frame to get the top basis
	mesh.mergeBasis(lower_left, upper_right).regenTexCoords(); // Merge the frame basis with the current basis
	// Only regenTexCoords at the end here, we don't need to do it while undoing the previous basis
	updateMesh(); // Send new mesh to VBO
}

void AnimSprite::animate(float deltaTime, float fps) {
	static float time_since_last_frame = 0;
	time_since_last_frame += deltaTime;
	nextFrame(time_since_last_frame / fps);
	time_since_last_frame = fmod(time_since_last_frame, fps);
}