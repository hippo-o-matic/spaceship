#pragma once

#include "logs.h"
#include "mesh2d.h"
#include "object2d.h"
#include "rigidbody.h"

#include "glm/glm.hpp"

#include <vector>
#include <limits>

// struct BoxNode {
// 	BoxNode *prev, *next;
// 	float left_x, right_x;
// 	float lower_y, upper_y;
// };

struct BoundingBox {
	BoundingBox() = default;
	BoundingBox(Polygon& poly) {
		setBounds(poly);
	}
	BoundingBox(glm::vec2 lower_left, glm::vec2 upper_right) :
		lower_left(lower_left),
		upper_right(upper_right)
	{}

	glm::vec2 position;
	glm::vec2 lower_left;
	glm::vec2 upper_right;

	void setBounds(std::vector<glm::vec2> points) {
		Polygon::getPositionMinMax(
			points, 
			&lower_left.x,
			&upper_right.x,
			&lower_left.y,
			&upper_right.y
		);
	}

	void setBounds(Polygon& poly) {
		Polygon::getPositionMinMax(
			extractStructField(poly.vertices, &Vertex2d::pos), 
			&lower_left.x,
			&upper_right.x,
			&lower_left.y,
			&upper_right.y
		);
	}

	// Check to see if 2 bounding boxes intersect
	bool intersects(const BoundingBox& b2) {
		const BoundingBox& b1 = *this;

		// glm::mat4 b1_transform = getWorldTransfrom();
		// std::vector<glm::vec2> b1_verts();
		// for(glm::vec2 vert : b1_verts) {
		// 	vert = glm::vec4(vert, 0, 0) * b1_transform;
		// }
		// b1.setBounds(b1_verts);
		

		if(b1.upper_right.x >= b2.lower_left.x &&
		   b2.upper_right.x >= b1.lower_left.x && // Intersection x axis
		   b1.upper_right.y >= b2.lower_left.y &&
		   b2.upper_right.y >= b1.lower_left.y)   // Intersection y axis
		{
			return true;
		} else {
			return false;
		}
	}

};

struct Simplex {
public:
	Simplex() : 
		m_points({glm::vec2(0), glm::vec2(0), glm::vec2(0)}),
		m_size(0)
	{}

	Simplex& operator=(std::initializer_list<glm::vec2> list) {
		for (auto v = list.begin(); v != list.end(); v++) {
			m_points[std::distance(list.begin(), v)] = *v;
		}
		m_size = list.size();

		return *this;
	}

	void push_front(glm::vec2 point) {
		m_points = { point, m_points[0], m_points[1]};
		m_size = std::min(m_size + 1, 3u);
	}

	glm::vec2& operator[](unsigned i) { return m_points[i]; }
	unsigned size() const { return m_size; }

	auto begin() const { return m_points.begin(); }
	auto end() const { return m_points.end() - (3 - m_size); }

	std::vector<glm::vec2> copyToVec() {
		std::vector<glm::vec2> temp;
		std::copy(m_points.begin(), m_points.end(), std::back_inserter(temp));
		return temp; 
	}

private:
	std::array<glm::vec2, 3> m_points;
	unsigned m_size;
};


class Collider : public Object2d {
public:
	Collider(std::string id);
	~Collider();

	BoundingBox bounding_box;
	float elasticity = 0;

	virtual glm::vec2 furthestPoint(glm::vec2 direction) = 0;

	static bool checkCollision(Collider& colliderA, Collider& colliderB, Simplex* resultSimplex);
	static glm::vec2 resolutionVector(Collider& colliderA, Collider& colliderB, std::vector<glm::vec2> polytope);
	static void checkAll(float deltaTime);

private:
	static std::vector<Collider*> colliders;

	static glm::vec2 getSupport(Collider& a, Collider& b, glm::vec2 direction);
	static const glm::vec2 perpendicularCW(glm::vec2 vec);	
	static const glm::vec2 perpendicularCCW(glm::vec2 vec);
	static bool sameDirection(const glm::vec2& direction, const glm::vec2& ao);

	static bool nextSimplex(Simplex& points, glm::vec2& direction);
	static bool lineCheck(Simplex& points, glm::vec2& direction);
	static bool triangleCheck(Simplex& points, glm::vec2& direction);
};

struct CircleCollider : public Collider {
	glm::vec2 furthestPoint(glm::vec2 direction) override {
		return direction;
	}
};

struct MeshCollider : public Collider {
	MeshCollider(std::string id, std::vector<glm::vec2> points);

	Object2d& setPos(glm::vec2 position) override;
	Object2d& setRot(float angle) override;
	Object2d& setScl(glm::vec2 scale) override;

	glm::vec2 furthestPoint(glm::vec2 direction) override {
		glm::vec2 max_point;
		float max_dist = -std::numeric_limits<float>::max();

		for(auto vert : vertices) {
			vert = getWorldTransform() * glm::vec4(vert, 0, 1);
			float distance = glm::dot(vert, direction);
			if(distance > max_dist) {
				max_dist = distance;
				max_point = vert;
			}
		}

		return max_point;
	}

private:
	std::vector<glm::vec2> vertices;
};