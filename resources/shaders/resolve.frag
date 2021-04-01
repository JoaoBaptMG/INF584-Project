#version 450

#include "gbuffer.glsl"
#include "lighting.glsl"

struct MaterialDefinition
{
	vec3 specularColor;
};

uniform MaterialDefinition Material;

uniform mat4 InvViewShadowViewProjection;

out vec4 fragColor;

void main()
{
	GBufferData data = readFromGbuffer();
	vec4 positionLight = InvViewShadowViewProjection * vec4(data.position, 1.0);
	
	// Compute lighting data
	vec3 color = computeLighting(data.color, data.color, Material.specularColor, data.shininess, data.position, data.normal, positionLight);
	fragColor = vec4(color, 1.0);
}
