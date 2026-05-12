#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <glm/glm.hpp>

struct GLFWwindow;

class CameraController {
public:
	CameraController();

	void update(GLFWwindow* window, float deltaTime);
	glm::mat4 getViewMatrix() const;
	glm::vec3 getPosition() const;

private:
	float yaw;
	float pitch;
	float distance;

	void setPreset(int preset);
};

#endif
