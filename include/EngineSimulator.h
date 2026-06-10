#ifndef ENGINE_SIMULATOR_H
#define ENGINE_SIMULATOR_H

#include <glm/glm.hpp>
#include <vector>

#include "Cylinder.h"
#include "EngineMeshes.h"

struct GLFWwindow;
class ShaderProgram;

/**
 * @brief Coordinates the complete R4 engine simulation and workshop scene.
 *
 * EngineSimulator owns the animation state, four cylinders, procedural meshes,
 * textures, engine assemblies, garage environment and interactive lighting.
 * Cylinders use phase offsets that produce the 1-3-4-2 firing order.
 */
class EngineSimulator {
public:
	/** @brief Creates a paused engine at idle RPM and initializes R4 phases. */
	EngineSimulator();

	/**
	 * @brief Updates input, throttle, RPM, pause state and crank angle.
	 * @param window Active GLFW window.
	 * @param deltaTime Time since the previous frame in seconds.
	 */
	void update(GLFWwindow* window, float deltaTime);

	/** @brief Draws the lit workshop scene using the main material shader. */
	void draw(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection);

	/**
	 * @brief Draws shadow-casting engine geometry into the depth framebuffer.
	 * @param lightView View matrix from the work lamp.
	 * @param lightProjection Perspective projection of the work lamp.
	 */
	void drawShadow(ShaderProgram* shader, const glm::mat4& lightView, const glm::mat4& lightProjection);

	/** @brief Releases all meshes and textures owned by the simulator. */
	void destroy();

private:
	float crankAngle;
	float rpm;
	float targetRpm;
	float throttle;
	bool paused;
	bool spaceWasPressed;
	bool resetWasPressed;
	bool lampOn;
	bool lampWasPressed;
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
	unsigned int woodTexture;
	unsigned int garageWallTexture;
	unsigned int concreteFloorTexture;
	unsigned int castAluminumTexture;
	std::vector<Cylinder> cylinders;

	void initMeshes();
	void initTextures();
	void destroyTextures();
	unsigned int createProceduralTexture(int variant) const;
	unsigned int loadPngTexture(const char* path) const;
	unsigned int chooseTexture(const glm::vec4& color) const;
	void drawGarageShell(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawWallLamp(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawWorkLamp(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawWorkshopStand(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawFuelCanister(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawEngineBlockCutaway(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawCrankshaftAssembly(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawValveTrain(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawTimingChain(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawManifolds(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawStatusPanel(ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection) const;
	void drawMesh(const EngineMesh& mesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const glm::vec4& color) const;
	void drawUnlitMesh(const EngineMesh& mesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const glm::vec4& color) const;
	void drawTexturedMesh(const EngineMesh& mesh, ShaderProgram* shader, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const glm::vec4& color, unsigned int texture) const;
};

#endif
