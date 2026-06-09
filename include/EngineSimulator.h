#ifndef ENGINE_SIMULATOR_H
#define ENGINE_SIMULATOR_H

#include <glm/glm.hpp>
#include <vector>

#include "Cylinder.h"
#include "EngineMeshes.h"

struct GLFWwindow;
class ShaderProgram;

class EngineSimulator {
public:
	EngineSimulator();

	void update(GLFWwindow* window, float deltaTime);
	void draw(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection);
	void destroy();

private:
	float crankAngle;
	float rpm;
	float targetRpm;
	float throttle;
	bool paused;
	bool spaceWasPressed;
	bool resetWasPressed;
	bool meshesReady;

	EngineMesh boxMesh;
	EngineMesh cylinderMesh;
	EngineMesh camLobeMesh;
	EngineMesh crankSprocketMesh;
	EngineMesh camSprocketMesh;
	EngineMesh halfCylinderMesh;
	EngineMesh intakeManifoldMesh;
	EngineMesh exhaustManifoldMesh;
	EngineMesh valvePlateMesh;
	EngineMesh valveSeatMesh;
	unsigned int metalTexture;
	unsigned int darkMetalTexture;
	unsigned int rubberTexture;
	std::vector<Cylinder> cylinders;

	void initMeshes();
	void initTextures();
	void destroyTextures();
	unsigned int createProceduralTexture(int variant) const;
	unsigned int chooseTexture(const glm::vec4& color) const;
	void drawEngineBlockCutaway(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawCrankshaftAssembly(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawValveTrain(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawTimingChain(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawManifolds(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawStatusPanel(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawMesh(const EngineMesh& mesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const glm::vec4& color) const;
};

#endif
