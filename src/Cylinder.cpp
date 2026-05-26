#define GLM_FORCE_RADIANS

#include "Cylinder.h"

#include <GL/glew.h>
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EngineMeshes.h"
#include "constants.h"
#include "shaderprogram.h"

Cylinder::Cylinder(int index, float xPosition, float phaseOffset)
	: index(index),
	  xPosition(xPosition),
	  phaseOffset(phaseOffset),
	  currentMetalTexture(0),
	  currentDarkMetalTexture(0),
	  currentRubberTexture(0) {
}

void Cylinder::draw(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, const EngineMesh& halfCylinderMesh, const EngineMesh& halfDiskMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float crankAngle, unsigned int metalTexture, unsigned int darkMetalTexture, unsigned int rubberTexture) const {
	currentMetalTexture = metalTexture;
	currentDarkMetalTexture = darkMetalTexture;
	currentRubberTexture = rubberTexture;

	const float currentPhase = phase(crankAngle);
	const StrokeType stroke = getStroke(crankAngle);

	const float crankRad = (crankAngle + phaseOffset) * PI / 180.0f;
	const glm::vec3 crankPin(xPosition, -1.05f + 0.32f * std::sin(crankRad), 0.32f * std::cos(crankRad));
	const float pistonPinY = pistonPinHeight(crankRad);
	const float pistonY = pistonPinY + 0.38f;
	const glm::vec3 pistonPin(xPosition, pistonPinY, 0.0f);

	glm::mat4 sleeve = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 0.23f, 0.0f));
	sleeve = glm::scale(sleeve, glm::vec3(0.74f, 1.96f, 0.74f));
	drawMesh(halfCylinderMesh, shader, view, projection, sleeve, glm::vec4(0.36f, 0.39f, 0.40f, 1.0f));

	glm::mat4 sleeveLiner = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 0.23f, 0.0f));
	sleeveLiner = glm::scale(sleeveLiner, glm::vec3(0.64f, 1.98f, 0.64f));
	drawMesh(halfCylinderMesh, shader, view, projection, sleeveLiner, glm::vec4(0.68f, 0.70f, 0.68f, 1.0f));
	drawCutawayEdges(boxMesh, shader, view, projection);

	glm::mat4 chamber = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.10f, -0.01f));
	chamber = glm::scale(chamber, glm::vec3(0.62f, 0.24f, 0.62f));
	drawMesh(halfCylinderMesh, shader, view, projection, chamber, strokeColor(stroke));

	glm::mat4 piston = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, pistonPinY + 0.08f, 0.0f));
	piston = glm::scale(piston, glm::vec3(0.58f, 0.28f, 0.58f));
	drawMesh(cylinderMesh, shader, view, projection, piston, glm::vec4(0.86f, 0.86f, 0.80f, 1.0f));
	drawPistonDetails(cylinderMesh, shader, view, projection, pistonPinY);

	const glm::mat4 rod = alignCylinderBetween(crankPin, pistonPin, 0.10f);
	drawMesh(cylinderMesh, shader, view, projection, rod, glm::vec4(0.70f, 0.70f, 0.64f, 1.0f));

	glm::mat4 bigEnd = glm::translate(glm::mat4(1.0f), crankPin);
	bigEnd = glm::rotate(bigEnd, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
	bigEnd = glm::scale(bigEnd, glm::vec3(0.21f, 0.12f, 0.21f));
	drawMesh(cylinderMesh, shader, view, projection, bigEnd, glm::vec4(0.54f, 0.55f, 0.50f, 1.0f));

	drawValveSeats(cylinderMesh, halfDiskMesh, shader, view, projection);

	const float intakeLift = valveOffset(currentPhase, StrokeType::Intake);
	const float exhaustLift = valveOffset(currentPhase, StrokeType::Exhaust);
	drawValvePair(cylinderMesh, shader, view, projection, -0.25f, intakeLift, glm::vec4(0.66f, 0.72f, 0.74f, 1.0f));
	drawValvePair(cylinderMesh, shader, view, projection, 0.25f, exhaustLift, glm::vec4(0.72f, 0.67f, 0.60f, 1.0f));
	drawInjector(cylinderMesh, shader, view, projection);
	drawHeadFasteners(cylinderMesh, shader, view, projection);
	drawStrokeEffects(boxMesh, cylinderMesh, shader, view, projection, stroke, currentPhase, pistonY);
}

