#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in vec4 pos;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float shadow;

float constant = 0.5f;
float linear = 0.000045f;
float quadratic = 0.000075f;

uniform int yellow_light;

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	//vec3 lightDirN = normalize(lightDir);
	vec3 lightDirN = normalize(lightDir - fPosEye.xyz);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;

	//attenuation
	float distance = length(lightDir - fPosEye.xyz);
	float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
}

float computeShadow()
{
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;

	// Check whether current frag pos is in shadow
	//float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
	
	//return shadow;
	float bias = 0.005f;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	return shadow;
}

float computeFog()
{
 float fogDensity = 0.005f;
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}


//POINT LIGHT
uniform vec3 Lposition;
uniform	vec3 lightColor2;

uniform vec3 Lposition2;
uniform	vec3 lightColor3;


vec3 ambient2;
vec3 diffuse2;
vec3 specular2;

float ambientStrength2 = 0.2f;
float specularStrength2 = 0.5f;
float shininess2 = 32.0f;

float constant1 = 1.0f;
float linear1 = 0.0045f;
float quadratic1 = 0.0075f;

vec3 computePointComponents(vec3 lightPosition, vec3 lightCol)
{
	vec3 cameraPosEye = vec3(0.0f);

	//compute light direction
	//vec3 LDir = normalize(Lposition - pos.xyz);
	vec3 LDir = normalize(lightPosition - pos.xyz);

	//compute ambient light
	//ambient2 = ambientStrength2 * lightColor2;
	ambient2 = ambientStrength2 * lightCol;

	vec3 normalEye = normalize(fNormal);

	//diffuse shading
	float diff = max(dot(normalEye, LDir), 0.0f);
	//diffuse2 = diff * lightColor2;
	diffuse2 = diff * lightCol;

	//specular shading
	vec3 reflectDir = reflect(-LDir, normalEye);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	float spec = pow(max(dot(viewDirN, reflectDir), 0.0f), shininess2);
	//specular2 = specularStrength2 * spec * lightColor2;
	specular2 = specularStrength2 * spec * lightCol;

	//attenuation
	float distance = length(lightPosition - pos.xyz);
	float attenuation = 1.0f / (constant1 + linear1 * distance + quadratic1 * (distance * distance));

	ambient2 *= attenuation * texture(diffuseTexture, fTexCoords).rgb;;
	diffuse2 *= attenuation * texture(diffuseTexture, fTexCoords).rgb;;
	specular2 *= attenuation * texture(specularTexture, fTexCoords).rgb;;

	return (ambient2 + diffuse2 + specular2);
}

void main() 
{
	computeLightComponents();
	
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	
	vec3 l2 = computePointComponents(Lposition, lightColor2);

	
	constant1 = 1.0f;
	linear1 = 0.07f;
	quadratic1 = 0.017f;
	vec3 l3 = computePointComponents(Lposition2, lightColor3);


	shadow = computeShadow();

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.0f, 0.5f, 0.5f, 1.0f);
	vec4 fogC = vec4(0.0f, 0.0f, 0.25f, 1.0f);

	if(yellow_light == 0)
	{
		vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
		color += l2;
		fColor = fogColor * (1.0 - fogFactor) + color * fogFactor + fogC * (1.0 - fogFactor) + color * fogFactor;
	}
	else
	{
		vec3 color = vec3(0.0f, 0.015f, 0.03f);
		color += l3;
		fColor = vec4(color, 1.0f);
	}
	
	
}
