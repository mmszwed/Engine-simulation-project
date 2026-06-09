#define GLM_FORCE_RADIANS

#include "EngineMeshes.h"

#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

#include "constants.h"

namespace {
	void addVertex(std::vector<float>& data, float x, float y, float z, float nx, float ny, float nz, float u, float v) {
		data.push_back(x);
		data.push_back(y);
		data.push_back(z);
		data.push_back(1.0f);
		data.push_back(nx);
		data.push_back(ny);
		data.push_back(nz);
		data.push_back(0.0f);
		data.push_back(u);
		data.push_back(v);
	}

	void addBoxFace(std::vector<float>& data, float nx, float ny, float nz, const float corners[4][3]) {
		addVertex(data, corners[0][0], corners[0][1], corners[0][2], nx, ny, nz, 0.0f, 0.0f);
		addVertex(data, corners[1][0], corners[1][1], corners[1][2], nx, ny, nz, 1.0f, 0.0f);
		addVertex(data, corners[2][0], corners[2][1], corners[2][2], nx, ny, nz, 1.0f, 1.0f);
		addVertex(data, corners[0][0], corners[0][1], corners[0][2], nx, ny, nz, 0.0f, 0.0f);
		addVertex(data, corners[2][0], corners[2][1], corners[2][2], nx, ny, nz, 1.0f, 1.0f);
		addVertex(data, corners[3][0], corners[3][1], corners[3][2], nx, ny, nz, 0.0f, 1.0f);
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
	vertexCount = static_cast<GLsizei>(vertexData.size() / 10);

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexData.size() * sizeof(float)), vertexData.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), reinterpret_cast<void*>(4 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), reinterpret_cast<void*>(8 * sizeof(float)));

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

		const float u0 = static_cast<float>(i) / static_cast<float>(segments);
		const float u1 = static_cast<float>(i + 1) / static_cast<float>(segments);

		addVertex(data, x0, -halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0), u0, 0.0f);
		addVertex(data, x1, -halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1), u1, 0.0f);
		addVertex(data, x1, halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1), u1, 1.0f);
		addVertex(data, x0, -halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0), u0, 0.0f);
		addVertex(data, x1, halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1), u1, 1.0f);
		addVertex(data, x0, halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0), u0, 1.0f);

		addVertex(data, 0.0f, halfHeight, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f);
		addVertex(data, x1, halfHeight, z1, 0.0f, 1.0f, 0.0f, 0.5f + x1, 0.5f + z1);
		addVertex(data, x0, halfHeight, z0, 0.0f, 1.0f, 0.0f, 0.5f + x0, 0.5f + z0);

		addVertex(data, 0.0f, -halfHeight, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f);
		addVertex(data, x0, -halfHeight, z0, 0.0f, -1.0f, 0.0f, 0.5f + x0, 0.5f + z0);
		addVertex(data, x1, -halfHeight, z1, 0.0f, -1.0f, 0.0f, 0.5f + x1, 0.5f + z1);
	}

	mesh.upload(data);
	return mesh;
}

