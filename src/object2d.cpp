#include "object2d.h"

Object2d::Object2d(std::string _id, glm::vec2 _pos, float _rot, glm::vec2 _scl) : 
Object(_id), position(_pos), rotation(_rot), scale(_scl) {}

// Object2d::Object2d(Object2d* _parent, glm::vec2 _pos, float _rot, glm::vec2 _scl) : 
// position(_pos), rotation(_rot), scale(_scl) {
// 	parent = _parent;
// 	parent.move(std::make_unique)
// }

Object2d::Object2d(Json::Value j) : Object(j) {
	Json::Value jPos = j["pos"];
	Json::Value jScl = j["scl"];
	Json::ArrayIndex x=0, y=1; // Gee thanks implicit conversion of int to const char*, now we have to explicitly specify the type here

	position.x = jPos.get(x, default_pos.x).asFloat();
	position.y = jPos.get(y, default_pos.y).asFloat();

	rotation = j["rot"].asFloat();

	scale.x = jScl.get(x, default_scl.x).asFloat();
	scale.y = jScl.get(y, default_scl.y).asFloat();

	obj_layer = j["obj_layer"].asInt();
}

Object2d::~Object2d() {}


glm::vec2 Object2d::getWorldPos() {
	if(!prevent_inherit_pos)
		return glm::vec2(getWorldTransform()[3]);
	else
		return position;
}

glm::vec2 Object2d::setWorldPos(glm::vec2 dest) {
	return position = dest - getWorldPos();
}


float Object2d::getWorldRot() {
	if(!prevent_inherit_rot)
		return degreeFromMat4(getWorldTransform());
	else
		return rotation;
}

float Object2d::setWorldRot(float dest) {
	return rotate(dest - getWorldRot());  
}


float Object2d::setRot(float dest) {
	if(dest >= 360)
		dest = fmod(dest, 360);

	return rotation = dest;
}

float Object2d::rotate(float in) {
	rotation += in;
	if(rotation >= 360)
		rotation = fmod(rotation, 360);

	return rotation;
}

glm::vec2 Object2d::getWorldScl() {
	if(!prevent_inherit_scl) {
		glm::mat4 transform = getWorldTransform();
		return glm::vec2(
			glm::length(transform[0]),
			glm::length(transform[1])
		);
	} else
		return scale;
}

glm::vec2 Object2d::setWorldScl(glm::vec2 dest) {
	return scale = dest / getWorldScl();
}

float degreeFromMat4(glm::mat4 in) {
	// We need to first compute the scale components
	float s_x = glm::length(in[0]);
	float s_y = glm::length(in[1]);

	// The trace is the sum of diagonal elements of the rotation matrix
	// Technically it would include a +1, but we subtract 1 later anyway
	float trace = (in[0][0] / s_x) + (in[1][1] / s_y);
	
	// This should give us the counterclockwise angle of the object
	// Theta = arccos((Tr(R) - 1) / 2)
	return glm::degrees(acos(trace / 2));
}

glm::mat4 rotationMat4(float degree) {
	glm::mat4 rot = glm::mat4(1);
	rot[0][0] = cos(glm::radians(degree));
	rot[1][1] = cos(glm::radians(degree));
	rot[0][1] = sin(glm::radians(degree));
	rot[1][0] = -sin(glm::radians(degree));

	return rot;
}

// Returns an objects local transformation matrix
glm::mat4 Object2d::getTransform() {
	// Get matricies for each transformation
	glm::mat4 translate_mat = glm::translate(glm::mat4(1), glm::vec3(position, obj_layer));
	glm::mat4 rotate_mat = rotationMat4(rotation); 

	// Check that scale is nonzero
	if(scale.x == 0)
		scale.x = 1;
	if(scale.y == 0)
		scale.y = 1;
	
	glm::mat4 scale_mat = glm::scale(glm::mat4(1), glm::vec3(scale, 0));

	// Combine them all here
	glm::mat4 transform = translate_mat * rotate_mat * scale_mat;

	return transform;
}

// Applies parent transformation matricies if any
glm::mat4 Object2d::getWorldTransform() {
	glm::mat4 transform = getTransform();

	if(parent) {
		// This function checks if a parent has a transform, and applies it if it does
		std::function<void(Object*, glm::mat4&)> parentTransform = [](Object* _parent, glm::mat4& _transform) {
			try { // Try to convert the parent to Object2d
				Object2d* ptr_2d = _parent->as<Object2d>();
				_transform = ptr_2d->getTransform() * _transform; // Apply the transformation if it worked
			} catch(ObjectCastException &e) {
				return; // If it failed, continue to the next parent
			} 
		};

		// Now that the function is defined, run it for each parent in the hierarchy
		parent->runup<glm::mat4&>(parentTransform, transform);
		// The reason we use runup instead of regular recursion is that in the case
		// one of the parents in the path upwards isn't Object2d, the recursion would stop
	}

	return transform;
}

glm::mat4 Object2d::setTransform(glm::mat4 in) {
	position = glm::vec2(in[0][3], in[1][3]);

	scale = glm::vec2(
		glm::length(in[0]),
		glm::length(in[1])
	);

	rotation = degreeFromMat4(in);

	return getTransform(); // TODO: For testing mostly, should return the same thing as `in`
}

glm::mat4 Object2d::setWorldTransform(glm::mat4 in) {
	glm::mat4 current = getWorldTransform();
	return setTransform(in * glm::inverse(current) * current); // Multiply the current transform by itself, effectively setting it to 0, then multiply the new transform
}

glm::mat4 Object2d::transformBy(glm::mat4 in) {
	return setTransform(in * getTransform());
}


glm::vec2 Object2d::up() {
	return glm::vec2(
		cos(glm::radians(rotation + 90.f)), 
		sin(glm::radians(rotation + 90.f))
	);
}

glm::vec2 Object2d::right() {
	return glm::vec2(
		cos(glm::radians(rotation)), 
		sin(glm::radians(rotation))
	);
}
