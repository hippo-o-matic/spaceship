#include "collider.h"

std::vector<Collider*> Collider::colliders;
std::vector<Rigidbody2d*> Rigidbody2d::bodies; // TODO: BIG TEMPORARY

Collider::Collider(std::string id) : Object2d(id) {
	colliders.push_back(this);
}

Collider::~Collider() {
	colliders.erase(std::find(colliders.begin(), colliders.end(), this));
}

Object2d& MeshCollider::setPos(glm::vec2 position) {
	Object2d::setPos(position);
	bounding_box.position = getWorldPos();

	return *this;
}

Object2d& MeshCollider::setRot(float angle) {
	Object2d::setRot(angle);

	if(angle != getRot()) { // Prevent uneccesary updates
		glm::mat4 r = glm::rotate(glm::mat4(1), glm::radians(getWorldRot()), glm::vec3(0, 0, 1));
		std::vector<glm::vec2> copy_vertices(vertices);
		// Rotate each vertex to get new mins and maxes
		for(glm::vec2 vert : copy_vertices) {
			vert = r * glm::vec4(vert, 0, 1);
		}

		bounding_box.setBounds(copy_vertices);
	}

	return *this;
}

Object2d& MeshCollider::setScl(glm::vec2 scale) {
	Object2d::setScl(scale);

	bounding_box.lower_left *= getWorldScl();
	bounding_box.upper_right *= getWorldScl();

	return *this;
}

Line& debug_rvec() {
	static Line l(glm::vec2(0), glm::vec2(0), glm::vec3(1, 0, 0));
	return l;
}

void Collider::checkAll(float deltaTime) {
	for(auto it = colliders.begin(); it != colliders.end(); it++) { // For every collider
		Collider& c1 = **it;

		for(auto jt = std::next(it); jt != colliders.end(); jt++) { // Compare with every other collider after this one
			Collider& c2 = **jt;
			c2.getWorldPos();
			// if(c1.bounding_box.intersects(c2.bounding_box)) { // If their bounding boxes intersect
				Simplex simplex;
				bool colliding = checkCollision(c1, c2, &simplex);
				if(colliding) {
					glm::vec2 resolve = resolutionVector(c1, c2, simplex.copyToVec());
					debug_rvec().startPoint = c1.getWorldPos();
					debug_rvec().endPoint = c1.getWorldPos() + resolve;
					
					Rigidbody2d *b1 = nullptr, *b2 = nullptr;

					// Check to see if this collider belongs to a rigidbody. If it does, apply appropriate force to the object
					try {
						b1 = c1.parent->as<Rigidbody2d>();
					} catch(ObjectCastException&) {}

					try {
						b2 = c2.parent->as<Rigidbody2d>();
					} catch(ObjectCastException&) {}

					if(b1 != nullptr && b2 != nullptr) {
						b1->displace(resolve / 2.f);
						b2->displace(-resolve / 2.f);
						Rigidbody2d::collide(resolve, b1, b2, 0);
					} else if(b1 != nullptr) {
						b1->displace(resolve);
						b1->applyForce(b1->getMass() * -b1->velocity * c1.elasticity, resolve); // this is wrong, -velocity will send a diagonally approaching object back the way it came
					} else if(b2 != nullptr) {
						b2->displace(-resolve);
						b2->applyForce(b2->getMass() * -b2->velocity * c2.elasticity, resolve); // this is wrong, -velocity will send a diagonally approaching object back the way it came
					}

				}
			// }
		}
	}
}


// Returns the vertex on the Minkowski difference of these two colliders
glm::vec2 Collider::getSupport(Collider& a, Collider& b, glm::vec2 direction) {
	return a.furthestPoint(direction) - b.furthestPoint(-direction);
}

const glm::vec2 Collider::perpendicularCW(glm::vec2 vec) {	
	return glm::vec2(vec.y, -vec.x);
}

const glm::vec2 Collider::perpendicularCCW(glm::vec2 vec) {
	return glm::vec2(-vec.y, vec.x);
}

