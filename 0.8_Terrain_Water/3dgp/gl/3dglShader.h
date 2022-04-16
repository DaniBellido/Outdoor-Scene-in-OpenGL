/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Partially based on Luke Benstead GLSLProgram class.
Version 2.2 23/03/15

Copyright (C) 2013-15 Jarek Francik, Kingston University, London, UK

Implementation of a simple 3D model class
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
#ifndef __3dglShader_h_
#define __3dglShader_h_

#include "3dglObject.h"
#include <string>
#include <map>
#include <set>

#include "../glm/mat4x4.hpp"

//////////////////////////////////////////////////////////
// NOTE: To keep compatibility with older versions
// function names in this file start with a capital letter
// unlike the rest of 3DGL
//////////////////////////////////////////////////////////

namespace _3dgl
{

class C3dglProgram;

class C3dglShader : public C3dglObject
{
	GLenum m_type;
	GLuint m_id;
	std::string m_source;
	std::string m_fname;
public:
	C3dglShader() : C3dglObject()		{ m_type = 0; m_id = 0; }

	bool Create(GLenum type);
	bool Load(std::string source);
	bool LoadFromFile(std::string fname);
	bool Compile();

	GLenum getType()		{ return m_type; }
	GLuint getId()			{ return m_id; }
	std::string getSource()	{ return m_source; }
	std::string getFName()	{ return m_fname; }
	std::string getName();	// "Vertex Shader", "Fragment Shader" etc
};

class C3dglProgram : public C3dglObject
{
public:
	// Standard attribute and uniform locations
	enum ATTRIB_STD { ATTR_VERTEX, ATTR_NORMAL, ATTR_TEXCOORD, ATTR_TANGENT, ATTR_BITANGENT, ATTR_COLOR, ATTR_BONE_ID, ATTR_BONE_WEIGHT, ATTR_LAST };
	enum UNI_STD { UNI_MODELVIEW, UNI_MAT_AMBIENT, UNI_MAT_DIFFUSE, UNI_MAT_SPECULAR, UNI_MAT_EMISSIVE, UNI_MAT_SHININESS, UNI_LAST };


private:
	static C3dglProgram *c_pCurrentProgram;

	struct UNIFORM
	{
		UNIFORM(GLuint _location = -1, GLenum _type = 0) : location(_location), type(_type) { }
		GLuint location;
		GLenum type;
	};

	GLuint m_id;
	std::map<std::string, GLuint> m_attribs;
	std::map<std::string, UNIFORM> m_uniforms;

	GLuint m_stdAttr[ATTR_LAST];
	UNIFORM m_stdUni[UNI_LAST];

	std::map<GLenum, unsigned> m_types;

public:
	C3dglProgram();

	bool Create();
	bool Attach(C3dglShader &shader);
	bool Link(std::string std_attrib_names = "", std::string std_uni_names = "");
	bool Use(bool bValidate = false);

	GLuint GetId()			{ return m_id; }
	bool IsUsed()			{ return c_pCurrentProgram == this; }

	static C3dglProgram *GetCurrentProgram()		{ return c_pCurrentProgram; }

	// numerical locations for attributes
	void GetAttribLocation(std::string idUniform, GLuint &location);
	GLuint GetAttribLocation(std::string idUniform)							{ GLuint location; GetAttribLocation(idUniform, location); return location; }
	void GetAttribLocation(ATTRIB_STD attr, GLuint &location)				{ location = m_stdAttr[attr]; }
	GLuint GetAttribLocation(ATTRIB_STD attr)								{ return m_stdAttr[attr]; }

	// numerical locations and types for attribute and uniform names
	void GetUniformLocation(std::string idUniform, GLuint &location, GLenum &type, GLenum &targetType);
	GLuint GetUniformLocation(std::string idUniform)						{ GLuint location; GLenum type, targetType; GetUniformLocation(idUniform, location, type, targetType); return location; }
	void GetUniformLocation(UNI_STD uniId, GLuint &location, GLenum &type, GLenum &targetType);
	GLuint GetUniformLocation(UNI_STD uniId)								{ GLuint location; GLenum type, targetType; GetUniformLocation(uniId, location, type, targetType); return location; }

