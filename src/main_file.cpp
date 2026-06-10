#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <cstdlib>

#include "CameraController.h"
#include "EngineSimulator.h"
#include "constants.h"
#include "shaderprogram.h"

GLuint vao = 0;
GLuint shadowFramebuffer = 0;
GLuint shadowDepthTexture = 0;
constexpr int SHADOW_MAP_SIZE = 2048;
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

	glGenFramebuffers(1, &shadowFramebuffer);
	glGenTextures(1, &shadowDepthTexture);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT24,
		SHADOW_MAP_SIZE,
		SHADOW_MAP_SIZE,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "Nie mozna utworzyc mapy cienia.\n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void freeOpenGLProgram(GLFWwindow* window) {
	(void)window;
	engine.destroy();
	if (shadowDepthTexture != 0) {
		glDeleteTextures(1, &shadowDepthTexture);
		shadowDepthTexture = 0;
	}
	if (shadowFramebuffer != 0) {
		glDeleteFramebuffers(1, &shadowFramebuffer);
		shadowFramebuffer = 0;
	}
	freeShaders();
	if (vao != 0) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
}

void drawScene(GLFWwindow* window) {
	const glm::vec3 workLightPosition(5.18f, 0.85f, -1.78f);
	const glm::vec3 workLightTarget(0.0f, -0.10f, 0.0f);
	const glm::mat4 lightProjection = glm::perspective(56.0f * PI / 180.0f, 1.0f, 0.5f, 18.0f);
	const glm::mat4 lightView = glm::lookAt(workLightPosition, workLightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f, 4.0f);
	engine.drawShadow(spShadow, lightView, lightProjection);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glClearColor(0.16f, 0.17f, 0.18f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const float aspect = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
	const glm::mat4 view = camera.getViewMatrix();
	const glm::mat4 projection = glm::perspective(50.0f * PI / 180.0f, aspect, 0.1f, 80.0f);

	spEngine->use();
	glUniformMatrix4fv(spEngine->u("lightSpaceMatrix"), 1, false, glm::value_ptr(lightSpaceMatrix));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
	glUniform1i(spEngine->u("shadowMap"), 1);
	engine.draw(spEngine, view, projection);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

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
