#version 330 core

out vec4 FragColor;

in vec3 color;
in vec2 uv;
in vec3 normal;

uniform vec3 lightDirection;
uniform vec3 camPos;
uniform sampler2D diffuseSampler;

void main() {
	//FragColor = vec4(1, 1, 1, 1);

	vec4 diffuseColor = texture(diffuseSampler, uv);

	FragColor = diffuseColor;// vec4(diffuseColor.rgb, 1.0f);
}