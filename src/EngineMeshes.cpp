#define GLM_FORCE_RADIANS

#include "EngineMeshes.h"

#include <cmath>

#include "constants.h"

namespace {
	void addVertex(std::vector<float>& data, float x, float y, float z, float nx, float ny, float nz) {
		data.push_back(x);
		data.push_back(y);
		data.push_back(z);
		data.push_back(1.0f);
		data.push_back(nx);
		data.push_back(ny);
		data.push_back(nz);
		data.push_back(0.0f);
	}

	void addBoxFace(std::vector<float>& data, float nx, float ny, float nz, const float corners[4][3]) {
		addVertex(data, corners[0][0], corners[0][1], corners[0][2], nx, ny, nz);
		addVertex(data, corners[1][0], corners[1][1], corners[1][2], nx, ny, nz);
		addVertex(data, corners[2][0], corners[2][1], corners[2][2], nx, ny, nz);
		addVertex(data, corners[0][0], corners[0][1], corners[0][2], nx, ny, nz);
		addVertex(data, corners[2][0], corners[2][1], corners[2][2], nx, ny, nz);
		addVertex(data, corners[3][0], corners[3][1], corners[3][2], nx, ny, nz);
	}
}

EngineMesh::EngineMesh()
	: vao(0),
	  vbo(0),
	  vertexCount(0) {
}

EngineMesh::~EngineMesh() {
	destroy();
}

EngineMesh::EngineMesh(EngineMesh&& other) noexcept
	: vao(other.vao),
	  vbo(other.vbo),
	  vertexCount(other.vertexCount) {
	other.vao = 0;
	other.vbo = 0;
	other.vertexCount = 0;
}

EngineMesh& EngineMesh::operator=(EngineMesh&& other) noexcept {
	if (this != &other) {
		destroy();
		vao = other.vao;
		vbo = other.vbo;
		vertexCount = other.vertexCount;
		other.vao = 0;
		other.vbo = 0;
		other.vertexCount = 0;
	}
	return *this;
}

void EngineMesh::upload(const std::vector<float>& vertexData) {
	destroy();
	vertexCount = static_cast<GLsizei>(vertexData.size() / 8);

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexData.size() * sizeof(float)), vertexData.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(4 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void EngineMesh::draw() const {
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	glBindVertexArray(0);
}

void EngineMesh::destroy() {
	if (vbo != 0) {
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}
	if (vao != 0) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
	vertexCount = 0;
}

EngineMesh EngineMesh::createBox() {
	EngineMesh mesh;
	std::vector<float> data;

	const float px[4][3] = {{0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}};
	const float nx[4][3] = {{-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}};
	const float py[4][3] = {{-0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, -0.5f}};
	const float ny[4][3] = {{-0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}};
	const float pz[4][3] = {{-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}};
	const float nz[4][3] = {{0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}};

	addBoxFace(data, 1.0f, 0.0f, 0.0f, px);
	addBoxFace(data, -1.0f, 0.0f, 0.0f, nx);
	addBoxFace(data, 0.0f, 1.0f, 0.0f, py);
	addBoxFace(data, 0.0f, -1.0f, 0.0f, ny);
	addBoxFace(data, 0.0f, 0.0f, 1.0f, pz);
	addBoxFace(data, 0.0f, 0.0f, -1.0f, nz);

	mesh.upload(data);
	return mesh;
}

EngineMesh EngineMesh::createCylinder(int segments) {
	EngineMesh mesh;
	std::vector<float> data;
	const float radius = 0.5f;
	const float halfHeight = 0.5f;

	for (int i = 0; i < segments; ++i) {
		const float a0 = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
		const float a1 = 2.0f * PI * static_cast<float>(i + 1) / static_cast<float>(segments);
		const float x0 = std::cos(a0) * radius;
		const float z0 = std::sin(a0) * radius;
		const float x1 = std::cos(a1) * radius;
		const float z1 = std::sin(a1) * radius;

		addVertex(data, x0, -halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0));
		addVertex(data, x1, -halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1));
		addVertex(data, x1, halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1));
		addVertex(data, x0, -halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0));
		addVertex(data, x1, halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1));
		addVertex(data, x0, halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0));

		addVertex(data, 0.0f, halfHeight, 0.0f, 0.0f, 1.0f, 0.0f);
		addVertex(data, x1, halfHeight, z1, 0.0f, 1.0f, 0.0f);
		addVertex(data, x0, halfHeight, z0, 0.0f, 1.0f, 0.0f);

		addVertex(data, 0.0f, -halfHeight, 0.0f, 0.0f, -1.0f, 0.0f);
		addVertex(data, x0, -halfHeight, z0, 0.0f, -1.0f, 0.0f);
		addVertex(data, x1, -halfHeight, z1, 0.0f, -1.0f, 0.0f);
	}

	mesh.upload(data);
	return mesh;
}
