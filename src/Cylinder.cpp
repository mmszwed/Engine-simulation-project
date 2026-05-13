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

void Cylinder::draw(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, const EngineMesh& halfCylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float crankAngle, unsigned int metalTexture, unsigned int darkMetalTexture, unsigned int rubberTexture) const {
	currentMetalTexture = metalTexture;
	currentDarkMetalTexture = darkMetalTexture;
	currentRubberTexture = rubberTexture;

	const float currentPhase = phase(crankAngle);
	const StrokeType stroke = getStroke(crankAngle);
	const float pistonY = pistonOffset(currentPhase);

	const float crankRad = (crankAngle + phaseOffset) * PI / 180.0f;
	const glm::vec3 crankPin(xPosition, -1.05f + 0.32f * std::sin(crankRad), 0.32f * std::cos(crankRad));
	const glm::vec3 pistonPin(xPosition, -0.28f + pistonY, 0.0f);

	glm::mat4 sleeve = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 0.55f, 0.0f));
	sleeve = glm::scale(sleeve, glm::vec3(0.66f, 1.70f, 0.66f));
	drawMesh(halfCylinderMesh, shader, view, projection, sleeve, glm::vec4(0.36f, 0.39f, 0.40f, 1.0f));

	glm::mat4 sleeveLiner = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 0.56f, 0.0f));
	sleeveLiner = glm::scale(sleeveLiner, glm::vec3(0.56f, 1.72f, 0.56f));
	drawMesh(halfCylinderMesh, shader, view, projection, sleeveLiner, glm::vec4(0.68f, 0.70f, 0.68f, 1.0f));
	drawCutawayEdges(boxMesh, shader, view, projection);

	glm::mat4 chamber = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.15f, -0.01f));
	chamber = glm::scale(chamber, glm::vec3(0.54f, 0.40f, 0.54f));
	drawMesh(halfCylinderMesh, shader, view, projection, chamber, strokeColor(stroke));

	glm::mat4 piston = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, -0.28f + pistonY, 0.0f));
	piston = glm::scale(piston, glm::vec3(0.50f, 0.26f, 0.50f));
	drawMesh(cylinderMesh, shader, view, projection, piston, glm::vec4(0.86f, 0.86f, 0.80f, 1.0f));
	drawPistonDetails(cylinderMesh, shader, view, projection, pistonY);

	const glm::mat4 rod = alignCylinderBetween(crankPin, pistonPin, 0.10f);
	drawMesh(cylinderMesh, shader, view, projection, rod, glm::vec4(0.70f, 0.70f, 0.64f, 1.0f));

	glm::mat4 crankPinMarker = glm::translate(glm::mat4(1.0f), crankPin);
	crankPinMarker = glm::scale(crankPinMarker, glm::vec3(0.18f, 0.18f, 0.18f));
	drawMesh(cylinderMesh, shader, view, projection, crankPinMarker, glm::vec4(0.18f, 0.20f, 0.21f, 1.0f));

	const float intakeLift = valveOffset(stroke, StrokeType::Intake);
	const float exhaustLift = valveOffset(stroke, StrokeType::Exhaust);

	drawValvePair(boxMesh, cylinderMesh, shader, view, projection, -0.25f, intakeLift, glm::vec4(0.66f, 0.72f, 0.74f, 1.0f));
	drawValvePair(boxMesh, cylinderMesh, shader, view, projection, 0.25f, exhaustLift, glm::vec4(0.72f, 0.67f, 0.60f, 1.0f));
	drawInjector(cylinderMesh, shader, view, projection);
	drawValveSeats(cylinderMesh, shader, view, projection);
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

float Cylinder::pistonOffset(float phaseDegrees) const {
	const float phaseRad = phaseDegrees * PI / 180.0f;
	return 0.95f * 0.5f * (1.0f + std::cos(phaseRad));
}

