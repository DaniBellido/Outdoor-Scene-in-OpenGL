#include "../GL/glew.h"
#include "../GL/3dglShader.h"

#include <fstream>
#include <vector>

using namespace std;
using namespace _3dgl;


/////////////////////////////////////////////////////////////////////////////////////////////////
// Uniform variable types

struct {
	GLenum glType;
	GLenum targetType;
	std::string name;
} c_uniTypes[] =
{
	{ GLenum(0), GLenum(0), "???" },
	
	{ GL_FLOAT, GL_FLOAT, "GL_FLOAT" },
	{ GL_FLOAT_VEC2, GL_FLOAT_VEC2, "GL_FLOAT_VEC2" },
	{ GL_FLOAT_VEC3, GL_FLOAT_VEC3, "GL_FLOAT_VEC3" },
	{ GL_FLOAT_VEC4, GL_FLOAT_VEC4, "GL_FLOAT_VEC4" },
	{ GL_INT, GL_INT, "GL_INT" },
	{ GL_INT_VEC2, GL_INT_VEC2, "GL_INT_VEC2" },
	{ GL_INT_VEC3, GL_INT_VEC3, "GL_INT_VEC3" },
	{ GL_INT_VEC4, GL_INT_VEC4, "GL_INT_VEC4" },
	{ GL_UNSIGNED_INT, GL_UNSIGNED_INT, "GL_UNSIGNED_INT" },
	{ GL_UNSIGNED_INT_VEC2, GL_UNSIGNED_INT_VEC2, "GL_UNSIGNED_INT_VEC2" },
	{ GL_UNSIGNED_INT_VEC3, GL_UNSIGNED_INT_VEC3, "GL_UNSIGNED_INT_VEC3" },
	{ GL_UNSIGNED_INT_VEC4, GL_UNSIGNED_INT_VEC4, "GL_UNSIGNED_INT_VEC4" },
	{ GL_BOOL, GL_BOOL, "GL_BOOL" },
	{ GL_BOOL_VEC2, GL_BOOL_VEC2, "GL_BOOL_VEC2" },
	{ GL_BOOL_VEC3, GL_BOOL_VEC3, "GL_BOOL_VEC3" },
	{ GL_BOOL_VEC4, GL_BOOL_VEC4, "GL_BOOL_VEC4" },
	
	{ GL_FLOAT_MAT2, GL_FLOAT_MAT2, "GL_FLOAT_MAT2" },
	{ GL_FLOAT_MAT3, GL_FLOAT_MAT3, "GL_FLOAT_MAT3" },
	{ GL_FLOAT_MAT4, GL_FLOAT_MAT4, "GL_FLOAT_MAT4" },
	{ GL_FLOAT_MAT2x3, GL_FLOAT_MAT2x3, "GL_FLOAT_MAT2x3" },
	{ GL_FLOAT_MAT2x4, GL_FLOAT_MAT2x4, "GL_FLOAT_MAT2x4" },
	{ GL_FLOAT_MAT3x2, GL_FLOAT_MAT3x2, "GL_FLOAT_MAT3x2" },
	{ GL_FLOAT_MAT3x4, GL_FLOAT_MAT3x4, "GL_FLOAT_MAT3x4" },
	{ GL_FLOAT_MAT4x2, GL_FLOAT_MAT4x2, "GL_FLOAT_MAT4x2" },
	{ GL_FLOAT_MAT4x3, GL_FLOAT_MAT4x3, "GL_FLOAT_MAT4x3" },

	{ GL_SAMPLER_1D, GL_INT, "GL_SAMPLER_1D" },
	{ GL_SAMPLER_2D, GL_INT, "GL_SAMPLER_2D" },
	{ GL_SAMPLER_3D, GL_INT, "GL_SAMPLER_3D" },
	{ GL_SAMPLER_CUBE, GL_INT, "GL_SAMPLER_CUBE" },
	{ GL_SAMPLER_1D_SHADOW, GL_INT, "GL_SAMPLER_1D_SHADOW" },
	{ GL_SAMPLER_2D_SHADOW, GL_INT, "GL_SAMPLER_2D_SHADOW" },
	{ GL_SAMPLER_1D_ARRAY, GL_INT, "GL_SAMPLER_1D_ARRAY" },
	{ GL_SAMPLER_2D_ARRAY, GL_INT, "GL_SAMPLER_2D_ARRAY" },
	{ GL_SAMPLER_1D_ARRAY_SHADOW, GL_INT, "GL_SAMPLER_1D_ARRAY_SHADOW" },
	{ GL_SAMPLER_2D_ARRAY_SHADOW, GL_INT, "GL_SAMPLER_2D_ARRAY_SHADOW" },
	{ GL_SAMPLER_2D_MULTISAMPLE, GL_INT, "GL_SAMPLER_2D_MULTISAMPLE" },
	{ GL_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_INT, "GL_SAMPLER_2D_MULTISAMPLE_ARRAY" },
	{ GL_SAMPLER_CUBE_SHADOW, GL_INT, "GL_SAMPLER_CUBE_SHADOW" },
	{ GL_SAMPLER_BUFFER, GL_INT, "GL_SAMPLER_BUFFER" },
	{ GL_SAMPLER_2D_RECT, GL_INT, "GL_SAMPLER_2D_RECT" },
	{ GL_SAMPLER_2D_RECT_SHADOW, GL_INT, "GL_SAMPLER_2D_RECT_SHADOW" },

	{ GL_INT_SAMPLER_1D, GL_INT, "GL_INT_SAMPLER_1D" },
	{ GL_INT_SAMPLER_2D, GL_INT, "GL_INT_SAMPLER_2D" },
	{ GL_INT_SAMPLER_3D, GL_INT, "GL_INT_SAMPLER_3D" },
	{ GL_INT_SAMPLER_CUBE, GL_INT, "GL_INT_SAMPLER_CUBE" },
	{ GL_INT_SAMPLER_1D_ARRAY, GL_INT, "GL_INT_SAMPLER_1D_ARRAY" },
	{ GL_INT_SAMPLER_2D_ARRAY, GL_INT, "GL_INT_SAMPLER_2D_ARRAY" },
	{ GL_INT_SAMPLER_2D_MULTISAMPLE, GL_INT, "GL_INT_SAMPLER_2D_MULTISAMPLE" },
	{ GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_INT, "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY" },
	{ GL_INT_SAMPLER_BUFFER, GL_INT, "GL_INT_SAMPLER_BUFFER" },
	{ GL_INT_SAMPLER_2D_RECT, GL_INT, "GL_INT_SAMPLER_2D_RECT" },

	{ GL_UNSIGNED_INT_SAMPLER_1D, GL_INT, "GL_UNSIGNED_INT_SAMPLER_1D" },
	{ GL_UNSIGNED_INT_SAMPLER_2D, GL_INT, "GL_UNSIGNED_INT_SAMPLER_2D" },
	{ GL_UNSIGNED_INT_SAMPLER_3D, GL_INT, "GL_UNSIGNED_INT_SAMPLER_3D" },
	{ GL_UNSIGNED_INT_SAMPLER_CUBE, GL_INT, "GL_UNSIGNED_INT_SAMPLER_CUBE" },
	{ GL_UNSIGNED_INT_SAMPLER_1D_ARRAY, GL_INT, "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY" },
	{ GL_UNSIGNED_INT_SAMPLER_2D_ARRAY, GL_INT, "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY" },
	{ GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, GL_INT, "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE" },
	{ GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_INT, "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY" },
	{ GL_UNSIGNED_INT_SAMPLER_BUFFER, GL_INT, "GL_UNSIGNED_INT_SAMPLER_BUFFER" },
	{ GL_UNSIGNED_INT_SAMPLER_2D_RECT, GL_INT, "GL_UNSIGNED_INT_SAMPLER_2D_RECT" },
};

