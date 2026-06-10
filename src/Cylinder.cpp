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

Cylinder::Cylinder(int index, float xPosition, float crankOffset, float cycleOffset)
	: index(index),
	  xPosition(xPosition),
	  crankOffset(crankOffset),
	  cycleOffset(cycleOffset),
	  currentMetalTexture(0),
	  currentDarkMetalTexture(0),
	  currentRubberTexture(0),
	  currentLampOn(false) {
}

void Cylinder::draw(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, const EngineMesh& halfCylinderMesh, const EngineMesh& valvePlateMesh, const EngineMesh& valveSeatMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float crankAngle, unsigned int metalTexture, unsigned int darkMetalTexture, unsigned int rubberTexture, bool lampOn) const {
	currentMetalTexture = metalTexture;
	currentDarkMetalTexture = darkMetalTexture;
	currentRubberTexture = rubberTexture;
	currentLampOn = lampOn;

	const float currentPhase = phase(crankAngle);
	const StrokeType stroke = getStroke(crankAngle);

	const float crankRad = (crankAngle + crankOffset) * PI / 180.0f + PI * 0.5f;
	const glm::vec3 crankPin(xPosition, -1.05f + 0.32f * std::sin(crankRad), 0.32f * std::cos(crankRad));
	const float pistonPinY = pistonPinHeight(crankRad);
	const glm::vec3 pistonPin(xPosition, pistonPinY, 0.0f);

	glm::mat4 sleeve = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 0.24f, 0.0f));
	sleeve = glm::scale(sleeve, glm::vec3(0.86f, 2.00f, 0.86f));
	drawMesh(halfCylinderMesh, shader, view, projection, sleeve, glm::vec4(0.36f, 0.39f, 0.40f, 1.0f));

	glm::mat4 sleeveLiner = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 0.24f, 0.0f));
	sleeveLiner = glm::scale(sleeveLiner, glm::vec3(0.76f, 2.00f, 0.76f));
	drawMesh(halfCylinderMesh, shader, view, projection, sleeveLiner, glm::vec4(0.68f, 0.70f, 0.68f, 1.0f));
	drawCutawayEdges(boxMesh, shader, view, projection);

	glm::mat4 chamber = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.10f, -0.01f));
	chamber = glm::scale(chamber, glm::vec3(0.74f, 0.24f, 0.74f));
	drawMesh(halfCylinderMesh, shader, view, projection, chamber, strokeColor(stroke));

	glm::mat4 piston = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, pistonPinY + 0.08f, 0.0f));
	piston = glm::scale(piston, glm::vec3(0.70f, 0.28f, 0.70f));
	drawMesh(cylinderMesh, shader, view, projection, piston, glm::vec4(0.86f, 0.86f, 0.80f, 1.0f));
	drawPistonDetails(cylinderMesh, shader, view, projection, pistonPinY);

	const glm::mat4 rod = alignCylinderBetween(crankPin, pistonPin, 0.10f);
	drawMesh(cylinderMesh, shader, view, projection, rod, glm::vec4(0.70f, 0.70f, 0.64f, 1.0f));

	glm::mat4 bigEnd = glm::translate(glm::mat4(1.0f), crankPin);
	bigEnd = glm::rotate(bigEnd, PI * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
	bigEnd = glm::scale(bigEnd, glm::vec3(0.21f, 0.12f, 0.21f));
	drawMesh(cylinderMesh, shader, view, projection, bigEnd, glm::vec4(0.54f, 0.55f, 0.50f, 1.0f));

	drawValveSeats(valvePlateMesh, shader, view, projection);

	const float pistonTopY = pistonPinY + 0.2625f;
	const float valveAxisY = glm::normalize(glm::vec3(0.0f, 0.665f, 0.22f)).y;
	const float closedValveBottomY = 1.225f - valveAxisY * 0.0175f;
	const float minimumClearance = 0.08f;
	const float safeLift = std::max(0.0f, (closedValveBottomY - pistonTopY - minimumClearance) / valveAxisY);
	const float intakeLift = std::min(valveOffset(currentPhase, StrokeType::Intake), safeLift);
	const float exhaustLift = std::min(valveOffset(currentPhase, StrokeType::Exhaust), safeLift);
	drawValvePair(cylinderMesh, valveSeatMesh, shader, view, projection, -0.25f, intakeLift, glm::vec4(0.66f, 0.72f, 0.74f, 1.0f));
	drawValvePair(cylinderMesh, valveSeatMesh, shader, view, projection, 0.25f, exhaustLift, glm::vec4(0.72f, 0.67f, 0.60f, 1.0f));
	drawInjector(boxMesh, cylinderMesh, shader, view, projection, currentPhase);
	drawHeadFasteners(cylinderMesh, shader, view, projection);
	drawStrokeEffects(boxMesh, cylinderMesh, shader, view, projection, stroke, currentPhase, pistonTopY);
}

