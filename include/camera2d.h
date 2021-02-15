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
	unsigned int* display_width;
	unsigned int* display_height;

	static Camera2d* main_camera;

	Camera2d(std::string id,
		unsigned* display_w,
		unsigned* display_h, 
		glm::vec2 position = glm::vec2(0), 
		float rotation = 0.f, 
		PROJ_TYPE _projection = ORTHO
	);
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();
	float getAspectRatio();
};

