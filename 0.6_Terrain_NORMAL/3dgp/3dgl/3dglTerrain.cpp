#include <fstream>
#include <iostream>

#include <Windows.h>
#include "../GL/glew.h"
#include "../GL/3dglShader.h"
#include "../GL/3dglTerrain.h"
#include "../GL/3dglBitmap.h"

using std::vector;
using namespace _3dgl;

C3dglTerrain::C3dglTerrain()
{
    m_nSizeX = m_nSizeZ = m_vertexBuffer = m_normalBuffer = m_texCoordBuffer = m_indexBuffer = 0;
	m_fScaleHeight = 1;
}

float C3dglTerrain::getHeight(int x, int z)
{
	x += m_nSizeX/2;
	z += m_nSizeZ/2;
	if (x < 0 || x >= m_nSizeX) return 0;
	if (z < 0 || z >= m_nSizeZ) return 0;
	return m_heights[x * m_nSizeZ + z];
}

	// compute the area of a triangle using Heron's formula
	float triarea(float a, float b, float c)
	{
		float s = (a + b + c)/2.0f;
		float area=sqrt(fabs(s*(s-a)*(s-b)*(s-c)));
		return area;     
	}

	// compute the distance between two 
	float dist(float x0, float y0, float x1, float y1)
	{
		float a = x1 - x0;	  
		float b = y1 - y0;
		return sqrt(a*a + b*b);
	}

	// barycentric interpolation of (x, y) within a triangle (x0, y0), (x1, y1), (x2, y2) with values v0, v1, v2
	float barycent(float x, float y, float x0, float y0, float v0, float x1, float y1, float v1, float x2, float y2, float v2)
	{
		// compute the area of the big triangle
		float a = dist(x0, y0, x1, y1);
		float b = dist(x1, y1, x2, y2);
		float c = dist(x2, y2, x0, y0);
		float totalarea = triarea(a, b, c);

		// compute the distances from the outer vertices to the inner vertex
		float length0 = dist(x0, y0, x, y);
		float length1 = dist(x1, y1, x, y);
		float length2 = dist(x2, y2, x, y);

		float f0 = triarea(b, length1, length2) / totalarea;
		float f1 = triarea(c, length0, length2) / totalarea;
		float f2 = triarea(a, length0, length1) / totalarea;

		return v0 * f0 + v1 * f1 + v2 * f2;
	}

float C3dglTerrain::getInterpolatedHeight(float fx, float fz)
{
	int x = (int)floor(fx);
	int z = (int)floor(fz);
	fx -= x;
	fz -= z;
	if (fx + fz < 1)
		return barycent(fx, fz, 0, 0, getHeight(x, z), 0, 1, getHeight(x, z + 1), 1, 0, getHeight(x + 1, z));
	else
		return barycent(fx, fz, 0, 1, getHeight(x, z + 1), 1, 0, getHeight(x + 1, z), 1, 1, getHeight(x + 1, z + 1));


	return m_heights[z * m_nSizeX + x];
}