/////////////////////////////////////////////////////////////////////////////////////////////////
// C3dglShader

bool C3dglShader::Create(GLenum type)
{
	m_type = type;
	m_id = glCreateShader(m_type);
	if (m_id == 0) return logError("Shader creation error. Wrong type of shader.");
	return logSuccess("created successfully.");
}

bool C3dglShader::Load(std::string source)
{
	if (m_id == 0) return logError("Shader creation error. Wrong type of shader.");
	m_source = source;
	if (m_source.empty()) return false;
	const GLchar *pSource = static_cast<const GLchar*>(m_source.c_str());
	glShaderSource(m_id, 1, &pSource, NULL);
	return logSuccess("source code loaded.");		// always successful
}

bool C3dglShader::LoadFromFile(std::string fname)
{
	m_fname = fname;
	ifstream file(m_fname.c_str());
	string source(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));
	return Load(source);
}

bool C3dglShader::Compile()
{
	if (m_id == 0) return logError("Shader creation error. Wrong type of shader.");

	// compile
	glCompileShader(m_id);

	// check status
	GLint result = 0;
	glGetShaderiv(m_id, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		// collect info log
		GLint infoLen;
		glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen < 1) return logError("unknown compilation error");
		vector<char> log(infoLen);
		glGetShaderInfoLog(m_id, log.size(), &infoLen, &log[0]);
		return logError(string(log.begin(), log.end()));
	}
	return logSuccess("compiled successfully.");
}

