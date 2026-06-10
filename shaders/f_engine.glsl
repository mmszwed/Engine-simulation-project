#version 330

in vec3 fragPosView;
in vec3 normalView;
in vec2 uv;
in vec4 fragPosLightSpace;

out vec4 pixelColor;

uniform vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec3 dirLightDirView = vec3(-0.35, -0.85, -0.35);
uniform vec3 pointLightPosView = vec3(2.2, 3.2, 3.2);
uniform vec3 spotDirectionView = vec3(-1.0, -0.2, 0.0);
uniform vec3 workLightPosView = vec3(5.2, 1.1, -2.0);
uniform vec3 workSpotDirectionView = vec3(-1.0, -0.2, 0.2);
uniform float lampIntensity = 0.0;
uniform float shininess = 42.0;
uniform float specularStrength = 0.45;
uniform sampler2D texture0;
uniform sampler2D shadowMap;
uniform int useTexture = 0;
uniform int unlit = 0;

float shadowAmount(vec3 normal, vec3 lightDirection) {
	vec3 projected = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projected = projected * 0.5 + 0.5;
	if (projected.z > 1.0
		|| projected.x < 0.0 || projected.x > 1.0
		|| projected.y < 0.0 || projected.y > 1.0) {
		return 0.0;
	}

	float bias = max(0.0010 * (1.0 - dot(normal, lightDirection)), 0.00030);
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	float shadow = 0.0;
	for (int x = -2; x <= 2; ++x) {
		for (int y = -2; y <= 2; ++y) {
			float closestDepth = texture(shadowMap, projected.xy + vec2(x, y) * texelSize).r;
			shadow += projected.z - bias > closestDepth ? 1.0 : 0.0;
		}
	}
	return shadow / 25.0;
}

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

	vec3 workVector = workLightPosView - fragPosView;
	float workDistance = length(workVector);
	vec3 workLight = normalize(workVector);
	float workAttenuation = 1.0 / (1.0 + 0.045 * workDistance + 0.012 * workDistance * workDistance);
	float workDiffuse = max(dot(n, workLight), 0.0) * workAttenuation;
	vec3 workHalfway = normalize(workLight + viewDir);
	float workSpec = pow(max(dot(n, workHalfway), 0.0), shininess) * workAttenuation;
	vec3 workLightToFragment = normalize(fragPosView - workLightPosView);
	float workSpotCosine = dot(workLightToFragment, normalize(workSpotDirectionView));
	float workSpotFactor = smoothstep(0.70, 0.88, workSpotCosine) * lampIntensity;
	float workVisibility = 1.0 - 0.82 * shadowAmount(n, workLight);

	vec3 texColor = texture(texture0, uv * 3.0).rgb;
	vec3 baseColor = useTexture == 1 ? color.rgb * texColor : color.rgb;
	if (unlit == 1) {
		pixelColor = vec4(baseColor, color.a);
		return;
	}

	vec3 lightToFragment = normalize(fragPosView - pointLightPosView);
	float spotCosine = dot(lightToFragment, normalize(spotDirectionView));
	float spotFactor = smoothstep(0.68, 0.86, spotCosine) * lampIntensity;

	vec3 ambient = baseColor * (0.24 + 0.035 * lampIntensity);
	vec3 diffuse = baseColor * (
		0.62 * dirDiffuse
		+ 0.72 * pointDiffuse * spotFactor
		+ 1.45 * workDiffuse * workSpotFactor * workVisibility
	);
	vec3 specular = vec3(1.0, 0.91, 0.72) * specularStrength
		* (
			0.55 * dirSpec
			+ 0.72 * pointSpec * spotFactor
			+ 1.60 * workSpec * workSpotFactor * workVisibility
		);

	vec3 finalColor = ambient + diffuse + specular;
	pixelColor = vec4(finalColor, color.a);
}
