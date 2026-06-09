#define GLM_FORCE_RADIANS

#include "EngineSimulator.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <vector>
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
	  meshesReady(false),
	  metalTexture(0),
	  darkMetalTexture(0),
	  rubberTexture(0) {
	// Inline-four crank pairs: 1+4 together, 2+3 opposite.
	// Cycle offsets produce the conventional 1-3-4-2 firing order.
	cylinders.emplace_back(0, -2.1f, 0.0f, 360.0f);
	cylinders.emplace_back(1, -0.7f, 180.0f, 540.0f);
	cylinders.emplace_back(2, 0.7f, 180.0f, 180.0f);
	cylinders.emplace_back(3, 2.1f, 0.0f, 0.0f);
}

void EngineSimulator::update(GLFWwindow* window, float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) throttle += 0.55f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) throttle -= 0.55f * deltaTime;

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
		cylinder.draw(boxMesh, cylinderMesh, halfCylinderMesh, valvePlateMesh, valveSeatMesh, shader, view, projection, crankAngle, metalTexture, darkMetalTexture, rubberTexture);
	}

	drawCrankshaftAssembly(shader, view, projection);
	drawValveTrain(shader, view, projection);
	drawManifolds(shader, view, projection);
	drawStatusPanel(shader, view, projection);
}

void EngineSimulator::destroy() {
	boxMesh.destroy();
	cylinderMesh.destroy();
	camLobeMesh.destroy();
	halfCylinderMesh.destroy();
	intakeManifoldMesh.destroy();
	exhaustManifoldMesh.destroy();
	valvePlateMesh.destroy();
	valveSeatMesh.destroy();
	destroyTextures();
	meshesReady = false;
}

void EngineSimulator::initMeshes() {
	if (meshesReady) {
		return;
	}

	boxMesh = EngineMesh::createBox();
	cylinderMesh = EngineMesh::createCylinder(64);
	camLobeMesh = EngineMesh::createCamLobe(96);
	halfCylinderMesh = EngineMesh::createHalfCylinder(48);
	intakeManifoldMesh = EngineMesh::createPortedHalfCylinder(48, 96, false);
	exhaustManifoldMesh = EngineMesh::createPortedHalfCylinder(48, 96, true);
	valvePlateMesh = EngineMesh::createValvePlate(96, 40);
	valveSeatMesh = EngineMesh::createRing(64, 0.72f);
	initTextures();
	meshesReady = true;
}

void EngineSimulator::initTextures() {
	if (metalTexture != 0) {
		return;
	}

	metalTexture = createProceduralTexture(0);
	darkMetalTexture = createProceduralTexture(1);
	rubberTexture = createProceduralTexture(2);
}

void EngineSimulator::destroyTextures() {
	if (metalTexture != 0) glDeleteTextures(1, &metalTexture);
	if (darkMetalTexture != 0) glDeleteTextures(1, &darkMetalTexture);
	if (rubberTexture != 0) glDeleteTextures(1, &rubberTexture);
	metalTexture = 0;
	darkMetalTexture = 0;
	rubberTexture = 0;
}

unsigned int EngineSimulator::createProceduralTexture(int variant) const {
	const int size = 64;
	std::vector<unsigned char> pixels(size * size * 3);

	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			const int index = (y * size + x) * 3;
			const int stripe = (x / 6 + y / 17) % 2;
			const int fine = (x * 13 + y * 7 + variant * 31) % 23;
			unsigned char r = 0;
			unsigned char g = 0;
			unsigned char b = 0;

			if (variant == 0) {
				const unsigned char value = static_cast<unsigned char>(172 + stripe * 22 + fine);
				r = value;
				g = value;
				b = static_cast<unsigned char>(value - 8);
			} else if (variant == 1) {
				const unsigned char value = static_cast<unsigned char>(62 + stripe * 20 + fine / 2);
				r = value;
				g = static_cast<unsigned char>(value + 2);
				b = static_cast<unsigned char>(value + 4);
			} else {
				const unsigned char value = static_cast<unsigned char>(34 + stripe * 12 + fine / 3);
				r = value;
				g = value;
				b = value;
			}

			pixels[index + 0] = r;
			pixels[index + 1] = g;
			pixels[index + 2] = b;
		}
	}

	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

unsigned int EngineSimulator::chooseTexture(const glm::vec4& color) const {
	const float brightness = (color.r + color.g + color.b) / 3.0f;
	if (brightness < 0.20f) {
		return rubberTexture;
	}
	if (brightness < 0.42f) {
		return darkMetalTexture;
	}
	return metalTexture;
}

