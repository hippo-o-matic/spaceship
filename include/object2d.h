#pragma once

#include "object.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
// #include "glm/gtx/matrix_transform_2d.hpp"

class Object2d : public Object {
public:
	Object2d(
		std::string _id,
		glm::vec2 _pos = default_pos,
		float _rot = default_rot,
		glm::vec2 _scl = default_scl
	);
	// Object2d(
	// 	Object2d* _parent,
	// 	glm::vec2 _pos = default_pos,
	// 	float _rot = default_rot,
	// 	glm::vec2 _scl = default_scl
	// );

	Object2d(Json::Value j);

	~Object2d();

	glm::vec2 position;
	float rotation;
	glm::vec2 scale;
	int layer = 0;
	
	glm::vec2 getWorldPos();
	glm::vec2 setWorldPos(glm::vec2);

	float getWorldRot();
	float setWorldRot(float);
	
	float setRot(float);
	float rotate(float);

	glm::vec2 getWorldScl();
	glm::vec2 setWorldScl(glm::vec2);

	glm::mat4 getTransform();
	glm::mat4 getWorldTransform();
	glm::mat4 setTransform(glm::mat4);
	glm::mat4 setWorldTransform(glm::mat4);
	glm::mat4 transformBy(glm::mat4);
	// glm::mat4 transformBy(glm::mat4);
	// glm::mat4 transformBy(glm::vec2, float, glm::vec2);
	// glm::mat4 transformBy(glm::vec2, glm::vec2, glm::vec2);
	// glm::mat4 setTransform(glm::mat4);

	// Directional vectors
	// glm::vec2 forward;
	// glm::vec2 right;

	// // TODO: not a fan of having to call a function as values can become outdated, maybe use getters and setters?
	// void updateVectors();
	// Prototype
	// float lastRot;
	// glm::vec2 getForwardVec(); 

protected:
	// Default values for spatial values
	static constexpr glm::vec2 default_pos = glm::vec2(0);
	static constexpr float default_rot = 0.0f;
	static constexpr glm::vec2 default_scl = glm::vec2(1);
};

float degreeFromMat4(glm::mat4 in);
glm::mat4 rotationMat4(float degree);
