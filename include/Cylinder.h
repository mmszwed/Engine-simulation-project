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
	Cylinder(int index, float xPosition, float phaseOffset);

	void draw(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float crankAngle) const;
	StrokeType getStroke(float crankAngle) const;

private:
	int index;
	float xPosition;
	float phaseOffset;

	float phase(float crankAngle) const;
	float pistonOffset(float phaseDegrees) const;
	float valveOffset(StrokeType stroke, StrokeType activeStroke) const;
	glm::mat4 alignBoxBetween(const glm::vec3& start, const glm::vec3& end, float thickness) const;
	glm::vec4 strokeColor(StrokeType stroke) const;
	void drawValvePair(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float z, float drop, const glm::vec4& color) const;
	void drawInjector(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawSpring(const EngineMesh& cylinderMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float x, float z, float drop) const;
	void drawMesh(const EngineMesh& mesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const glm::vec4& color) const;
};

#endif
