/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Version 2.2 23/03/15

Copyright (C) 2013-15 Jarek Francik, Kingston University, London, UK

A very simple bitmap class.
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
#ifndef __3dglBitmap_h_
#define __3dglBitmap_h_

#include "3dglObject.h"

#include <string>
#include <string>

namespace _3dgl
{

class C3dglBitmap : public C3dglObject
{
	unsigned int m_idImage;
	static C3dglBitmap *c_pBound;

public:
	C3dglBitmap()	{ m_idImage = 0; }
	~C3dglBitmap()	{ destroy(); }
	C3dglBitmap(const std::string fname, unsigned format);

	bool Load(const std::string fname, unsigned format)	{ return load(fname, format); }
	bool load(const std::string fname, unsigned format);
	void destroy();
	void texture(GLuint &textureId);

	long GetWidth()					{ return getWidth(); }
	long getWidth();
	long GetHeight()				{ return getHeight(); }
	long getHeight();
	void *GetBits()					{ return getBits(); }
	void *getBits();

	std::string getName()	{ return "Texture"; }
};

}; // namespace _3dgl

#endif