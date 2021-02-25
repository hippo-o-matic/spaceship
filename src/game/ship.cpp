#include "game/ship.h"

std::vector<ShipClass> Ship::ship_classes;
std::vector<Ship*> Ship::ships;

Ship::Ship(std::string id, std::string class_name) : Object2d(id) {
	auto ship_class_it = std::find_if(ship_classes.begin(), ship_classes.end(), [&class_name](ShipClass& c){
		return c.name == class_name;
	});

	ship_class = *ship_class_it;
	ships.push_back(this);

	take(newObj<Sprite>("sprite", ship_class.sprite_path));

	velocity = glm::vec2(0);
	angular_velocity = 0;
	thrust = glm::vec2(0);
}

Ship::~Ship() {
	ships.erase(std::find(ships.begin(), ships.end(), this));
}

glm::vec2 safeNormal(glm::vec2 in) {
	if(in == glm::vec2(0))
		return in;
	return glm::normalize(in);
}

void Ship::update(float deltaTime) {
	glm::vec2 accel_thrust = (thrust * ship_class.thrust_power) / ship_class.mass;
	glm::vec2 accel_damping = (velocity - drift) / (2.f * ship_class.mass);
	float accel_ang = (angular_thrust * ship_class.turn_power) / ship_class.mass;
	if(velocity != glm::vec2(0)) {
		// velocity += (2.f*thrust - (velocity / (ship_class.thrust_power * ship_class.mass))) * ship_class.thrust_power / ship_class.mass;
		velocity += 2.f * accel_thrust - accel_damping; 

	} else {
		velocity += accel_thrust;
	}

	// velocity += 2.f * accel - velocity * ship_class.mass / ship_class.thrust_power;

	if(abs(velocity.x) < 0.01)
		velocity.x = 0;
	if(abs(velocity.y) < 0.01)
		velocity.y = 0;
	
	angular_velocity += 2.f * accel_ang - angular_velocity / (2.f * ship_class.mass);;

	if(abs(angular_velocity) < 0.5)
		angular_velocity = 0;

	if(velocity.x > VELOCITY_MAX)
		velocity.x = VELOCITY_MAX;
	if(velocity.x < -VELOCITY_MAX)
		velocity.x = -VELOCITY_MAX;
		
	if(velocity.y > VELOCITY_MAX)
		velocity.y = VELOCITY_MAX;
	if(velocity.y < -VELOCITY_MAX)
		velocity.y = -VELOCITY_MAX;
		
	if(angular_velocity > ROT_VELOCITY_MAX)
		angular_velocity = ROT_VELOCITY_MAX;
	if(angular_velocity < -ROT_VELOCITY_MAX)
		angular_velocity = -ROT_VELOCITY_MAX;


	setPos(getPos() + velocity * deltaTime);
	setRot(getRot() + angular_velocity * deltaTime);

	angular_thrust = 0;
	thrust = glm::vec2(0);
}

void Ship::updateShips(float deltaTime) {
	for(auto ship : ships) {
		ship->update(deltaTime);
	}
}