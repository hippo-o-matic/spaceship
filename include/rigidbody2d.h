#pragma once

#include "object2d.h"
#include "collider.h"

#include "glm/glm.hpp"
#include "glm/gtx/vector_angle.hpp"

#include <cmath>

class Rigidbody2d : public Object {
public:
	Rigidbody2d(std::string id, std::unique_ptr<Collider> collider, float mass = 1);
	Rigidbody2d(std::string id, std::vector<glm::vec2> mesh, float mass = 1);
	~Rigidbody2d();

	std::unique_ptr<Collider> collider;
	glm::vec2 velocity = glm::vec2(0);
	float angular_velocity = 0;

	float getMass() const;
	glm::vec2 getNetForce() const;

	float setMass(float mass);
	
	void teleport_w(glm::vec2 position);
	void displace_w(glm::vec2 offset);
	void displace(glm::vec2 offset);
	
	void applyForce(glm::vec2 force, glm::vec2 pos = glm::vec2(0));
	void applyTorque(float torque);

	static void collide(glm::vec2 resolution_vec, Rigidbody2d* b1, Rigidbody2d* b2, float deltaTime, float elastic = 0);

	void update(float deltaTime);
	static void updateAll(float deltaTime);

private:
	float mass;
	float moment_of_inertia;
	glm::vec2 net_force = glm::vec2(0);
	float net_torque = 0;

	static std::vector<Rigidbody2d*> bodies;
};