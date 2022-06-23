#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec2 vUV;
layout(location = 3) in vec3 vNormal;

uniform mat4 world, view, projection;

out vec3 color;
out vec2 uv;
out vec3 normal;
out vec4 worldPix;

uniform sampler2D heightmap;

void main() {
	//mat4 trs = world * view * projection;
	worldPix = world * vec4(vPos, 1.0f);

	vec4 diffuseColor = texture(heightmap, vUV);
	diffuseColor.r -= 0.8f;
	diffuseColor.r = -abs(diffuseColor.r);
	diffuseColor.r += 0.8f;

	worldPix.y += diffuseColor.r * 100;

	diffuseColor = texture(heightmap, vUV);

	gl_Position = projection * view * worldPix;
	color = vColor;
	uv = vUV;
	normal = mat3(world) * vNormal;
}