std::string C3dglShader::getName()
{
	switch (m_type)
	{
	case GL_VERTEX_SHADER: return "Vertex Shader";
	case GL_FRAGMENT_SHADER: return "Fragment Shader";
	//case GL_COMPUTE_SHADER: return "Compute Shader";
	//case GL_TESS_CONTROL_SHADER: return "Tesselation Control Shader";
	//case GL_TESS_EVALUATION_SHADER: return "Tesselation Evaluation Shader";
	//case GL_GEOMETRY_SHADER: return "Geometry Shader";
	default: return "Shader";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// C3dglProgram

C3dglProgram *C3dglProgram::c_pCurrentProgram = NULL;

C3dglProgram::C3dglProgram() : C3dglObject()
{
	m_id = 0;
	memset(m_stdAttr, -1, sizeof(m_stdAttr));
	memset(m_stdUni, -1, sizeof(m_stdUni));
}

bool C3dglProgram::Create()
{
	m_id = glCreateProgram();
	if (m_id == 0) return logError("creation error.");
	return logSuccess("created successfully.");
}

bool C3dglProgram::Attach(C3dglShader &shader)
{
	if (m_id == 0) return logError("not created.");
	if (shader.getId() == 0) return logError("cannot attach shader: Shader not created.");

	glAttachShader(m_id, shader.getId());
	return logSuccess("has successfully attached a " + shader.getName());
}

bool C3dglProgram::Link(std::string std_attrib_names, std::string std_uni_names)
{
	if (m_id == 0) return logError("not created.");

	// link
	glLinkProgram(m_id);

	// check status
	GLint result = 0;
	glGetProgramiv(m_id, GL_LINK_STATUS, &result);
	if (!result)
	{
		// collect info log
		GLint infoLen;
		glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen < 1) return logError("unknown linking error");
		vector<char> log(infoLen);
		glGetProgramInfoLog(m_id, log.size(), &infoLen, &log[0]);
		return logError("linking error: " + string(log.begin(), log.end()));
	}

	// create type mappings
	unsigned i = 0;
	for (auto type : c_uniTypes)
		m_types[type.glType] = i++;

	// register active variables
	GLint nUniforms, maxLen;
	glGetProgramiv(GetId(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen);
	glGetProgramiv(GetId(), GL_ACTIVE_UNIFORMS, &nUniforms);
	GLchar *buf = new GLchar[maxLen];
	for (int i = 0; i < nUniforms; ++i) 
	{
		GLsizei written;
		GLint size, location;
		GLenum type;
		glGetActiveUniform(GetId(), i, maxLen, &written, &size, &type, buf);
		location = glGetUniformLocation(GetId(), buf);
		string name = buf;
		m_uniforms[name] = UNIFORM(location, m_types[type]);

		// special entry for arrays...
		size_t nPos = name.find('[');
		if (nPos != string::npos)
		{
			string nameArray = name.substr(0, nPos);
			location = glGetUniformLocation(GetId(), nameArray.c_str());
			m_uniforms[nameArray] = UNIFORM(location, m_types[type]);
		}
	}
	delete[] buf;

	//for (auto pair : m_uniforms)
	//{
	//	string name = pair.first;
	//	UNIFORM u = pair.second;
	//	GLuint location = u.location;
	//	string type = c_uniTypes[u.type].name;
	//	printf(" %-8d | %s %s\n", location, type.c_str(), name.c_str());
	//}

	// Collect Standard Attribute Locations
	string STD_ATTRIB_NAMES[] = {
		"a_vertex|a_Vertex|aVertex|avertex|vertex|Vertex",
		"a_normal|a_Normal|aNormal|anormal|normal|Normal",
		"a_texcoord|a_TexCoord|aTexCoord|atexcoord|texcoord|TexCoord",
		"a_tangent|a_Tangent|aTangent|atangent|tangent|Tangent",
		"a_bitangent|a_Bitangent|aBitangent|abitangent|bitangent|Bitangent|a_biTangent|a_BiTangent|aBiTangent|abiTangent|biTangent|BiTangent",
		"a_color|a_Color|aColor|acolor|color|Color",
		"a_boneid|a_Boneid|aBoneid|aboneid|boneid|Boneid|a_boneId|a_BoneId|aBoneId|aboneId|boneId|BoneId|"
		"a_boneids|a_Boneids|aBoneids|aboneids|boneids|Boneids|a_boneIds|a_BoneIds|aBoneIds|aboneIds|boneIds|BoneIds",
		"a_boneweight|a_Boneweight|aBoneweight|aboneweight|boneweight|Boneweight|a_boneWeight|a_BoneWeight|aBoneWeight|aboneWeight|boneWeight|BoneWeight|a_weight|aweight|weight|a_Weight|aWeight|Weight|"
		"a_boneweights|a_Boneweights|aBoneweights|aboneweights|boneweights|Boneweights|a_boneWeights|a_BoneWeights|aBoneWeights|aboneWeights|boneWeights|BoneWeights|a_weights|aweights|weights|a_Weights|aWeights|Weights",
	};
	int astart = 0, aend = 0;
	std_attrib_names += ";";
	for (GLuint i = 0; i < ATTR_LAST; i++)
	{
		string str = "";
		aend = std_attrib_names.find(";", astart);
		if (aend != string::npos)
		{
			str = std_attrib_names.substr(astart, aend - astart);
			astart = aend + 1;
		}
		if (str.empty()) str = STD_ATTRIB_NAMES[i];
		str += "|";

		int nstart = 0, nend = 0;
		while ((nend = str.find("|", nstart)) != string::npos)
		{
			string name = str.substr(nstart, nend - nstart);
			nstart = nend + 1;
			if (name.empty()) continue;
			
			m_stdAttr[i] = glGetAttribLocation(m_id, name.c_str());
			if (m_stdAttr[i] != (GLuint)-1)
			{
				logSuccess("attribute location found: " + name + " = " + to_string(m_stdAttr[i]));
				break;
			}
		}
	}
	
	// Collect Standard Uniform Locations
	string STD_UNI_NAMES[] = {
		"modelview_matrix|modelView_matrix|ModelView_matrix|Modelview_matrix|modelview_Matrix|modelView_Matrix|ModelView_Matrix|Modelview_Matrix|"
		"matrix_modelview|matrix_modelView|matrix_ModelView|matrix_Modelview|Matrix_modelview|Matrix_modelView|Matrix_ModelView|Matrix_Modelview|" 
		"modelviewmatrix|modelViewmatrix|ModelViewmatrix|Modelviewmatrix|modelviewMatrix|modelViewMatrix|ModelViewMatrix|ModelviewMatrix|" 
		"matrixmodelview|matrixmodelView|matrixModelView|matrixModelview|Matrixmodelview|MatrixmodelView|MatrixModelView|MatrixModelview|",
		"mat_ambient|material_ambient|mat_Ambient|material_Ambient|matambient|materialambient|matAmbient|materialAmbient",
		"mat_diffuse|material_diffuse|mat_Diffuse|material_Diffuse|matdiffuse|materialdiffuse|matDiffuse|materialDiffuse",
		"mat_specular|material_specular|mat_Specular|material_Specular|matspecular|materialspecular|matSpecular|materialSpecular",
		"mat_emissive|material_emissive|mat_Emissive|material_Emissive|matemissive|materialemissive|matEmissive|materialEmissive",
		"shininess|Shininess|mat_shininess|material_shininess|mat_Shininess|material_Shininess|matshininess|materialshininess|matShininess|materialShininess"
	};
	int lstart = 0, lend = 0;
	std_uni_names += ";";
	for (GLuint i = 0; i < UNI_LAST; i++)
	{
		string str = "";
		lend = std_uni_names.find(";", lstart);
		if (lend != string::npos)
		{
			str = std_uni_names.substr(lstart, lend - lstart);
			lstart = lend + 1;
		}
		if (str.empty()) str = STD_UNI_NAMES[i];
		str += "|";

		int nstart = 0, nend = 0;
		while ((nend = str.find("|", nstart)) != string::npos)
		{
			string name = str.substr(nstart, nend - nstart);
			nstart = nend + 1;
			if (name.empty()) continue;

			// find the name among the registered variables
			std::map<std::string, UNIFORM>::iterator it = m_uniforms.find(name.c_str());
			if (it != m_uniforms.end())
			{
				m_stdUni[i] = it->second;
				logSuccess("uniform location found: " + name + " = " + to_string(m_stdUni[i].location));
			}
		}
	}

	return logSuccess("linked successfully.");
}

bool C3dglProgram::Use(bool bValidate)
{
	if (m_id == 0) return logError("not created.");
	glUseProgram(m_id);

	c_pCurrentProgram = this;

	if (!bValidate) return true;

	glValidateProgram(m_id);

	// collect info log
	GLint infoLen;
	glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLen);
	infoLen = 0;
	if (infoLen < 1) return logSuccess("verification result: OK");
	vector<char> log(infoLen);
	glGetProgramInfoLog(m_id, log.size(), &infoLen, &log[0]);
	return logSuccess("verification result: " + (log.size() <= 1 ? "OK" : string(log.begin(), log.end())));
}

void C3dglProgram::GetAttribLocation(std::string idAttrib, GLuint &location)
{
	auto i = m_attribs.find(idAttrib);
	if (i == m_attribs.end())
	{
		GLuint nAttrib = glGetAttribLocation(m_id, idAttrib.c_str());
		m_attribs[idAttrib] = nAttrib;
		location = nAttrib;
	}
	else
		location = i->second;
}

void C3dglProgram::GetUniformLocation(std::string idUniform, GLuint &location, GLenum &type, GLenum &targetType)
{
	auto i = m_uniforms.find(idUniform);
	
	// first - a direct hit
	if (i != m_uniforms.end())
	{
		// Uniform found
		UNIFORM uni = (i->second);
		location = uni.location;
		type = c_uniTypes[uni.type].glType;
		targetType = c_uniTypes[uni.type].targetType;
		//printf(" %-20s | %s\n", idUniform.c_str(), c_uniTypes[uni.type].name.c_str());
		return;
	}

	// if not found, search for corresponding array name...
	size_t nPos = idUniform.find('[');
	if (nPos != string::npos)
	{
		// look for an array
		i = m_uniforms.find(idUniform.substr(0, nPos));
		if (i != m_uniforms.end())
		{
			// array located...
			UNIFORM uni = (i->second);
			uni.location = glGetUniformLocation(m_id, idUniform.c_str());
			m_uniforms[idUniform] = uni;
			type = c_uniTypes[uni.type].glType;
			targetType = c_uniTypes[uni.type].targetType;
			//printf(" %-20s | %s\n", idUniform.c_str(), c_uniTypes[uni.type].name.c_str());
			return;
		}
	}

	// if all else fails, process as unregistred variable
	location = glGetUniformLocation(m_id, idUniform.c_str());
	type = targetType = 0;
	m_uniforms[idUniform] = UNIFORM(location, m_types[type]);
	if (location == (GLuint)-1) logWarning("uniform location not found: " + idUniform);
	else logWarning("unregistered uniform used: " + idUniform);
}

void C3dglProgram::GetUniformLocation(UNI_STD uniId, GLuint &location, GLenum &type, GLenum &targetType)
{
	location = -1;
	type = targetType = GLenum(0);

	if (uniId < 0 || uniId >= UNI_LAST)
		return;

	UNIFORM uni = m_stdUni[uniId];
	location = uni.location;
	
	if (uni.type < 0 || uni.type >= sizeof(c_uniTypes) / sizeof(c_uniTypes[0]))
		return;

	auto T = c_uniTypes[uni.type];
	type = T.glType;
	targetType = T.targetType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// SendUniform and its overloads

bool C3dglProgram::_error(string name, GLenum actual, GLenum expected)
{
	string msg;
	if (expected == (GLenum)0)
	{
		msg = string("uniform ") + name + " not found or not currently in use.";
		if (m_errlookup.find(msg) == m_errlookup.end())
		{
			m_errlookup.insert(msg);
			logWarning(msg);
		}
		return true;
	}
	else
	{
		msg = string("type mismatch in uniform: ") + name + ":\n\r    Sending value of " + c_uniTypes[m_types[actual]].name + " but expected was: " + c_uniTypes[m_types[expected]].name + ".";
		if (m_errlookup.find(msg) == m_errlookup.end())
		{
			m_errlookup.insert(msg);
			logError(msg);
		}
		return false;
	}
}

bool C3dglProgram::SendUniform(std::string name, GLint v0)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_INT || t == 0) SendUniform(location, (GLint)v0);
	else if (t == GL_UNSIGNED_INT) SendUniform(location, (GLuint)v0);
	else if (t == GL_BOOL) SendUniform(location, v0 != 0);
	else if (t == GL_FLOAT) SendUniform(location, (GLfloat)v0);
	else return _error(name, GL_INT, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, GLint v0, GLint v1)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_INT_VEC2 || t == 0) SendUniform(location, (GLint)v0, (GLint)v1);
	else if (t == GL_UNSIGNED_INT_VEC2) SendUniform(location, (GLuint)v0, (GLuint)v1);
	else if (t == GL_BOOL_VEC2) SendUniform(location, v0 != 0, v1 != 0);
	else if (t == GL_FLOAT_VEC2) SendUniform(location, (GLfloat)v0, (GLfloat)v1);
	else return _error(name, GL_INT_VEC2, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, GLint v0, GLint v1, GLint v2)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_INT_VEC3 || t == 0) SendUniform(location, (GLint)v0, (GLint)v1, (GLint)v2);
	else if (t == GL_UNSIGNED_INT_VEC3) SendUniform(location, (GLuint)v0, (GLuint)v1, (GLuint)v2);
	else if (t == GL_BOOL_VEC3) SendUniform(location, v0 != 0, v1 != 0, v2 != 0);
	else if (t == GL_FLOAT_VEC3) SendUniform(location, (GLfloat)v0, (GLfloat)v1, (GLfloat)v2);
	else return _error(name, GL_INT_VEC3, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, GLint v0, GLint v1, GLint v2, GLint v3)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_INT_VEC4 || t == 0) SendUniform(location, (GLint)v0, (GLint)v1, (GLint)v2, (GLint)v3);
	else if (t == GL_UNSIGNED_INT_VEC4) SendUniform(location, (GLuint)v0, (GLuint)v1, (GLuint)v2, (GLuint)v3);
	else if (t == GL_BOOL_VEC4) SendUniform(location, v0 != 0, v1 != 0, v2 != 0, v3 != 0);
	else if (t == GL_FLOAT_VEC4) SendUniform(location, (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3);
	else return _error(name, GL_INT_VEC4, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, GLuint v0)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_INT) SendUniform(location, (GLint)v0);
	else if (t == GL_UNSIGNED_INT || t == 0) SendUniform(location, (GLuint)v0);
	else if (t == GL_BOOL) SendUniform(location, v0 != 0);
	else if (t == GL_FLOAT) SendUniform(location, (GLfloat)v0);
	else return _error(name, GL_UNSIGNED_INT, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, GLuint v0, GLuint v1)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_INT_VEC2) SendUniform(location, (GLint)v0, (GLint)v1);
	else if (t == GL_UNSIGNED_INT_VEC2 || t == 0) SendUniform(location, (GLuint)v0, (GLuint)v1);
	else if (t == GL_BOOL_VEC2) SendUniform(location, v0 != 0, v1 != 0);
	else if (t == GL_FLOAT_VEC2) SendUniform(location, (GLfloat)v0, (GLfloat)v1);
	else return _error(name, GL_UNSIGNED_INT_VEC2, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, GLuint v0, GLuint v1, GLuint v2)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_INT_VEC3) SendUniform(location, (GLint)v0, (GLint)v1, (GLint)v2);
	else if (t == GL_UNSIGNED_INT_VEC3 || t == 0) SendUniform(location, (GLuint)v0, (GLuint)v1, (GLuint)v2);
	else if (t == GL_BOOL_VEC3) SendUniform(location, v0 != 0, v1 != 0, v2 != 0);
	else if (t == GL_FLOAT_VEC3) SendUniform(location, (GLfloat)v0, (GLfloat)v1, (GLfloat)v2);
	else return _error(name, GL_UNSIGNED_INT_VEC3, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_INT_VEC4) SendUniform(location, (GLint)v0, (GLint)v1, (GLint)v2, (GLint)v3);
	else if (t == GL_UNSIGNED_INT_VEC4 || t == 0) SendUniform(location, (GLuint)v0, (GLuint)v1, (GLuint)v2, (GLuint)v3);
	else if (t == GL_BOOL_VEC4) SendUniform(location, v0 != 0, v1 != 0, v2 != 0, v3 != 0);
	else if (t == GL_FLOAT_VEC4) SendUniform(location, (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3);
	else return _error(name, GL_UNSIGNED_INT_VEC4, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, GLfloat v0)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT || t == 0) SendUniform(location, v0);
	else return _error(name, GL_FLOAT, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, GLfloat v0, GLfloat v1)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT_VEC2 || t == 0) SendUniform(location, v0, v1);
	else return _error(name, GL_FLOAT_VEC2, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, GLfloat v0, GLfloat v1, GLfloat v2)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT_VEC3 || t == 0) SendUniform(location, v0, v1, v2);
	else return _error(name, GL_FLOAT_VEC3, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT_VEC4 || t == 0) SendUniform(location, v0, v1, v2, v3);
	else return _error(name, GL_FLOAT_VEC4, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, double v0)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT || t == 0) SendUniform(location, v0);
	else return _error(name, GL_FLOAT, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, double v0, double v1)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT_VEC2 || t == 0) SendUniform(location, v0, v1);
	else return _error(name, GL_FLOAT_VEC2, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, double v0, double v1, double v2)
{ 
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);  
	if (t == GL_FLOAT_VEC3 || t == 0) SendUniform(location, v0, v1, v2);
	else return _error(name, GL_FLOAT_VEC3, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, double v0, double v1, double v2, double v3)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT_VEC4 || t == 0) SendUniform(location, v0, v1, v2, v3);
	else return _error(name, GL_FLOAT_VEC4, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, GLfloat pMatrix[16])
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT_MAT4 || t == 0) SendUniform(location, pMatrix);
	else return _error(name, GL_FLOAT_MAT4, t);
	return true;
}

