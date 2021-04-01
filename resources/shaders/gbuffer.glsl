//? #version 450

uniform sampler2D ColorTexture;
uniform sampler2D DepthTexture;
uniform sampler2D NormalTexture;
uniform sampler2D SpecularShininessTexture;
uniform mat4 InverseProjection;
uniform vec2 ScreenSize;

struct GBufferData
{
	vec3 position, normal, color;
	float specular, shininess, ssDepth;
	ivec2 fragCoord;
};

GBufferData readFromGbuffer()
{
	GBufferData result;

	// Get all the required values
	ivec2 fragCoord = ivec2(gl_FragCoord.xy);
	
	vec4 colorData = texelFetch(ColorTexture, fragCoord, 0);
	float depthData = texelFetch(DepthTexture, fragCoord, 0).r;
	vec2 normalData = texelFetch(NormalTexture, fragCoord, 0).rg;
	vec2 specShinyData = texelFetch(SpecularShininessTexture, fragCoord, 0).rg;

	// Discard if depth data is still the same
	if (depthData == 1.0) discard;

	// Reconstruct the data
	result.color = colorData.rgb;
	result.normal = vec3(normalData, sqrt(1.0 - dot(normalData, normalData)));
	if (colorData.a == 1.0) result.normal.z = -result.normal.z;

	// Reconstruct the position
	vec3 deviceCoords = 2.0 * vec3(gl_FragCoord.xy / ScreenSize, depthData) - 1.0;
	vec4 homogeneousPos = InverseProjection * vec4(deviceCoords, 1.0);
	result.position = homogeneousPos.xyz / homogeneousPos.w;

	result.ssDepth = depthData;
	result.fragCoord = fragCoord;
	result.specular = specShinyData.r;
	result.shininess = specShinyData.g;

	return result;
}