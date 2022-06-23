#version 330 core

out vec4 FragColor;

in vec4 worldPix;

uniform vec3 camPos;
uniform vec3 lightDirection;

vec3 lerp(vec3 a, vec3 b, float t) {
	return a + (b - a) * t;
}

void main() {
	vec3 lightDir = normalize(lightDirection);
	vec3 viewDir = normalize(worldPix.xyz - camPos);

	vec3 top = vec3(20, 20, 20) / 255.0;
	vec3 bot = vec3(70, 70, 100) / 255.0;

	float moon = pow(max(smoothstep(0.99, 0.999, dot(-viewDir, lightDir)), 0.0), 40);

	FragColor = vec4(lerp(bot, top, viewDir.y) + moon * vec3(150, 150, 150) / 255, 1.0);
}