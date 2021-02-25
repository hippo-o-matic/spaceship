#pragma once

#include "object2d.h"

#include "glm/glm.hpp"
#include "glm/gtx/vector_angle.hpp"

class Rigidbody2d : public Object {
public:
	Rigidbody2d(std::string id, float mass = 1) : Object(id) {
		setMass(mass);
		bodies.push_back(this);
	};

	~Rigidbody2d() {
		bodies.erase(std::find(bodies.begin(), bodies.end(), this));
	}

	glm::vec2 velocity = glm::vec2(0);
	float angular_velocity = 0;

	float getMass() const {
		return mass;
	}

	glm::vec2 getNetForce() const {
		return net_force;
	}

	float setMass(float mass) {
		this->mass = mass;
		// set MOI
		return mass;
	}

	void teleport_w(glm::vec2 position) {
		try{
			Object2d* p = parent->as<Object2d>();
			p->setWorldPos(position);
		} catch(ObjectCastException&) {}
	}

	void displace_w(glm::vec2 offset) {
		try{
			Object2d* p = parent->as<Object2d>();
			p->setWorldPos(p->getWorldPos() + offset);
		} catch(ObjectCastException&) {}
	}

	void displace(glm::vec2 offset) {
		try{
			Object2d* p = parent->as<Object2d>();
			p->setPos(p->getPos() + offset);
		} catch(ObjectCastException&) {}
	}

	void applyForce(glm::vec2 force, glm::vec2 pos = glm::vec2(0)) {
		float angle = 0;
		if(pos != glm::vec2(0))
			angle = glm::radians(glm::angle(glm::normalize(force), glm::normalize(pos)));
		net_torque += glm::distance(glm::vec2(0), force) * glm::distance(glm::vec2(0), pos) * (float)sin(angle);
		net_force += force * (float)cos(angle);
	}

	// no way this works
	static void collide(glm::vec2 resolution_vec, Rigidbody2d* b1, Rigidbody2d* b2, float deltaTime, float elastic = 0) {
		float total_mass = b1->mass + b2->mass;
		glm::vec2 wtf = 2.f * b1->mass * b2->mass * (b2->velocity - b1->velocity) / total_mass; // divide by deltaTime??
		// glm::vec2 this_accel((velocity / 2.f) * (elastic ? -1.f : 1.f));
		b1->applyForce(wtf, resolution_vec);
		b2->applyForce(-wtf, -resolution_vec);
	}

	void update(float deltaTime) {
		velocity += net_force / mass * deltaTime;
		// angular_velocity += net_torque / moment_of_inertia * deltaTime;

		if(abs(velocity.x) < 0.01)
			velocity.x = 0;
		if(abs(velocity.y) < 0.01)
			velocity.y = 0;
		
		try{
			Object2d* p = parent->as<Object2d>();
			p->setPos(p->getPos() + velocity * deltaTime);
			p->setRot(p->getRot() + angular_velocity * deltaTime);
		} catch(ObjectCastException&) {
			// Set an error state or something, as this rigidbody doesn't have an object to update
		}

		// Reset net forces
		net_force = glm::vec2(0);
		net_torque = 0;
	}

	static void updateAll(float deltaTime) {
		for(auto body : bodies) {
			body->update(deltaTime);
		}
	}

private:
	float mass;
	float moment_of_inertia;
	glm::vec2 net_force = glm::vec2(0);
	float net_torque = 0;

	static std::vector<Rigidbody2d*> bodies;
};