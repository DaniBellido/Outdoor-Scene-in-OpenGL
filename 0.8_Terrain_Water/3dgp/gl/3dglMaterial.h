/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Partially based on http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html
Version 2.2 23/03/15

Copyright (C) 2013-15 Jarek Francik, Kingston University, London, UK

Implementation of a simple Material class
Uses AssImp (Open Asset Import Library) Library to load model files
Uses DevIL Image Library to load textures
Main features:
- VBO based rendering (vertices, normals, tangents, bitangents, colours, bone ids & weights
- automatically loads textures
- integration with C3dglProgram shader program
- very simple Bounding Boxes
- support for skeletal animation
----------------------------------------------------------------------------------
This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source distribution.

   Jarek Francik
   jarek@kingston.ac.uk
*********************************************************************************/

#ifndef __3dglMaterial_h_
#define __3dglMaterial_h_

#include "3dglObject.h"

// AssImp Scene include
#include "assimp/scene.h"
#include "assimp/postprocess.h"

// standard libraries
#include <vector>
#include <map>

#include "../glm/mat4x4.hpp"

namespace _3dgl
{

	struct CMaterial
	{
	private:
		// texture id
		unsigned m_idTexture[GL_TEXTURE31 - GL_TEXTURE0 + 1];

		// materials
		float m_amb[3];
		float m_diff[3];
		float m_spec[3];
		float m_emiss[3];
		float m_shininess;
		static unsigned c_idTexBlank;

	public:
		CMaterial();
		void create(float* pAmbient = NULL, float* pDiffuse = NULL, float* pSpecular = NULL, float* pEmissive = NULL, float shininess = 0);
		void create(const aiMaterial* pMat, const char* pDefTexPath);
		void destroy();
		void bind();

		void getAmbientMaterial(float& r, float& g, float& b) { r = m_amb[0];   g = m_amb[1];   b = m_amb[2]; }
		void getDiffuseMaterial(float& r, float& g, float& b) { r = m_diff[0];  g = m_diff[1];  b = m_diff[2]; }
		void getSpecularMaterial(float& r, float& g, float& b) { r = m_spec[0];  g = m_spec[1];  b = m_spec[2]; }
		void getEmissiveMaterial(float& r, float& g, float& b) { r = m_emiss[0]; g = m_emiss[1]; b = m_emiss[2]; }
		float getShininess() { return m_shininess; }

		void setAmbientMaterial(float r, float g, float b) { m_amb[0] = r;   m_amb[1] = g;   m_amb[2] = b; }
		void setDiffuseMaterial(float r, float g, float b) { m_diff[0] = r;  m_diff[1] = g;  m_diff[2] = b; }
		void setSpecularMaterial(float r, float g, float b) { m_spec[0] = r;  m_spec[1] = g;  m_spec[2] = b; }
		void setEmissiveMaterial(float r, float g, float b) { m_emiss[0] = r; m_emiss[1] = g; m_emiss[2] = b; }
		void setShininess(float s) { m_shininess = s; }

		void loadTexture(GLenum texUnit, std::string strPath);
		void loadTexture(GLenum texUnit, std::string strTexRootPath, std::string strPath);
		void loadBlankTexture(GLenum texUnit);

		void loadTexture(std::string strPath) { loadTexture(GL_TEXTURE0, strPath); }
		void loadTexture(std::string strTexRootPath, std::string strPath) { loadTexture(GL_TEXTURE0, strTexRootPath, strPath); }
		void loadBlankTexture() { loadBlankTexture(GL_TEXTURE0); }
	};

}

#endif