	// send uniform using numerical location
	void SendUniform(GLuint location, GLint v0)													{ if (!IsUsed()) Use(); glUniform1i(location, v0); }
	void SendUniform(GLuint location, GLint v0, GLint v1)										{ if (!IsUsed()) Use(); glUniform2i(location, v0, v1); }
	void SendUniform(GLuint location, GLint v0, GLint v1, GLint v2)								{ if (!IsUsed()) Use(); glUniform3i(location, v0, v1, v2); }
	void SendUniform(GLuint location, GLint v0, GLint v1, GLint v2, GLint v3)					{ if (!IsUsed()) Use(); glUniform4i(location, v0, v1, v2, v3); }
	void SendUniform(GLuint location, GLuint v0)												{ if (!IsUsed()) Use(); glUniform1ui(location, v0); }
	void SendUniform(GLuint location, GLuint v0, GLuint v1)										{ if (!IsUsed()) Use(); glUniform2ui(location, v0, v1); }
	void SendUniform(GLuint location, GLuint v0, GLuint v1, GLuint v2)							{ if (!IsUsed()) Use(); glUniform3ui(location, v0, v1, v2); }
	void SendUniform(GLuint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)				{ if (!IsUsed()) Use(); glUniform4ui(location, v0, v1, v2, v3); }
	void SendUniform(GLuint location, GLfloat v0)												{ if (!IsUsed()) Use(); glUniform1f(location, v0); }
	void SendUniform(GLuint location, GLfloat v0, GLfloat v1)									{ if (!IsUsed()) Use(); glUniform2f(location, v0, v1); }
	void SendUniform(GLuint location, GLfloat v0, GLfloat v1, GLfloat v2)						{ if (!IsUsed()) Use(); glUniform3f(location, v0, v1, v2); }
	void SendUniform(GLuint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)			{ if (!IsUsed()) Use(); glUniform4f(location, v0, v1, v2, v3); }
	void SendUniform(GLuint location, double v0)												{ if (!IsUsed()) Use(); glUniform1f(location, (float)v0); }
	void SendUniform(GLuint location, double v0, double v1)										{ if (!IsUsed()) Use(); glUniform2f(location, (float)v0, (float)v1); }
	void SendUniform(GLuint location, double v0, double v1, double v2)							{ if (!IsUsed()) Use(); glUniform3f(location, (float)v0, (float)v1, (float)v2); }
	void SendUniform(GLuint location, double v0, double v1, double v2, double v3)				{ if (!IsUsed()) Use(); glUniform4f(location, (float)v0, (float)v1, (float)v2, (float)v3); }
	void SendUniform(GLuint location, GLfloat pMatrix[16])										{ if (!IsUsed()) Use(); glUniformMatrix4fv(location, 1, GL_FALSE, pMatrix); }
	void SendUniform(GLuint location, glm::mat4 matrix)											{ if (!IsUsed()) Use(); glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]); }

	void SendUniform1v(GLuint location, GLint *p, GLuint count = 1)								{ if (!IsUsed()) Use(); glUniform1iv(location, count, p); }
	void SendUniform2v(GLuint location, GLint *p, GLuint count = 1)								{ if (!IsUsed()) Use(); glUniform2iv(location, count, p); }
	void SendUniform3v(GLuint location, GLint *p, GLuint count = 1)								{ if (!IsUsed()) Use(); glUniform3iv(location, count, p); }
	void SendUniform4v(GLuint location, GLint *p, GLuint count = 1)								{ if (!IsUsed()) Use(); glUniform4iv(location, count, p); }
	void SendUniform1v(GLuint location, GLuint *p, GLuint count = 1)							{ if (!IsUsed()) Use(); glUniform1uiv(location, count, p); }
	void SendUniform2v(GLuint location, GLuint *p, GLuint count = 1)							{ if (!IsUsed()) Use(); glUniform2uiv(location, count, p); }
	void SendUniform3v(GLuint location, GLuint *p, GLuint count = 1)							{ if (!IsUsed()) Use(); glUniform3uiv(location, count, p); }
	void SendUniform4v(GLuint location, GLuint *p, GLuint count = 1)							{ if (!IsUsed()) Use(); glUniform4uiv(location, count, p); }
	void SendUniform1v(GLuint location, GLfloat *p, GLuint count = 1)							{ if (!IsUsed()) Use(); glUniform1fv(location, count, p); }
	void SendUniform2v(GLuint location, GLfloat *p, GLuint count = 1)							{ if (!IsUsed()) Use(); glUniform2fv(location, count, p); }
	void SendUniform3v(GLuint location, GLfloat *p, GLuint count = 1)							{ if (!IsUsed()) Use(); glUniform3fv(location, count, p); }
	void SendUniform4v(GLuint location, GLfloat *p, GLuint count = 1)							{ if (!IsUsed()) Use(); glUniform4fv(location, count, p); }
	void SendUniformMatrixv(GLuint location, GLfloat *pMatrix, GLuint count = 1)				{ if (!IsUsed()) Use(); glUniformMatrix4fv(location, count, GL_FALSE, pMatrix); }

	// send uniform using a name. Internally uses a look-up list to speed up and provide additional control
	bool SendUniform(std::string name, GLint v0);
	bool SendUniform(std::string name, GLint v0, GLint v1);
	bool SendUniform(std::string name, GLint v0, GLint v1, GLint v2);
	bool SendUniform(std::string name, GLint v0, GLint v1, GLint v2, GLint v3);
	bool SendUniform(std::string name, GLuint v0);
	bool SendUniform(std::string name, GLuint v0, GLuint v1);
	bool SendUniform(std::string name, GLuint v0, GLuint v1, GLuint v2);
	bool SendUniform(std::string name, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	bool SendUniform(std::string name, GLfloat v0);
	bool SendUniform(std::string name, GLfloat v0, GLfloat v1);
	bool SendUniform(std::string name, GLfloat v0, GLfloat v1, GLfloat v2);
	bool SendUniform(std::string name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	bool SendUniform(std::string name, double v0);
	bool SendUniform(std::string name, double v0, double v1);
	bool SendUniform(std::string name, double v0, double v1, double v2);
	bool SendUniform(std::string name, double v0, double v1, double v2, double v3);
	bool SendUniform(std::string name, GLfloat pMatrix[16]);
	bool SendUniform(std::string name, glm::mat4 matrix);

	bool SendUniform1v(std::string name, GLint *p, GLuint count = 1);
	bool SendUniform2v(std::string name, GLint *p, GLuint count = 1);
	bool SendUniform3v(std::string name, GLint *p, GLuint count = 1);
	bool SendUniform4v(std::string name, GLint *p, GLuint count = 1);
	bool SendUniform1v(std::string name, GLuint *p, GLuint count = 1);
	bool SendUniform2v(std::string name, GLuint *p, GLuint count = 1);
	bool SendUniform3v(std::string name, GLuint *p, GLuint count = 1);
	bool SendUniform4v(std::string name, GLuint *p, GLuint count = 1);
	bool SendUniform1v(std::string name, GLfloat *p, GLuint count = 1);
	bool SendUniform2v(std::string name, GLfloat *p, GLuint count = 1);
	bool SendUniform3v(std::string name, GLfloat *p, GLuint count = 1);
	bool SendUniform4v(std::string name, GLfloat *p, GLuint count = 1);
	bool SendUniformMatrixv(std::string name, GLfloat *pMatrix, GLuint count = 1);

	// send uniform using an indexed name
	bool SendIndUniform(std::string name, GLuint i, GLint v0)									{ SendUniform(name + "[" + std::to_string(i) + "]", v0); }
	bool SendIndUniform(std::string name, GLuint i, GLint v0, GLint v1)							{ SendUniform(name + "[" + std::to_string(i) + "]", v0, v1); }
	bool SendIndUniform(std::string name, GLuint i, GLint v0, GLint v1, GLint v2)				{ SendUniform(name + "[" + std::to_string(i) + "]", v0, v1, v2); }
	bool SendIndUniform(std::string name, GLuint i, GLint v0, GLint v1, GLint v2, GLint v3)		{ SendUniform(name + "[" + std::to_string(i) + "]", v0, v1, v2, v3); }
	bool SendIndUniform(std::string name, GLuint i, GLuint v0)									{ SendUniform(name + "[" + std::to_string(i) + "]", v0); }
	bool SendIndUniform(std::string name, GLuint i, GLuint v0, GLuint v1)						{ SendUniform(name + "[" + std::to_string(i) + "]", v0, v1); }
	bool SendIndUniform(std::string name, GLuint i, GLuint v0, GLuint v1, GLuint v2)			{ SendUniform(name + "[" + std::to_string(i) + "]", v0, v1, v2); }
	bool SendIndUniform(std::string name, GLuint i, GLuint v0, GLuint v1, GLuint v2, GLuint v3)	{ SendUniform(name + "[" + std::to_string(i) + "]", v0, v1, v2, v3); }
	bool SendIndUniform(std::string name, GLuint i, GLfloat v0)									{ SendUniform(name + "[" + std::to_string(i) + "]", v0); }
	bool SendIndUniform(std::string name, GLuint i, GLfloat v0, GLfloat v1)						{ SendUniform(name + "[" + std::to_string(i) + "]", v0, v1); }
	bool SendIndUniform(std::string name, GLuint i, GLfloat v0, GLfloat v1, GLfloat v2)			{ SendUniform(name + "[" + std::to_string(i) + "]", v0, v1, v2); }
	bool SendIndUniform(std::string name, GLuint i, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)	{ SendUniform(name + "[" + std::to_string(i) + "]", v0, v1, v2, v3); }
	bool SendIndUniform(std::string name, GLuint i, double v0)									{ SendUniform(name + "[" + std::to_string(i) + "]", (float)v0); }
	bool SendIndUniform(std::string name, GLuint i, double v0, double v1)						{ SendUniform(name + "[" + std::to_string(i) + "]", (float)v0, (float)v1); }
	bool SendIndUniform(std::string name, GLuint i, double v0, double v1, double v2)			{ SendUniform(name + "[" + std::to_string(i) + "]", (float)v0, (float)v1, (float)v2); }
	bool SendIndUniform(std::string name, GLuint i, double v0, double v1, double v2, double v3)	{ SendUniform(name + "[" + std::to_string(i) + "]", (float)v0, (float)v1, (float)v2, (float)v3); }
	bool SendIndUniform(std::string name, GLuint i, GLfloat pMatrix[16])						{ SendUniform(name + "[" + std::to_string(i) + "]", pMatrix); }
	void SendUniform(std::string name, GLuint i, glm::mat4 matrix)								{ SendUniform(name + "[" + std::to_string(i) + "]", matrix); }

	// send a standard uniform using one of the UNI_STD values
	bool SendStandardUniform(enum UNI_STD loc, GLfloat v0);
	bool SendStandardUniform(enum UNI_STD loc, GLfloat v0, GLfloat v1, GLfloat v2);
	bool SendStandardUniform(enum UNI_STD loc, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	bool SendStandardUniform(enum UNI_STD loc, GLfloat pMatrix[16]);
	bool SendStandardUniform(enum UNI_STD loc, glm::mat4 matrix);

	std::string getName()	{ return "GLSL Program"; }

private:
	std::set<std::string> m_errlookup;
	bool _error(std::string name, GLenum actual, GLenum expected);
};

}; // namespace _3dgl

#endif // __3dglShader_h_