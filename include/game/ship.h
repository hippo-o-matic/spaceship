#pragma once

#include "object2d.h"
#include "sprite.h"
#include "input.h"

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
	float angular_thrust;
	glm::vec2 thrust;

	float angular_velocity;
	glm::vec2 velocity;

	glm::vec2 drift = glm::vec2(0);

	Ship(std::string id, std::string class_name);
	~Ship();

	void update(float deltaTime);
	static void updateShips(float deltaTime);

	static std::vector<ShipClass> ship_classes;
	const float THRUST_MAX = 3;
	const float ROT_THRUST_MAX = 3;
	const float VELOCITY_MAX = 25;
	const float ROT_VELOCITY_MAX = 300;

	ShipClass ship_class;

private:
	static std::vector<Ship*> ships;
};