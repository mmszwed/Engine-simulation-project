#version 330

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 lightSpaceMatrix;

layout (location = 0) in vec4 vertex;
layout (location = 1) in vec4 normal;
layout (location = 2) in vec2 texCoord;

out vec3 fragPosView;
out vec3 normalView;
out vec2 uv;
out vec4 fragPosLightSpace;

void main(void) {
	vec4 viewPos = V * M * vertex;
	mat3 normalMatrix = transpose(inverse(mat3(V * M)));

	fragPosView = viewPos.xyz;
	normalView = normalize(normalMatrix * normal.xyz);
	uv = texCoord;
	fragPosLightSpace = lightSpaceMatrix * M * vertex;
	gl_Position = P * viewPos;
}
