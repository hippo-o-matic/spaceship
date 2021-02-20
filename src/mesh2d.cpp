#include "mesh2d.h"

Polygon::Polygon(std::vector<float> positions, std::vector<unsigned> indices) {
	for(auto it = positions.begin(); it != positions.end(); it++) {
		float x = *it;
		float y = *(++it);	
		vertices.push_back(Vertex2d{ glm::vec2(x, y), glm::vec2(0) });
	}
	this->indices = indices;
	regenTexCoords();
}

Polygon::Polygon(std::vector<glm::vec2> vertices, std::vector<unsigned> indices) {
	this->indices = indices;
}

Polygon::Polygon(std::vector<Vertex2d> vertices, std::vector<unsigned> indices) {
	this->vertices = vertices;
	this->indices = indices;	
}

Polygon::Polygon(Polygon& copy) {
	vertices = copy.vertices;
	indices = copy.indices;
}

// Polygon& applyTexCoords(std::vector<glm::vec2>) {

// }

// Polygon& applyVertexPos(std::vector<glm::vec2>) {

// }

Polygon& Polygon::setOptions(unsigned options) {
	option_transform = glm::mat4(1);
	// Check attributes and perform their operations
	if(options & ROTATE_90)
		option_transform = glm::rotate(option_transform, glm::radians(90.f), glm::vec3(0, 0, 1));
	if(options & ROTATE_180)
		option_transform = glm::rotate(option_transform, glm::radians(180.f), glm::vec3(0, 0, 1));
	if(options & REFLECT_V)
		option_transform[1][1] *= -1;
	if(options & REFLECT_H)
		option_transform[0][0] *= -1;

	glm::mat4 offsetMat = glm::translate(glm::mat4(1), glm::vec3(.5, .5, 0)); // Translate so attrib operations are centered
	option_transform = offsetMat * option_transform * glm::inverse(offsetMat);

	return *this;
}

Polygon& Polygon::setBasis(glm::vec2 lower_left, glm::vec2 upper_right) {
	basis_transform = glm::translate(glm::mat4(1), glm::vec3(lower_left, 0)); // Move from the original origin to the new origin (lower left corner of basis)
	basis_transform = glm::scale(basis_transform, glm::vec3(upper_right - lower_left, 0)); // Scale to the texBasis' size
	return *this;
}

Polygon& Polygon::setIndexOffset(unsigned offset) {
	for(unsigned& index : indices) {
		index += offset;
	}

	return *this;
}

Polygon& Polygon::regenTexCoords() {
	float x_min, x_max, y_min, y_max;
	getPositionMinMax(extractStructField(vertices, &Vertex2d::pos), &x_min, &x_max, &y_min, &y_max);

	for(auto& vertex : vertices) {
		// The texture coordinate for this vertex
		glm::vec3 texCoord = glm::vec3(
			(vertex.pos.x - x_min)/(x_max - x_min),
			(vertex.pos.y - y_min)/(y_max - y_min),
			0
		);

		glm::mat4 tCoordMat = glm::translate(glm::mat4(1), texCoord); // Translate to this point
		tCoordMat = basis_transform * option_transform * tCoordMat;

		vertex.tex = tCoordMat[3]; // Extract new position from the matrix
	}

	return *this;
}

// std::vector<glm::vec2> Polygon::pairPositions(const std::vector<float>& positions) {
// 	for(auto it = positions.begin(); it != positions.end(); it++) {
// 		vertices.emplace_back(*it);
// 		vertices.emplace_back(*++it);
// 	}
// }

void Polygon::getPositionMinMax(const std::vector<float>& positions, float* x_min, float* x_max, float* y_min, float* y_max) {
	// Assign the current minimum and maximums as the first value in the vector
	auto it = positions.begin();
	float _x_min = *it, _x_max = *it;
	it++; // Positions are kept as one x and y value together, this moves to the y value for this pair
	float _y_min = *it, _y_max = *it;

	// Sort them and find the minimum and maximum values
	it++; // Start at second position pair
	for(; it != positions.end(); it++) {
		if(*it < _x_min)
			_x_min = *it;
		if(*it > _x_max)
			_x_max = *it;
		
		it++; // Move to y value
		if(*it < _y_min)
			_y_min = *it;
		if(*it > _y_max)
			_y_max = *it;
	}
	
	(*x_min) = _x_min;
	(*x_max) = _x_max;
	(*y_min) = _y_min;
	(*y_max) = _y_max;
}

void Polygon::getPositionMinMax(const std::vector<glm::vec2>& positions, float* x_min, float* x_max, float* y_min, float* y_max) {
	// Assign the current minimum and maximums as the first value in the vector
	auto it = positions.begin();
	float _x_min = it->x, _x_max = it->x;
	float _y_min = it->y, _y_max = it->y;

	// Sort them and find the minimum and maximum values
	it++; // Start at second position pair
	for(; it != positions.end(); it++) {
		if(it->x < _x_min)
			_x_min = it->x;
		if(it->x > _x_max)
			_x_max = it->x;
		
		if(it->y < _y_min)
			_y_min = it->y;
		if(it->y > _y_max)
			_y_max = it->y;
	}
	
	(*x_min) = _x_min;
	(*x_max) = _x_max;
	(*y_min) = _y_min;
	(*y_max) = _y_max;
}

Polygon& Polygon::mergeBasis(glm::vec2 lower_left, glm::vec2 upper_right) {
	glm::mat4 new_basis = glm::translate(glm::mat4(1), glm::vec3(lower_left, 0)); // Move from the original origin to the new origin (lower left corner of basis)
	new_basis = glm::scale(new_basis, glm::vec3(upper_right - lower_left, 1)); // Scale to the texBasis' size
	basis_transform = basis_transform * new_basis;
	return *this;
}

Polygon& Polygon::unmergeBasis(glm::vec2 lower_left, glm::vec2 upper_right) {
	glm::mat4 new_basis = glm::translate(glm::mat4(1), glm::vec3(lower_left, 0)); // Move from the original origin to the new origin (lower left corner of basis)
	new_basis = glm::scale(new_basis, glm::vec3(upper_right - lower_left, 1)); // Scale to the texBasis' size
	basis_transform = basis_transform * glm::inverse(new_basis);
	return *this;
}

Polygon Primitive::rect(
	glm::vec2 size,
	glm::vec2 offset
){
	Polygon p(
		{
			offset.x + -size.x / 2, offset.y + -size.y / 2,
			offset.x + size.x / 2,  offset.y + -size.y / 2,
			offset.x + size.x / 2,  offset.y + size.y / 2,
			offset.x + -size.x / 2, offset.y + size.y / 2
		},
		{0, 1, 2, 0, 2, 3}
	);

	return p;
}
