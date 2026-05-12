#define GLM_FORCE_RADIANS

#include "EngineSimulator.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "constants.h"
#include "shaderprogram.h"

EngineSimulator::EngineSimulator()
	: crankAngle(0.0f),
	  rpm(120.0f),
	  targetRpm(120.0f),
	  throttle(0.0f),
	  paused(true),
	  spaceWasPressed(false),
	  resetWasPressed(false),
	  meshesReady(false) {
	cylinders.emplace_back(0, -2.1f, 0.0f);
	cylinders.emplace_back(1, -0.7f, 180.0f);
	cylinders.emplace_back(2, 0.7f, 360.0f);
	cylinders.emplace_back(3, 2.1f, 540.0f);
}

void EngineSimulator::update(GLFWwindow* window, float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) throttle += 0.55f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) throttle -= 0.55f * deltaTime;

	const bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
	if (spacePressed && !spaceWasPressed) {
		paused = !paused;
	}
	spaceWasPressed = spacePressed;

	const bool resetPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
	if (resetPressed && !resetWasPressed) {
		crankAngle = 0.0f;
		rpm = 120.0f;
		targetRpm = 120.0f;
		throttle = 0.0f;
		paused = true;
	}
	resetWasPressed = resetPressed;

	throttle = std::clamp(throttle, 0.0f, 1.0f);

	const float minDemoRpm = 120.0f;
	const float maxDemoRpm = 1200.0f;
	targetRpm = minDemoRpm + throttle * (maxDemoRpm - minDemoRpm);
	rpm += (targetRpm - rpm) * 2.2f * deltaTime;

	if (!paused) {
		const float animationScale = 0.35f;
		const float degreesPerSecond = (rpm / 60.0f) * 360.0f * animationScale;
		crankAngle = std::fmod(crankAngle + degreesPerSecond * deltaTime, 720.0f);
	}
}

void EngineSimulator::draw(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) {
	initMeshes();

	drawEngineBlockCutaway(shader, view, projection);

	for (const Cylinder& cylinder : cylinders) {
		cylinder.draw(boxMesh, cylinderMesh, shader, view, projection, crankAngle);
	}

	drawCrankshaftAssembly(shader, view, projection);
	drawValveTrain(shader, view, projection);
	drawManifolds(shader, view, projection);
	drawStatusPanel(shader, view, projection);
}

void EngineSimulator::destroy() {
	boxMesh.destroy();
	cylinderMesh.destroy();
	meshesReady = false;
}

void EngineSimulator::initMeshes() {
	if (meshesReady) {
		return;
	}

	boxMesh = EngineMesh::createBox();
	cylinderMesh = EngineMesh::createCylinder(32);
	meshesReady = true;
}

void EngineSimulator::drawEngineBlockCutaway(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const glm::vec4 blockColor(0.50f, 0.53f, 0.50f, 1.0f);
	const glm::vec4 edgeColor(0.18f, 0.42f, 0.58f, 1.0f);
	const glm::mat4 base = glm::mat4(1.0f);

	glm::mat4 lowerCrankcase = glm::translate(base, glm::vec3(0.0f, -1.30f, 0.16f));
	lowerCrankcase = glm::scale(lowerCrankcase, glm::vec3(5.9f, 0.42f, 1.35f));
	drawMesh(boxMesh, shader, view, projection, lowerCrankcase, blockColor);

	glm::mat4 rearWall = glm::translate(base, glm::vec3(0.0f, -0.18f, 0.78f));
	rearWall = glm::scale(rearWall, glm::vec3(5.9f, 2.35f, 0.18f));
	drawMesh(boxMesh, shader, view, projection, rearWall, glm::vec4(0.42f, 0.45f, 0.43f, 1.0f));

	glm::mat4 frontLip = glm::translate(base, glm::vec3(0.0f, -1.05f, -0.58f));
	frontLip = glm::scale(frontLip, glm::vec3(5.9f, 0.28f, 0.16f));
	drawMesh(boxMesh, shader, view, projection, frontLip, edgeColor);

	glm::mat4 leftWall = glm::translate(base, glm::vec3(-3.05f, -0.10f, 0.12f));
	leftWall = glm::scale(leftWall, glm::vec3(0.22f, 2.25f, 1.35f));
	drawMesh(boxMesh, shader, view, projection, leftWall, blockColor);

	glm::mat4 rightWall = glm::translate(base, glm::vec3(3.05f, -0.10f, 0.12f));
	rightWall = glm::scale(rightWall, glm::vec3(0.22f, 2.25f, 1.35f));
	drawMesh(boxMesh, shader, view, projection, rightWall, blockColor);

	glm::mat4 headRail = glm::translate(base, glm::vec3(0.0f, 1.47f, 0.16f));
	headRail = glm::scale(headRail, glm::vec3(5.9f, 0.26f, 1.25f));
	drawMesh(boxMesh, shader, view, projection, headRail, glm::vec4(0.56f, 0.58f, 0.55f, 1.0f));

	for (int i = 0; i < 3; ++i) {
		const float x = -1.4f + i * 1.4f;
		glm::mat4 web = glm::translate(base, glm::vec3(x, -0.30f, 0.48f));
		web = glm::scale(web, glm::vec3(0.18f, 1.65f, 0.72f));
		drawMesh(boxMesh, shader, view, projection, web, glm::vec4(0.44f, 0.47f, 0.45f, 1.0f));
	}

	for (int i = 0; i < 4; ++i) {
		const float x = -2.1f + i * 1.4f;
		glm::mat4 blueEdge = glm::translate(base, glm::vec3(x - 0.36f, 0.55f, -0.62f));
		blueEdge = glm::scale(blueEdge, glm::vec3(0.05f, 1.55f, 0.05f));
		drawMesh(boxMesh, shader, view, projection, blueEdge, edgeColor);

		glm::mat4 deckRing = glm::translate(base, glm::vec3(x, 1.18f, 0.0f));
		deckRing = glm::scale(deckRing, glm::vec3(0.80f, 0.10f, 0.80f));
		drawMesh(cylinderMesh, shader, view, projection, deckRing, glm::vec4(0.60f, 0.62f, 0.58f, 1.0f));
	}
}