bool C3dglTerrain::loadHeightmap(const std::string filename, float scaleHeight)
{
	C3dglBitmap bm;
	bm.load(filename, GL_RGBA);

	m_nSizeX = bm.getWidth();
	m_nSizeZ = abs(bm.getHeight());

	m_fScaleHeight = scaleHeight;

	// Collect Height Values
	m_heights.clear();
    m_heights.reserve(m_nSizeX * m_nSizeZ); //Reserve some space (faster)
	for (int i = 0; i < m_nSizeX; i++)
		for (int j = m_nSizeZ - 1; j >= 0; j--)
		{
			int index = (i + j * m_nSizeX) * 4;
			unsigned char *pBytes = (unsigned char*)(bm.GetBits());
			unsigned char val = pBytes[index];
			float f = (float)val / 256.0f;
			m_heights.push_back(f * m_fScaleHeight);
		}

//bool C3dglTerrain::loadHeightmap(const std::wstring& rawFile, float scaleHeight)
//{
//	// Windows-specific code
//	// We are using WinAPI-32 functions to load a bitmap and read height map
//	HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, rawFile.c_str(), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
//	if (!hBitmap) return false;		// file not found, bad format or other error
//	BITMAP bitmap;
//	GetObject(hBitmap, sizeof(bitmap), &bitmap);
//	m_nSizeX = bitmap.bmWidth;
//	m_nSizeZ = abs(bitmap.bmHeight);
//	HDC hdc = CreateCompatibleDC(GetDC(0));
//	SelectObject(hdc, (HGDIOBJ)hBitmap);
//
//	// Collect Height Values
//    m_heights.reserve(m_nSizeX * m_nSizeZ); //Reserve some space (faster)
//	for (int i = 0; i < m_nSizeX; i++)
//		for (int j = 0; j < m_nSizeZ; j++)
//		{
//			auto pixel = GetPixel(hdc, i, j);
//			auto g = GetGValue(pixel);
//			float f = (float)g / 256.0f * scaleHeight;
//			m_heights.push_back(f);
//		}

	// Collect Vertices, Normals and Lines (the latter - for the visualisation of normal vectors)
    vector<float> vertices;
    vector<float> normals;
    vector<float> texCoords;
	vector<float> lines;
	int minx = -m_nSizeX/2;
	int minz = -m_nSizeZ/2;
	for (int x = minx; x < minx + m_nSizeX; x++)
		for (int z = minz; z < minz + m_nSizeZ; z++)
		{
			vertices.push_back((float)x);
			vertices.push_back(getHeight(x, z));
			vertices.push_back((float)z);

			int x0 = (x == minx) ? x : x - 1;
			int x1 = (x == minx + m_nSizeX-1) ? x : x + 1;
			int z0 = (z == minz) ? z : z - 1;
			int z1 = (z == minz + m_nSizeZ-1) ? z : z + 1;

			float dy_x = getHeight(x1, z) - getHeight(x0, z);
			float dy_z = getHeight(x, z1) - getHeight(x, z0);
			float m = sqrt(dy_x * dy_x + 4 + dy_z * dy_z);
			normals.push_back(-dy_x / m);
			normals.push_back(2 / m);
			normals.push_back(-dy_z / m);

			texCoords.push_back((float)x / 2.f);
			texCoords.push_back((float)z / 2.f);

			lines.push_back((float)x);
			lines.push_back(getHeight(x, z));
			lines.push_back((float)z);
			lines.push_back(x - dy_x / m);
			lines.push_back(getHeight(x, z) + 2 / m);
			lines.push_back(z - dy_z / m);
		}

	// Prepare Vertex Buffer
    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// Prepare Normal Buffer
    glGenBuffers(1, &m_normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normals.size(), &normals[0], GL_STATIC_DRAW);

	// Prepare TexCoords Buffer
	glGenBuffers(1, &m_texCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_texCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);

	// Prepare Vertex Buffer for Visualisation of Normal Vectors
    glGenBuffers(1, &m_linesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_linesBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * lines.size(), &lines[0], GL_STATIC_DRAW);

	// Generate Indices
	
    /*
        We loop through building the triangles that
        make up each grid square in the heightmap

        (z*w+x) *----* (z*w+x+1)
                |   /|
                |  / |
                | /  |
     ((z+1)*w+x)*----* ((z+1)*w+x+1)
    */
    //Generate the triangle indices
	vector<unsigned int> indices;
	for (int z = 0; z < m_nSizeZ - 1; ++z)
		for (int x = 0; x < m_nSizeX - 1; ++x)
		{
			indices.push_back(x * m_nSizeZ + z); // current point
			indices.push_back(x * m_nSizeZ + z + 1); // next row
			indices.push_back((x + 1) * m_nSizeZ + z); // same row, next col

			indices.push_back(x * m_nSizeZ + z + 1); // next row
			indices.push_back((x + 1) * m_nSizeZ + z + 1); //next row, next col
			indices.push_back((x + 1) * m_nSizeZ + z); // same row, next col
		}

	// Prepare Index Buffer
    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);

    return true;
}

bool C3dglTerrain::storeAsOBJ(const std::string filename)
{
	std::ofstream wf(filename, std::ios::out);

	wf << "# 3dgl Mesh Exporter - (c)2021 Jarek Francik" << std::endl << std::endl;
	wf << "# object Terrain1" << std::endl << std::endl;

	// recreate the buffers - this is not a good style of programming!
	// Collect Vertices, Normals and Lines (the latter - for the visualisation of normal vectors)
	vector<float> vertices;
	vector<float> normals;
	vector<float> texCoords;
	int minx = -m_nSizeX / 2;
	int minz = -m_nSizeZ / 2;
	for (int x = minx; x < minx + m_nSizeX; x++)
		for (int z = minz; z < minz + m_nSizeZ; z++)
		{
			vertices.push_back((float)x);
			vertices.push_back(getHeight(x, z));
			vertices.push_back((float)z);

			int x0 = (x == minx) ? x : x - 1;
			int x1 = (x == minx + m_nSizeX - 1) ? x : x + 1;
			int z0 = (z == minz) ? z : z - 1;
			int z1 = (z == minz + m_nSizeZ - 1) ? z : z + 1;

			float dy_x = getHeight(x1, z) - getHeight(x0, z);
			float dy_z = getHeight(x, z1) - getHeight(x, z0);
			float m = sqrt(dy_x * dy_x + 4 + dy_z * dy_z);
			normals.push_back(-dy_x / m);
			normals.push_back(2 / m);
			normals.push_back(-dy_z / m);

			texCoords.push_back((float)x / 2.f);
			texCoords.push_back((float)z / 2.f);
		}

	//Generate the triangle indices
	vector<unsigned int> indices;
	for (int z = 0; z < m_nSizeZ - 1; ++z)
		for (int x = 0; x < m_nSizeX - 1; ++x)
		{
			indices.push_back(x * m_nSizeZ + z); // current point
			indices.push_back(x * m_nSizeZ + z + 1); // next row
			indices.push_back((x + 1) * m_nSizeZ + z); // same row, next col

			indices.push_back(x * m_nSizeZ + z + 1); // next row
			indices.push_back((x + 1) * m_nSizeZ + z + 1); //next row, next col
			indices.push_back((x + 1) * m_nSizeZ + z); // same row, next col
		}

	for (size_t i = 0; i < vertices.size(); i += 3)
		wf << "v  " << vertices[i] << " " << vertices[i + 1] << " " << vertices[i + 2] << std::endl;
	wf << std::endl;

	for (size_t i = 0; i < normals.size(); i += 3)
		wf << "vn  " << normals[i] << " " << normals[i + 1] << " " << normals[i + 2] << std::endl;
	wf << std::endl;

	for (size_t i = 0; i < texCoords.size(); i += 2)
		wf << "vt  " << texCoords[i] << " " << texCoords[i + 1] << " " << 0.0f << std::endl;
	wf << std::endl;

	wf << "g Terrain1" << std::endl;
	wf << "s 1" << std::endl;
	for (size_t i = 0; i < indices.size(); i += 3)
		wf << "f  " << indices[i] << "/" << indices[i] << "/" << indices[i] << " "
				    << indices[i+1] << "/" << indices[i+1] << "/" << indices[i+1] << " "
				    << indices[i+2] << "/" << indices[i+2] << "/" << indices[i+2] << std::endl;

	return true;
}

