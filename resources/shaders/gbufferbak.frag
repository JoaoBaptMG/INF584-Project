#version 450

struct MaterialDefinition
{
	vec3 specularColor;
	float shininess;
};

struct DirectionalLight
{
	vec3 directionView;
	vec3 ambient, diffuse, specular;
};

uniform DirectionalLight Light;
uniform MaterialDefinition Material;

uniform sampler2DShadow ShadowMapTexture;

in vec3 position;
in vec4 positionLight;
in vec3 normal;
in vec3 color;

out vec4 fragColor;

float computeLightFactor(vec4 positionLight)
{
	vec3 projectedTexcoord = positionLight.xyz / positionLight.w * 0.5 + 0.5;
	projectedTexcoord.z -= max(0.005 * (1.0 - dot(normalize(normal), -Light.directionView)), 0.0005);
	return texture(ShadowMapTexture, projectedTexcoord);
}

vec4 computeLighting(vec3 ambient, vec3 diffuse, vec3 specular, float shininess)
{
	// get the ambient contribution
	ambient = Light.ambient * ambient;

	// diffuse contribution
	vec3 norm = normalize(normal);
	float cosd = max(dot(norm, -Light.directionView), 0.0);
	diffuse = Light.diffuse * (cosd * diffuse);

	// specular contribution
	vec3 viewDirection = normalize(-position);
	vec3 halfwayDir = normalize(-Light.directionView + viewDirection);
	float energyCons = (8.0 + shininess) / 25.1327412287;
	float specf = energyCons * pow(max(dot(halfwayDir, norm), 0.0), shininess);
	specular = Light.specular * (specf * specular);

	// Shadow (add some bias)
	float light = computeLightFactor(positionLight);
	return vec4(ambient + light * (diffuse + specular), 1.0);
}

void main()
{
	fragColor = computeLighting(color, color, Material.specularColor, Material.shininess);
}
