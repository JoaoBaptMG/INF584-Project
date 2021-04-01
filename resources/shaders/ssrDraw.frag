#version 450

uniform sampler2D ResolveTexture;
uniform isampler2D SSRTexcoordTexture;
uniform sampler2D SSRVisibilityTexture;

out vec4 fragColor;

void main()
{
	ivec2 fragCoord = ivec2(gl_FragCoord.xy);
	fragColor = texelFetch(ResolveTexture, fragCoord, 0);

	ivec2 ssrCoord = texelFetch(SSRTexcoordTexture, fragCoord, 0).xy;
	float visibility = texelFetch(SSRVisibilityTexture, fragCoord, 0).r;
	if (ssrCoord.x >= 0 && ssrCoord.y >= 0)
		fragColor += texelFetch(ResolveTexture, ssrCoord, 0) * visibility;
	fragColor.a = 1.0;
}
