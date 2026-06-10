#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <glm/glm.hpp>

struct GLFWwindow;

/**
 * @brief Orbit camera used to inspect the engine scene.
 *
 * The controller stores yaw, pitch and distance from the engine. It supports
 * continuous keyboard movement and four predefined presentation views.
 */
class CameraController {
public:
	/** @brief Creates the camera in the default front-left presentation view. */
	CameraController();

	/**
	 * @brief Reads keyboard input and updates the camera.
	 * @param window Active GLFW window used to query key states.
	 * @param deltaTime Time elapsed since the previous frame in seconds.
	 */
	void update(GLFWwindow* window, float deltaTime);

	/** @return View matrix directed at the center of the engine. */
	glm::mat4 getViewMatrix() const;

	/** @return Current camera position in world space. */
	glm::vec3 getPosition() const;

private:
	float yaw;
	float pitch;
	float distance;

	/** @brief Selects one of the four predefined camera views. */
	void setPreset(int preset);
};

#endif