EngineMesh EngineMesh::createCamLobe(int segments) {
	EngineMesh mesh;
	std::vector<float> data;
	const int lobeSegments = segments < 32 ? 32 : segments;
	const float halfWidth = 0.5f;
	const float baseRadius = 0.31f;
	const float noseLift = 0.19f;

	const auto profileRadius = [&](float angle) {
		const float towardNose = std::max(std::cos(angle), 0.0f);
		const float smoothNose = towardNose * towardNose * towardNose;
		return baseRadius + noseLift * smoothNose;
	};

	for (int segment = 0; segment < lobeSegments; ++segment) {
		const float a0 = 2.0f * PI * static_cast<float>(segment) / static_cast<float>(lobeSegments);
		const float a1 = 2.0f * PI * static_cast<float>(segment + 1) / static_cast<float>(lobeSegments);
		const float r0 = profileRadius(a0);
		const float r1 = profileRadius(a1);
		const float x0 = std::cos(a0) * r0;
		const float z0 = std::sin(a0) * r0;
		const float x1 = std::cos(a1) * r1;
		const float z1 = std::sin(a1) * r1;
		const float u0 = static_cast<float>(segment) / static_cast<float>(lobeSegments);
		const float u1 = static_cast<float>(segment + 1) / static_cast<float>(lobeSegments);

		const float previousAngle = a0 - 0.001f;
		const float nextAngle = a0 + 0.001f;
		const float previousRadius = profileRadius(previousAngle);
		const float nextRadius = profileRadius(nextAngle);
		const glm::vec2 tangent0(
			std::cos(nextAngle) * nextRadius - std::cos(previousAngle) * previousRadius,
			std::sin(nextAngle) * nextRadius - std::sin(previousAngle) * previousRadius
		);
		const glm::vec2 normal0 = glm::normalize(glm::vec2(tangent0.y, -tangent0.x));

		const float previousAngle1 = a1 - 0.001f;
		const float nextAngle1 = a1 + 0.001f;
		const float previousRadius1 = profileRadius(previousAngle1);
		const float nextRadius1 = profileRadius(nextAngle1);
		const glm::vec2 tangent1(
			std::cos(nextAngle1) * nextRadius1 - std::cos(previousAngle1) * previousRadius1,
			std::sin(nextAngle1) * nextRadius1 - std::sin(previousAngle1) * previousRadius1
		);
		const glm::vec2 normal1 = glm::normalize(glm::vec2(tangent1.y, -tangent1.x));

		addVertex(data, x0, -halfWidth, z0, normal0.x, 0.0f, normal0.y, u0, 0.0f);
		addVertex(data, x1, -halfWidth, z1, normal1.x, 0.0f, normal1.y, u1, 0.0f);
		addVertex(data, x1, halfWidth, z1, normal1.x, 0.0f, normal1.y, u1, 1.0f);
		addVertex(data, x0, -halfWidth, z0, normal0.x, 0.0f, normal0.y, u0, 0.0f);
		addVertex(data, x1, halfWidth, z1, normal1.x, 0.0f, normal1.y, u1, 1.0f);
		addVertex(data, x0, halfWidth, z0, normal0.x, 0.0f, normal0.y, u0, 1.0f);

		addVertex(data, 0.0f, halfWidth, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f);
		addVertex(data, x1, halfWidth, z1, 0.0f, 1.0f, 0.0f, 0.5f + x1, 0.5f + z1);
		addVertex(data, x0, halfWidth, z0, 0.0f, 1.0f, 0.0f, 0.5f + x0, 0.5f + z0);

		addVertex(data, 0.0f, -halfWidth, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f);
		addVertex(data, x0, -halfWidth, z0, 0.0f, -1.0f, 0.0f, 0.5f + x0, 0.5f + z0);
		addVertex(data, x1, -halfWidth, z1, 0.0f, -1.0f, 0.0f, 0.5f + x1, 0.5f + z1);
	}

	mesh.upload(data);
	return mesh;
}

EngineMesh EngineMesh::createSprocket(int toothCount) {
	EngineMesh mesh;
	std::vector<float> data;
	const int teeth = std::max(toothCount, 8);
	const int pointsPerTooth = 6;
	const int profilePoints = teeth * pointsPerTooth;
	const float halfWidth = 0.5f;
	const float rootRadius = 0.74f;
	const float tipRadius = 0.90f;

	const auto profileRadius = [&](int point) {
		switch (point % pointsPerTooth) {
		case 0:
		case 5:
			return rootRadius;
		case 1:
		case 4:
			return glm::mix(rootRadius, tipRadius, 0.62f);
		default:
			return tipRadius;
		}
	};

	for (int point = 0; point < profilePoints; ++point) {
		const int next = (point + 1) % profilePoints;
		const float a0 = 2.0f * PI * static_cast<float>(point) / static_cast<float>(profilePoints);
		const float a1 = 2.0f * PI * static_cast<float>(next) / static_cast<float>(profilePoints);
		const float r0 = profileRadius(point);
		const float r1 = profileRadius(next);
		const float x0 = std::cos(a0) * r0;
		const float z0 = std::sin(a0) * r0;
		const float x1 = std::cos(a1) * r1;
		const float z1 = std::sin(a1) * r1;

		addVertex(data, 0.0f, halfWidth, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f);
		addVertex(data, x1, halfWidth, z1, 0.0f, 1.0f, 0.0f, 0.5f + x1 * 0.5f, 0.5f + z1 * 0.5f);
		addVertex(data, x0, halfWidth, z0, 0.0f, 1.0f, 0.0f, 0.5f + x0 * 0.5f, 0.5f + z0 * 0.5f);

		addVertex(data, 0.0f, -halfWidth, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f);
		addVertex(data, x0, -halfWidth, z0, 0.0f, -1.0f, 0.0f, 0.5f + x0 * 0.5f, 0.5f + z0 * 0.5f);
		addVertex(data, x1, -halfWidth, z1, 0.0f, -1.0f, 0.0f, 0.5f + x1 * 0.5f, 0.5f + z1 * 0.5f);

		const glm::vec3 edge(x1 - x0, 0.0f, z1 - z0);
		const glm::vec3 sideNormal = glm::normalize(glm::vec3(edge.z, 0.0f, -edge.x));
		const float u0 = static_cast<float>(point) / static_cast<float>(profilePoints);
		const float u1 = static_cast<float>(point + 1) / static_cast<float>(profilePoints);

		addVertex(data, x0, -halfWidth, z0, sideNormal.x, 0.0f, sideNormal.z, u0, 0.0f);
		addVertex(data, x1, -halfWidth, z1, sideNormal.x, 0.0f, sideNormal.z, u1, 0.0f);
		addVertex(data, x1, halfWidth, z1, sideNormal.x, 0.0f, sideNormal.z, u1, 1.0f);
		addVertex(data, x0, -halfWidth, z0, sideNormal.x, 0.0f, sideNormal.z, u0, 0.0f);
		addVertex(data, x1, halfWidth, z1, sideNormal.x, 0.0f, sideNormal.z, u1, 1.0f);
		addVertex(data, x0, halfWidth, z0, sideNormal.x, 0.0f, sideNormal.z, u0, 1.0f);
	}

	mesh.upload(data);
	return mesh;
}

