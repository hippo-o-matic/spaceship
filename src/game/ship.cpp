#include "game/ship.h"

std::vector<ShipClass> Ship::ship_classes;
std::vector<Ship*> Ship::ships;

Ship::Ship(std::string id, std::string class_name) : 
	Object2d(id)
{
	auto ship_class_it = std::find_if(ship_classes.begin(), ship_classes.end(), [&class_name](ShipClass& c){
		return c.name == class_name;
	});

	ship_class = *ship_class_it;
	ships.push_back(this);

	take(newObj<Sprite>("sprite", ship_class.sprite_path));

	const std::vector<glm::vec2>& sprite_mesh = extractStructField(
		get<Sprite>("sprite").mesh.vertices, 
		&Vertex2d::pos
	);
	auto collider = std::make_unique<MeshCollider>("collider", sprite_mesh);
	take(newObj<Rigidbody2d>("rigidbody", sprite_mesh, ship_class.mass));
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
	Rigidbody2d& body = get<Rigidbody2d>("rigidbody");
	if(body.velocity != glm::vec2(0))
		body.applyForce(-body.velocity * 10.f);
	
	// yummy branches
	int rotation_sign;
	if(body.angular_velocity == 0)
		rotation_sign = 0;
	else if(body.angular_velocity > 0)
		rotation_sign = -1;
	else
		rotation_sign = 1;

	body.applyTorque(-body.angular_velocity * 2);

	if(body.velocity.x > VELOCITY_MAX)
		body.velocity.x = VELOCITY_MAX;
	if(body.velocity.x < -VELOCITY_MAX)
		body.velocity.x = -VELOCITY_MAX;
		
	if(body.velocity.y > VELOCITY_MAX)
		body.velocity.y = VELOCITY_MAX;
	if(body.velocity.y < -VELOCITY_MAX)
		body.velocity.y = -VELOCITY_MAX;
		
	if(body.angular_velocity > ROT_VELOCITY_MAX)
		body.angular_velocity = ROT_VELOCITY_MAX;
	if(body.angular_velocity < -ROT_VELOCITY_MAX)
		body.angular_velocity = -ROT_VELOCITY_MAX;
}

void Ship::updateShips(float deltaTime) {
	for(auto ship : ships) {
		ship->update(deltaTime);
	}
}