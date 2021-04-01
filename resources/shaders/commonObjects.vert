#version 450

#include "vertexDefs.glsl"

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 ShadowViewProjection;

POSITION in vec4 inPosition;
NORMAL in vec3 inNormal;
COLOR in vec4 inColor;
MODEL in mat4 inModel;

out vec3 position;
out vec4 positionLight;
out vec3 normal;
out vec3 color;

void computePosition()
{
	mat4 modelView = View * inModel;

	vec4 viewPos = modelView * inPosition;
	positionLight = ShadowViewProjection * inModel * inPosition;
	gl_Position = Projection * viewPos;

	position = viewPos.xyz;
	normal = transpose(inverse(mat3(modelView))) * inNormal;
}

void main()
{
	computePosition();
	color = inColor.xyz;
}
