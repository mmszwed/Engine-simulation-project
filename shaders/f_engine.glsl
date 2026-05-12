#version 330

in vec3 fragPosView;
in vec3 normalView;
in vec2 uv;

out vec4 pixelColor;

uniform vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec3 dirLightDirView = vec3(-0.35, -0.85, -0.35);
uniform vec3 pointLightPosView = vec3(2.2, 3.2, 3.2);
uniform float shininess = 42.0;
uniform float specularStrength = 0.45;
uniform sampler2D texture0;
uniform int useTexture = 0;

void main(void) {
	vec3 n = normalize(normalView);
	vec3 viewDir = normalize(-fragPosView);

	vec3 dirLight = normalize(-dirLightDirView);
	float dirDiffuse = max(dot(n, dirLight), 0.0);
	vec3 dirHalfway = normalize(dirLight + viewDir);
	float dirSpec = pow(max(dot(n, dirHalfway), 0.0), shininess);

	vec3 pointVector = pointLightPosView - fragPosView;
	float dist = length(pointVector);
	vec3 pointLight = normalize(pointVector);
	float attenuation = 1.0 / (1.0 + 0.08 * dist + 0.025 * dist * dist);
	float pointDiffuse = max(dot(n, pointLight), 0.0) * attenuation;
	vec3 pointHalfway = normalize(pointLight + viewDir);
	float pointSpec = pow(max(dot(n, pointHalfway), 0.0), shininess) * attenuation;

	vec3 texColor = texture(texture0, uv * 3.0).rgb;
	vec3 baseColor = useTexture == 1 ? color.rgb * texColor : color.rgb;
	vec3 ambient = baseColor * 0.36;
	vec3 diffuse = baseColor * (0.88 * dirDiffuse + 0.78 * pointDiffuse);
	vec3 specular = vec3(1.0, 0.96, 0.88) * specularStrength * (dirSpec + pointSpec);

	vec3 finalColor = ambient + diffuse + specular;
	pixelColor = vec4(finalColor, color.a);
}
