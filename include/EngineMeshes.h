#ifndef ENGINE_MESHES_H
#define ENGINE_MESHES_H

#include <GL/glew.h>

#include <vector>

/**
 * @brief RAII-style OpenGL mesh stored in a VAO and VBO.
 *
 * EngineMesh contains interleaved position, normal and UV data. Static factory
 * methods generate the procedural primitives used to assemble the engine.
 */
class EngineMesh {
public:
	/** @brief Creates an empty mesh without GPU resources. */
	EngineMesh();

	/** @brief Releases owned OpenGL resources. */
	~EngineMesh();

	EngineMesh(const EngineMesh&) = delete;
	EngineMesh& operator=(const EngineMesh&) = delete;
	EngineMesh(EngineMesh&& other) noexcept;
	EngineMesh& operator=(EngineMesh&& other) noexcept;

	/**
	 * @brief Uploads interleaved vertex data to the GPU.
	 * @param vertexData Position, normal and texture-coordinate data.
	 */
	void upload(const std::vector<float>& vertexData);

	/** @brief Draws the uploaded mesh using GL_TRIANGLES. */
	void draw() const;

	/** @brief Explicitly releases the VAO and VBO. */
	void destroy();

	/** @return Unit box with normals and UV coordinates. */
	static EngineMesh createBox();
	/** @param segments Number of radial subdivisions. */
	static EngineMesh createCylinder(int segments);
	/** @brief Creates an asymmetric camshaft lobe profile. */
	static EngineMesh createCamLobe(int segments);
	/** @brief Creates a filled sprocket with fixed teeth. */
	static EngineMesh createSprocket(int toothCount);
	/** @brief Creates a longitudinal half-cylinder for cutaway views. */
	static EngineMesh createHalfCylinder(int segments);
	/** @brief Creates an open manifold section with visible ports. */
	static EngineMesh createPortedHalfCylinder(int arcSegments, int axialSegments, bool portsOnPositiveSide);
	static EngineMesh createHalfDisk(int segments);
	/** @brief Creates the cylinder-head plate with valve and injector openings. */
	static EngineMesh createValvePlate(int gridResolution, int holeSegments);
	/** @brief Creates a flat ring used for seats, collars and bearings. */
	static EngineMesh createRing(int segments, float innerRadius);

private:
	GLuint vao;
	GLuint vbo;
	GLsizei vertexCount;
};

#endif
