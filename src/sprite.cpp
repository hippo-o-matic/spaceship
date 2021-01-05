#include "sprite.h"

Sprite::Sprite(std::string image_path, glm::vec2 _pos, double _rot, glm::vec2 _scl) : Object2d(_pos, _rot, _scl) {
	mesh = Primitive::rect(1, 1);
	texture = loadTexture(image_path);

	init_buffers();
}

Sprite::Sprite(Texture tex, glm::vec2 _pos, double _rot, glm::vec2 _scl) : Object2d(_pos, _rot, _scl) {
	texture = tex;
	mesh = Primitive::rect(1, 1, 0, 0, 0, tex.basis);

	init_buffers();
}

// render the mesh
void Sprite::draw(Shader &shader){
	unsigned tex_unit = GL_TEXTURE0;
	glActiveTexture(tex_unit);
	glBindTexture(GL_TEXTURE_2D, texture.glID);
	shader.set("sprite", (int)tex_unit);

	shader.set("transform", getWorldTransform());

	// draw mesh
	glBindVertexArray(VAO);
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2d), (void*)0);
	glEnableVertexAttribArray(0);

	// vertex texture coords
	glVertexAttribPointer(1, 2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex2d),
		(void*)offsetof(Vertex2d, tex)
	);
	glEnableVertexAttribArray(1);


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