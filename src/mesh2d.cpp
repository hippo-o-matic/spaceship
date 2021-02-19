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

// const std::vector<Vertex2d> genTexCoords2d(std::vector<float> v_positions, std::vector<glm::vec2> tex_basis, unsigned int attribs) {
// 	std::vector<Vertex2d> vertices;

// 	float x_min, x_max, y_min, y_max;
// 	getPositionMinMax(v_positions, &x_min, &x_max, &y_min, &y_max);

// 	// Construct vertexes with the same positions, and texcoords from a scale of 0 to 1
// 	// depending on where the vertexes are relative to the polygon
// 	for(auto it = v_positions.begin(); it != v_positions.end(); it++) {
// 		float x = *it, y = *(++it);		

// 		// The texture coordinate for this vertex
// 		glm::vec3 texCoord = glm::vec3(
// 			(x-x_min)/(x_max - x_min),
// 			(y-y_min)/(y_max - y_min),
// 			0
// 		);
// 		glm::mat4 tCoordMat = glm::translate(glm::mat4(1), texCoord); // Translate to this point
// 		glm::mat4 offsetMat = glm::translate(glm::mat4(1), glm::vec3(.5, .5, 0)); // Translate so attrib operations are centered

// 		glm::mat4 basisMat = glm::translate(glm::mat4(1), glm::vec3(tex_basis[0], 0)); // Move from the original origin to the new origin (lower left corner of basis) 
// 		basisMat = glm::scale(basisMat, glm::vec3(tex_basis[1] - tex_basis[0], 0)); // Scale to the texBasis' size 

// 		glm::mat4 attribMat = glm::mat4(1);
// 		// Check attributes and perform their operations
// 		if(attribs & ROTATE_90)
// 			attribMat = glm::rotate(attribMat, glm::radians(90.f), glm::vec3(0, 0, 1));
// 		if(attribs & ROTATE_180)
// 			attribMat = glm::rotate(attribMat, glm::radians(180.f), glm::vec3(0, 0, 1));
// 		if(attribs & REFLECT_V)
// 			attribMat[1][1] *= -1;
// 		if(attribs & REFLECT_H)
// 			attribMat[0][0] *= -1;

// 		attribMat = offsetMat * attribMat * glm::inverse(offsetMat);
// 		tCoordMat = basisMat * attribMat * tCoordMat;
		
// 		vertices.push_back(
// 			Vertex2d {
// 				pos: glm::vec2(x, y),
// 				tex: glm::vec2(tCoordMat[3])
// 			}
// 		);
// 	}

// 	return vertices;
// }

const std::vector<Vertex2d> joinCoords(std::vector<float> pos, std::vector<float> tex) {
	std::vector<Vertex2d> result;
	if(pos.size() != tex.size()) {
		return result;
	}
	for(unsigned i=0; i<pos.size(); i+=2) {
		result.push_back(Vertex2d {
			pos: glm::vec2(pos[i], pos[i+1]),
			tex: glm::vec2(tex[i], pos[i+1])
		});
	}

	return result;
}

const std::vector<Vertex2d> joinCoords(std::vector<glm::vec2> pos, std::vector<glm::vec2> tex) {
	std::vector<Vertex2d> result;
	if(pos.size() != tex.size()) {
		return result;
	}
	for(unsigned i=0; i<pos.size(); i++) {
		result.push_back(Vertex2d {
			pos: pos[i],
			tex: tex[i]
		});
	}

	return result;
}

const std::vector<Vertex2d> joinCoords(std::vector<Vertex2d> pos, std::vector<glm::vec2> tex, float layer) {
	if(pos.size() != tex.size()) {
		return pos;
	}
	for(unsigned i=0; i<pos.size(); i++) {
		pos[i].tex = tex[i];
	}

	return pos;
}

const std::vector<Vertex2d> joinCoords(std::vector<glm::vec2> pos, std::vector<Vertex2d> tex, float layer) {
	if(pos.size() != tex.size()) {
		return tex;
	}
	for(unsigned i=0; i<pos.size(); i++) {
		tex[i].pos = pos[i];
	}

	return tex;
}

Polygon& Polygon::mergeBasis(glm::vec2 lower_left, glm::vec2 upper_right) {
	for(Vertex2d& v : vertices) {
		v.tex *= (upper_right - lower_left);
		v.tex += lower_left;
	}
	return *this;
}

Polygon& Polygon::unmergeBasis(glm::vec2 lower_left, glm::vec2 upper_right) {
	for(Vertex2d& v : vertices) {
		v.tex -= lower_left;
		v.tex /= (upper_right - lower_left);
	}
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