float Cylinder::valveOffset(StrokeType stroke, StrokeType activeStroke) const {
	return stroke == activeStroke ? 0.22f : 0.0f;
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

void Cylinder::drawValvePair(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float z, float lift, const glm::vec4& color) const {
	const float xOffsets[2] = {-0.20f, 0.20f};
	const float camZ = z < 0.0f ? -0.46f : 0.46f;
	for (float localX : xOffsets) {
		const float valveX = xPosition + localX;
		glm::mat4 stem = glm::translate(glm::mat4(1.0f), glm::vec3(valveX, 1.66f + lift, z));
		stem = glm::rotate(stem, z < 0.0f ? -0.18f : 0.18f, glm::vec3(1.0f, 0.0f, 0.0f));
		stem = glm::scale(stem, glm::vec3(0.055f, 0.55f, 0.055f));
		drawMesh(cylinderMesh, shader, view, projection, stem, color);

		glm::mat4 head = glm::translate(glm::mat4(1.0f), glm::vec3(valveX, 1.30f + lift, z * 0.72f));
		head = glm::scale(head, glm::vec3(0.18f, 0.06f, 0.18f));
		drawMesh(cylinderMesh, shader, view, projection, head, color);

		glm::mat4 retainer = glm::translate(glm::mat4(1.0f), glm::vec3(valveX, 1.96f + lift, z));
		retainer = glm::scale(retainer, glm::vec3(0.15f, 0.05f, 0.15f));
		drawMesh(cylinderMesh, shader, view, projection, retainer, glm::vec4(0.78f, 0.78f, 0.72f, 1.0f));

		drawSpring(cylinderMesh, shader, view, projection, valveX, z, lift);

		glm::mat4 tappet = alignCylinderBetween(glm::vec3(valveX, 2.00f + lift, z), glm::vec3(valveX, 2.24f, camZ), 0.035f);
		drawMesh(cylinderMesh, shader, view, projection, tappet, glm::vec4(0.48f, 0.50f, 0.48f, 1.0f));

		glm::mat4 camFollower = glm::translate(glm::mat4(1.0f), glm::vec3(valveX, 2.24f, camZ));
		camFollower = glm::scale(camFollower, glm::vec3(0.11f, 0.055f, 0.11f));
		drawMesh(cylinderMesh, shader, view, projection, camFollower, glm::vec4(0.76f, 0.76f, 0.70f, 1.0f));

		glm::mat4 rocker = glm::translate(glm::mat4(1.0f), glm::vec3(valveX, 2.12f + lift * 0.45f, z));
		rocker = glm::rotate(rocker, localX > 0.0f ? 0.25f : -0.25f, glm::vec3(0.0f, 0.0f, 1.0f));
		rocker = glm::scale(rocker, glm::vec3(0.32f, 0.06f, 0.08f));
		drawMesh(boxMesh, shader, view, projection, rocker, glm::vec4(0.50f, 0.52f, 0.50f, 1.0f));
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

void Cylinder::drawPistonDetails(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float pistonY) const {
	for (int ring = 0; ring < 3; ++ring) {
		glm::mat4 pistonRing = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, -0.15f + pistonY - ring * 0.075f, 0.0f));
		pistonRing = glm::scale(pistonRing, glm::vec3(0.515f, 0.025f, 0.515f));
		drawMesh(cylinderMesh, shader, view, projection, pistonRing, glm::vec4(0.08f, 0.09f, 0.09f, 1.0f));
	}

	glm::mat4 crown = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, -0.10f + pistonY, 0.0f));
	crown = glm::scale(crown, glm::vec3(0.38f, 0.035f, 0.38f));
	drawMesh(cylinderMesh, shader, view, projection, crown, glm::vec4(0.94f, 0.94f, 0.88f, 1.0f));
}

void Cylinder::drawHeadFasteners(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const float xOffsets[2] = {-0.44f, 0.44f};
	const float zOffsets[2] = {-0.44f, 0.44f};
	for (float localX : xOffsets) {
		for (float localZ : zOffsets) {
			glm::mat4 bolt = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition + localX, 1.38f, localZ));
			bolt = glm::scale(bolt, glm::vec3(0.07f, 0.10f, 0.07f));
			drawMesh(cylinderMesh, shader, view, projection, bolt, glm::vec4(0.18f, 0.19f, 0.19f, 1.0f));
		}
	}
}

void Cylinder::drawValveSeats(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const float zRows[2] = {-0.18f, 0.18f};
	const float xRows[2] = {-0.20f, 0.20f};
	for (float localZ : zRows) {
		for (float localX : xRows) {
			glm::mat4 seat = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition + localX, 1.28f, localZ));
			seat = glm::scale(seat, glm::vec3(0.22f, 0.045f, 0.22f));
			drawMesh(cylinderMesh, shader, view, projection, seat, glm::vec4(0.24f, 0.25f, 0.24f, 1.0f));
		}
	}
}