StrokeType Cylinder::getStroke(float crankAngle) const {
	const float p = phase(crankAngle);
	if (p < 180.0f) return StrokeType::Intake;
	if (p < 360.0f) return StrokeType::Compression;
	if (p < 540.0f) return StrokeType::Power;
	return StrokeType::Exhaust;
}

float Cylinder::phase(float crankAngle) const {
	float p = std::fmod(crankAngle + phaseOffset, 720.0f);
	if (p < 0.0f) {
		p += 720.0f;
	}
	return p;
}

float Cylinder::pistonPinHeight(float crankRadians) const {
	const float mainY = -1.05f;
	const float throwRadius = 0.32f;
	const float rodLength = 1.28f;
	const float crankY = mainY + throwRadius * std::sin(crankRadians);
	const float crankZ = throwRadius * std::cos(crankRadians);
	const float verticalReach = std::sqrt(std::max(rodLength * rodLength - crankZ * crankZ, 0.0f));
	return crankY + verticalReach;
}

float Cylinder::valveOffset(float phaseDegrees, StrokeType activeStroke) const {
	float start = 0.0f;
	float end = 0.0f;
	if (activeStroke == StrokeType::Intake) {
		start = 0.0f;
		end = 180.0f;
	} else if (activeStroke == StrokeType::Exhaust) {
		start = 540.0f;
		end = 720.0f;
	} else {
		return 0.0f;
	}

	if (phaseDegrees < start || phaseDegrees >= end) {
		return 0.0f;
	}

	const auto smoothStep = [](float edge0, float edge1, float value) {
		const float t = std::clamp((value - edge0) / (edge1 - edge0), 0.0f, 1.0f);
		return t * t * (3.0f - 2.0f * t);
	};

	const float local = (phaseDegrees - start) / (end - start);
	const float opening = smoothStep(0.05f, 0.24f, local);
	const float closing = 1.0f - smoothStep(0.76f, 0.95f, local);
	const float liftCurve = std::min(opening, closing);
	const float maxLift = 0.22f;
	return maxLift * liftCurve;
}

glm::mat4 Cylinder::alignBoxBetween(const glm::vec3& start, const glm::vec3& end, float thickness) const {
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

	model = glm::scale(model, glm::vec3(thickness, length, thickness));
	return model;
}

glm::mat4 Cylinder::alignCylinderBetween(const glm::vec3& start, const glm::vec3& end, float radius) const {
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

	model = glm::scale(model, glm::vec3(radius, length, radius));
	return model;
}

glm::vec4 Cylinder::strokeColor(StrokeType stroke) const {
	switch (stroke) {
	case StrokeType::Intake:
		return glm::vec4(0.16f, 0.34f, 0.66f, 1.0f);
	case StrokeType::Compression:
		return glm::vec4(0.48f, 0.51f, 0.52f, 1.0f);
	case StrokeType::Power:
		return glm::vec4(0.82f, 0.42f, 0.12f, 1.0f);
	case StrokeType::Exhaust:
		return glm::vec4(0.66f, 0.18f, 0.14f, 1.0f);
	default:
		return glm::vec4(0.45f, 0.49f, 0.51f, 1.0f);
	}
}

unsigned int Cylinder::chooseTexture(const glm::vec4& color) const {
	const float brightness = (color.r + color.g + color.b) / 3.0f;
	if (brightness < 0.20f) {
		return currentRubberTexture;
	}
	if (brightness < 0.42f) {
		return currentDarkMetalTexture;
	}
	return currentMetalTexture;
}