bool C3dglTerrain::storeAsRAW(const std::string filename)
{
	// store heightmap
	std::ofstream wf(filename, std::ios::out | std::ios::binary);
	for (int i = 0; i < m_nSizeX; i++)
		for (int j = 0; j < m_nSizeZ; j++)
		{
			int index = j + i * m_nSizeX;
			unsigned char val = (unsigned char)(m_heights[index] * 256 / m_fScaleHeight);
			wf.put(val);
		}
	wf.close();
	return true;
}

void C3dglTerrain::render(glm::mat4 matrix)
{
	// check if a shading program is active
	C3dglProgram *pProgram = C3dglProgram::GetCurrentProgram();

	if (pProgram)
	{
		pProgram->SendStandardUniform(C3dglProgram::UNI_MODELVIEW, matrix);

		GLuint attribVertex = pProgram->GetAttribLocation(C3dglProgram::ATTR_VERTEX);
		GLuint attribNormal = pProgram->GetAttribLocation(C3dglProgram::ATTR_NORMAL);
		GLuint attribTexCoord = pProgram->GetAttribLocation(C3dglProgram::ATTR_TEXCOORD);

		// programmable pipeline
		glEnableVertexAttribArray(attribVertex);
		glEnableVertexAttribArray(attribNormal);
		glEnableVertexAttribArray(attribTexCoord);

		//Bind the vertex array and set the vertex pointer to point at it
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
		glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// Bind the normal array and set the normal pointer to point at it
		glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
		glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// Bind the tex coord array and set the tex coord pointer to point at it
		glBindBuffer(GL_ARRAY_BUFFER, m_texCoordBuffer);
		glVertexAttribPointer(attribTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

		//Bind the index array and draw triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
		glDrawElements(GL_TRIANGLES, (m_nSizeX - 1) * (m_nSizeZ - 1) * 6, GL_UNSIGNED_INT, 0);

		glDisableVertexAttribArray(attribVertex);
		glDisableVertexAttribArray(attribNormal);
		glDisableVertexAttribArray(attribTexCoord);
	}
	else
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixf((GLfloat*)&matrix);

		// fixed pipeline rendering
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		//Bind the vertex array and set the vertex pointer to point at it
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
		glVertexPointer(3, GL_FLOAT, 0, 0);

		// Bind the normal array and set the normal pointer to point at it
		glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
		glNormalPointer(GL_FLOAT, 0, 0);

		// Bind the tex coord array and set the tex coord pointer to point at it
		glBindBuffer(GL_ARRAY_BUFFER, m_texCoordBuffer);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);

		//Bind the index array and draw triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
		glDrawElements(GL_TRIANGLES, (m_nSizeX - 1) * (m_nSizeZ - 1) * 6, GL_UNSIGNED_INT, 0);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
	}
}

void C3dglTerrain::render()
{
	glm::mat4 m;
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)&m);
	render(m);
}

void C3dglTerrain::renderNormals()
{
	// check if a shading program is active
	C3dglProgram *pProgram = C3dglProgram::GetCurrentProgram();
	if (pProgram)
	{
		GLuint attribVertex = pProgram->GetAttribLocation(C3dglProgram::ATTR_VERTEX);

		// programmable pipeline
		glDisable(GL_LIGHTING);
		glEnableVertexAttribArray(attribVertex);
		glBindBuffer(GL_ARRAY_BUFFER, m_linesBuffer);
		glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_LINES, 0, m_nSizeX * m_nSizeZ * 2);
		glDisableVertexAttribArray(attribVertex);
		glEnable(GL_LIGHTING);
	}
	else
	{
		// fixed pipeline rendering
		glDisable(GL_LIGHTING);
		glEnableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, m_linesBuffer);
		glVertexPointer(3, GL_FLOAT, 0, 0);
		glDrawArrays(GL_LINES, 0, m_nSizeX * m_nSizeZ * 2);
		glDisableClientState(GL_VERTEX_ARRAY);
		glEnable(GL_LIGHTING);
	}
}