void Cylinder::drawCutawayEdges(const EngineMesh& boxMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	const glm::vec4 edgeColor(0.14f, 0.45f, 0.62f, 1.0f);
	const float edgeX[2] = {-0.34f, 0.34f};
	for (float localX : edgeX) {
		glm::mat4 verticalEdge = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition + localX, 0.56f, -0.015f));
		verticalEdge = glm::scale(verticalEdge, glm::vec3(0.045f, 1.74f, 0.055f));
		drawMesh(boxMesh, shader, view, projection, verticalEdge, edgeColor);
	}

	glm::mat4 topEdge = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.42f, -0.015f));
	topEdge = glm::scale(topEdge, glm::vec3(0.72f, 0.055f, 0.055f));
	drawMesh(boxMesh, shader, view, projection, topEdge, edgeColor);

	glm::mat4 bottomEdge = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, -0.30f, -0.015f));
	bottomEdge = glm::scale(bottomEdge, glm::vec3(0.72f, 0.055f, 0.055f));
	drawMesh(boxMesh, shader, view, projection, bottomEdge, edgeColor);
}

void Cylinder::drawStrokeEffects(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, StrokeType stroke, float currentPhase, float pistonY) const {
	if (stroke == StrokeType::Intake) {
		glm::mat4 intakeFlow = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.42f, -0.48f));
		intakeFlow = glm::rotate(intakeFlow, 0.85f, glm::vec3(1.0f, 0.0f, 0.0f));
		intakeFlow = glm::scale(intakeFlow, glm::vec3(0.08f, 0.78f, 0.08f));
		drawMesh(cylinderMesh, shader, view, projection, intakeFlow, glm::vec4(0.18f, 0.54f, 0.88f, 1.0f));
	}

	if (stroke == StrokeType::Power) {
		const float pulse = 0.5f + 0.5f * std::sin(currentPhase * PI / 45.0f);
		glm::mat4 flame = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 0.82f + pistonY * 0.18f, 0.0f));
		flame = glm::scale(flame, glm::vec3(0.28f + 0.08f * pulse, 0.18f + 0.06f * pulse, 0.28f + 0.08f * pulse));
		drawMesh(cylinderMesh, shader, view, projection, flame, glm::vec4(0.95f, 0.52f, 0.10f, 1.0f));
	}

	if (stroke == StrokeType::Exhaust) {
		glm::mat4 exhaustFlow = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.32f, 0.56f));
		exhaustFlow = glm::rotate(exhaustFlow, -0.82f, glm::vec3(1.0f, 0.0f, 0.0f));
		exhaustFlow = glm::scale(exhaustFlow, glm::vec3(0.09f, 0.82f, 0.09f));
		drawMesh(cylinderMesh, shader, view, projection, exhaustFlow, glm::vec4(0.34f, 0.34f, 0.32f, 1.0f));

		for (int puff = 0; puff < 3; ++puff) {
			glm::mat4 smoke = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition + puff * 0.10f, 1.56f + puff * 0.05f, 0.84f + puff * 0.16f));
			smoke = glm::scale(smoke, glm::vec3(0.12f + puff * 0.04f, 0.12f + puff * 0.04f, 0.12f + puff * 0.04f));
			drawMesh(cylinderMesh, shader, view, projection, smoke, glm::vec4(0.42f, 0.42f, 0.40f, 1.0f));
		}
	}

	if (stroke == StrokeType::Compression) {
		glm::mat4 compressionMark = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.02f, 0.0f));
		compressionMark = glm::scale(compressionMark, glm::vec3(0.40f, 0.055f, 0.40f));
		drawMesh(boxMesh, shader, view, projection, compressionMark, glm::vec4(0.78f, 0.78f, 0.74f, 1.0f));
	}
}

void Cylinder::drawSpring(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float x, float z, float lift) const {
	for (int ring = 0; ring < 5; ++ring) {
		glm::mat4 springRing = glm::translate(glm::mat4(1.0f), glm::vec3(x, 1.76f + ring * 0.055f + lift, z));
		springRing = glm::scale(springRing, glm::vec3(0.13f, 0.025f, 0.13f));
		drawMesh(cylinderMesh, shader, view, projection, springRing, glm::vec4(0.15f, 0.15f, 0.14f, 1.0f));
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