EngineMesh EngineMesh::createHalfCylinder(int segments) {
	EngineMesh mesh;
	std::vector<float> data;
	const float radius = 0.5f;
	const float halfHeight = 0.5f;
	const float startAngle = 0.0f;
	const float endAngle = PI;

	for (int i = 0; i < segments; ++i) {
		const float t0 = static_cast<float>(i) / static_cast<float>(segments);
		const float t1 = static_cast<float>(i + 1) / static_cast<float>(segments);
		const float a0 = startAngle + (endAngle - startAngle) * t0;
		const float a1 = startAngle + (endAngle - startAngle) * t1;
		const float x0 = std::cos(a0) * radius;
		const float z0 = std::sin(a0) * radius;
		const float x1 = std::cos(a1) * radius;
		const float z1 = std::sin(a1) * radius;

		addVertex(data, x0, -halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0), t0, 0.0f);
		addVertex(data, x1, -halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1), t1, 0.0f);
		addVertex(data, x1, halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1), t1, 1.0f);
		addVertex(data, x0, -halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0), t0, 0.0f);
		addVertex(data, x1, halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1), t1, 1.0f);
		addVertex(data, x0, halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0), t0, 1.0f);

	}

	mesh.upload(data);
	return mesh;
}

EngineMesh EngineMesh::createPortedHalfCylinder(int arcSegments, int axialSegments, bool portsOnPositiveSide) {
	EngineMesh mesh;
	std::vector<float> data;
	const int arcs = arcSegments < 12 ? 12 : arcSegments;
	const int rows = axialSegments < 24 ? 24 : axialSegments;
	const float radius = 0.5f;
	const float portPositions[4] = {
		-2.1f / 6.0f,
		-0.7f / 6.0f,
		0.7f / 6.0f,
		2.1f / 6.0f
	};
	const float axialRadius = 0.055f;
	const float edgeAngle = 0.62f;

	const auto isPortCell = [&](float axial, float angle) {
		const float sideDistance = portsOnPositiveSide ? angle : PI - angle;
		if (sideDistance > edgeAngle) {
			return false;
		}
		for (float port : portPositions) {
			const float normalizedAxial = (axial - port) / axialRadius;
			const float normalizedAngle = sideDistance / edgeAngle;
			if (normalizedAxial * normalizedAxial + normalizedAngle * normalizedAngle < 1.0f) {
				return true;
			}
		}
		return false;
	};

	for (int row = 0; row < rows; ++row) {
		const float v0 = static_cast<float>(row) / static_cast<float>(rows);
		const float v1 = static_cast<float>(row + 1) / static_cast<float>(rows);
		const float y0 = -0.5f + v0;
		const float y1 = -0.5f + v1;
		const float middleY = (y0 + y1) * 0.5f;

		for (int arc = 0; arc < arcs; ++arc) {
			const float u0 = static_cast<float>(arc) / static_cast<float>(arcs);
			const float u1 = static_cast<float>(arc + 1) / static_cast<float>(arcs);
			const float a0 = u0 * PI;
			const float a1 = u1 * PI;
			const float middleAngle = (a0 + a1) * 0.5f;
			if (isPortCell(middleY, middleAngle)) {
				continue;
			}

			const float x0 = std::cos(a0) * radius;
			const float z0 = std::sin(a0) * radius;
			const float x1 = std::cos(a1) * radius;
			const float z1 = std::sin(a1) * radius;

			addVertex(data, x0, y0, z0, std::cos(a0), 0.0f, std::sin(a0), u0, v0);
			addVertex(data, x1, y0, z1, std::cos(a1), 0.0f, std::sin(a1), u1, v0);
			addVertex(data, x1, y1, z1, std::cos(a1), 0.0f, std::sin(a1), u1, v1);
			addVertex(data, x0, y0, z0, std::cos(a0), 0.0f, std::sin(a0), u0, v0);
			addVertex(data, x1, y1, z1, std::cos(a1), 0.0f, std::sin(a1), u1, v1);
			addVertex(data, x0, y1, z0, std::cos(a0), 0.0f, std::sin(a0), u0, v1);
		}
	}

	mesh.upload(data);
	return mesh;
}