void Cylinder::drawValvePair(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float z, float lift, const glm::vec4& color) const {
	const float xOffsets[2] = {-0.20f, 0.20f};
	const float camZ = z < 0.0f ? -0.46f : 0.46f;
	const float valveZ = z * 0.72f;
	const float valveTopZ = z * 1.03f;
	const float valveTravel = -lift;
	for (float localX : xOffsets) {
		const float valveX = xPosition + localX;
		const glm::vec3 valveHeadCenter(valveX, 1.312f + valveTravel, valveZ);
		const glm::vec3 lowerStem(valveX, 1.350f + valveTravel, valveZ);
		const glm::vec3 upperStem(valveX, 1.910f + valveTravel, valveTopZ);
		const glm::vec3 springBase = glm::mix(lowerStem, upperStem, 0.48f);
		const glm::vec3 springTop = glm::mix(lowerStem, upperStem, 0.92f);

		glm::mat4 stem = alignCylinderBetween(lowerStem, upperStem, 0.034f);
		drawMesh(cylinderMesh, shader, view, projection, stem, color);

		glm::mat4 head = glm::translate(glm::mat4(1.0f), valveHeadCenter);
		head = glm::scale(head, glm::vec3(0.155f, 0.035f, 0.155f));
		drawMesh(cylinderMesh, shader, view, projection, head, color);

		glm::mat4 lowerRetainer = glm::translate(glm::mat4(1.0f), springBase);
		lowerRetainer = glm::scale(lowerRetainer, glm::vec3(0.120f, 0.026f, 0.120f));
		drawMesh(cylinderMesh, shader, view, projection, lowerRetainer, glm::vec4(0.72f, 0.72f, 0.66f, 1.0f));

		glm::mat4 retainer = glm::translate(glm::mat4(1.0f), springTop);
		retainer = glm::scale(retainer, glm::vec3(0.120f, 0.032f, 0.120f));
		drawMesh(cylinderMesh, shader, view, projection, retainer, glm::vec4(0.78f, 0.78f, 0.72f, 1.0f));

		drawSpring(cylinderMesh, shader, view, projection, springBase, springTop, lift);

		glm::mat4 tappet = alignCylinderBetween(glm::vec3(valveX, 1.945f + valveTravel, valveTopZ), glm::vec3(valveX, 2.16f + valveTravel * 0.35f, camZ), 0.026f);
		drawMesh(cylinderMesh, shader, view, projection, tappet, glm::vec4(0.64f, 0.66f, 0.60f, 1.0f));

		glm::mat4 roller = glm::translate(glm::mat4(1.0f), glm::vec3(valveX, 2.17f + valveTravel * 0.35f, camZ));
		roller = glm::rotate(roller, PI * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
		roller = glm::scale(roller, glm::vec3(0.090f, 0.045f, 0.090f));
		drawMesh(cylinderMesh, shader, view, projection, roller, glm::vec4(0.76f, 0.76f, 0.70f, 1.0f));
	}
}

void Cylinder::drawInjector(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	glm::mat4 injector = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.95f, 0.0f));
	injector = glm::scale(injector, glm::vec3(0.08f, 0.42f, 0.08f));
	drawMesh(cylinderMesh, shader, view, projection, injector, glm::vec4(0.82f, 0.66f, 0.18f, 1.0f));

	glm::mat4 tip = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.63f, 0.0f));
	tip = glm::scale(tip, glm::vec3(0.12f, 0.08f, 0.12f));
	drawMesh(cylinderMesh, shader, view, projection, tip, glm::vec4(0.92f, 0.82f, 0.42f, 1.0f));
}

void Cylinder::drawPistonDetails(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float pistonPinY) const {
	glm::mat4 crown = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, pistonPinY + 0.245f, 0.0f));
	crown = glm::scale(crown, glm::vec3(0.51f, 0.035f, 0.51f));
	drawMesh(cylinderMesh, shader, view, projection, crown, glm::vec4(0.95f, 0.95f, 0.88f, 1.0f));

	for (int ring = 0; ring < 3; ++ring) {
		glm::mat4 pistonRing = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, pistonPinY + 0.185f - ring * 0.052f, 0.0f));
		pistonRing = glm::scale(pistonRing, glm::vec3(0.595f, 0.025f, 0.595f));
		drawMesh(cylinderMesh, shader, view, projection, pistonRing, glm::vec4(0.08f, 0.09f, 0.09f, 1.0f));
	}

	for (float z : {-0.24f, 0.24f}) {
		glm::mat4 pinBoss = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, pistonPinY, z));
		pinBoss = glm::rotate(pinBoss, PI * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
		pinBoss = glm::scale(pinBoss, glm::vec3(0.12f, 0.08f, 0.12f));
		drawMesh(cylinderMesh, shader, view, projection, pinBoss, glm::vec4(0.74f, 0.74f, 0.68f, 1.0f));
	}

	glm::mat4 lowerSkirt = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, pistonPinY - 0.055f, 0.0f));
	lowerSkirt = glm::scale(lowerSkirt, glm::vec3(0.50f, 0.11f, 0.50f));
	drawMesh(cylinderMesh, shader, view, projection, lowerSkirt, glm::vec4(0.72f, 0.72f, 0.66f, 1.0f));
}

