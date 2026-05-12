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
	  phaseOffset(phaseOffset) {
}

void Cylinder::draw(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float crankAngle) const {
	const float currentPhase = phase(crankAngle);
	const StrokeType stroke = getStroke(crankAngle);
	const float pistonY = pistonOffset(currentPhase);

	const float crankRad = (crankAngle + phaseOffset) * PI / 180.0f;
	const glm::vec3 crankPin(xPosition, -1.05f + 0.32f * std::sin(crankRad), 0.32f * std::cos(crankRad));
	const glm::vec3 pistonPin(xPosition, -0.28f + pistonY, 0.0f);

	glm::mat4 sleeve = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 0.55f, 0.0f));
	sleeve = glm::scale(sleeve, glm::vec3(0.62f, 1.65f, 0.62f));
	drawMesh(cylinderMesh, shader, view, projection, sleeve, glm::vec4(0.45f, 0.49f, 0.51f, 1.0f));

	glm::mat4 chamber = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.15f, -0.01f));
	chamber = glm::scale(chamber, glm::vec3(0.54f, 0.40f, 0.54f));
	drawMesh(cylinderMesh, shader, view, projection, chamber, strokeColor(stroke));

	glm::mat4 piston = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, -0.28f + pistonY, 0.0f));
	piston = glm::scale(piston, glm::vec3(0.48f, 0.24f, 0.48f));
	drawMesh(cylinderMesh, shader, view, projection, piston, glm::vec4(0.78f, 0.78f, 0.72f, 1.0f));

	const glm::mat4 rod = alignBoxBetween(crankPin, pistonPin, 0.10f);
	drawMesh(boxMesh, shader, view, projection, rod, glm::vec4(0.70f, 0.70f, 0.64f, 1.0f));

	glm::mat4 crankPinMarker = glm::translate(glm::mat4(1.0f), crankPin);
	crankPinMarker = glm::scale(crankPinMarker, glm::vec3(0.18f, 0.18f, 0.18f));
	drawMesh(cylinderMesh, shader, view, projection, crankPinMarker, glm::vec4(0.18f, 0.20f, 0.21f, 1.0f));

	const float intakeDrop = valveOffset(stroke, StrokeType::Intake);
	const float exhaustDrop = valveOffset(stroke, StrokeType::Exhaust);

	glm::mat4 intakeStem = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition - 0.22f, 1.66f - intakeDrop, -0.22f));
	intakeStem = glm::scale(intakeStem, glm::vec3(0.07f, 0.52f, 0.07f));
	drawMesh(cylinderMesh, shader, view, projection, intakeStem, glm::vec4(0.66f, 0.72f, 0.74f, 1.0f));

	glm::mat4 intakeHead = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition - 0.22f, 1.36f - intakeDrop, -0.22f));
	intakeHead = glm::scale(intakeHead, glm::vec3(0.18f, 0.08f, 0.18f));
	drawMesh(cylinderMesh, shader, view, projection, intakeHead, glm::vec4(0.66f, 0.72f, 0.74f, 1.0f));

	glm::mat4 exhaustStem = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition + 0.22f, 1.66f - exhaustDrop, -0.22f));
	exhaustStem = glm::scale(exhaustStem, glm::vec3(0.07f, 0.52f, 0.07f));
	drawMesh(cylinderMesh, shader, view, projection, exhaustStem, glm::vec4(0.72f, 0.67f, 0.60f, 1.0f));

	glm::mat4 exhaustHead = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition + 0.22f, 1.36f - exhaustDrop, -0.22f));
	exhaustHead = glm::scale(exhaustHead, glm::vec3(0.18f, 0.08f, 0.18f));
	drawMesh(cylinderMesh, shader, view, projection, exhaustHead, glm::vec4(0.72f, 0.67f, 0.60f, 1.0f));
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

void Cylinder::drawMesh(const EngineMesh& mesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const glm::vec4& color) const {
	shader->use();
	glUniformMatrix4fv(shader->u("P"), 1, false, glm::value_ptr(projection));
	glUniformMatrix4fv(shader->u("V"), 1, false, glm::value_ptr(view));
	glUniformMatrix4fv(shader->u("M"), 1, false, glm::value_ptr(model));
	glUniform4fv(shader->u("color"), 1, glm::value_ptr(color));
	glUniform4f(shader->u("lightDir"), 0.0f, 0.0f, 1.0f, 0.0f);
	mesh.draw();
}