EngineMesh EngineMesh::createHalfDisk(int segments) {
	EngineMesh mesh;
	std::vector<float> data;
	const float radius = 0.5f;
	const float halfHeight = 0.5f;
	const float startAngle = 0.0f;
	const float endAngle = PI;

	for (int i = 0; i < segments; ++i) {
		const float t0 = static_cast<float>(i) / static_cast<float>(segments);
		const float t1 = static_cast<float>(i + 1) / static_cast<float>(segments);
		const float a0 = startAngle + (endAngle - startAngle) * t0;
		const float a1 = startAngle + (endAngle - startAngle) * t1;
		const float x0 = std::cos(a0) * radius;
		const float z0 = std::sin(a0) * radius;
		const float x1 = std::cos(a1) * radius;
		const float z1 = std::sin(a1) * radius;

		addVertex(data, 0.0f, halfHeight, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f);
		addVertex(data, x1, halfHeight, z1, 0.0f, 1.0f, 0.0f, 0.5f + x1, 0.5f + z1);
		addVertex(data, x0, halfHeight, z0, 0.0f, 1.0f, 0.0f, 0.5f + x0, 0.5f + z0);

		addVertex(data, 0.0f, -halfHeight, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f);
		addVertex(data, x0, -halfHeight, z0, 0.0f, -1.0f, 0.0f, 0.5f + x0, 0.5f + z0);
		addVertex(data, x1, -halfHeight, z1, 0.0f, -1.0f, 0.0f, 0.5f + x1, 0.5f + z1);

		addVertex(data, x0, -halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0), t0, 0.0f);
		addVertex(data, x1, -halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1), t1, 0.0f);
		addVertex(data, x1, halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1), t1, 1.0f);
		addVertex(data, x0, -halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0), t0, 0.0f);
		addVertex(data, x1, halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1), t1, 1.0f);
		addVertex(data, x0, halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0), t0, 1.0f);
	}

	addVertex(data, -radius, -halfHeight, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	addVertex(data, -radius, halfHeight, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	addVertex(data, radius, halfHeight, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);
	addVertex(data, -radius, -halfHeight, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	addVertex(data, radius, halfHeight, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);
	addVertex(data, radius, -halfHeight, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	mesh.upload(data);
	return mesh;
}

EngineMesh EngineMesh::createValvePlate(int gridResolution, int holeSegments) {
	EngineMesh mesh;
	std::vector<float> data;
	const float radius = 0.5f;
	const float halfHeight = 0.5f;
	const float valveHoleRadius = 0.105f;
	const float injectorHoleRadius = 0.052f;
	const float holeX[5] = {-0.227f, 0.227f, -0.227f, 0.227f, 0.0f};
	const float holeZ[5] = {-0.205f, -0.205f, 0.205f, 0.205f, 0.0f};
	const float holeRadius[5] = {
		valveHoleRadius,
		valveHoleRadius,
		valveHoleRadius,
		valveHoleRadius,
		injectorHoleRadius
	};

	const auto isSolid = [&](float x, float z) {
		if (x * x + z * z > radius * radius) {
			return false;
		}
		for (int hole = 0; hole < 5; ++hole) {
			const float dx = x - holeX[hole];
			const float dz = z - holeZ[hole];
			if (dx * dx + dz * dz < holeRadius[hole] * holeRadius[hole]) {
				return false;
			}
		}
		return true;
	};

	const auto addSurfaceTriangle = [&](float y, float normalY, float ax, float az, float bx, float bz, float cx, float cz) {
		addVertex(data, ax, y, az, 0.0f, normalY, 0.0f, ax + 0.5f, az + 0.5f);
		addVertex(data, bx, y, bz, 0.0f, normalY, 0.0f, bx + 0.5f, bz + 0.5f);
		addVertex(data, cx, y, cz, 0.0f, normalY, 0.0f, cx + 0.5f, cz + 0.5f);
	};

	const int resolution = gridResolution < 24 ? 24 : gridResolution;
	const float step = 1.0f / static_cast<float>(resolution);
	for (int xIndex = 0; xIndex < resolution; ++xIndex) {
		for (int zIndex = 0; zIndex < resolution; ++zIndex) {
			const float x0 = -0.5f + static_cast<float>(xIndex) * step;
			const float x1 = x0 + step;
			const float z0 = -0.5f + static_cast<float>(zIndex) * step;
			const float z1 = z0 + step;

			if (isSolid(x0, z0) && isSolid(x1, z1) && isSolid(x1, z0)) {
				addSurfaceTriangle(halfHeight, 1.0f, x0, z0, x1, z1, x1, z0);
				addSurfaceTriangle(-halfHeight, -1.0f, x0, z0, x1, z0, x1, z1);
			}
			if (isSolid(x0, z0) && isSolid(x0, z1) && isSolid(x1, z1)) {
				addSurfaceTriangle(halfHeight, 1.0f, x0, z0, x0, z1, x1, z1);
				addSurfaceTriangle(-halfHeight, -1.0f, x0, z0, x1, z1, x0, z1);
			}
		}
	}

	const int wallSegments = holeSegments < 16 ? 16 : holeSegments;
	for (int segment = 0; segment < wallSegments; ++segment) {
		const float a0 = 2.0f * PI * static_cast<float>(segment) / static_cast<float>(wallSegments);
		const float a1 = 2.0f * PI * static_cast<float>(segment + 1) / static_cast<float>(wallSegments);
		const float x0 = std::cos(a0) * radius;
		const float z0 = std::sin(a0) * radius;
		const float x1 = std::cos(a1) * radius;
		const float z1 = std::sin(a1) * radius;
		const float u0 = static_cast<float>(segment) / static_cast<float>(wallSegments);
		const float u1 = static_cast<float>(segment + 1) / static_cast<float>(wallSegments);

		addVertex(data, x0, -halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0), u0, 0.0f);
		addVertex(data, x1, -halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1), u1, 0.0f);
		addVertex(data, x1, halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1), u1, 1.0f);
		addVertex(data, x0, -halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0), u0, 0.0f);
		addVertex(data, x1, halfHeight, z1, std::cos(a1), 0.0f, std::sin(a1), u1, 1.0f);
		addVertex(data, x0, halfHeight, z0, std::cos(a0), 0.0f, std::sin(a0), u0, 1.0f);
	}

	for (int hole = 0; hole < 5; ++hole) {
		for (int segment = 0; segment < wallSegments; ++segment) {
			const float a0 = 2.0f * PI * static_cast<float>(segment) / static_cast<float>(wallSegments);
			const float a1 = 2.0f * PI * static_cast<float>(segment + 1) / static_cast<float>(wallSegments);
			const float x0 = holeX[hole] + std::cos(a0) * holeRadius[hole];
			const float z0 = holeZ[hole] + std::sin(a0) * holeRadius[hole];
			const float x1 = holeX[hole] + std::cos(a1) * holeRadius[hole];
			const float z1 = holeZ[hole] + std::sin(a1) * holeRadius[hole];
			const float nx0 = -std::cos(a0);
			const float nz0 = -std::sin(a0);
			const float nx1 = -std::cos(a1);
			const float nz1 = -std::sin(a1);
			const float u0 = static_cast<float>(segment) / static_cast<float>(wallSegments);
			const float u1 = static_cast<float>(segment + 1) / static_cast<float>(wallSegments);

			addVertex(data, x0, -halfHeight, z0, nx0, 0.0f, nz0, u0, 0.0f);
			addVertex(data, x0, halfHeight, z0, nx0, 0.0f, nz0, u0, 1.0f);
			addVertex(data, x1, halfHeight, z1, nx1, 0.0f, nz1, u1, 1.0f);
			addVertex(data, x0, -halfHeight, z0, nx0, 0.0f, nz0, u0, 0.0f);
			addVertex(data, x1, halfHeight, z1, nx1, 0.0f, nz1, u1, 1.0f);
			addVertex(data, x1, -halfHeight, z1, nx1, 0.0f, nz1, u1, 0.0f);
		}
	}

	mesh.upload(data);
	return mesh;
}

