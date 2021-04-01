#version 450

#include "gbuffer.glsl"

const int NumCoarseIterations = 128;
const int CoarsePixels = 4;
const int NumFineIterations = 8;
const float Epsilon = 0.001;

uniform mat4 Projection;

layout(location = 0) out ivec2 ssrTexcoords;
layout(location = 1) out float visibility;

// Advance routine for the first step
void advanceRay(inout vec4 ssCur, vec4 ssRay)
{
	vec2 incr = ScreenSize * abs(ssRay.xy - ssRay.w * ssCur.xy / ssCur.z) * 0.5;
	ssCur += ssRay * CoarsePixels * ssCur.w / (max(incr.x, incr.y) - ssRay.w);
}

void main()
{
	ssrTexcoords = ivec2(-1, -1);
	visibility = 0.0;
	GBufferData data = readFromGbuffer();

	vec3 norm = normalize(data.normal);
	vec3 dir = normalize(data.position);

	// Reflect the light direction and project to screen space
	vec3 reflectedDir = reflect(dir, norm);

	vec4 ssCur = Projection * vec4(data.position, 1.0);
	//ssCur.z = ssCur.w * data.ssDepth;
	vec4 ssRay = Projection * vec4(reflectedDir, 0.0);
	
	// First iteration
	vec4 ssInitial = ssCur;
	vec4 ssPrev = ssCur;
	advanceRay(ssCur, ssRay);

	int i;
	for (i = 0; i <= NumCoarseIterations; i++)
	{
		vec3 ssCurN = ssCur.xyz / ssCur.w;
		// If we go out of bounds, break
		if (abs(ssCurN.x) > 1.0 || abs(ssCurN.y) > 1.0) break;

		vec3 ssCurP = ssCurN * 0.5 + 0.5;
		ivec2 cand = ivec2(ssCurP.xy * ScreenSize);
		float sampleDepth = texelFetch(DepthTexture, cand, 0).r;
		if (ssCurP.z > sampleDepth) break;

		// Compute the correct homogeneous coordinates
		// This guarantees that the normalized coordinates change by at most 1
		ssPrev = ssCur;
		advanceRay(ssCur, ssRay);
	}

	// If we didn't find any reflection, just discard
	if (i == NumCoarseIterations+1) { discard; return; }
	// Now, we do a fine iteration using binary search
	vec4 ssBegin = ssPrev;
	vec4 ssEnd = ssCur;

	for (i = 0; i < NumFineIterations; i++)
	{
		vec4 ssMid = (ssBegin + ssEnd) * 0.5;
		vec3 ssMidN = ssMid.xyz / ssMid.w;
		// If we go out of bounds, discard and return
		if (abs(ssMidN.x) > 1.0 || abs(ssMidN.y) > 1.0) { discard; return; }

		vec3 ssMidP = ssMidN * 0.5 + 0.5;
		ssrTexcoords = ivec2(ssMidP.xy * ScreenSize);
		float sampleDepth = texelFetch(DepthTexture, ssrTexcoords, 0).r;

		// If found, exit
		if (abs(ssMidP.z - sampleDepth) < Epsilon) break;
		else if (ssMidP.z > sampleDepth) ssEnd = ssMid;
		else ssBegin = ssMid;
	}

	if (i == NumFineIterations) { discard; return; }

	// Set the visibility
	visibility = clamp(data.specular * (1.0 - exp(-data.shininess/12.5)), 0.0, 1.0);
}
