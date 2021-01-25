#pragma once

#include <vector>
#include <memory>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "object2d.h"

// A Camera object that processes a view for displaying in OpenGL. 
class Camera2d : public Object2d {
public:
	enum PROJ_TYPE {
		ORTHO, PERSPECTIVE
	};

	// Camera Attributes
	const glm::vec3 Front = glm::vec3(0, 0, 1);
	float fov;
	PROJ_TYPE projection = ORTHO;
	float far = 100;

	// Window size
	unsigned int* display_height;
	unsigned int* display_width;

	Camera2d(std::string id, glm::vec2 position = glm::vec2(0.0f), float rotation = 0.0, PROJ_TYPE _projection = ORTHO);
	
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();
	float getAspectRatio();
	
	glm::vec2 up();
	glm::vec2 right();
};

