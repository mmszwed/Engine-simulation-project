#define GLM_FORCE_RADIANS

#include "CameraController.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#include "constants.h"

CameraController::CameraController()
	: yaw(-35.0f * PI / 180.0f),
	  pitch(24.0f * PI / 180.0f),
	  distance(12.0f) {
}

void CameraController::update(GLFWwindow* window, float deltaTime) {
	const float rotateSpeed = 1.6f;
	const float zoomSpeed = 6.0f;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) yaw -= rotateSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) yaw += rotateSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pitch += rotateSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pitch -= rotateSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) distance += zoomSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) distance -= zoomSpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) setPreset(1);
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) setPreset(2);
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) setPreset(3);
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) setPreset(4);

	distance = std::clamp(distance, 5.0f, 24.0f);
	pitch = std::clamp(pitch, -1.2f, 1.2f);
}

glm::mat4 CameraController::getViewMatrix() const {
	return glm::lookAt(getPosition(), glm::vec3(0.0f, 0.4f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 CameraController::getPosition() const {
	const float horizontal = distance * std::cos(pitch);
	return glm::vec3(
		horizontal * std::sin(yaw),
		distance * std::sin(pitch),
		horizontal * std::cos(yaw)
	);
}

void CameraController::setPreset(int preset) {
	switch (preset) {
	case 1:
		yaw = -35.0f * PI / 180.0f;
		pitch = 24.0f * PI / 180.0f;
		distance = 12.0f;
		break;
	case 2:
		yaw = 90.0f * PI / 180.0f;
		pitch = 8.0f * PI / 180.0f;
		distance = 12.0f;
		break;
	case 3:
		yaw = 0.0f;
		pitch = 78.0f * PI / 180.0f;
		distance = 14.0f;
		break;
	case 4:
		yaw = 0.0f;
		pitch = 10.0f * PI / 180.0f;
		distance = 12.0f;
		break;
	default:
		break;
	}
}
