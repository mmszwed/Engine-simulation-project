#ifndef ENGINE_MESHES_H
#define ENGINE_MESHES_H

#include <GL/glew.h>

#include <vector>

class EngineMesh {
public:
	EngineMesh();
	~EngineMesh();

	EngineMesh(const EngineMesh&) = delete;
	EngineMesh& operator=(const EngineMesh&) = delete;
	EngineMesh(EngineMesh&& other) noexcept;
	EngineMesh& operator=(EngineMesh&& other) noexcept;

	void upload(const std::vector<float>& vertexData);
	void draw() const;
	void destroy();

	static EngineMesh createBox();
	static EngineMesh createCylinder(int segments);
	static EngineMesh createHalfCylinder(int segments);

private:
	GLuint vao;
	GLuint vbo;
	GLsizei vertexCount;
};

#endif
