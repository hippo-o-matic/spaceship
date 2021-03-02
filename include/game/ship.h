#pragma once

#include "object2d.h"
#include "sprite.h"
#include "input.h"
#include "rigidbody2d.h"
#include "collider.h"

#include "glm/glm.hpp"

#include <vector>

struct ShipClass {
	std::string name;
	std::string sprite_path;
	float mass;
	float thrust_power;
	float turn_power;
	unsigned weapon_slots;
};

class Ship : public Object2d {
public:
	Ship(std::string id, std::string class_name);
	~Ship();

	glm::vec2 drift = glm::vec2(0);
	ShipClass ship_class;


	void update(float deltaTime);

	static void updateShips(float deltaTime);

	static std::vector<ShipClass> ship_classes;

private:
	const float THRUST_MAX = 3;
	const float ROT_THRUST_MAX = 3;
	const float VELOCITY_MAX = 25;
	const float ROT_VELOCITY_MAX = 300;
	
	static std::vector<Ship*> ships;
};

class PlayerShip : public Ship {
public:
	PlayerShip() : Ship("player", "testclass") {
		init_control();
		control.activate();
	}

	Input control;

	void init_control() {
		control.addBind("down", 
			[this](){ get<Rigidbody2d>("rigidbody").applyForce(-this->up() * ship_class.thrust_power); },
			GLFW_KEY_S
		);
		control.addBind("up", 
			[this](){ get<Rigidbody2d>("rigidbody").applyForce(this->up() * ship_class.thrust_power); },
			GLFW_KEY_W
		);
		control.addBind("left", 
			[this](){ get<Rigidbody2d>("rigidbody").applyTorque(ship_class.turn_power); },
			GLFW_KEY_A
		);
		control.addBind("right", 
			[this](){ get<Rigidbody2d>("rigidbody").applyTorque(-ship_class.turn_power); },
			GLFW_KEY_D
		);
		control.addBind("boost", 
			[this](){ ship_class.thrust_power += 3; },
			GLFW_KEY_LEFT_SHIFT, INPUT_ONCE
		);
		control.addBind("unboost", 
			[this](){ ship_class.thrust_power -= 3; },
			GLFW_KEY_LEFT_SHIFT, INPUT_ONCE_RELEASE
		);
	}
};