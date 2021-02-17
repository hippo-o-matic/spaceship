#include "camera2d.h"

Camera2d* Camera2d::main_camera = nullptr;

Camera2d::Camera2d(std::string id,
	unsigned* display_w,
	unsigned* display_h, 
	glm::vec2 position, 
	float rotation, 
	PROJ_TYPE _projection
): Object2d(id, position, rotation), display_width(display_w), display_height(display_h) 
{
	projection = _projection;

	// Set sane values for zoom level
	if(projection == ORTHO)
		fov = 3;
	else if(projection == PERSPECTIVE)
		fov = 60;
}

// Returns the view matrix
glm::mat4 Camera2d::getViewMatrix(){
	glm::mat4 rotate = glm::rotate(glm::mat4(1), glm::radians(-getWorldRot()), Front); // Rotate
	glm::mat4 pos = glm::translate(glm::mat4(1), glm::vec3(getWorldPos(), 0)); // And finally translate by position
	
	return rotate * glm::inverse(pos);
}

glm::mat4 Camera2d::getProjectionMatrix(){
	if(projection == ORTHO) {
		float view_range = fov * getAspectRatio();
		return glm::ortho(-view_range, view_range, -fov, fov, -far, far);
	} else {
		return glm::perspective(glm::radians(fov), getAspectRatio(), 0.1f, far);
	}
}

float Camera2d::getAspectRatio() {
	return (float)(*display_width) / (float)(*display_height);
}