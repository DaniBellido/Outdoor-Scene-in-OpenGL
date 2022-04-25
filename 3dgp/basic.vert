// VERTEX SHADER
#version 330
//Bone Transform
#define MAX_BONES 100

uniform mat4 bones[MAX_BONES];

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialEmissive;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;    //texture
out vec2 texCoord0;  //texture
in vec3 aTangent;   //normal
in vec3 aBiTangent; //normal
out mat3 matrixTangent; //normal
in ivec4 aBoneId;
in vec4 aBoneWeight;

out vec4 color;
out vec4 position;
out vec3 normal;

//fog variables
out float fogFactor;
uniform float fogDensity;

// Light declarations

//AMBIENT LIGHT 
struct AMBIENT
{	
	vec3 color;
};
uniform AMBIENT lightAmbient;

vec4 AmbientLight(AMBIENT light)
{
// Calculate Ambient Light
	return vec4(materialAmbient * light.color, 1);
}

//DIRECTIONAL LIGHT
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
	float NdotL = dot(normal, L);
	if (NdotL >0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
}


struct EMISSIVE
{
	vec3 color;
};

uniform EMISSIVE lightEmissive;

vec4 EmissiveLight(EMISSIVE light)
{
	// Calculate Emissive Light
	return vec4(materialEmissive * light.color, 1);
}





void main(void) 
{
	mat4 matrixBone;
	if (aBoneWeight[0] == 0.0)
		matrixBone = mat4(1);
	else
		matrixBone = (bones[aBoneId[0]] * aBoneWeight[0] +
					  bones[aBoneId[1]] * aBoneWeight[1] +
					  bones[aBoneId[2]] * aBoneWeight[2] +
					  bones[aBoneId[3]] * aBoneWeight[3]);

	// calculate position
	position = matrixModelView * matrixBone *vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate normal
	normal = normalize(mat3(matrixModelView) * mat3(matrixBone) *aNormal);


	// calculate light
	color = vec4(0, 0, 0, 1);
	color += AmbientLight(lightAmbient);
	color += DirectionalLight(lightDir);
	color += EmissiveLight(lightEmissive);
	fogFactor = exp2(-fogDensity * length(position));

	// calculate texture coordinate
	texCoord0 = 1 * aTexCoord; //texture

	vec3 aTangent = vec3(1,0,0);
	vec3 aBiTangent = cross(aNormal, aTangent);
	vec3 tangent = normalize(mat3(matrixModelView) * aTangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);	// Gramm-Schmidt process
	vec3 biTangent = normalize(mat3(matrixModelView) * aBiTangent);
	matrixTangent = mat3(tangent, biTangent, normal);



}
