#include <iostream>
#include <fstream>
#include "../GL/glew.h"
#include "../GL/3dglMaterial.h"
#include "../GL/3dglShader.h"
#include "../GL/3dglBitmap.h"

// assimp include file
#include "../GL/assimp/cimport.h"

// GLM include files
#include "../glm/vec3.hpp"
#include "../glm/vec4.hpp"
#include "../glm/mat4x4.hpp"
#include "../glm/trigonometric.hpp"
#include "../glm/gtc/type_ptr.hpp"

#include <assert.h>

using namespace std;
using namespace _3dgl;

unsigned CMaterial::c_idTexBlank = 0xFFFFFFFF;

CMaterial::CMaterial()
{
	memset(m_idTexture, 0xFFFFFFFF, sizeof(m_idTexture));
	memset(m_amb, 0, sizeof(m_amb));;
	memset(m_diff, 0, sizeof(m_diff));;
	memset(m_spec, 0, sizeof(m_spec));;
	memset(m_emiss, 0, sizeof(m_emiss));;
	m_shininess = 0.0f;
}

void CMaterial::create(float* pAmbient, float* pDiffuse, float* pSpecular, float* pEmissive, float shininess)
{
	// solid colours
	static float pWhite[3] = { 1.0, 1.0, 1.0 };
	static float pBlack[3] = { 1.0, 1.0, 1.0 };
	
	memcpy(m_diff, pDiffuse ? pDiffuse : pWhite, sizeof(m_diff));
	memcpy(m_amb, pDiffuse ? pDiffuse : pWhite, sizeof(m_amb));
	memcpy(m_spec, pSpecular ? pSpecular : (shininess > 1 ? pWhite : pBlack), sizeof(m_amb));
	memcpy(m_emiss, pEmissive ? pEmissive : pBlack, sizeof(m_amb));
	m_shininess = shininess;
}

void CMaterial::create(const aiMaterial *pMat, const char* pDefTexPath)
{
	const char* pPath = NULL;
	float *pAmbient = NULL, *pDiffuse = NULL, *pSpecular = NULL, *pEmissive = NULL;
	float shininess = 0;

	// texture
	aiString texPath;	// contains filename of texture
	if (pMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
		pPath = texPath.C_Str();
	
	aiColor4D colorAmb, colorDiff, colorSpec, colorEmiss;
	if (AI_SUCCESS == aiGetMaterialColor(pMat, AI_MATKEY_COLOR_AMBIENT, &colorAmb))
		pAmbient = &colorAmb[0];
	if (AI_SUCCESS == aiGetMaterialColor(pMat, AI_MATKEY_COLOR_DIFFUSE, &colorDiff))
		pDiffuse = &colorDiff[0];
	if (AI_SUCCESS == aiGetMaterialColor(pMat, AI_MATKEY_COLOR_SPECULAR, &colorSpec))
		pSpecular = &colorSpec[0];
	if (AI_SUCCESS == aiGetMaterialColor(pMat, AI_MATKEY_COLOR_EMISSIVE, &colorEmiss))
		pEmissive = &colorEmiss[0];
	
	unsigned int max;
	if(AI_SUCCESS != aiGetMaterialFloatArray(pMat, AI_MATKEY_SHININESS, &shininess, &max))
		m_shininess = 0;

	create(pAmbient, pDiffuse, pSpecular, pEmissive, shininess);

	// texture
	if (pPath)
		if (pDefTexPath)
			loadTexture(pDefTexPath, pPath);
		else
			loadTexture(pPath);
	if (m_idTexture[0] == 0xFFFFFFFF)
		loadBlankTexture();
}

void CMaterial::destroy()
{
	for (unsigned& idTexture : m_idTexture)
		if (idTexture != 0xffffffff)
			glDeleteTextures(1, &idTexture);
}

void CMaterial::bind()
{
	GLenum texUnit = GL_TEXTURE0;
	for (unsigned& idTexture : m_idTexture)
	{
		if (idTexture != 0xffffffff)
		{
			glActiveTexture(texUnit);
			glBindTexture(GL_TEXTURE_2D, idTexture);
		}
		texUnit++;
	}

	// check if a shading program is active
	C3dglProgram *pProgram = C3dglProgram::GetCurrentProgram();
	if (pProgram)
	{
		pProgram->SendStandardUniform(C3dglProgram::UNI_MAT_AMBIENT, m_amb[0], m_amb[1], m_amb[2]);	
		pProgram->SendStandardUniform(C3dglProgram::UNI_MAT_DIFFUSE, m_diff[0], m_diff[1], m_diff[2]);
		pProgram->SendStandardUniform(C3dglProgram::UNI_MAT_SPECULAR, m_spec[0], m_spec[1], m_spec[2]);
		pProgram->SendStandardUniform(C3dglProgram::UNI_MAT_EMISSIVE, m_emiss[0], m_emiss[1], m_emiss[2]);
		pProgram->SendStandardUniform(C3dglProgram::UNI_MAT_SHININESS, m_shininess);
	}
}

void CMaterial::loadTexture(GLenum texUnit, string strDefTexPath, string strPath)
{
	// prepare path
	std::ifstream f(strPath);
	if (f.is_open())
	{
		f.close();
	}
	else if (!strDefTexPath.empty())
	{
		size_t i = strPath.find_last_of("/\\");
		if (i != string::npos) strPath = strPath.substr(i + 1);


		if (strDefTexPath.back() == '/' || strDefTexPath.back() == '\\')
			strPath = strDefTexPath + strPath;
		else
			strPath = strDefTexPath + "/" + strPath;
	}

	loadTexture(texUnit, strPath);
}

void CMaterial::loadTexture(GLenum texUnit, string strPath)
{
	// generate IL image id
	C3dglBitmap bm;
	if (bm.load(strPath, GL_RGBA))
	{
		// generate texture id
		glGenTextures(1, &m_idTexture[texUnit - GL_TEXTURE0]);

		// load texture
		glActiveTexture(texUnit);
		glBindTexture(GL_TEXTURE_2D, m_idTexture[texUnit - GL_TEXTURE0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits());
	}
}

void CMaterial::loadBlankTexture(GLenum texUnit)
{
	if (c_idTexBlank == 0xffffffff)
	{
		glGenTextures(1, &c_idTexBlank);
		glActiveTexture(texUnit);
		glBindTexture(GL_TEXTURE_2D, c_idTexBlank);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		unsigned char bytes[] = { 255, 255, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);
	}
	m_idTexture[texUnit - GL_TEXTURE0] = c_idTexBlank;
}