void EngineSimulator::drawCrankshaftAssembly(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const glm::mat4 base = glm::mat4(1.0f);
	const glm::vec4 darkSteel(0.13f, 0.15f, 0.16f, 1.0f);
	const glm::vec4 counterweightColor(0.08f, 0.09f, 0.10f, 1.0f);

	glm::mat4 mainShaft = glm::translate(base, glm::vec3(0.0f, -1.05f, 0.0f));
	mainShaft = glm::rotate(mainShaft, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
	mainShaft = glm::rotate(mainShaft, crankAngle * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	mainShaft = glm::scale(mainShaft, glm::vec3(0.18f, 5.45f, 0.18f));
	drawMesh(cylinderMesh, shader, view, projection, mainShaft, darkSteel);

	for (int i = 0; i < 4; ++i) {
		const float x = -2.1f + i * 1.4f;
		const float phase = (crankAngle + i * 180.0f) * PI / 180.0f;
		const glm::vec3 pin(x, -1.05f + 0.32f * std::sin(phase), 0.32f * std::cos(phase));

		glm::mat4 crankThrow = glm::translate(base, glm::vec3(x, -1.05f, 0.0f));
		crankThrow = glm::rotate(crankThrow, phase, glm::vec3(1.0f, 0.0f, 0.0f));
		crankThrow = glm::scale(crankThrow, glm::vec3(0.12f, 0.62f, 0.12f));
		drawMesh(boxMesh, shader, view, projection, crankThrow, counterweightColor);

		glm::mat4 pinMesh = glm::translate(base, pin);
		pinMesh = glm::rotate(pinMesh, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		pinMesh = glm::scale(pinMesh, glm::vec3(0.14f, 0.46f, 0.14f));
		drawMesh(cylinderMesh, shader, view, projection, pinMesh, glm::vec4(0.20f, 0.22f, 0.23f, 1.0f));

		glm::mat4 counterweight = glm::translate(base, glm::vec3(x, -1.05f - 0.28f * std::sin(phase), -0.28f * std::cos(phase)));
		counterweight = glm::rotate(counterweight, phase, glm::vec3(1.0f, 0.0f, 0.0f));
		counterweight = glm::scale(counterweight, glm::vec3(0.46f, 0.16f, 0.34f));
		drawMesh(boxMesh, shader, view, projection, counterweight, counterweightColor);
	}

	glm::mat4 flywheel = glm::translate(base, glm::vec3(3.20f, -1.05f, 0.0f));
	flywheel = glm::rotate(flywheel, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
	flywheel = glm::rotate(flywheel, crankAngle * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	flywheel = glm::scale(flywheel, glm::vec3(0.72f, 0.18f, 0.72f));
	drawMesh(cylinderMesh, shader, view, projection, flywheel, glm::vec4(0.16f, 0.18f, 0.19f, 1.0f));
}

void EngineSimulator::drawValveTrain(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const glm::mat4 base = glm::mat4(1.0f);
	const glm::vec4 camColor(0.10f, 0.11f, 0.12f, 1.0f);
	const glm::vec4 lobeColor(0.72f, 0.72f, 0.68f, 1.0f);
	const float camAngle = crankAngle * 0.5f * PI / 180.0f;

	for (int row = 0; row < 2; ++row) {
		const float z = row == 0 ? -0.46f : 0.46f;
		glm::mat4 camshaft = glm::translate(base, glm::vec3(0.0f, 2.25f, z));
		camshaft = glm::rotate(camshaft, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		camshaft = glm::rotate(camshaft, camAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		camshaft = glm::scale(camshaft, glm::vec3(0.12f, 5.6f, 0.12f));
		drawMesh(cylinderMesh, shader, view, projection, camshaft, camColor);

		for (int i = 0; i < 4; ++i) {
			const float x = -2.1f + i * 1.4f;
			glm::mat4 lobe = glm::translate(base, glm::vec3(x, 2.25f, z));
			lobe = glm::rotate(lobe, camAngle + i * PI * 0.5f + row * PI * 0.25f, glm::vec3(1.0f, 0.0f, 0.0f));
			lobe = glm::scale(lobe, glm::vec3(0.22f, 0.12f, 0.34f));
			drawMesh(boxMesh, shader, view, projection, lobe, lobeColor);
		}
	}

	for (int gear = 0; gear < 3; ++gear) {
		const float y = gear == 0 ? -1.05f : 2.25f;
		const float z = gear == 1 ? -0.46f : (gear == 2 ? 0.46f : 0.0f);
		glm::mat4 wheel = glm::translate(base, glm::vec3(-3.38f, y, z));
		wheel = glm::rotate(wheel, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		wheel = glm::rotate(wheel, (gear == 0 ? crankAngle : crankAngle * 0.5f) * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		wheel = glm::scale(wheel, glm::vec3(0.42f, 0.12f, 0.42f));
		drawMesh(cylinderMesh, shader, view, projection, wheel, glm::vec4(0.18f, 0.19f, 0.18f, 1.0f));
	}

	glm::mat4 chainLeft = glm::translate(base, glm::vec3(-3.42f, 0.55f, -0.54f));
	chainLeft = glm::scale(chainLeft, glm::vec3(0.10f, 3.25f, 0.10f));
	drawMesh(boxMesh, shader, view, projection, chainLeft, glm::vec4(0.05f, 0.05f, 0.05f, 1.0f));

	glm::mat4 chainRight = glm::translate(base, glm::vec3(-3.42f, 0.55f, 0.54f));
	chainRight = glm::scale(chainRight, glm::vec3(0.10f, 3.25f, 0.10f));
	drawMesh(boxMesh, shader, view, projection, chainRight, glm::vec4(0.05f, 0.05f, 0.05f, 1.0f));
}

void EngineSimulator::drawManifolds(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const glm::mat4 base = glm::mat4(1.0f);

	glm::mat4 intakeRail = glm::translate(base, glm::vec3(0.0f, 1.82f, -1.05f));
	intakeRail = glm::rotate(intakeRail, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
	intakeRail = glm::scale(intakeRail, glm::vec3(0.16f, 5.4f, 0.16f));
	drawMesh(cylinderMesh, shader, view, projection, intakeRail, glm::vec4(0.28f, 0.37f, 0.42f, 1.0f));

	glm::mat4 exhaustRail = glm::translate(base, glm::vec3(0.0f, 1.60f, 1.05f));
	exhaustRail = glm::rotate(exhaustRail, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
	exhaustRail = glm::scale(exhaustRail, glm::vec3(0.18f, 5.4f, 0.18f));
	drawMesh(cylinderMesh, shader, view, projection, exhaustRail, glm::vec4(0.40f, 0.32f, 0.25f, 1.0f));

	for (int i = 0; i < 4; ++i) {
		const float x = -2.1f + i * 1.4f;

		glm::mat4 intakeRunner = glm::translate(base, glm::vec3(x, 1.66f, -0.72f));
		intakeRunner = glm::rotate(intakeRunner, 0.60f, glm::vec3(1.0f, 0.0f, 0.0f));
		intakeRunner = glm::scale(intakeRunner, glm::vec3(0.10f, 0.78f, 0.10f));
		drawMesh(cylinderMesh, shader, view, projection, intakeRunner, glm::vec4(0.30f, 0.43f, 0.50f, 1.0f));

		glm::mat4 exhaustRunner = glm::translate(base, glm::vec3(x, 1.48f, 0.72f));
		exhaustRunner = glm::rotate(exhaustRunner, -0.60f, glm::vec3(1.0f, 0.0f, 0.0f));
		exhaustRunner = glm::scale(exhaustRunner, glm::vec3(0.11f, 0.78f, 0.11f));
		drawMesh(cylinderMesh, shader, view, projection, exhaustRunner, glm::vec4(0.50f, 0.38f, 0.26f, 1.0f));

		glm::mat4 fuelLine = glm::translate(base, glm::vec3(x, 2.25f, -0.12f));
		fuelLine = glm::scale(fuelLine, glm::vec3(0.04f, 0.62f, 0.04f));
		drawMesh(cylinderMesh, shader, view, projection, fuelLine, glm::vec4(0.18f, 0.20f, 0.22f, 1.0f));
	}
}

void EngineSimulator::drawStatusPanel(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const glm::mat4 base = glm::mat4(1.0f);
	const glm::vec3 panelOrigin(4.35f, 0.45f, -0.95f);

	glm::mat4 panel = glm::translate(base, panelOrigin);
	panel = glm::scale(panel, glm::vec3(1.35f, 1.55f, 0.08f));
	drawMesh(boxMesh, shader, view, projection, panel, glm::vec4(0.10f, 0.12f, 0.13f, 1.0f));

	glm::mat4 rpmTrack = glm::translate(base, panelOrigin + glm::vec3(0.0f, 0.35f, -0.08f));
	rpmTrack = glm::scale(rpmTrack, glm::vec3(1.05f, 0.12f, 0.04f));
	drawMesh(boxMesh, shader, view, projection, rpmTrack, glm::vec4(0.25f, 0.27f, 0.28f, 1.0f));

	const float rpmFillValue = std::clamp((rpm - 120.0f) / 1080.0f, 0.0f, 1.0f);
	glm::mat4 rpmFill = glm::translate(base, panelOrigin + glm::vec3(-0.525f + 0.525f * rpmFillValue, 0.35f, -0.12f));
	rpmFill = glm::scale(rpmFill, glm::vec3(1.05f * rpmFillValue, 0.13f, 0.05f));
	drawMesh(boxMesh, shader, view, projection, rpmFill, glm::vec4(0.20f, 0.55f, 0.78f, 1.0f));

	glm::mat4 throttleTrack = glm::translate(base, panelOrigin + glm::vec3(0.0f, -0.10f, -0.08f));
	throttleTrack = glm::scale(throttleTrack, glm::vec3(1.05f, 0.12f, 0.04f));
	drawMesh(boxMesh, shader, view, projection, throttleTrack, glm::vec4(0.25f, 0.27f, 0.28f, 1.0f));

	glm::mat4 throttleFill = glm::translate(base, panelOrigin + glm::vec3(-0.525f + 0.525f * throttle, -0.10f, -0.12f));
	throttleFill = glm::scale(throttleFill, glm::vec3(1.05f * throttle, 0.13f, 0.05f));
	drawMesh(boxMesh, shader, view, projection, throttleFill, glm::vec4(0.78f, 0.48f, 0.16f, 1.0f));

	glm::mat4 pauseLamp = glm::translate(base, panelOrigin + glm::vec3(0.0f, -0.58f, -0.12f));
	pauseLamp = glm::scale(pauseLamp, glm::vec3(0.32f, 0.32f, 0.06f));
	drawMesh(cylinderMesh, shader, view, projection, pauseLamp, paused ? glm::vec4(0.75f, 0.18f, 0.12f, 1.0f) : glm::vec4(0.16f, 0.55f, 0.30f, 1.0f));
}

void EngineSimulator::drawMesh(const EngineMesh& mesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const glm::vec4& color) const {
	shader->use();
	glUniformMatrix4fv(shader->u("P"), 1, false, glm::value_ptr(projection));
	glUniformMatrix4fv(shader->u("V"), 1, false, glm::value_ptr(view));
	glUniformMatrix4fv(shader->u("M"), 1, false, glm::value_ptr(model));
	glUniform4fv(shader->u("color"), 1, glm::value_ptr(color));
	glUniform4f(shader->u("lightDir"), 0.0f, 0.0f, 1.0f, 0.0f);
	mesh.draw();
}
