#include "../GL/glew.h"
#include "../GL/3dglShader.h"
#include "../GL/3dglBitmap.h"
#include "../GL/3dglSkyBox.h"

using namespace _3dgl;
using namespace std;

C3dglSkyBox::C3dglSkyBox()
{
}

bool C3dglSkyBox::load(const char* pFd, const char* pRt, const char* pBk, const char* pLt, const char* pUp, const char* pDn) 
{
	glGenTextures(6, m_idTex);

	// load six textures
	glActiveTexture(GL_TEXTURE0);
	const char*pFilenames[] = { pBk, pRt, pFd, pLt, pUp, pDn };
	for (int i = 0; i < 6; ++i)
	{
		C3dglBitmap bm(pFilenames[i], GL_RGBA);
		glGenTextures(1, &m_idTex[i]);
		glBindTexture(GL_TEXTURE_2D, m_idTex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), abs(bm.GetHeight()), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());
	}

	float vertices[] = 
	{
		-1.0f,-1.0f,-1.0f,	 1.0f,-1.0f,-1.0f,	 1.0f, 1.0f,-1.0f,	-1.0f, 1.0f,-1.0f,	//Back
		-1.0f,-1.0f,-1.0f,	-1.0f, 1.0f,-1.0f,	-1.0f, 1.0f, 1.0f,	-1.0f,-1.0f, 1.0f,	//Left
		 1.0f,-1.0f, 1.0f,	-1.0f,-1.0f, 1.0f,	-1.0f, 1.0f, 1.0f,	 1.0f, 1.0f, 1.0f,	//Front
		 1.0f,-1.0f, 1.0f,	 1.0f, 1.0f, 1.0f,	 1.0f, 1.0f,-1.0f,	 1.0f,-1.0f,-1.0f,	//Right
		-1.0f, 1.0f, -1.0f,	 1.0f, 1.0f, -1.0f,	 1.0f, 1.0f,  1.0f,	-1.0f, 1.0f,  1.0f,	//Top
		 1.0f,-1.0f, -1.0f,	-1.0f,-1.0f, -1.0f,	-1.0f,-1.0f,  1.0f,	 1.0f,-1.0f,  1.0f	//Bottom
	};

	float normals[] = 
	{
		0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1,			//Back
		1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,			//Left
		0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1,		//Front
		-1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0,		//Right
		0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,		//Top
		0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0			//Bottom
	};

	float textCoord[] = 
	{
		0.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f,		//Back
		1.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f,		0.0f, 0.0f,		//Left
		0.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f,		//Front
		1.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f,		0.0f, 0.0f,		//Right
		0.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f,		//Top
		0.0f, 0.0f,		1.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f		//Bottom
	};

	glGenBuffers(1, &m_vertexBuffer); //Generate a buffer for the vertices
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer); //Bind the vertex buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW); //Send the data to OpenGL

	glGenBuffers(1, &m_normalBuffer); //Generate a buffer for the normals
	glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer); //Bind the normal buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), &normals[0], GL_STATIC_DRAW); //Send the data to OpenGL

	glGenBuffers(1, &m_texCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_texCoordBuffer); //Bind the tex coord buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(textCoord), &textCoord[0], GL_STATIC_DRAW); //Send the data to OpenGL

	return true;
}

void C3dglSkyBox::render(glm::mat4 matrix)
{
	// check if a shading program is active
	C3dglProgram *pProgram = C3dglProgram::GetCurrentProgram();
	if (!pProgram) return;

	// disable depth-buffer write cycles - so that the skybox cannot obscure anything
	GLboolean bDepthMask;
	::glGetBooleanv(GL_DEPTH_WRITEMASK, &bDepthMask);
	glDepthMask(GL_FALSE);

	// get shader configuration
	GLuint attribVertex = pProgram->GetAttribLocation(C3dglProgram::ATTR_VERTEX);
	GLuint attribNormal = pProgram->GetAttribLocation(C3dglProgram::ATTR_NORMAL);
	GLuint attribTexCoord = pProgram->GetAttribLocation(C3dglProgram::ATTR_TEXCOORD);
	GLuint locationMatrixModelView = pProgram->GetUniformLocation(C3dglProgram::UNI_MODELVIEW);

	// send model view matrix
	matrix[3][0] = matrix[3][1] = matrix[3][2] = 0;
	pProgram->SendStandardUniform(C3dglProgram::UNI_MODELVIEW, matrix);

	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribNormal);
	glEnableVertexAttribArray(attribTexCoord);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_texCoordBuffer);
	glVertexAttribPointer(attribTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glActiveTexture(GL_TEXTURE0);
	for (int i = 0; i < 6; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, m_idTex[i]);
		glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
	}

	glDisableVertexAttribArray(attribVertex);
	glDisableVertexAttribArray(attribNormal);
	glDisableVertexAttribArray(attribTexCoord);

	// enable depth-buffer write cycle
	glDepthMask(bDepthMask);
}

void C3dglSkyBox::render()
{
	glm::mat4 m;
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)&m);
	render(m);
}