bool C3dglProgram::SendUniform(std::string name, glm::mat4 matrix)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT_MAT4 || t == 0) SendUniform(location, matrix);
	else return _error(name, GL_FLOAT_MAT4, t);
	return true;
}

bool C3dglProgram::SendUniform1v(std::string name, GLint *p, GLuint count) 
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_INT || t == 0) SendUniform1v(location, p, count);
	else return _error(name, GL_INT, t);
	return true;
}

bool C3dglProgram::SendUniform2v(std::string name, GLint *p, GLuint count)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_INT_VEC2 || t == 0) SendUniform2v(location, p, count);
	else return _error(name, GL_INT_VEC2, t);
	return true;
}

bool C3dglProgram::SendUniform3v(std::string name, GLint *p, GLuint count)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_INT_VEC3 || t == 0) SendUniform3v(location, p, count);
	else return _error(name, GL_INT_VEC3, t);
	return true;
}

bool C3dglProgram::SendUniform4v(std::string name, GLint *p, GLuint count)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_INT_VEC4 || t == 0) SendUniform4v(location, p, count);
	else return _error(name, GL_INT_VEC4, t);
	return true;
}

bool C3dglProgram::SendUniform1v(std::string name, GLuint *p, GLuint count)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_UNSIGNED_INT || t == 0) SendUniform1v(location, p, count);
	else return _error(name, GL_UNSIGNED_INT, t);
	return true;
}

