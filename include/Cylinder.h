#ifndef CYLINDER_H
#define CYLINDER_H

#include <glm/glm.hpp>

class EngineMesh;
class ShaderProgram;

enum class StrokeType {
	Intake,
	Compression,
	Power,
	Exhaust
};

class Cylinder {
public:
	Cylinder(int index, float xPosition, float crankOffset, float cycleOffset);

	void draw(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, const EngineMesh& halfCylinderMesh, const EngineMesh& valvePlateMesh, const EngineMesh& valveSeatMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float crankAngle, unsigned int metalTexture, unsigned int darkMetalTexture, unsigned int rubberTexture) const;
	StrokeType getStroke(float crankAngle) const;

private:
	int index;
	float xPosition;
	float crankOffset;
	float cycleOffset;
	mutable unsigned int currentMetalTexture;
	mutable unsigned int currentDarkMetalTexture;
	mutable unsigned int currentRubberTexture;

	float phase(float crankAngle) const;
	float pistonPinHeight(float crankRadians) const;
	float valveOffset(float phaseDegrees, StrokeType activeStroke) const;
	glm::mat4 alignBoxBetween(const glm::vec3& start, const glm::vec3& end, float thickness) const;
	glm::mat4 alignCylinderBetween(const glm::vec3& start, const glm::vec3& end, float radius) const;
	unsigned int chooseTexture(const glm::vec4& color) const;
	glm::vec4 strokeColor(StrokeType stroke) const;
	void drawValvePair(const EngineMesh& cylinderMesh, const EngineMesh& valveSeatMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float z, float lift, const glm::vec4& color) const;
	void drawInjector(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float currentPhase) const;
	void drawPistonDetails(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float pistonPinY) const;
	void drawHeadFasteners(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawValveSeats(const EngineMesh& valvePlateMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawCutawayEdges(const EngineMesh& boxMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawStrokeEffects(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, StrokeType stroke, float currentPhase, float pistonY) const;
	void drawSpring(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& bottom, const glm::vec3& top, float springRadius = 0.095f, float wireRadius = 0.014f, int turns = 6) const;
	void drawMesh(const EngineMesh& mesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const glm::vec4& color) const;
};

#endif