EngineMesh EngineMesh::createRing(int segments, float innerRadius) {
	EngineMesh mesh;
	std::vector<float> data;
	const float outerRadius = 0.5f;
	const float inner = std::max(0.05f, std::min(innerRadius, 0.48f));
	const float halfHeight = 0.5f;
	const int ringSegments = segments < 16 ? 16 : segments;

	for (int segment = 0; segment < ringSegments; ++segment) {
		const float a0 = 2.0f * PI * static_cast<float>(segment) / static_cast<float>(ringSegments);
		const float a1 = 2.0f * PI * static_cast<float>(segment + 1) / static_cast<float>(ringSegments);
		const float cos0 = std::cos(a0);
		const float sin0 = std::sin(a0);
		const float cos1 = std::cos(a1);
		const float sin1 = std::sin(a1);
		const float ox0 = cos0 * outerRadius;
		const float oz0 = sin0 * outerRadius;
		const float ox1 = cos1 * outerRadius;
		const float oz1 = sin1 * outerRadius;
		const float ix0 = cos0 * inner;
		const float iz0 = sin0 * inner;
		const float ix1 = cos1 * inner;
		const float iz1 = sin1 * inner;
		const float u0 = static_cast<float>(segment) / static_cast<float>(ringSegments);
		const float u1 = static_cast<float>(segment + 1) / static_cast<float>(ringSegments);

		addVertex(data, ix0, halfHeight, iz0, 0.0f, 1.0f, 0.0f, u0, 0.0f);
		addVertex(data, ox1, halfHeight, oz1, 0.0f, 1.0f, 0.0f, u1, 1.0f);
		addVertex(data, ox0, halfHeight, oz0, 0.0f, 1.0f, 0.0f, u0, 1.0f);
		addVertex(data, ix0, halfHeight, iz0, 0.0f, 1.0f, 0.0f, u0, 0.0f);
		addVertex(data, ix1, halfHeight, iz1, 0.0f, 1.0f, 0.0f, u1, 0.0f);
		addVertex(data, ox1, halfHeight, oz1, 0.0f, 1.0f, 0.0f, u1, 1.0f);

		addVertex(data, ix0, -halfHeight, iz0, 0.0f, -1.0f, 0.0f, u0, 0.0f);
		addVertex(data, ox0, -halfHeight, oz0, 0.0f, -1.0f, 0.0f, u0, 1.0f);
		addVertex(data, ox1, -halfHeight, oz1, 0.0f, -1.0f, 0.0f, u1, 1.0f);
		addVertex(data, ix0, -halfHeight, iz0, 0.0f, -1.0f, 0.0f, u0, 0.0f);
		addVertex(data, ox1, -halfHeight, oz1, 0.0f, -1.0f, 0.0f, u1, 1.0f);
		addVertex(data, ix1, -halfHeight, iz1, 0.0f, -1.0f, 0.0f, u1, 0.0f);

		addVertex(data, ox0, -halfHeight, oz0, cos0, 0.0f, sin0, u0, 0.0f);
		addVertex(data, ox1, -halfHeight, oz1, cos1, 0.0f, sin1, u1, 0.0f);
		addVertex(data, ox1, halfHeight, oz1, cos1, 0.0f, sin1, u1, 1.0f);
		addVertex(data, ox0, -halfHeight, oz0, cos0, 0.0f, sin0, u0, 0.0f);
		addVertex(data, ox1, halfHeight, oz1, cos1, 0.0f, sin1, u1, 1.0f);
		addVertex(data, ox0, halfHeight, oz0, cos0, 0.0f, sin0, u0, 1.0f);

		addVertex(data, ix0, -halfHeight, iz0, -cos0, 0.0f, -sin0, u0, 0.0f);
		addVertex(data, ix1, halfHeight, iz1, -cos1, 0.0f, -sin1, u1, 1.0f);
		addVertex(data, ix1, -halfHeight, iz1, -cos1, 0.0f, -sin1, u1, 0.0f);
		addVertex(data, ix0, -halfHeight, iz0, -cos0, 0.0f, -sin0, u0, 0.0f);
		addVertex(data, ix0, halfHeight, iz0, -cos0, 0.0f, -sin0, u0, 1.0f);
		addVertex(data, ix1, halfHeight, iz1, -cos1, 0.0f, -sin1, u1, 1.0f);
	}

	mesh.upload(data);
	return mesh;
}