bool C3dglProgram::SendUniform2v(std::string name, GLuint *p, GLuint count)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_UNSIGNED_INT_VEC2 || t == 0) SendUniform2v(location, p, count);
	else return _error(name, GL_UNSIGNED_INT_VEC2, t);
	return true;
}

bool C3dglProgram::SendUniform3v(std::string name, GLuint *p, GLuint count)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_UNSIGNED_INT_VEC3 || t == 0) SendUniform3v(location, p, count);
	else return _error(name, GL_UNSIGNED_INT_VEC3, t);
	return true;
}

bool C3dglProgram::SendUniform4v(std::string name, GLuint *p, GLuint count)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_UNSIGNED_INT_VEC4 || t == 0) SendUniform4v(location, p, count);
	else return _error(name, GL_UNSIGNED_INT_VEC4, t);
	return true;
}

bool C3dglProgram::SendUniform1v(std::string name, GLfloat *p, GLuint count)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT || t == 0) SendUniform1v(location, p, count);
	else return _error(name, GL_FLOAT, t);
	return true;
}

bool C3dglProgram::SendUniform2v(std::string name, GLfloat *p, GLuint count)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT_VEC2 || t == 0) SendUniform2v(location, p, count);
	else return _error(name, GL_FLOAT_VEC2, t);
	return true;
}

