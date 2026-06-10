#ifndef CYLINDER_H
#define CYLINDER_H

#include <glm/glm.hpp>

class EngineMesh;
class ShaderProgram;

/** @brief Four strokes of a spark-ignition engine cycle. */
enum class StrokeType {
	/** Intake valves open and the piston draws mixture into the cylinder. */
	Intake,
	/** All valves are closed while the mixture is compressed. */
	Compression,
	/** Combustion forces the piston down with all valves closed. */
	Power,
	/** Exhaust valves open and combustion products leave the cylinder. */
	Exhaust
};

/**
 * @brief Animated representation of one cylinder in the inline-four engine.
 *
 * A cylinder owns its phase offsets and calculates piston, connecting rod,
 * valve, injector and combustion-effect positions from the shared crank angle.
 */
class Cylinder {
public:
	/**
	 * @param index Cylinder number used by the engine assembly.
	 * @param xPosition Position along the inline engine block.
	 * @param crankOffset Mechanical crank-pin offset in degrees.
	 * @param cycleOffset Four-stroke cycle offset in degrees.
	 */
	Cylinder(int index, float xPosition, float crankOffset, float cycleOffset);

	/**
	 * @brief Draws all visible and animated parts of this cylinder.
	 *
	 * @param crankAngle Shared crankshaft angle in the 0-720 degree cycle.
	 * @param lampOn Enables contribution from the workshop spot lights.
	 */
	void draw(const EngineMesh& boxMesh, const EngineMesh& cylinderMesh, const EngineMesh& halfCylinderMesh, const EngineMesh& valvePlateMesh, const EngineMesh& valveSeatMesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, float crankAngle, unsigned int metalTexture, unsigned int darkMetalTexture, unsigned int rubberTexture, bool lampOn) const;

	/**
	 * @brief Determines the current four-stroke phase.
	 * @param crankAngle Shared crankshaft angle.
	 * @return Intake, compression, power or exhaust stroke.
	 */
	StrokeType getStroke(float crankAngle) const;

private:
	int index;
	float xPosition;
	float crankOffset;
	float cycleOffset;
	mutable unsigned int currentMetalTexture;
	mutable unsigned int currentDarkMetalTexture;
	mutable unsigned int currentRubberTexture;
	mutable bool currentLampOn;

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