void EngineSimulator::drawEngineBlockCutaway(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const glm::vec4 blockColor(0.57f, 0.58f, 0.53f, 1.0f);
	const glm::vec4 edgeColor(0.12f, 0.44f, 0.64f, 1.0f);
	const glm::mat4 base = glm::mat4(1.0f);

	glm::mat4 rearCrankcaseWall = glm::translate(base, glm::vec3(0.0f, -1.48f, 0.66f));
	rearCrankcaseWall = glm::scale(rearCrankcaseWall, glm::vec3(5.9f, 0.58f, 0.24f));
	drawMesh(boxMesh, shader, view, projection, rearCrankcaseWall, blockColor);

	glm::mat4 frontCrankcaseRail = glm::translate(base, glm::vec3(0.0f, -1.48f, -0.62f));
	frontCrankcaseRail = glm::scale(frontCrankcaseRail, glm::vec3(5.9f, 0.58f, 0.14f));
	drawMesh(boxMesh, shader, view, projection, frontCrankcaseRail, edgeColor);

	glm::mat4 oilPan = glm::translate(base, glm::vec3(0.0f, -1.90f, 0.22f));
	oilPan = glm::scale(oilPan, glm::vec3(5.55f, 0.30f, 1.05f));
	drawMesh(boxMesh, shader, view, projection, oilPan, glm::vec4(0.36f, 0.38f, 0.35f, 1.0f));

	glm::mat4 rearOilShelf = glm::translate(base, glm::vec3(0.0f, -1.16f, 0.66f));
	rearOilShelf = glm::scale(rearOilShelf, glm::vec3(5.65f, 0.10f, 0.22f));
	drawMesh(boxMesh, shader, view, projection, rearOilShelf, glm::vec4(0.50f, 0.52f, 0.48f, 1.0f));

	glm::mat4 frontOilShelf = glm::translate(base, glm::vec3(0.0f, -1.16f, -0.62f));
	frontOilShelf = glm::scale(frontOilShelf, glm::vec3(5.65f, 0.10f, 0.12f));
	drawMesh(boxMesh, shader, view, projection, frontOilShelf, edgeColor);

	glm::mat4 rearWall = glm::translate(base, glm::vec3(0.0f, -0.18f, 0.78f));
	rearWall = glm::scale(rearWall, glm::vec3(5.9f, 2.35f, 0.18f));
	drawMesh(boxMesh, shader, view, projection, rearWall, glm::vec4(0.48f, 0.50f, 0.47f, 1.0f));

	glm::mat4 frontLip = glm::translate(base, glm::vec3(0.0f, -1.32f, -0.58f));
	frontLip = glm::scale(frontLip, glm::vec3(5.9f, 0.58f, 0.16f));
	drawMesh(boxMesh, shader, view, projection, frontLip, edgeColor);

	glm::mat4 leftWall = glm::translate(base, glm::vec3(-3.05f, -0.10f, 0.12f));
	leftWall = glm::scale(leftWall, glm::vec3(0.22f, 2.25f, 1.35f));
	drawMesh(boxMesh, shader, view, projection, leftWall, blockColor);

	glm::mat4 rightWall = glm::translate(base, glm::vec3(3.05f, -0.10f, 0.12f));
	rightWall = glm::scale(rightWall, glm::vec3(0.22f, 2.25f, 1.35f));
	drawMesh(boxMesh, shader, view, projection, rightWall, blockColor);

	for (int i = 0; i < 3; ++i) {
		const float x = -1.4f + i * 1.4f;
		glm::mat4 web = glm::translate(base, glm::vec3(x, -0.30f, 0.48f));
		web = glm::scale(web, glm::vec3(0.18f, 1.65f, 0.72f));
		drawMesh(boxMesh, shader, view, projection, web, glm::vec4(0.44f, 0.47f, 0.45f, 1.0f));
	}

	for (int i = 0; i < 9; ++i) {
		const float x = -2.8f + i * 0.7f;
		glm::mat4 railBolt = glm::translate(base, glm::vec3(x, -1.05f, -0.70f));
		railBolt = glm::scale(railBolt, glm::vec3(0.055f, 0.10f, 0.055f));
		drawMesh(cylinderMesh, shader, view, projection, railBolt, glm::vec4(0.12f, 0.13f, 0.13f, 1.0f));
	}
}