void Cylinder::drawHeadFasteners(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	(void)cylinderMesh;
	(void)shader;
	(void)view;
	(void)projection;
}

void Cylinder::drawValveSeats(const EngineMesh& cylinderMesh, const EngineMesh& halfDiskMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	(void)halfDiskMesh;

	glm::mat4 headPlate = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.265f, 0.0f));
	headPlate = glm::scale(headPlate, glm::vec3(0.88f, 0.045f, 0.88f));
	drawMesh(cylinderMesh, shader, view, projection, headPlate, glm::vec4(0.72f, 0.73f, 0.68f, 1.0f));

	const float zRows[2] = {-0.18f, 0.18f};
	const float xRows[2] = {-0.20f, 0.20f};
	for (float localZ : zRows) {
		for (float localX : xRows) {
			glm::mat4 seatRing = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition + localX, 1.294f, localZ));
			seatRing = glm::scale(seatRing, glm::vec3(0.175f, 0.008f, 0.175f));
			drawMesh(cylinderMesh, shader, view, projection, seatRing, glm::vec4(0.50f, 0.52f, 0.48f, 1.0f));

			glm::mat4 valveOpening = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition + localX, 1.300f, localZ));
			valveOpening = glm::scale(valveOpening, glm::vec3(0.130f, 0.010f, 0.130f));
			drawMesh(cylinderMesh, shader, view, projection, valveOpening, glm::vec4(0.045f, 0.048f, 0.047f, 1.0f));
		}
	}

	glm::mat4 injectorHole = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.300f, 0.0f));
	injectorHole = glm::scale(injectorHole, glm::vec3(0.090f, 0.010f, 0.090f));
	drawMesh(cylinderMesh, shader, view, projection, injectorHole, glm::vec4(0.08f, 0.075f, 0.055f, 1.0f));
}

void Cylinder::drawCutawayEdges(const EngineMesh& boxMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	(void)boxMesh;
	(void)shader;
	(void)view;
	(void)projection;
}

void Cylinder::drawStrokeEffects(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, StrokeType stroke, float currentPhase, float pistonY) const {
	(void)boxMesh;
	(void)cylinderMesh;
	(void)shader;
	(void)view;
	(void)projection;
	(void)stroke;
	(void)currentPhase;
	(void)pistonY;
}

void Cylinder::drawSpring(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& bottom, const glm::vec3& top, float lift) const {
	const glm::vec3 compressedTop = glm::mix(top, bottom, lift * 0.55f);
	const glm::vec3 axis = compressedTop - bottom;
	const glm::vec3 direction = glm::length(axis) > 0.0001f ? glm::normalize(axis) : glm::vec3(0.0f, 1.0f, 0.0f);
	const glm::vec3 up(0.0f, 1.0f, 0.0f);
	const float dotValue = std::clamp(glm::dot(up, direction), -1.0f, 1.0f);
	const float angle = std::acos(dotValue);
	const glm::vec3 rotationAxis = glm::cross(up, direction);

	for (int ring = 0; ring < 6; ++ring) {
		const float t = static_cast<float>(ring) / 5.0f;
		const glm::vec3 center = glm::mix(bottom, compressedTop, t);
		glm::mat4 springRing = glm::translate(glm::mat4(1.0f), center);
		if (glm::length(rotationAxis) > 0.0001f) {
			springRing = glm::rotate(springRing, angle, glm::normalize(rotationAxis));
		}
		springRing = glm::scale(springRing, glm::vec3(0.096f, 0.010f, 0.096f));
		drawMesh(cylinderMesh, shader, view, projection, springRing, glm::vec4(0.12f, 0.12f, 0.11f, 1.0f));

		glm::mat4 innerGap = glm::translate(glm::mat4(1.0f), center + direction * 0.001f);
		if (glm::length(rotationAxis) > 0.0001f) {
			innerGap = glm::rotate(innerGap, angle, glm::normalize(rotationAxis));
		}
		innerGap = glm::scale(innerGap, glm::vec3(0.064f, 0.012f, 0.064f));
		drawMesh(cylinderMesh, shader, view, projection, innerGap, glm::vec4(0.58f, 0.60f, 0.56f, 1.0f));
	}
}

void Cylinder::drawMesh(const EngineMesh& mesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const glm::vec4& color) const {
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
