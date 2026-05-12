#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <cstdlib>

#include "CameraController.h"
#include "EngineSimulator.h"
#include "constants.h"
#include "shaderprogram.h"

GLuint vao = 0;
CameraController camera;
EngineSimulator engine;

void error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

void initOpenGLProgram(GLFWwindow* window) {
	initShaders();
	glEnable(GL_DEPTH_TEST);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
}

void freeOpenGLProgram(GLFWwindow* window) {
	(void)window;
	engine.destroy();
	freeShaders();
	if (vao != 0) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
}

void drawScene(GLFWwindow* window) {
	glClearColor(0.08f, 0.09f, 0.10f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	const float aspect = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
	const glm::mat4 view = camera.getViewMatrix();
	const glm::mat4 projection = glm::perspective(50.0f * PI / 180.0f, aspect, 0.1f, 80.0f);

	engine.draw(spLambert, view, projection);

	glfwSwapBuffers(window);
}

int main(void) {
	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		fprintf(stderr, "Nie mozna zainicjowac GLFW.\n");
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(1000, 700, "Symulator silnika R4", NULL, NULL);
	if (!window) {
		fprintf(stderr, "Nie mozna utworzyc okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Nie mozna zainicjowac GLEW.\n");
		exit(EXIT_FAILURE);
	}
	glGetError();

	initOpenGLProgram(window);

	float previousTime = static_cast<float>(glfwGetTime());
	while (!glfwWindowShouldClose(window)) {
		const float currentTime = static_cast<float>(glfwGetTime());
		const float deltaTime = currentTime - previousTime;
		previousTime = currentTime;

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		camera.update(window, deltaTime);
		engine.update(window, deltaTime);
		drawScene(window);
		glfwPollEvents();
	}

	freeOpenGLProgram(window);
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