bool C3dglProgram::SendUniform3v(std::string name, GLfloat *p, GLuint count)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT_VEC3 || t == 0) SendUniform3v(location, p, count);
	else return _error(name, GL_FLOAT_VEC3, t);
	return true;
}

bool C3dglProgram::SendUniform4v(std::string name, GLfloat *p, GLuint count)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT_VEC4 || t == 0) SendUniform4v(location, p, count);
	else return _error(name, GL_FLOAT_VEC4, t);
	return true;
}

bool C3dglProgram::SendUniformMatrixv(std::string name, GLfloat *pMatrix, GLuint count)
{
	GLuint location; GLenum _t, t; GetUniformLocation(name, location, _t, t);
	if (t == GL_FLOAT_MAT4 || t == 0) SendUniformMatrixv(location, pMatrix, count);
	else return _error(name, GL_FLOAT_MAT4, t);
	return true;
}

bool C3dglProgram::SendStandardUniform(enum UNI_STD loc, GLfloat v0)
{
	GLuint location; GLenum _t, t; GetUniformLocation(loc, location, _t, t);
	SendUniform(location, v0);
	return true;
}

bool C3dglProgram::SendStandardUniform(enum UNI_STD loc, GLfloat v0, GLfloat v1, GLfloat v2)
{
	GLuint location; GLenum _t, t; GetUniformLocation(loc, location, _t, t);
	SendUniform(location, v0, v1, v2);
	return true;
}

bool C3dglProgram::SendStandardUniform(enum UNI_STD loc, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	GLuint location; GLenum _t, t; GetUniformLocation(loc, location, _t, t);
	SendUniform(location, v0, v1, v2, v3);
	return true;
}

bool C3dglProgram::SendStandardUniform(enum UNI_STD loc, GLfloat pMatrix[16])
{
	GLuint location; GLenum _t, t; GetUniformLocation(loc, location, _t, t);
	SendUniform(location, pMatrix);
	return true;
}

bool C3dglProgram::SendStandardUniform(enum UNI_STD loc, glm::mat4 matrix)
{
	GLuint location; GLenum _t, t; GetUniformLocation(loc, location, _t, t);
	SendUniform(location, matrix);
	return true;
}

