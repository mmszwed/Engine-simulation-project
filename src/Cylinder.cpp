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

void Cylinder::draw(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float crankAngle, unsigned int metalTexture, unsigned int darkMetalTexture, unsigned int rubberTexture) const {
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
	drawMesh(cylinderMesh, shader, view, projection, sleeve, glm::vec4(0.36f, 0.39f, 0.40f, 1.0f));

	glm::mat4 sleeveLiner = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 0.56f, 0.0f));
	sleeveLiner = glm::scale(sleeveLiner, glm::vec3(0.56f, 1.72f, 0.56f));
	drawMesh(cylinderMesh, shader, view, projection, sleeveLiner, glm::vec4(0.68f, 0.70f, 0.68f, 1.0f));

	glm::mat4 chamber = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.15f, -0.01f));
	chamber = glm::scale(chamber, glm::vec3(0.54f, 0.40f, 0.54f));
	drawMesh(cylinderMesh, shader, view, projection, chamber, strokeColor(stroke));

	glm::mat4 piston = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, -0.28f + pistonY, 0.0f));
	piston = glm::scale(piston, glm::vec3(0.50f, 0.26f, 0.50f));
	drawMesh(cylinderMesh, shader, view, projection, piston, glm::vec4(0.86f, 0.86f, 0.80f, 1.0f));
	drawPistonDetails(cylinderMesh, shader, view, projection, pistonY);

	const glm::mat4 rod = alignCylinderBetween(crankPin, pistonPin, 0.10f);
	drawMesh(cylinderMesh, shader, view, projection, rod, glm::vec4(0.70f, 0.70f, 0.64f, 1.0f));

	glm::mat4 crankPinMarker = glm::translate(glm::mat4(1.0f), crankPin);
	crankPinMarker = glm::scale(crankPinMarker, glm::vec3(0.18f, 0.18f, 0.18f));
	drawMesh(cylinderMesh, shader, view, projection, crankPinMarker, glm::vec4(0.18f, 0.20f, 0.21f, 1.0f));

	const float intakeDrop = valveOffset(stroke, StrokeType::Intake);
	const float exhaustDrop = valveOffset(stroke, StrokeType::Exhaust);

	drawValvePair(boxMesh, cylinderMesh, shader, view, projection, -0.25f, intakeDrop, glm::vec4(0.66f, 0.72f, 0.74f, 1.0f));
	drawValvePair(boxMesh, cylinderMesh, shader, view, projection, 0.25f, exhaustDrop, glm::vec4(0.72f, 0.67f, 0.60f, 1.0f));
	drawInjector(cylinderMesh, shader, view, projection);
	drawHeadFasteners(cylinderMesh, shader, view, projection);
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

void Cylinder::drawValvePair(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float z, float drop, const glm::vec4& color) const {
	const float xOffsets[2] = {-0.20f, 0.20f};
	for (float localX : xOffsets) {
		const float valveX = xPosition + localX;
		glm::mat4 stem = glm::translate(glm::mat4(1.0f), glm::vec3(valveX, 1.66f - drop, z));
		stem = glm::scale(stem, glm::vec3(0.055f, 0.55f, 0.055f));
		drawMesh(cylinderMesh, shader, view, projection, stem, color);

		glm::mat4 head = glm::translate(glm::mat4(1.0f), glm::vec3(valveX, 1.34f - drop, z));
		head = glm::scale(head, glm::vec3(0.15f, 0.07f, 0.15f));
		drawMesh(cylinderMesh, shader, view, projection, head, color);

		glm::mat4 retainer = glm::translate(glm::mat4(1.0f), glm::vec3(valveX, 1.96f - drop, z));
		retainer = glm::scale(retainer, glm::vec3(0.15f, 0.05f, 0.15f));
		drawMesh(cylinderMesh, shader, view, projection, retainer, glm::vec4(0.78f, 0.78f, 0.72f, 1.0f));

		drawSpring(cylinderMesh, shader, view, projection, valveX, z, drop);

		glm::mat4 rocker = glm::translate(glm::mat4(1.0f), glm::vec3(valveX, 2.12f, z));
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

void Cylinder::drawSpring(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float x, float z, float drop) const {
	for (int ring = 0; ring < 5; ++ring) {
		glm::mat4 springRing = glm::translate(glm::mat4(1.0f), glm::vec3(x, 1.76f + ring * 0.055f - drop, z));
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
