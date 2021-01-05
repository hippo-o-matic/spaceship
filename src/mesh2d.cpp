#include "mesh2d.h"

const std::vector<Vertex2d> genTexCoords2d(std::vector<float> v_positions, std::vector<glm::vec2> tex_basis) {
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

	// Construct vertexes with the same positions, and texcoords from a scale of 0 to 1
	// depending on where the vertexes are relative to the polygon
	for(auto it = v_positions.begin(); it != v_positions.end(); it++) {
		float x = *it, y = *(++it);
		vertices.push_back(
			Vertex2d {
				pos: glm::vec2(x, y),
				layer: 0,
				tex: glm::vec2(
						(x-x_min)/(x_max - x_min) * (tex_basis[1].x - tex_basis[0].x),
						(y-y_min)/(y_max - y_min) * (tex_basis[1].y - tex_basis[0].y)
					)
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

const Polygon Primitive::rect(float width, float height, float offsetX, float offsetY, unsigned index_offset, std::vector<glm::vec2> tex_basis) {
	Polygon p;
	p.vertices = genTexCoords2d({
		offsetX + -width / 2, offsetY + -height / 2,
		offsetX + width / 2,  offsetY + -height / 2,
		offsetX + width / 2,  offsetY + height / 2,
		offsetX + -width / 2, offsetY + height / 2
	}, tex_basis);

	p.indices = {
		index_offset + 0, index_offset + 1, index_offset + 2,
		index_offset + 0, index_offset + 2, index_offset + 3	
	};

	return p;
}