void EngineSimulator::drawCrankshaftAssembly(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const glm::mat4 base = glm::mat4(1.0f);
	const glm::vec4 darkSteel(0.13f, 0.15f, 0.16f, 1.0f);
	const glm::vec4 webColor(0.20f, 0.22f, 0.22f, 1.0f);
	const glm::vec4 counterweightColor(0.12f, 0.13f, 0.13f, 1.0f);
	const glm::vec4 journalColor(0.30f, 0.32f, 0.32f, 1.0f);
	const float mainY = -1.05f;
	const float throwRadius = 0.32f;
	const auto alignCylinderBetween = [](const glm::vec3& start, const glm::vec3& end, float radius) {
		const glm::vec3 middle = (start + end) * 0.5f;
		const glm::vec3 direction = end - start;
		const float length = glm::length(direction);

		glm::mat4 model = glm::translate(glm::mat4(1.0f), middle);
		if (length > 0.0001f) {
			const glm::vec3 up(0.0f, 1.0f, 0.0f);
			const glm::vec3 target = glm::normalize(direction);
			const float dotValue = std::clamp(glm::dot(up, target), -1.0f, 1.0f);
			const float angle = std::acos(dotValue);
			glm::vec3 axis = glm::cross(up, target);
			if (glm::length(axis) > 0.0001f) {
				model = glm::rotate(model, angle, glm::normalize(axis));
			}
		}

		return glm::scale(model, glm::vec3(radius, length, radius));
	};
	const float journalSegments[5][2] = {
		{-2.95f, -2.44f},
		{-1.76f, -1.04f},
		{-0.36f, 0.36f},
		{1.04f, 1.76f},
		{2.44f, 2.95f}
	};

	for (const auto& segment : journalSegments) {
		const float middleX = (segment[0] + segment[1]) * 0.5f;
		const float length = segment[1] - segment[0];
		glm::mat4 mainJournal = glm::translate(base, glm::vec3(middleX, mainY, 0.0f));
		mainJournal = glm::rotate(mainJournal, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		mainJournal = glm::scale(mainJournal, glm::vec3(0.18f, length, 0.18f));
		drawMesh(cylinderMesh, shader, view, projection, mainJournal, darkSteel);

		glm::mat4 journalCollar = glm::translate(base, glm::vec3(middleX, mainY, 0.0f));
		journalCollar = glm::rotate(journalCollar, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		journalCollar = glm::scale(journalCollar, glm::vec3(0.24f, 0.08f, 0.24f));
		drawMesh(cylinderMesh, shader, view, projection, journalCollar, journalColor);
	}

	for (int i = 0; i < 4; ++i) {
		const float x = -2.1f + i * 1.4f;
		const float crankOffsets[4] = {0.0f, 180.0f, 180.0f, 0.0f};
		const float phase = (crankAngle + crankOffsets[i]) * PI / 180.0f + PI * 0.5f;
		const glm::vec3 pin(x, mainY + throwRadius * std::sin(phase), throwRadius * std::cos(phase));
		const glm::vec3 counterCenter(x, mainY - throwRadius * 0.74f * std::sin(phase), -throwRadius * 0.74f * std::cos(phase));

		for (float cheekX : {-0.26f, 0.26f}) {
			const glm::vec3 mainPoint(x + cheekX, mainY, 0.0f);
			const glm::vec3 pinPoint(x + cheekX, pin.y, pin.z);
			const glm::vec3 counterPoint(x + cheekX, counterCenter.y, counterCenter.z);

			glm::mat4 crankWeb = alignCylinderBetween(mainPoint, pinPoint, 0.075f);
			drawMesh(cylinderMesh, shader, view, projection, crankWeb, webColor);

			glm::mat4 counterArm = alignCylinderBetween(mainPoint, counterPoint, 0.065f);
			drawMesh(cylinderMesh, shader, view, projection, counterArm, counterweightColor);

			glm::mat4 mainCheek = glm::translate(base, mainPoint);
			mainCheek = glm::rotate(mainCheek, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
			mainCheek = glm::scale(mainCheek, glm::vec3(0.22f, 0.07f, 0.22f));
			drawMesh(cylinderMesh, shader, view, projection, mainCheek, journalColor);

			glm::mat4 pinCheek = glm::translate(base, pinPoint);
			pinCheek = glm::rotate(pinCheek, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
			pinCheek = glm::scale(pinCheek, glm::vec3(0.20f, 0.075f, 0.20f));
			drawMesh(cylinderMesh, shader, view, projection, pinCheek, webColor);

			glm::mat4 counterDisc = glm::translate(base, glm::vec3(x + cheekX, counterCenter.y, counterCenter.z));
			counterDisc = glm::rotate(counterDisc, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
			counterDisc = glm::scale(counterDisc, glm::vec3(0.32f, 0.09f, 0.24f));
			drawMesh(cylinderMesh, shader, view, projection, counterDisc, counterweightColor);
		}

		glm::mat4 pinMesh = glm::translate(base, pin);
		pinMesh = glm::rotate(pinMesh, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		pinMesh = glm::scale(pinMesh, glm::vec3(0.15f, 0.52f, 0.15f));
		drawMesh(cylinderMesh, shader, view, projection, pinMesh, glm::vec4(0.20f, 0.22f, 0.23f, 1.0f));

		glm::mat4 pinCollar = glm::translate(base, pin);
		pinCollar = glm::rotate(pinCollar, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		pinCollar = glm::scale(pinCollar, glm::vec3(0.23f, 0.18f, 0.23f));
		drawMesh(cylinderMesh, shader, view, projection, pinCollar, journalColor);
	}

	for (float endX : {-2.88f, 2.88f}) {
		glm::mat4 endBearing = glm::translate(base, glm::vec3(endX, mainY, 0.0f));
		endBearing = glm::rotate(endBearing, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		endBearing = glm::scale(endBearing, glm::vec3(0.27f, 0.16f, 0.27f));
		drawMesh(cylinderMesh, shader, view, projection, endBearing, glm::vec4(0.34f, 0.36f, 0.35f, 1.0f));

		glm::mat4 bearingHousing = glm::translate(base, glm::vec3(endX, mainY - 0.18f, 0.0f));
		bearingHousing = glm::scale(bearingHousing, glm::vec3(0.46f, 0.18f, 0.54f));
		drawMesh(boxMesh, shader, view, projection, bearingHousing, glm::vec4(0.40f, 0.42f, 0.39f, 1.0f));
	}
}

void EngineSimulator::drawValveTrain(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const glm::mat4 base = glm::mat4(1.0f);
	const glm::vec4 camColor(0.10f, 0.11f, 0.12f, 1.0f);
	const glm::vec4 lobeColor(0.72f, 0.72f, 0.68f, 1.0f);
	const auto alignBoxBetween = [](const glm::vec3& start, const glm::vec3& end, float thickness) {
		const glm::vec3 direction = end - start;
		const float length = glm::length(direction);
		glm::mat4 model = glm::translate(glm::mat4(1.0f), (start + end) * 0.5f);
		if (length > 0.0001f) {
			const glm::vec3 up(0.0f, 1.0f, 0.0f);
			const glm::vec3 target = direction / length;
			const float dotValue = std::clamp(glm::dot(up, target), -1.0f, 1.0f);
			const float angle = std::acos(dotValue);
			const glm::vec3 axis = glm::cross(up, target);
			if (glm::length(axis) > 0.0001f) {
				model = glm::rotate(model, angle, glm::normalize(axis));
			}
		}
		return glm::scale(model, glm::vec3(thickness, length, thickness));
	};
	const float camAngle = crankAngle * 0.5f * PI / 180.0f;
	const float camVisibleLength = 5.10f;
	const float sprocketX = -3.22f;
	const float camEndX = -2.55f;
	const float connectorLength = sprocketX - camEndX;
	const float connectorMidX = (sprocketX + camEndX) * 0.5f;

	for (int row = 0; row < 2; ++row) {
		const float z = row == 0 ? -0.46f : 0.46f;
		glm::mat4 camshaft = glm::translate(base, glm::vec3(0.0f, 2.25f, z));
		camshaft = glm::rotate(camshaft, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		camshaft = glm::rotate(camshaft, camAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		camshaft = glm::scale(camshaft, glm::vec3(0.12f, camVisibleLength, 0.12f));
		drawMesh(cylinderMesh, shader, view, projection, camshaft, camColor);

		glm::mat4 camConnector = glm::translate(base, glm::vec3(connectorMidX, 2.25f, z));
		camConnector = glm::rotate(camConnector, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		camConnector = glm::rotate(camConnector, camAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		camConnector = glm::scale(camConnector, glm::vec3(0.13f, std::abs(connectorLength), 0.13f));
		drawMesh(cylinderMesh, shader, view, projection, camConnector, camColor);

		glm::mat4 camEndCollar = glm::translate(base, glm::vec3(camEndX, 2.25f, z));
		camEndCollar = glm::rotate(camEndCollar, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		camEndCollar = glm::scale(camEndCollar, glm::vec3(0.20f, 0.08f, 0.20f));
		drawMesh(cylinderMesh, shader, view, projection, camEndCollar, glm::vec4(0.42f, 0.44f, 0.42f, 1.0f));

		for (int i = 0; i < 4; ++i) {
			const float x = -2.1f + i * 1.4f;
			const float cycleOffsets[4] = {360.0f, 540.0f, 180.0f, 0.0f};
			const float cylinderPhase = std::fmod(crankAngle + cycleOffsets[i], 720.0f);
			const float phaseOnCam = cylinderPhase * 0.5f * PI / 180.0f;
			const float timingOffset = row == 0 ? 3.0f * PI / 4.0f : 5.0f * PI / 4.0f;
			const float lobeAngle = phaseOnCam + timingOffset;

			for (float valveOffsetX : {-0.20f, 0.20f}) {
				const float lobeX = x + valveOffsetX;
				glm::mat4 lobe = glm::translate(base, glm::vec3(lobeX, 2.25f, z));
				lobe = glm::rotate(lobe, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
				lobe = glm::rotate(lobe, lobeAngle, glm::vec3(0.0f, 1.0f, 0.0f));
				lobe = glm::scale(lobe, glm::vec3(0.52f, 0.14f, 0.52f));
				drawMesh(camLobeMesh, shader, view, projection, lobe, lobeColor);
			}

			if (row == 0) {
				float injectionStroke = 0.0f;
				if (cylinderPhase >= 330.0f && cylinderPhase <= 390.0f) {
					const float injectionLocal = (cylinderPhase - 330.0f) / 60.0f;
					injectionStroke = 0.15f * std::sin(PI * injectionLocal) * std::sin(PI * injectionLocal);
				}

				// At peak injection the lobe nose points toward the follower.
				const float injectionLobeAngle = phaseOnCam;
				glm::mat4 injectionLobe = glm::translate(base, glm::vec3(x, 2.25f, z));
				injectionLobe = glm::rotate(injectionLobe, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
				injectionLobe = glm::rotate(injectionLobe, injectionLobeAngle, glm::vec3(0.0f, 1.0f, 0.0f));
				injectionLobe = glm::scale(injectionLobe, glm::vec3(0.40f, 0.13f, 0.40f));
				drawMesh(camLobeMesh, shader, view, projection, injectionLobe, glm::vec4(0.62f, 0.64f, 0.60f, 1.0f));

				const glm::vec3 injectorAxis = glm::normalize(glm::vec3(0.0f, 0.625f, -0.18f));
				const glm::vec3 injectorPad = glm::vec3(x, 1.91f, -0.18f)
					+ injectorAxis * (0.025f - injectionStroke);
				const float injectionRatio = injectionStroke / 0.15f;
				const glm::vec3 rockerRoller(x, 2.04f - injectionRatio * 0.075f, -0.43f);
				const glm::vec3 rockerPivot(x, 1.985f, -0.285f);

				glm::mat4 rollerArm = alignBoxBetween(rockerPivot, rockerRoller, 0.078f);
				drawMesh(boxMesh, shader, view, projection, rollerArm, glm::vec4(0.58f, 0.60f, 0.57f, 1.0f));

				glm::mat4 pressureArm = alignBoxBetween(rockerPivot, injectorPad, 0.082f);
				drawMesh(boxMesh, shader, view, projection, pressureArm, glm::vec4(0.58f, 0.60f, 0.57f, 1.0f));

				glm::mat4 rockerRollerMesh = glm::translate(base, rockerRoller);
				rockerRollerMesh = glm::rotate(rockerRollerMesh, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
				rockerRollerMesh = glm::scale(rockerRollerMesh, glm::vec3(0.085f, 0.070f, 0.085f));
				drawMesh(cylinderMesh, shader, view, projection, rockerRollerMesh, glm::vec4(0.76f, 0.77f, 0.72f, 1.0f));

				glm::mat4 rockerPivotMesh = glm::translate(base, rockerPivot);
				rockerPivotMesh = glm::rotate(rockerPivotMesh, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
				rockerPivotMesh = glm::scale(rockerPivotMesh, glm::vec3(0.125f, 0.110f, 0.125f));
				drawMesh(cylinderMesh, shader, view, projection, rockerPivotMesh, glm::vec4(0.42f, 0.44f, 0.42f, 1.0f));

				glm::mat4 pressureFoot = glm::translate(base, injectorPad);
				pressureFoot = glm::rotate(pressureFoot, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
				pressureFoot = glm::scale(pressureFoot, glm::vec3(0.125f, 0.055f, 0.125f));
				drawMesh(cylinderMesh, shader, view, projection, pressureFoot, glm::vec4(0.68f, 0.69f, 0.65f, 1.0f));
			}

			glm::mat4 camBearing = glm::translate(base, glm::vec3(x + 0.40f, 2.25f, z));
			camBearing = glm::rotate(camBearing, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
			camBearing = glm::scale(camBearing, glm::vec3(0.18f, 0.10f, 0.18f));
			drawMesh(cylinderMesh, shader, view, projection, camBearing, glm::vec4(0.58f, 0.60f, 0.56f, 1.0f));
		}
	}

	for (int gear = 0; gear < 3; ++gear) {
		const float y = gear == 0 ? -1.05f : 2.25f;
		const float z = gear == 1 ? -0.46f : (gear == 2 ? 0.46f : 0.0f);
		const float wheelAngle = (gear == 0 ? crankAngle : crankAngle * 0.5f) * PI / 180.0f;
		glm::mat4 wheel = glm::translate(base, glm::vec3(sprocketX, y, z));
		wheel = glm::rotate(wheel, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
		wheel = glm::rotate(wheel, wheelAngle, glm::vec3(0.0f, 1.0f, 0.0f));
		wheel = glm::scale(wheel, glm::vec3(0.42f, 0.12f, 0.42f));
		drawMesh(cylinderMesh, shader, view, projection, wheel, glm::vec4(0.18f, 0.19f, 0.18f, 1.0f));

		if (gear == 0) {
			glm::mat4 lowerConnector = glm::translate(base, glm::vec3((sprocketX + camEndX) * 0.5f, y, z));
			lowerConnector = glm::rotate(lowerConnector, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
			lowerConnector = glm::rotate(lowerConnector, crankAngle * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			lowerConnector = glm::scale(lowerConnector, glm::vec3(0.14f, std::abs(connectorLength), 0.14f));
			drawMesh(cylinderMesh, shader, view, projection, lowerConnector, camColor);
		}

		for (int tooth = 0; tooth < 12; ++tooth) {
			const float a = tooth * 2.0f * PI / 12.0f + wheelAngle;
			glm::mat4 gearTooth = glm::translate(base, glm::vec3(sprocketX, y + 0.45f * std::sin(a), z + 0.45f * std::cos(a)));
			gearTooth = glm::rotate(gearTooth, a, glm::vec3(1.0f, 0.0f, 0.0f));
			gearTooth = glm::scale(gearTooth, glm::vec3(0.08f, 0.08f, 0.05f));
			drawMesh(boxMesh, shader, view, projection, gearTooth, glm::vec4(0.08f, 0.08f, 0.08f, 1.0f));
		}
	}

	drawTimingChain(shader, view, projection);
}

void EngineSimulator::drawTimingChain(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const glm::mat4 base = glm::mat4(1.0f);
	const float chainX = -3.26f;
	const glm::vec3 bottom(chainX, -1.05f, 0.0f);
	const glm::vec3 topIntake(chainX, 2.25f, -0.46f);
	const glm::vec3 topExhaust(chainX, 2.25f, 0.46f);
	const float leftLength = glm::length(topIntake - bottom);
	const float topLength = glm::length(topExhaust - topIntake);
	const float rightLength = glm::length(bottom - topExhaust);
	const float totalLength = leftLength + topLength + rightLength;
	const int linkCount = 44;
	const float linkSpacing = totalLength / static_cast<float>(linkCount);
	const float travel = std::fmod(crankAngle * 0.018f, totalLength);

	for (int i = 0; i < linkCount; ++i) {
		float distance = std::fmod(i * linkSpacing + travel, totalLength);
		glm::vec3 start;
		glm::vec3 end;
		float local = distance;

		if (local < leftLength) {
			const float t = local / leftLength;
			start = bottom;
			end = topIntake;
			local = t;
		} else if (local < leftLength + topLength) {
			const float t = (local - leftLength) / topLength;
			start = topIntake;
			end = topExhaust;
			local = t;
		} else {
			const float t = (local - leftLength - topLength) / rightLength;
			start = topExhaust;
			end = bottom;
			local = t;
		}

		const glm::vec3 point = start + (end - start) * local;
		const glm::vec3 tangent = glm::normalize(end - start);
		const float angleX = std::atan2(tangent.z, tangent.y);

		glm::mat4 link = glm::translate(base, point);
		link = glm::rotate(link, angleX, glm::vec3(1.0f, 0.0f, 0.0f));
		link = glm::scale(link, glm::vec3(0.11f, 0.16f, 0.045f));
		const glm::vec4 chainColor = i % 2 == 0 ? glm::vec4(0.035f, 0.035f, 0.035f, 1.0f) : glm::vec4(0.12f, 0.12f, 0.11f, 1.0f);
		drawMesh(boxMesh, shader, view, projection, link, chainColor);
	}
}

void EngineSimulator::drawManifolds(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const glm::mat4 base = glm::mat4(1.0f);
	const auto alignOpenPipeBetween = [](const glm::vec3& start, const glm::vec3& end, float radius) {
		const glm::vec3 direction = end - start;
		const float length = glm::length(direction);
		if (length < 0.0001f) {
			return glm::mat4(1.0f);
		}

		const glm::vec3 pipeAxis = direction / length;
		const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
		glm::vec3 openingUp = worldUp - pipeAxis * glm::dot(worldUp, pipeAxis);
		if (glm::length(openingUp) < 0.0001f) {
			openingUp = glm::vec3(0.0f, 0.0f, -1.0f);
		}
		openingUp = glm::normalize(openingUp);

		const glm::vec3 shellDirection = -openingUp;
		const glm::vec3 localX = glm::normalize(glm::cross(pipeAxis, shellDirection));
		glm::mat4 orientation(1.0f);
		orientation[0] = glm::vec4(localX, 0.0f);
		orientation[1] = glm::vec4(pipeAxis, 0.0f);
		orientation[2] = glm::vec4(shellDirection, 0.0f);

		glm::mat4 model = glm::translate(glm::mat4(1.0f), (start + end) * 0.5f) * orientation;
		return glm::scale(model, glm::vec3(radius, length, radius));
	};
	const auto alignCylinderBetween = [](const glm::vec3& start, const glm::vec3& end, float radius) {
		const glm::vec3 direction = end - start;
		const float length = glm::length(direction);
		glm::mat4 model = glm::translate(glm::mat4(1.0f), (start + end) * 0.5f);
		if (length > 0.0001f) {
			const glm::vec3 up(0.0f, 1.0f, 0.0f);
			const glm::vec3 target = direction / length;
			const float dotValue = std::clamp(glm::dot(up, target), -1.0f, 1.0f);
			const float angle = std::acos(dotValue);
			const glm::vec3 axis = glm::cross(up, target);
			if (glm::length(axis) > 0.0001f) {
				model = glm::rotate(model, angle, glm::normalize(axis));
			}
		}
		return glm::scale(model, glm::vec3(radius, length, radius));
	};
	const auto pointOnCurve = [](const glm::vec3& start, const glm::vec3& control, const glm::vec3& end, float t) {
		const float inverse = 1.0f - t;
		return inverse * inverse * start + 2.0f * inverse * t * control + t * t * end;
	};
	const auto drawCurvedPipe = [&](const glm::vec3& start, const glm::vec3& control, const glm::vec3& end, float startRadius, float endRadius, const glm::vec4& color) {
		const int segments = 7;
		glm::vec3 previous = start;
		for (int segment = 1; segment <= segments; ++segment) {
			const float t = static_cast<float>(segment) / static_cast<float>(segments);
			const glm::vec3 current = pointOnCurve(start, control, end, t);
			const float radius = glm::mix(startRadius, endRadius, (t - 0.5f / static_cast<float>(segments)));
			drawMesh(
				halfCylinderMesh,
				shader,
				view,
				projection,
				alignOpenPipeBetween(previous, current, radius),
				color
			);
			previous = current;
		}
	};

	glm::mat4 intakeRail = alignOpenPipeBetween(
		glm::vec3(-3.0f, 1.72f, -1.10f),
		glm::vec3(3.0f, 1.72f, -1.10f),
		0.28f
	);
	drawMesh(intakeManifoldMesh, shader, view, projection, intakeRail, glm::vec4(0.34f, 0.48f, 0.53f, 1.0f));

	const int plenumMarkerCount = 14;
	for (int marker = 0; marker < plenumMarkerCount; ++marker) {
		const float markerOffset = static_cast<float>(marker) / static_cast<float>(plenumMarkerCount);
		const float travel = std::fmod(crankAngle / 720.0f * 0.85f + markerOffset, 1.0f);
		const float markerX = glm::mix(-2.82f, 2.82f, travel);
		const float wave = std::sin(travel * 4.0f * PI + static_cast<float>(marker) * 0.35f);
		const glm::vec3 markerPosition(markerX, 1.66f + wave * 0.035f, -1.10f);

		glm::mat4 plenumMarker = glm::translate(base, markerPosition);
		plenumMarker = glm::scale(plenumMarker, glm::vec3(0.070f, 0.032f, 0.055f));
		drawMesh(
			cylinderMesh,
			shader,
			view,
			projection,
			plenumMarker,
			glm::vec4(0.26f, 0.66f, 0.82f, 1.0f)
		);
	}

	glm::mat4 exhaustRail = alignOpenPipeBetween(
		glm::vec3(-3.0f, 1.60f, 1.10f),
		glm::vec3(3.0f, 1.60f, 1.10f),
		0.30f
	);
	drawMesh(exhaustManifoldMesh, shader, view, projection, exhaustRail, glm::vec4(0.48f, 0.36f, 0.27f, 1.0f));

	const glm::vec3 fuelRailStart(-2.85f, 1.82f, -0.52f);
	const glm::vec3 fuelRailEnd(2.85f, 1.82f, -0.52f);
	glm::mat4 fuelRail = alignOpenPipeBetween(fuelRailStart, fuelRailEnd, 0.095f);
	drawMesh(halfCylinderMesh, shader, view, projection, fuelRail, glm::vec4(0.64f, 0.50f, 0.20f, 1.0f));

	for (int marker = 0; marker < 12; ++marker) {
		const float offset = static_cast<float>(marker) / 12.0f;
		const float travel = std::fmod(crankAngle / 720.0f * 0.75f + offset, 1.0f);
		const glm::vec3 markerPosition = glm::mix(fuelRailStart, fuelRailEnd, travel)
			+ glm::vec3(0.0f, -0.035f, 0.0f);
		glm::mat4 fuelMarker = glm::translate(base, markerPosition);
		fuelMarker = glm::scale(fuelMarker, glm::vec3(0.045f, 0.022f, 0.035f));
		drawMesh(cylinderMesh, shader, view, projection, fuelMarker, glm::vec4(0.96f, 0.67f, 0.12f, 1.0f));
	}

	const int exhaustMarkerCount = 12;
	for (int marker = 0; marker < exhaustMarkerCount; ++marker) {
		const float markerOffset = static_cast<float>(marker) / static_cast<float>(exhaustMarkerCount);
		const float travel = std::fmod(crankAngle / 720.0f * 0.62f + markerOffset, 1.0f);
		const float markerX = glm::mix(-2.82f, 2.82f, travel);
		const float wave = std::sin(travel * 3.0f * PI + static_cast<float>(marker) * 0.42f);
		const glm::vec3 markerPosition(markerX, 1.54f + wave * 0.040f, 1.10f);

		glm::mat4 exhaustPlenumMarker = glm::translate(base, markerPosition);
		exhaustPlenumMarker = glm::scale(exhaustPlenumMarker, glm::vec3(0.075f, 0.035f, 0.060f));
		drawMesh(
			cylinderMesh,
			shader,
			view,
			projection,
			exhaustPlenumMarker,
			glm::vec4(0.52f, 0.24f, 0.14f, 1.0f)
		);
	}

	for (int i = 0; i < 4; ++i) {
		const float x = -2.1f + i * 1.4f;
		const float cycleOffsets[4] = {360.0f, 540.0f, 180.0f, 0.0f};
		float cylinderPhase = std::fmod(crankAngle + cycleOffsets[i], 720.0f);
		if (cylinderPhase < 0.0f) {
			cylinderPhase += 720.0f;
		}
		float intakeFlow = 0.0f;
		float exhaustFlow = 0.0f;
		float flowProgress = 0.0f;
		if (cylinderPhase < 180.0f) {
			flowProgress = cylinderPhase / 180.0f;
			intakeFlow = std::sin(PI * flowProgress);
			intakeFlow *= intakeFlow;
		} else if (cylinderPhase >= 540.0f) {
			flowProgress = (cylinderPhase - 540.0f) / 180.0f;
			exhaustFlow = std::sin(PI * flowProgress);
			exhaustFlow *= exhaustFlow;
		}

		const glm::vec3 intakeMerge(x, 1.52f, -0.61f);
		const glm::vec3 intakeThroat(x, 1.57f, -0.68f);
		const glm::vec3 intakeTrunkControl(x, 1.61f, -0.83f);
		const glm::vec3 intakeJunction(x, 1.70f, -0.96f);
		const glm::vec3 exhaustMerge(x, 1.49f, 0.61f);
		const glm::vec3 exhaustThroat(x, 1.53f, 0.68f);
		const glm::vec3 exhaustTrunkControl(x, 1.53f, 0.83f);
		const glm::vec3 exhaustJunction(x, 1.58f, 0.96f);

		const glm::vec3 injectorAxis = glm::normalize(glm::vec3(0.0f, 0.625f, -0.18f));
		const glm::vec3 injectorFuelPort = glm::vec3(x, 1.285f, 0.0f)
			+ injectorAxis * 0.45f
			+ glm::vec3(0.0f, 0.0f, -0.15f);
		const glm::vec3 fuelRailTap(x, 1.82f, -0.52f);
		const glm::vec3 fuelLineElbow(x, injectorFuelPort.y + 0.03f, -0.43f);

		glm::mat4 fuelDrop = alignCylinderBetween(fuelRailTap, fuelLineElbow, 0.030f);
		drawMesh(cylinderMesh, shader, view, projection, fuelDrop, glm::vec4(0.70f, 0.61f, 0.38f, 1.0f));

		glm::mat4 injectorFeed = alignCylinderBetween(fuelLineElbow, injectorFuelPort, 0.030f);
		drawMesh(cylinderMesh, shader, view, projection, injectorFeed, glm::vec4(0.70f, 0.61f, 0.38f, 1.0f));

		for (int marker = 0; marker < 2; ++marker) {
			const float local = std::fmod(
				crankAngle / 720.0f * 1.15f + static_cast<float>(marker) * 0.5f,
				1.0f
			);
			const glm::vec3 fuelPosition = local < 0.55f
				? glm::mix(fuelRailTap, fuelLineElbow, local / 0.55f)
				: glm::mix(fuelLineElbow, injectorFuelPort, (local - 0.55f) / 0.45f);
			glm::mat4 branchMarker = glm::translate(base, fuelPosition);
			branchMarker = glm::scale(branchMarker, glm::vec3(0.036f, 0.024f, 0.036f));
			drawMesh(cylinderMesh, shader, view, projection, branchMarker, glm::vec4(0.98f, 0.70f, 0.14f, 1.0f));
		}

		for (float localX : {-0.20f, 0.20f}) {
			const glm::vec3 intakeSeat(x + localX, 1.315f, -0.20f);
			const glm::vec3 intakeBranchControl(x + localX * 0.72f, 1.40f, -0.43f);
			const glm::vec3 intakeBranchEnd(x + localX * 0.22f, 1.505f, -0.59f);
			drawCurvedPipe(
				intakeSeat,
				intakeBranchControl,
				intakeBranchEnd,
				0.105f,
				0.175f,
				glm::vec4(0.39f, 0.55f, 0.60f, 1.0f)
			);

			const glm::vec3 exhaustSeat(x + localX, 1.315f, 0.20f);
			const glm::vec3 exhaustBranchControl(x + localX * 0.72f, 1.39f, 0.43f);
			const glm::vec3 exhaustBranchEnd(x + localX * 0.22f, 1.475f, 0.59f);
			drawCurvedPipe(
				exhaustSeat,
				exhaustBranchControl,
				exhaustBranchEnd,
				0.110f,
				0.185f,
				glm::vec4(0.55f, 0.40f, 0.28f, 1.0f)
			);
		}

		drawMesh(
			halfCylinderMesh,
			shader,
			view,
			projection,
			alignOpenPipeBetween(intakeMerge, intakeThroat, 0.235f),
			glm::vec4(0.36f, 0.51f, 0.56f, 1.0f)
		);
		drawCurvedPipe(
			intakeThroat,
			intakeTrunkControl,
			intakeJunction,
			0.23f,
			0.28f,
			glm::vec4(0.34f, 0.48f, 0.53f, 1.0f)
		);
		drawMesh(
			halfCylinderMesh,
			shader,
			view,
			projection,
			alignOpenPipeBetween(exhaustMerge, exhaustThroat, 0.245f),
			glm::vec4(0.51f, 0.38f, 0.27f, 1.0f)
		);
		drawCurvedPipe(
			exhaustThroat,
			exhaustTrunkControl,
			exhaustJunction,
			0.24f,
			0.29f,
			glm::vec4(0.48f, 0.35f, 0.25f, 1.0f)
		);

		glm::mat4 intakeCollar = alignOpenPipeBetween(
			glm::vec3(x, 1.70f, -0.99f),
			glm::vec3(x, 1.70f, -0.93f),
			0.31f
		);
		drawMesh(valveSeatMesh, shader, view, projection, intakeCollar, glm::vec4(0.48f, 0.59f, 0.61f, 1.0f));

		glm::mat4 exhaustCollar = alignOpenPipeBetween(
			glm::vec3(x, 1.58f, 0.93f),
			glm::vec3(x, 1.58f, 0.99f),
			0.32f
		);
		drawMesh(valveSeatMesh, shader, view, projection, exhaustCollar, glm::vec4(0.58f, 0.46f, 0.34f, 1.0f));

		for (int marker = 0; marker < 4; ++marker) {
			const float markerOffset = static_cast<float>(marker) / 4.0f;
			const float branchSide = marker % 2 == 0 ? -0.20f : 0.20f;
			if (intakeFlow > 0.01f) {
				const float travel = std::fmod(flowProgress * 1.6f + markerOffset, 1.0f);
				glm::vec3 markerPosition;
				if (travel < 0.48f) {
					markerPosition = pointOnCurve(intakeJunction, intakeTrunkControl, intakeMerge, travel / 0.48f);
				} else {
					const glm::vec3 intakeSeat(x + branchSide, 1.315f, -0.20f);
					const glm::vec3 intakeBranchControl(x + branchSide * 0.72f, 1.40f, -0.43f);
					markerPosition = pointOnCurve(intakeMerge, intakeBranchControl, intakeSeat, (travel - 0.48f) / 0.52f);
				}
				const float markerSize = 0.035f + intakeFlow * 0.040f;
				glm::mat4 intakeMarker = glm::translate(base, markerPosition);
				intakeMarker = glm::scale(intakeMarker, glm::vec3(markerSize, markerSize * 0.55f, markerSize));
				drawMesh(cylinderMesh, shader, view, projection, intakeMarker, glm::vec4(0.28f, 0.68f, 0.82f, 1.0f));
			}
			if (exhaustFlow > 0.01f) {
				const float travel = std::fmod(flowProgress * 1.8f + markerOffset, 1.0f);
				glm::vec3 markerPosition;
				if (travel < 0.52f) {
					const glm::vec3 exhaustSeat(x + branchSide, 1.315f, 0.20f);
					const glm::vec3 exhaustBranchControl(x + branchSide * 0.72f, 1.39f, 0.43f);
					markerPosition = pointOnCurve(exhaustSeat, exhaustBranchControl, exhaustMerge, travel / 0.52f);
				} else {
					markerPosition = pointOnCurve(exhaustMerge, exhaustTrunkControl, exhaustJunction, (travel - 0.52f) / 0.48f);
				}
				const float markerSize = 0.040f + exhaustFlow * 0.045f;
				glm::mat4 exhaustMarker = glm::translate(base, markerPosition);
				exhaustMarker = glm::scale(exhaustMarker, glm::vec3(markerSize, markerSize * 0.55f, markerSize));
				drawMesh(cylinderMesh, shader, view, projection, exhaustMarker, glm::vec4(0.68f, 0.30f, 0.16f, 1.0f));
			}
		}

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
	glUniform3f(shader->u("dirLightDirView"), -0.35f, -0.85f, -0.35f);
	glUniform3f(shader->u("pointLightPosView"), 2.2f, 3.2f, 3.2f);
	glUniform1f(shader->u("shininess"), 48.0f);
	glUniform1f(shader->u("specularStrength"), 0.42f);
	const unsigned int texture = chooseTexture(color);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(shader->u("texture0"), 0);
	glUniform1i(shader->u("useTexture"), texture != 0 ? 1 : 0);
	mesh.draw();
	glBindTexture(GL_TEXTURE_2D, 0);
}
