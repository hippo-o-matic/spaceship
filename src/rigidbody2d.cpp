#include "rigidbody2d.h"

Rigidbody::Rigidbody2d(std::string id, float mass = 1) : Object(id) {
	setMass(mass);
	bodies.push_back(this);
}

Rigidbody::~Rigidbody2d() {
	bodies.erase(std::find(bodies.begin(), bodies.end(), this));
}

float Rigidbody::getMass() const {
	return mass;
}

glm::vec2 Rigidbody::getNetForce() const {
	return net_force;
}

float Rigidbody::setMass(float mass) {
	this->mass = mass;
	// set MOI
	return mass;
}

void Rigidbody::teleport_w(glm::vec2 position) {
	try{
		Object2d* p = parent->as<Object2d>();
		p->setWorldPos(position);
	} catch(ObjectCastException&) {}
}

void Rigidbody::displace_w(glm::vec2 offset) {
	try{
		Object2d* p = parent->as<Object2d>();
		p->setWorldPos(p->getWorldPos() + offset);
	} catch(ObjectCastException&) {}
}

void Rigidbody::displace(glm::vec2 offset) {
	try{
		Object2d* p = parent->as<Object2d>();
		p->setPos(p->getPos() + offset);
	} catch(ObjectCastException&) {}
}

void Rigidbody::applyForce(glm::vec2 force, glm::vec2 pos) {
	float angle = 0;
	if(pos != glm::vec2(0))
		angle = glm::radians(glm::angle(glm::normalize(force), glm::normalize(pos)));
	net_torque += glm::distance(glm::vec2(0), force) * glm::distance(glm::vec2(0), pos) * (float)sin(angle);
	net_force += force * (float)cos(angle);
}

void Rigidbody::applyTorque(float torque) {
	net_torque += torque;
}

// no way this works
void Rigidbody::collide(glm::vec2 resolution_vec, Rigidbody2d* b1, Rigidbody2d* b2, float deltaTime, float elastic) {
	float total_mass = b1->mass + b2->mass;
	glm::vec2 wtf = 2.f * b1->mass * b2->mass * (b2->velocity - b1->velocity) / total_mass; // divide by deltaTime??
	// glm::vec2 this_accel((velocity / 2.f) * (elastic ? -1.f : 1.f));
	b1->applyForce(wtf, resolution_vec);
	b2->applyForce(-wtf, -resolution_vec);
}

void Rigidbody::update(float deltaTime) {
	velocity += net_force / mass * deltaTime;
	angular_velocity += net_torque / moment_of_inertia * deltaTime;

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

void Rigidbody::updateAll(float deltaTime) {
	for(auto body : bodies) {
		body->update(deltaTime);
	}
}