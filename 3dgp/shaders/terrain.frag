#version 330

uniform bool useNormalMap = false; //normal
vec3 normalNew;// normal map normalV
uniform sampler2D textureNormal; //normal
in mat3 matrixTangent; //normal
uniform mat4 matrixView;
// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// Input Variables (received from Vertex Shader)
in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;

// Water-related
uniform vec3 waterColor;

//textures
uniform sampler2D textureBed;
uniform sampler2D textureShore;


// Input: Water Related
in float waterDepth;  // water depth (positive for underwater, negative for the shore)
in float fogFactor;

// Output Variable (sent down through the Pipeline)
out vec4 outColor;

struct DIRECTIONAL
{	
	vec3 direction;
	vec3 diffuse;
};
uniform DIRECTIONAL lightDir;

vec4 DirectionalLight(DIRECTIONAL light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normalNew, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
}

//PointLight
struct POINT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	mat4 matrix; //
};
uniform POINT lightPoint;

vec4 PointLight(POINT light)
{
	// Calculate Point Light
	vec4 outColor = vec4(0, 0, 0, 0);
	// calculation code to be inserted in place of this comment
	vec3 L = normalize(light.matrix * vec4(light.position, 1) - position).xyz;    //matrixView changed by light.matrix
	float NdotL = dot(normalNew, L); //convert to normalNew in fragment shader
	if (NdotL > 0)
		outColor += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	//Specular - Shininess
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normalNew); //convert to normalNew in fragment shader
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
	    outColor += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	return color;

}

void main(void) 
{

	if (useNormalMap)
	{
		normalNew = 2.0 * texture(textureNormal, texCoord0).xyz - vec3(1.0, 1.0, 1.0);
		normalNew = normalize(matrixTangent * normalNew);
		
	}
	 else
		normalNew = normal;

	outColor = color;
	outColor += DirectionalLight(lightDir);

	// shoreline multitexturing
	float isAboveWater = clamp(-waterDepth, 0, 1); 
	outColor *= mix(texture(textureBed, texCoord0), texture(textureShore, texCoord0), isAboveWater);

	//underwater fog
	outColor = mix(vec4(waterColor, 1), outColor, fogFactor);
	
	
}