// GJK
bool Collider::checkCollision(Collider& colliderA, Collider& colliderB, Simplex* resultSimplex) {
	// Get initial support point in any direction
	glm::vec2 support = getSupport(colliderA, colliderB, glm::vec2(1, 0));

	// Simplex is an array of points, max count is 4
	Simplex simplex;
	simplex.push_front(support);

	// New direction is towards the origin
	glm::vec2 direction = -support;

	while (true) {
		support = getSupport(colliderA, colliderB, direction);
 
		if (glm::dot(support, direction) <= 0) {
			return false; // no collision
		}

		simplex.push_front(support);

		if (nextSimplex(simplex, direction)) {
			*resultSimplex = simplex;
			return true;
		}
	}
}

bool Collider::nextSimplex(Simplex& points, glm::vec2& direction) {
	switch (points.size()) {
		case 2: return lineCheck(points, direction);
		case 3: return triangleCheck(points, direction);
		// In 3d, add tetrahedron case
	}
 
	// never should be here
	return false;
}

bool Collider::sameDirection(const glm::vec2& direction, const glm::vec2& ao) {
	return glm::dot(direction, ao) > 0;
}

bool Collider::lineCheck(Simplex& points, glm::vec2& direction) {
	glm::vec2 a = points[0];
	glm::vec2 b = points[1];

	glm::vec2 ab = b - a;
	glm::vec2 ao = -a;
 
	if (sameDirection(ab, ao)) {
		direction = ao; // In 3d this would need to be glm::cross(glm::cross(ab, ao), ab);
	}

	else {
		points = { a };
		direction = ao;
	}

	return false;
}

bool Collider::triangleCheck(Simplex& points, glm::vec2& direction) {
	glm::vec2 a = points[0];
	glm::vec2 b = points[1];
	glm::vec2 c = points[2];

	glm::vec2 ab = b - a;
	glm::vec2 ac = c - a;
	glm::vec2 ao = -a;
 
	// Add this vector in 3d: glm::vec3 abc = ab.cross(ac);
 
	if (sameDirection(perpendicularCCW(ac), ao)) { // In 3d, the first term is glm::cross(abc, ac)
		if (sameDirection(ac, ao)) {
			points = { a, c };
			direction = ao; // In 3d, this would be glm::cross(glm::cross(ac, ao), ac);
		} else {
			return lineCheck(points = { a, b }, direction);
		}
	} else {
		if (sameDirection(perpendicularCW(ab), ao)) { // In 3d, the first term here is glm::cross(ab, abc)
			return lineCheck(points = { a, b }, direction);
		}

		// In 3d, this checks if the point is above or below the triangle
		// else {
		// 	if (sameDirection(abc, ao)) {
		// 		direction = abc;
		// 	}

		// 	else {
		// 		points = { a, c, b };
		// 		direction = -abc;
		// 	}
		// }
	}

	return true;
}

// EPA
glm::vec2 Collider::resolutionVector(Collider& colliderA, Collider& colliderB, std::vector<glm::vec2> polytope) {
	auto minIndex = polytope.begin();
	float minDistance = std::numeric_limits<float>::max();
	glm::vec2 minNormal;

	while (minDistance == std::numeric_limits<float>::max()) {
		for (auto it = polytope.begin(); it != polytope.end(); it++) {
			auto jt = std::next(it); // Iterator to next element
			if(jt == polytope.end()) // Loop around if we reach the end
				jt = polytope.begin();

			glm::vec2 vertexI = *it;
			glm::vec2 vertexJ = *jt;

			glm::vec2 vecIJ = vertexJ - vertexI;

			glm::vec2 normal = glm::normalize(glm::vec2(vecIJ.y, -vecIJ.x));
			float distance = glm::dot(normal, vertexI);

			if (distance < 0) {
				distance *= -1;
				normal *= -1;
			}

			if (distance < minDistance) {
				minDistance = distance;
				minNormal = normal;
				minIndex = jt;
			}
		}

		glm::vec2 support = getSupport(colliderA, colliderB, minNormal);
		float sDistance = glm::dot(minNormal, support);

		if(abs(sDistance - minDistance) > 0.001) {
		 	minDistance = std::numeric_limits<float>::max();
			polytope.insert(minIndex, support);
		}
	}

	return minNormal * (minDistance + 0.001f);
}


// MeshCollider ////////////////////////////////////////////////////////

MeshCollider::MeshCollider(std::string id, std::vector<glm::vec2> points) :
	Collider(id),
	vertices(points)	
{
	bounding_box.setBounds(points);
}