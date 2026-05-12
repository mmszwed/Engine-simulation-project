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
	std::vector<Cylinder> cylinders;

	void initMeshes();
	void drawEngineBlockCutaway(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawCrankshaftAssembly(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawValveTrain(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawStatusPanel(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawMesh(const EngineMesh& mesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const glm::vec4& color) const;
};

#endif
