#include <iostream>
#include <fstream>
#include "../GL/glew.h"
#include "../GL/3dglBitmap.h"

// DevIL include file
#undef _UNICODE
#include "../GL/il/il.h"

using namespace std;
using namespace _3dgl;

C3dglBitmap *C3dglBitmap::c_pBound = NULL;

C3dglBitmap::C3dglBitmap(std::string fname, unsigned format)
{
	m_idImage = 0;
	Load(fname, format);
}

bool C3dglBitmap::load(std::string fname, unsigned format)
{
	// initialise IL
	static bool bIlInitialised = false;
	if (!bIlInitialised)
		ilInit(); 
	bIlInitialised = true;

	// destroy previous image
	destroy();
	
	// generate IL image id
	ilGenImages(1, &m_idImage); 

	// bind IL image and load
	ilBindImage(m_idImage);
	c_pBound = this;
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT); 
	if (ilLoadImage((ILstring)fname.c_str()))
	{
		ilConvertImage(format, IL_UNSIGNED_BYTE); 
		logSuccess(string("loaded from: ") + fname);
		return true;
	}
	else 
	{
		logWarning(string("couldn't load from: ") + fname);
		return false;
	}
}

void C3dglBitmap::destroy()
{
	if (m_idImage)
		ilDeleteImages(1, &m_idImage);
}

void C3dglBitmap::texture(GLuint &textureId)
{
	if (c_pBound != this)
	{
		ilBindImage(m_idImage);
		c_pBound = this;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData()); 
}

long C3dglBitmap::getWidth()
{
	if (c_pBound != this)
	{
		ilBindImage(m_idImage);
		c_pBound = this;
	}
	return ilGetInteger(IL_IMAGE_WIDTH);
}

long C3dglBitmap::getHeight()
{
	if (c_pBound != this)
	{
		ilBindImage(m_idImage);
		c_pBound = this;
	}
	return ilGetInteger(IL_IMAGE_HEIGHT);
}

void *C3dglBitmap::getBits()
{
	if (c_pBound != this)
	{
		ilBindImage(m_idImage);
		c_pBound = this;
	}
	return ilGetData();
}