StrokeType Cylinder::getStroke(float crankAngle) const {
	const float p = phase(crankAngle);
	if (p < 180.0f) return StrokeType::Intake;
	if (p < 360.0f) return StrokeType::Compression;
	if (p < 540.0f) return StrokeType::Power;
	return StrokeType::Exhaust;
}

float Cylinder::phase(float crankAngle) const {
	float p = std::fmod(crankAngle + cycleOffset, 720.0f);
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

	const float local = (phaseDegrees - start) / (end - start);
	const float liftCurve = std::sin(PI * local) * std::sin(PI * local);
	const float maxLift = 0.16f;
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

void Cylinder::drawValvePair(const EngineMesh& cylinderMesh, const EngineMesh& valveSeatMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float z, float lift, const glm::vec4& color) const {
	const float xOffsets[2] = {-0.20f, 0.20f};
	const float valveZ = z * 0.72f;
	const float valveTopZ = z < 0.0f ? -0.40f : 0.40f;
	for (float localX : xOffsets) {
		const float valveX = xPosition + localX;
		const glm::vec3 closedLowerStem(valveX, 1.245f, valveZ);
		const glm::vec3 closedUpperStem(valveX, 1.910f, valveTopZ);
		const glm::vec3 valveAxis = glm::normalize(closedUpperStem - closedLowerStem);
		const glm::vec3 valveTravel = -valveAxis * lift;
		const glm::vec3 lowerStem = closedLowerStem + valveTravel;
		const glm::vec3 upperStem = closedUpperStem + valveTravel;
		const glm::vec3 closedValveHead(valveX, 1.225f, valveZ);
		const glm::vec3 valveHeadCenter = closedValveHead + valveTravel;
		const glm::vec3 springBase = glm::mix(closedLowerStem, closedUpperStem, 0.48f);
		const glm::vec3 springTop = glm::mix(lowerStem, upperStem, 0.92f);

		glm::mat4 valveSeat = alignCylinderBetween(
			closedValveHead - valveAxis * 0.014f,
			closedValveHead + valveAxis * 0.014f,
			0.230f
		);
		drawMesh(valveSeatMesh, shader, view, projection, valveSeat, glm::vec4(0.46f, 0.48f, 0.45f, 1.0f));

		const glm::vec3 guideBottom = glm::mix(closedLowerStem, closedUpperStem, 0.20f);
		const glm::vec3 guideTop = glm::mix(closedLowerStem, closedUpperStem, 0.48f);
		glm::mat4 valveGuide = alignCylinderBetween(guideBottom, guideTop, 0.060f);
		drawMesh(cylinderMesh, shader, view, projection, valveGuide, glm::vec4(0.40f, 0.42f, 0.40f, 1.0f));

		glm::mat4 lowerGuideCollar = alignCylinderBetween(
			guideBottom - valveAxis * 0.018f,
			guideBottom + valveAxis * 0.018f,
			0.082f
		);
		drawMesh(cylinderMesh, shader, view, projection, lowerGuideCollar, glm::vec4(0.58f, 0.60f, 0.56f, 1.0f));

		glm::mat4 upperGuideCollar = alignCylinderBetween(
			guideTop - valveAxis * 0.018f,
			guideTop + valveAxis * 0.018f,
			0.082f
		);
		drawMesh(cylinderMesh, shader, view, projection, upperGuideCollar, glm::vec4(0.58f, 0.60f, 0.56f, 1.0f));

		glm::mat4 stem = alignCylinderBetween(lowerStem, upperStem, 0.034f);
		drawMesh(cylinderMesh, shader, view, projection, stem, color);

		glm::mat4 head = alignCylinderBetween(
			valveHeadCenter - valveAxis * 0.0175f,
			valveHeadCenter + valveAxis * 0.0175f,
			0.205f
		);
		drawMesh(cylinderMesh, shader, view, projection, head, color);

		glm::mat4 lowerRetainer = alignCylinderBetween(
			springBase - valveAxis * 0.013f,
			springBase + valveAxis * 0.013f,
			0.120f
		);
		drawMesh(cylinderMesh, shader, view, projection, lowerRetainer, glm::vec4(0.72f, 0.72f, 0.66f, 1.0f));

		glm::mat4 retainer = alignCylinderBetween(
			springTop - valveAxis * 0.016f,
			springTop + valveAxis * 0.016f,
			0.120f
		);
		drawMesh(cylinderMesh, shader, view, projection, retainer, glm::vec4(0.78f, 0.78f, 0.72f, 1.0f));

		drawSpring(cylinderMesh, shader, view, projection, springBase, springTop);

		const glm::vec3 followerBottom = upperStem - valveAxis * 0.015f;
		const glm::vec3 followerTop = upperStem + valveAxis * 0.125f;
		glm::mat4 bucketFollower = alignCylinderBetween(followerBottom, followerTop, 0.125f);
		drawMesh(cylinderMesh, shader, view, projection, bucketFollower, glm::vec4(0.46f, 0.48f, 0.46f, 1.0f));

		const glm::vec3 contactCenter = followerTop + valveAxis * 0.018f;
		glm::mat4 contactPad = alignCylinderBetween(
			contactCenter - valveAxis * 0.018f,
			contactCenter + valveAxis * 0.018f,
			0.155f
		);
		drawMesh(cylinderMesh, shader, view, projection, contactPad, glm::vec4(0.76f, 0.77f, 0.72f, 1.0f));

		glm::mat4 followerCollar = alignCylinderBetween(
			followerBottom - valveAxis * 0.014f,
			followerBottom + valveAxis * 0.014f,
			0.145f
		);
		drawMesh(cylinderMesh, shader, view, projection, followerCollar, glm::vec4(0.60f, 0.62f, 0.58f, 1.0f));
	}
}

void Cylinder::drawInjector(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float currentPhase) const {
	const glm::vec3 nozzle(xPosition, 1.285f, 0.0f);
	const glm::vec3 bodyTop(xPosition, 1.91f, -0.18f);
	const glm::vec3 injectorAxis = glm::normalize(bodyTop - nozzle);
	const glm::vec3 connectorDirection(-1.0f, 0.0f, 0.0f);
	const glm::vec3 fuelDirection(0.0f, 0.0f, -1.0f);

	float injectionStroke = 0.0f;
	if (currentPhase >= 330.0f && currentPhase <= 390.0f) {
		const float local = (currentPhase - 330.0f) / 60.0f;
		injectionStroke = 0.15f * std::sin(PI * local) * std::sin(PI * local);
	}

	// Stepped nozzle makes the injector readable even from the cutaway underside.
	glm::mat4 nozzleTip = alignCylinderBetween(nozzle - injectorAxis * 0.045f, nozzle + injectorAxis * 0.025f, 0.026f);
	drawMesh(cylinderMesh, shader, view, projection, nozzleTip, glm::vec4(0.84f, 0.70f, 0.30f, 1.0f));

	glm::mat4 nozzleNeedle = alignCylinderBetween(
		nozzle + injectorAxis * 0.015f,
		nozzle + injectorAxis * 0.085f,
		0.043f
	);
	drawMesh(cylinderMesh, shader, view, projection, nozzleNeedle, glm::vec4(0.74f, 0.75f, 0.71f, 1.0f));

	glm::mat4 nozzleShoulder = alignCylinderBetween(
		nozzle + injectorAxis * 0.075f,
		nozzle + injectorAxis * 0.135f,
		0.074f
	);
	drawMesh(cylinderMesh, shader, view, projection, nozzleShoulder, glm::vec4(0.66f, 0.67f, 0.63f, 1.0f));

	glm::mat4 injectorBody = alignCylinderBetween(nozzle + injectorAxis * 0.125f, bodyTop, 0.070f);
	drawMesh(cylinderMesh, shader, view, projection, injectorBody, glm::vec4(0.44f, 0.46f, 0.45f, 1.0f));

	const glm::vec3 pumpBottom = nozzle + injectorAxis * 0.34f;
	const glm::vec3 pumpTop = nozzle + injectorAxis * 0.62f;
	glm::mat4 pumpHousing = alignCylinderBetween(pumpBottom, pumpTop, 0.112f);
	drawMesh(cylinderMesh, shader, view, projection, pumpHousing, glm::vec4(0.31f, 0.33f, 0.32f, 1.0f));

	glm::mat4 lowerCollar = alignCylinderBetween(
		pumpBottom - injectorAxis * 0.020f,
		pumpBottom + injectorAxis * 0.020f,
		0.138f
	);
	drawMesh(cylinderMesh, shader, view, projection, lowerCollar, glm::vec4(0.68f, 0.69f, 0.65f, 1.0f));

	glm::mat4 upperCollar = alignCylinderBetween(
		pumpTop - injectorAxis * 0.024f,
		pumpTop + injectorAxis * 0.024f,
		0.138f
	);
	drawMesh(cylinderMesh, shader, view, projection, upperCollar, glm::vec4(0.70f, 0.71f, 0.67f, 1.0f));

	const glm::vec3 sealingCenter = nozzle + injectorAxis * 0.18f;
	for (float offset : {-0.025f, 0.025f}) {
		glm::mat4 sealingRing = alignCylinderBetween(
			sealingCenter + injectorAxis * (offset - 0.010f),
			sealingCenter + injectorAxis * (offset + 0.010f),
			0.090f
		);
		drawMesh(cylinderMesh, shader, view, projection, sealingRing, glm::vec4(0.08f, 0.09f, 0.09f, 1.0f));
	}

	const glm::vec3 fuelPortBase = nozzle + injectorAxis * 0.45f;
	const glm::vec3 fuelPortEnd = fuelPortBase + fuelDirection * 0.15f;
	glm::mat4 fuelPort = alignCylinderBetween(
		fuelPortBase,
		fuelPortEnd,
		0.044f
	);
	drawMesh(cylinderMesh, shader, view, projection, fuelPort, glm::vec4(0.68f, 0.69f, 0.65f, 1.0f));

	glm::mat4 fuelPortCollar = alignCylinderBetween(
		fuelPortEnd - fuelDirection * 0.018f,
		fuelPortEnd + fuelDirection * 0.018f,
		0.068f
	);
	drawMesh(cylinderMesh, shader, view, projection, fuelPortCollar, glm::vec4(0.80f, 0.71f, 0.42f, 1.0f));

	const glm::vec3 connectorNeckBase = nozzle + injectorAxis * 0.55f;
	const glm::vec3 connectorNeckEnd = connectorNeckBase + connectorDirection * 0.10f;
	glm::mat4 connectorNeck = alignCylinderBetween(connectorNeckBase, connectorNeckEnd, 0.042f);
	drawMesh(cylinderMesh, shader, view, projection, connectorNeck, glm::vec4(0.38f, 0.40f, 0.38f, 1.0f));

	const glm::vec3 connectorCenter = connectorNeckEnd + connectorDirection * 0.065f;
	glm::mat4 connector = glm::translate(glm::mat4(1.0f), connectorCenter);
	connector = glm::scale(connector, glm::vec3(0.13f, 0.10f, 0.12f));
	drawMesh(boxMesh, shader, view, projection, connector, glm::vec4(0.10f, 0.11f, 0.11f, 1.0f));

	glm::mat4 connectorSocket = glm::translate(glm::mat4(1.0f), connectorCenter + connectorDirection * 0.085f);
	connectorSocket = glm::scale(connectorSocket, glm::vec3(0.040f, 0.060f, 0.072f));
	drawMesh(boxMesh, shader, view, projection, connectorSocket, glm::vec4(0.18f, 0.20f, 0.19f, 1.0f));

	const glm::vec3 clampCenter = nozzle + injectorAxis * 0.30f;
	glm::mat4 injectorClamp = alignBoxBetween(
		clampCenter + connectorDirection * 0.13f,
		clampCenter - connectorDirection * 0.13f,
		0.055f
	);
	drawMesh(boxMesh, shader, view, projection, injectorClamp, glm::vec4(0.56f, 0.58f, 0.55f, 1.0f));

	glm::mat4 clampBolt = alignCylinderBetween(
		clampCenter - connectorDirection * 0.13f - injectorAxis * 0.035f,
		clampCenter - connectorDirection * 0.13f + injectorAxis * 0.055f,
		0.038f
	);
	drawMesh(cylinderMesh, shader, view, projection, clampBolt, glm::vec4(0.27f, 0.29f, 0.28f, 1.0f));

	const glm::vec3 plungerTop = bodyTop - injectorAxis * injectionStroke;
	const glm::vec3 plungerBottom = plungerTop - injectorAxis * 0.24f;
	glm::mat4 plunger = alignCylinderBetween(plungerBottom, plungerTop, 0.045f);
	drawMesh(cylinderMesh, shader, view, projection, plunger, glm::vec4(0.76f, 0.77f, 0.72f, 1.0f));

	if (injectionStroke > 0.001f) {
		const float strokeRatio = injectionStroke / 0.15f;
		const glm::vec3 pulseCenter = pumpTop - injectorAxis * (0.05f + 0.17f * strokeRatio);
		glm::mat4 pressurePulse = alignCylinderBetween(
			pulseCenter - injectorAxis * 0.014f,
			pulseCenter + injectorAxis * 0.014f,
			0.124f
		);
		drawMesh(cylinderMesh, shader, view, projection, pressurePulse, glm::vec4(0.94f, 0.66f, 0.12f, 1.0f));

		glm::mat4 nozzlePulse = alignCylinderBetween(
			nozzle + injectorAxis * 0.015f,
			nozzle + injectorAxis * 0.055f,
			0.050f + 0.012f * strokeRatio
		);
		drawMesh(cylinderMesh, shader, view, projection, nozzlePulse, glm::vec4(0.96f, 0.72f, 0.18f, 1.0f));
	}

	const glm::vec3 springBottom = pumpTop + injectorAxis * 0.035f;
	const glm::vec3 springTop = bodyTop - injectorAxis * (0.065f + injectionStroke);
	drawSpring(cylinderMesh, shader, view, projection, springBottom, springTop, 0.072f, 0.010f, 7);

	glm::mat4 springLowerSeat = alignCylinderBetween(
		springBottom - injectorAxis * 0.012f,
		springBottom + injectorAxis * 0.012f,
		0.092f
	);
	drawMesh(cylinderMesh, shader, view, projection, springLowerSeat, glm::vec4(0.62f, 0.64f, 0.60f, 1.0f));

	glm::mat4 springUpperSeat = alignCylinderBetween(
		springTop - injectorAxis * 0.012f,
		springTop + injectorAxis * 0.012f,
		0.092f
	);
	drawMesh(cylinderMesh, shader, view, projection, springUpperSeat, glm::vec4(0.68f, 0.69f, 0.65f, 1.0f));

	const glm::vec3 pressurePad = bodyTop + injectorAxis * (0.025f - injectionStroke);
	glm::mat4 injectorPad = alignCylinderBetween(
		pressurePad - injectorAxis * 0.022f,
		pressurePad + injectorAxis * 0.022f,
		0.125f
	);
	drawMesh(cylinderMesh, shader, view, projection, injectorPad, glm::vec4(0.74f, 0.75f, 0.70f, 1.0f));
}

void Cylinder::drawPistonDetails(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float pistonPinY) const {
	glm::mat4 crown = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, pistonPinY + 0.245f, 0.0f));
	crown = glm::scale(crown, glm::vec3(0.63f, 0.035f, 0.63f));
	drawMesh(cylinderMesh, shader, view, projection, crown, glm::vec4(0.95f, 0.95f, 0.88f, 1.0f));

	for (int ring = 0; ring < 3; ++ring) {
		glm::mat4 pistonRing = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, pistonPinY + 0.185f - ring * 0.052f, 0.0f));
		pistonRing = glm::scale(pistonRing, glm::vec3(0.715f, 0.025f, 0.715f));
		drawMesh(cylinderMesh, shader, view, projection, pistonRing, glm::vec4(0.08f, 0.09f, 0.09f, 1.0f));
	}

	for (float z : {-0.24f, 0.24f}) {
		glm::mat4 pinBoss = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, pistonPinY, z));
		pinBoss = glm::rotate(pinBoss, PI * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
		pinBoss = glm::scale(pinBoss, glm::vec3(0.12f, 0.08f, 0.12f));
		drawMesh(cylinderMesh, shader, view, projection, pinBoss, glm::vec4(0.74f, 0.74f, 0.68f, 1.0f));
	}

	glm::mat4 lowerSkirt = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, pistonPinY - 0.055f, 0.0f));
	lowerSkirt = glm::scale(lowerSkirt, glm::vec3(0.62f, 0.11f, 0.62f));
	drawMesh(cylinderMesh, shader, view, projection, lowerSkirt, glm::vec4(0.72f, 0.72f, 0.66f, 1.0f));
}

