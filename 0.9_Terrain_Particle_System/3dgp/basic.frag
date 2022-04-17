// FRAGMENT SHADER

#version 330

in vec4 color;
out vec4 outColor;

in vec4 position;
in vec3 normal; //"in" removed for normal map if you dont have new normal
vec3 normalNew;// normal map normalV
uniform bool useNormalMap = false; //normal

//textures
uniform sampler2D texture0;  //texture
in vec2 texCoord0;  //texture
in mat3 matrixTangent; //normal
uniform sampler2D textureNormal; //normal

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// Debug Text - Do Not Change!
uniform int Text = 0;

// View Matrix
uniform mat4 matrixView;

uniform vec3 fogColour;
in float fogFactor;


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

	return outColor;

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
	outColor += PointLight(lightPoint);

	outColor *= texture(texture0, texCoord0); //texture
	//outColor = vec4(normal * .5 + .5, 1.0);
	outColor = mix(vec4(fogColour, 1), outColor, fogFactor);
		// Debug Text - Do Not Change!
	if (Text == 1) outColor = vec4(1, 0.8, 0, 1);

	
}
