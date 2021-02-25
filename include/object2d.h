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

	Object2d(Json::Value j);

	~Object2d();

	int obj_layer = 1;

	bool prevent_inherit_pos = false;
	bool prevent_inherit_rot = false;
	bool prevent_inherit_scl = false;

	glm::vec2 getPos() const;
	float getRot() const;
	glm::vec2 getScl() const;

	// Primary spatial functions, override these if you need special behavior
	virtual Object2d& setPos(glm::vec2 pos);
	virtual Object2d& setRot(float rot);
	virtual Object2d& setScl(glm::vec2 scl);

	glm::vec2 getWorldPos() const;
	Object2d& setWorldPos(glm::vec2);

	float getWorldRot() const;
	Object2d& setWorldRot(float);

	glm::vec2 getWorldScl() const;
	Object2d& setWorldScl(glm::vec2);

	glm::mat4 getTransform() const;
	glm::mat4 getWorldTransform() const;
	Object2d& setTransform(glm::mat4);
	Object2d& setWorldTransform(glm::mat4);
	Object2d& transformBy(glm::mat4);

	// Directional vectors
	glm::vec2 up();
	glm::vec2 right();

protected:
	// Default values for spatial values
	static constexpr glm::vec2 default_pos = glm::vec2(0);
	static constexpr float default_rot = 0.0f;
	static constexpr glm::vec2 default_scl = glm::vec2(1);

private:
	glm::vec2 position;
	float rotation;
	glm::vec2 scale;
};

float degreeFromMat4(glm::mat4 in);
glm::mat4 rotationMat4(float degree);
