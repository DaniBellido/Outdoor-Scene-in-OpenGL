#ifndef __3dglObject_h_
#define __3dglObject_h_

/********************************************************************************************************
GLSL Shader Classes
Created by Jarek Francik for Kingston University students
Partially based on Luke Benstead GLSLProgram class written for the book: Beginning OpenGL Game Programming
**********************************************************************************************************/

#include <string>
#include <map>

namespace _3dgl
{

class C3dglObject
{
	bool m_bStatus;
	std::string m_info;
	static bool c_bQuietMode;
public:
	C3dglObject()									{ }
	bool logError(std::string info)					{ m_bStatus = false; m_info = info; if (!getQuietMode()) displayInfo(3); return m_bStatus; }
	void logWarning(std::string info)				{ m_info = info; if (!getQuietMode()) displayInfo(2); }
	void logInfo(std::string info)					{ m_info = info; if (!getQuietMode()) displayInfo(1); }
	bool logSuccess(std::string info = "OK")		{ m_bStatus = true;  m_info = info; if (!getQuietMode()) displayInfo(0); return m_bStatus; }

	virtual std::string getName() = 0;
	bool getStatus()								{ return m_bStatus; }
	std::string getInfo()							{ return m_info; }
	bool displayInfo(int nSeverity = 0);

	static void setQuietMode(bool bQuietMode)		{ c_bQuietMode = bQuietMode; }
	static bool getQuietMode()						{ return c_bQuietMode; }
};

}; // namespace _3dgl

#endif // __3dglObject_h_