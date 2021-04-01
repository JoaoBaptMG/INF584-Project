#version 450

#include "vertexDefs.glsl"

POSITION in vec4 position;

void main()
{
	gl_Position = position;
}