void Cylinder::drawHeadFasteners(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	(void)cylinderMesh;
	(void)shader;
	(void)view;
	(void)projection;
}

void Cylinder::drawValveSeats(const EngineMesh& valvePlateMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	glm::mat4 headPlate = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 1.265f, 0.0f));
	headPlate = glm::scale(headPlate, glm::vec3(0.88f, 0.045f, 0.88f));
	drawMesh(valvePlateMesh, shader, view, projection, headPlate, glm::vec4(0.72f, 0.73f, 0.68f, 1.0f));
}

void Cylinder::drawCutawayEdges(const EngineMesh& boxMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const {
	(void)boxMesh;
	(void)shader;
	(void)view;
	(void)projection;
}

void Cylinder::drawStrokeEffects(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, StrokeType stroke, float currentPhase, float pistonY) const {
	const float chamberTop = 1.16f;
	const float chamberHeight = std::max(chamberTop - pistonY, 0.06f);
	const float phaseTime = currentPhase * PI / 180.0f;

	const auto drawParticle = [&](const glm::vec3& position, float size, const glm::vec4& color) {
		glm::mat4 particle = glm::translate(glm::mat4(1.0f), position);
		particle = glm::scale(particle, glm::vec3(size, size * 0.65f, size));
		drawMesh(cylinderMesh, shader, view, projection, particle, color);
	};

	if (stroke == StrokeType::Intake) {
		const float progress = currentPhase / 180.0f;
		for (int particle = 0; particle < 7; ++particle) {
			const float offset = static_cast<float>(particle) / 7.0f;
			const float travel = std::fmod(progress * 1.35f + offset, 1.0f);
			const float spiral = phaseTime * 1.8f + static_cast<float>(particle) * 2.35f;
			const float radius = 0.08f + 0.16f * travel;
			const glm::vec3 position(
				xPosition + std::cos(spiral) * radius,
				chamberTop - travel * chamberHeight * 0.82f,
				-0.15f + std::sin(spiral) * radius * 0.65f
			);
			drawParticle(position, 0.035f, glm::vec4(0.32f, 0.66f, 0.82f, 1.0f));
		}
		return;
	}

	if (stroke == StrokeType::Compression) {
		const float progress = (currentPhase - 180.0f) / 180.0f;
		const float cloudRadius = glm::mix(0.25f, 0.10f, progress);
		const float cloudCenterY = pistonY + chamberHeight * glm::mix(0.55f, 0.76f, progress);
		for (int particle = 0; particle < 7; ++particle) {
			const float angle = phaseTime * 1.4f + static_cast<float>(particle) * 2.40f;
			const float layer = static_cast<float>(particle % 3) / 2.0f;
			const glm::vec3 position(
				xPosition + std::cos(angle) * cloudRadius * (0.55f + layer * 0.35f),
				cloudCenterY + (layer - 0.5f) * chamberHeight * 0.22f,
				std::sin(angle) * cloudRadius * 0.72f
			);
			const glm::vec4 color = glm::mix(
				glm::vec4(0.38f, 0.62f, 0.76f, 1.0f),
				glm::vec4(0.78f, 0.66f, 0.30f, 1.0f),
				progress
			);
			drawParticle(position, 0.032f + progress * 0.008f, color);
		}
		return;
	}

	if (stroke == StrokeType::Power) {
		const float progress = (currentPhase - 360.0f) / 180.0f;
		const float flash = std::clamp(1.0f - progress / 0.24f, 0.0f, 1.0f);
		if (flash > 0.0f) {
			glm::mat4 core = glm::translate(
				glm::mat4(1.0f),
				glm::vec3(xPosition, chamberTop - chamberHeight * 0.22f, 0.0f)
			);
			core = glm::scale(core, glm::vec3(
				0.10f + flash * 0.16f,
				0.035f + flash * 0.06f,
				0.10f + flash * 0.16f
			));
			drawMesh(cylinderMesh, shader, view, projection, core, glm::vec4(1.0f, 0.72f, 0.12f, 1.0f));
		}

		for (int particle = 0; particle < 6; ++particle) {
			const float angle = static_cast<float>(particle) * PI / 3.0f + phaseTime;
			const float expansion = 0.07f + std::min(progress, 0.55f) * 0.34f;
			const glm::vec3 position(
				xPosition + std::cos(angle) * expansion,
				chamberTop - chamberHeight * (0.20f + progress * 0.48f),
				std::sin(angle) * expansion * 0.60f
			);
			drawParticle(position, 0.038f, glm::vec4(0.90f, 0.34f, 0.08f, 1.0f));
		}
		return;
	}

	const float progress = (currentPhase - 540.0f) / 180.0f;
	for (int particle = 0; particle < 6; ++particle) {
		const float offset = static_cast<float>(particle) / 6.0f;
		const float travel = std::fmod(progress * 1.25f + offset, 1.0f);
		const glm::vec3 start(
			xPosition + std::cos(phaseTime + particle) * 0.18f,
			pistonY + chamberHeight * 0.45f,
			0.04f
		);
		const glm::vec3 exit(xPosition + (particle % 2 == 0 ? -0.18f : 0.18f), 1.24f, 0.20f);
		const glm::vec3 position = glm::mix(start, exit, travel);
		drawParticle(position, 0.034f, glm::vec4(0.46f, 0.25f, 0.18f, 1.0f));
	}

	(void)boxMesh;
}

void Cylinder::drawSpring(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& bottom, const glm::vec3& top, float springRadius, float wireRadius, int turns) const {
	const glm::vec3 springAxis = top - bottom;
	const float springLength = glm::length(springAxis);
	if (springLength < 0.0001f) {
		return;
	}

	const glm::vec3 direction = springAxis / springLength;
	glm::vec3 basisU = glm::cross(direction, glm::vec3(1.0f, 0.0f, 0.0f));
	if (glm::length(basisU) < 0.0001f) {
		basisU = glm::cross(direction, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	basisU = glm::normalize(basisU);
	const glm::vec3 basisV = glm::normalize(glm::cross(direction, basisU));

	const int segments = turns * 12;
	glm::vec3 previous = bottom + basisU * springRadius;

	for (int segment = 1; segment <= segments; ++segment) {
		const float t = static_cast<float>(segment) / static_cast<float>(segments);
		const float angle = t * static_cast<float>(turns) * 2.0f * PI;
		const glm::vec3 center = glm::mix(bottom, top, t);
		const glm::vec3 current = center
			+ basisU * (std::cos(angle) * springRadius)
			+ basisV * (std::sin(angle) * springRadius);
		const glm::mat4 coilSegment = alignCylinderBetween(previous, current, wireRadius);
		drawMesh(cylinderMesh, shader, view, projection, coilSegment, glm::vec4(0.15f, 0.16f, 0.15f, 1.0f));
		previous = current;
	}
}

void Cylinder::drawMesh(const EngineMesh& mesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const glm::vec4& color) const {
	shader->use();
	glUniformMatrix4fv(shader->u("P"), 1, false, glm::value_ptr(projection));
	glUniformMatrix4fv(shader->u("V"), 1, false, glm::value_ptr(view));
	glUniformMatrix4fv(shader->u("M"), 1, false, glm::value_ptr(model));
	glUniform4fv(shader->u("color"), 1, glm::value_ptr(color));
	const glm::vec3 dirLightView = glm::normalize(glm::mat3(view) * glm::vec3(-0.35f, -0.85f, -0.35f));
	const glm::vec3 lampPositionView = glm::vec3(view * glm::vec4(7.72f, 2.45f, 0.15f, 1.0f));
	const glm::vec3 lampDirectionView = glm::normalize(glm::mat3(view) * glm::normalize(glm::vec3(-7.72f, -2.25f, -0.15f)));
	const glm::vec3 workLightPositionView = glm::vec3(view * glm::vec4(5.18f, 0.85f, -1.78f, 1.0f));
	const glm::vec3 workLightDirectionView = glm::normalize(glm::mat3(view) * glm::normalize(glm::vec3(-5.18f, -0.95f, 1.78f)));
	glUniform3fv(shader->u("dirLightDirView"), 1, glm::value_ptr(dirLightView));
	glUniform3fv(shader->u("pointLightPosView"), 1, glm::value_ptr(lampPositionView));
	glUniform3fv(shader->u("spotDirectionView"), 1, glm::value_ptr(lampDirectionView));
	glUniform3fv(shader->u("workLightPosView"), 1, glm::value_ptr(workLightPositionView));
	glUniform3fv(shader->u("workSpotDirectionView"), 1, glm::value_ptr(workLightDirectionView));
	glUniform1f(shader->u("lampIntensity"), currentLampOn ? 1.0f : 0.0f);
	glUniform1f(shader->u("shininess"), 48.0f);
	glUniform1f(shader->u("specularStrength"), 0.42f);
	glUniform1i(shader->u("unlit"), 0);
	const unsigned int texture = chooseTexture(color);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(shader->u("texture0"), 0);
	glUniform1i(shader->u("useTexture"), texture != 0 ? 1 : 0);
	mesh.draw();
	glBindTexture(GL_TEXTURE_2D, 0);
}
