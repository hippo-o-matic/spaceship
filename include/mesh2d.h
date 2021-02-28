#pragma once

#include "logs.h"
#include "utility.h"
#include "render.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <algorithm>

struct Vertex2d {
	glm::vec2 pos; // Position
	glm::vec2 tex; // Texture coordinates
};

class Line : Renderable {
public:
	Line();
    Line(glm::vec2 start, glm::vec2 end, glm::vec3 color = glm::vec3(255));
    ~Line();

    void setColor(glm::vec3 color);
	void setPoints(glm::vec2 start, glm::vec2 end);
	glm::vec2 getStart() { return startPoint; }
	glm::vec2 getEnd() { return endPoint; }

    void draw(Shader& shader) override;

	static Shader* lineShader();

protected:
    std::vector<glm::vec2> vertices;
   	glm::vec2 startPoint;
    glm::vec2 endPoint;
    glm::vec3 lineColor;

	void update();

private:
	int shaderProgram;
    unsigned int VBO, VAO;
};

class Path : Line {
public:
	Path(glm::vec3 color);

	void push_point(glm::vec2 point);
	void setVerts(std::vector<glm::vec2> vertices);
	void setVertsLoop(std::vector<glm::vec2> vertices);
};

class Polygon {
public:
	Polygon(std::vector<float> vertices, std::vector<unsigned> indices);
	Polygon(std::vector<glm::vec2> vertices, std::vector<unsigned> indices);
	Polygon(std::vector<Vertex2d> vertices, std::vector<unsigned> indices);
	Polygon(std::vector<glm::vec2> vertices, std::vector<glm::vec2> tex_coords, std::vector<unsigned> indices);
	Polygon(Polygon&);

	std::vector<Vertex2d> vertices;
	std::vector<unsigned> indices;

	Polygon& buildFromFloats(const std::vector<float>&);

	Polygon& setOptions(unsigned options);
	Polygon& setBasis(glm::vec2 lower_left, glm::vec2 upper_right);
	Polygon& mergeBasis(glm::vec2 lower_left, glm::vec2 upper_right);
	Polygon& unmergeBasis(glm::vec2 lower_left, glm::vec2 upper_right);
	Polygon& setIndexOffset(unsigned offset);
	Polygon& regenTexCoords();

	// Polygon& genTexCoords();
	// Polygon& applyTexCoords(std::vector<glm::vec2>);
	// Polygon& applyVertexPos(std::vector<glm::vec2>);

	static void getPositionMinMax(const std::vector<float>& positions, float* x_min, float* x_max, float* y_min, float* y_max);
	static void getPositionMinMax(const std::vector<glm::vec2>& positions, float* x_min, float* x_max, float* y_min, float* y_max);


private:
	glm::mat4 basis_transform = glm::mat4(1);
	glm::mat4 option_transform = glm::mat4(1);
};

enum PolygonOptions {
	NONE = 0x00,
	REFLECT_H = 0x01,
	REFLECT_V = 0x02,
	ROTATE_90 = 0x04,
	ROTATE_180 = 0x08
};

// const std::vector<Vertex2d>& rebaseTexCoords(std::vector<Vertex2d>& verts, std::vector<glm::vec2> new_basis);

namespace Primitive {
	Polygon rect(
		glm::vec2 size = glm::vec2(1),
		glm::vec2 offset = glm::vec2(0)
	);

}