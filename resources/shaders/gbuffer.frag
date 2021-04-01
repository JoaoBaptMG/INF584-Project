#version 450

in vec3 position;
in vec4 positionLight;
in vec3 normal;
in vec4 color;
in float shininess;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;
layout(location = 2) out vec2 outSpecularShininess;

void main()
{
	vec3 norm = normalize(normal);

	outColor = vec4(color.xyz, norm.z < 0);
	outNormal = norm.xy;
	outSpecularShininess = vec2(color.w, shininess);
}
