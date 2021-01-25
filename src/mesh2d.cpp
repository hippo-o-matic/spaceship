#include "mesh2d.h"

const std::vector<Vertex2d> genTexCoords2d(std::vector<float> v_positions, std::vector<glm::vec2> tex_basis, unsigned int attribs) {
	std::vector<Vertex2d> vertices;

	// Gather all the x and y values
	std::vector<float> x_vals;
	std::vector<float> y_vals;

	for(auto it = v_positions.begin(); it != v_positions.end(); it++) {
		x_vals.push_back(*it);
		y_vals.push_back(*(++it));
	}

	// Sort them and find the minimum and maximum values
	std::sort(x_vals.begin(), x_vals.end());
	std::sort(y_vals.begin(), y_vals.end());

	float x_min = *x_vals.begin();
	float x_max = x_vals.back();
	float y_min = *y_vals.begin();
	float y_max = y_vals.back();

	glm::mat4 texTransform = glm::mat4(1);

	
	if(attribs & REFLECT_H)
		texTransform = texTransform * glm::mat4(glm::mat2(glm::vec2(1,0),glm::vec2(0,-1)));
	if(attribs & REFLECT_V) {
		glm::mat4 reflectMat = glm::mat4(1);
		reflectMat[1][1] = -1;
		texTransform = texTransform * reflectMat;
	}
	if(attribs & ROTATE_90)
		texTransform = glm::rotate(texTransform, glm::radians(90.f), glm::vec3(0, 0, 1));
	
	texTransform = glm::scale(texTransform, glm::vec3(tex_basis[1] - tex_basis[0], 0));
	texTransform = glm::translate(texTransform, glm::vec3(tex_basis[0], 0));


	// Construct vertexes with the same positions, and texcoords from a scale of 0 to 1
	// depending on where the vertexes are relative to the polygon
	for(auto it = v_positions.begin(); it != v_positions.end(); it++) {
		float x = *it, y = *(++it);		

		// The texture coordinate for this vertex
		glm::vec3 texCoord = glm::vec3(
			(x-x_min)/(x_max - x_min),
			(y-y_min)/(y_max - y_min),
			0
		);
		glm::mat4 tCoordMat = glm::translate(glm::mat4(1), texCoord); // Translate to this point
		glm::mat4 offsetMat = glm::translate(glm::mat4(1), glm::vec3(.5, .5, 0)); // Translate so attrib operations are centered

		glm::mat4 basisMat = glm::translate(glm::mat4(1), glm::vec3(tex_basis[0], 0)); // Move from the original origin to the new origin (lower left corner of basis) 
		basisMat = glm::scale(basisMat, glm::vec3(tex_basis[1] - tex_basis[0], 0)); // Scale to the texBasis' size 

		glm::mat4 attribMat = glm::mat4(1);
		// Check attributes and perform their operations
		if(attribs & ROTATE_90)
			attribMat = glm::rotate(attribMat, glm::radians(90.f), glm::vec3(0, 0, 1));
		if(attribs & ROTATE_180)
			attribMat = glm::rotate(attribMat, glm::radians(180.f), glm::vec3(0, 0, 1));
		if(attribs & REFLECT_V)
			attribMat[1][1] *= -1;
		if(attribs & REFLECT_H)
			attribMat[0][0] *= -1;

		attribMat = offsetMat * attribMat * glm::inverse(offsetMat);
		tCoordMat = basisMat * attribMat * tCoordMat;
		
		vertices.push_back(
			Vertex2d {
				pos: glm::vec2(x, y),
				layer: 0,
				tex: glm::vec2(tCoordMat[3])
			}
		);
	}

	return vertices;
}

const std::vector<Vertex2d> joinCoords(std::vector<float> pos, std::vector<float> tex, float layer) {
	std::vector<Vertex2d> result;
	if(pos.size() != tex.size()) {
		return result;
	}
	for(unsigned i=0; i<pos.size(); i+=2) {
		result.push_back(Vertex2d {
			pos: glm::vec2(pos[i], pos[i+1]),
			layer: layer,
			tex: glm::vec2(tex[i], pos[i+1])
		});
	}

	return result;
}

const std::vector<Vertex2d> joinCoords(std::vector<glm::vec2> pos, std::vector<glm::vec2> tex, float layer) {
	std::vector<Vertex2d> result;
	if(pos.size() != tex.size()) {
		return result;
	}
	for(unsigned i=0; i<pos.size(); i++) {
		result.push_back(Vertex2d {
			pos: pos[i],
			layer: layer,
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

const std::vector<Vertex2d>& rebaseTexCoords(std::vector<Vertex2d>& verts, std::vector<glm::vec2> new_basis) {
	for(Vertex2d& v : verts) {
		v.tex *= (new_basis[1] - new_basis[0]);
		v.tex += new_basis[0];
	}
	return verts;
}

const Polygon Primitive::rect(
	glm::vec2 size,
	glm::vec2 offset,
	unsigned int attribs,
	unsigned index_offset,
	std::vector<glm::vec2> tex_basis
){
	Polygon p;

	p.vertices = genTexCoords2d({
		offset.x + -size.x / 2, offset.y + -size.y / 2,
		offset.x + size.x / 2,  offset.y + -size.y / 2,
		offset.x + size.x / 2,  offset.y + size.y / 2,
		offset.x + -size.x / 2, offset.y + size.y / 2
	}, tex_basis, attribs);

	p.indices = {
		index_offset + 0, index_offset + 1, index_offset + 2,
		index_offset + 0, index_offset + 2, index_offset + 3	
	};

	return p;
}
