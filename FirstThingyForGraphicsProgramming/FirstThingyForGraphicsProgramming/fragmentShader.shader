#version 330 core

out vec4 FragColor;

in vec3 color;
in vec2 uv;
in vec3 normal;
in vec4 worldPix;

uniform sampler2D heightMap;
uniform sampler2D normalMap;
uniform sampler2D dirt, sand, grass, rock, snow;

uniform vec3 lightDirection;
uniform vec3 camPos;

vec3 lerp(vec3 a, vec3 b, float t) {
	return a + (b - a) * t;
}

vec3 TriplanerMap(vec4 worldPix, sampler2D tex, float scale) {
	vec3 xy = texture(tex, vec2(worldPix.x, worldPix.y) * scale).rgb;
	vec3 yz = texture(tex, vec2(worldPix.y, worldPix.z) * scale).rgb;
	vec3 zx = texture(tex, vec2(worldPix.z, worldPix.x) * scale).rgb;

	return(xy + yz + zx) / 3;
}

void main() {
	vec3 normalColor = (texture(normalMap, uv).rbg * 2 - 1);
	normalColor.b = -normalColor.b;
	normalColor.r = -normalColor.r;

	vec3 lightDir = normalize(lightDirection);
	float light = max(dot(-lightDir, normalColor), .5f);
	
	vec3 dirtcolor = TriplanerMap(worldPix, dirt, .02);
	vec3 sandcolor = TriplanerMap(worldPix, sand, .02);
	vec3 grasscolor = TriplanerMap(worldPix, grass, .02);
	vec3 rockcolor = TriplanerMap(worldPix, rock, .02);
	vec3 snowcolor = TriplanerMap(worldPix, snow, .02);

	float ds = clamp((worldPix.y - 0) / 10, 0, 1);
	float sg = clamp((worldPix.y - 10) / 10, 0, 1);
	float gr = clamp((worldPix.y - 40) / 10, 0, 1);
	float rs = clamp((worldPix.y - 75) / 10, 0, 1);

	vec3 diffuse = lerp(lerp(lerp(lerp(dirtcolor, sandcolor, ds), grasscolor, sg), rockcolor, gr), snowcolor, rs);

	float dis = distance(worldPix.xyz, camPos);
	float fogAmount = clamp((dis - 100) / 250, 0, 1);

	vec3 bot = vec3(70, 70, 100) / 255.0;

	FragColor = vec4(lerp(diffuse * light, bot, 0), 1.0);
	
	//vec3 lightDir = vec3(0, 0, 1);
	//vec3 camPos = vec3(0, 1, -3);
	//vec3 viewDir = normalize(worldPix.xyz - camPos);
	
	//vec3 reflDir = normalize(reflect(lightDir, normal));
	//float spec = pow(max(-dot(reflDir, viewDir), 0.0), 100);
}