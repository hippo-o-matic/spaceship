#include "sprite.h"

Sprite::Sprite(std::string id, std::string image_path, glm::vec2 _pos, double _rot, glm::vec2 _scl) : Object2d(id, _pos, _rot, _scl) {
	texture = loadTexture(image_path);

	init_buffers();
}

Sprite::Sprite(std::string id, Texture tex, glm::vec2 _pos, double _rot, glm::vec2 _scl) : Object2d(id, _pos, _rot, _scl) {
	texture = tex;
	mesh = Primitive::rect(glm::vec2(1), glm::vec2(0)).setBasis(tex.basis_lower_left, tex.basis_upper_right);

	init_buffers();
}

// render the mesh
void Sprite::draw(Shader &shader){
	unsigned tex_unit = GL_TEXTURE0;
	glActiveTexture(tex_unit);
	glBindTexture(GL_TEXTURE_2D, texture.glID);
	shader.set("sprite", (int)tex_unit);

	shader.set("transform", getWorldTransform());
	shader.set("layer", layer);

	// draw mesh
	glBindVertexArray(VAO);
	// glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
	
	// Cleanup
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
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