// VERTEX SHADER
#version 330

//Bone Transform
#define MAX_BONES 100

uniform mat4 bones[MAX_BONES];

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

//Vertex attributes
in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord; 
in vec3 aTangent;
in vec3 aBiTangent;

in ivec4 aBoneId;
in vec4 aBoneWeight;

vec4 position;
vec3 normal;


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



}
