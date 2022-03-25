#include "../GL/3dglObject.h"

#include <iostream>

using namespace std;
using namespace _3dgl;

/////////////////////////////////////////////////////////////////////////////////////////////////
// C3dglObject

bool C3dglObject::c_bQuietMode = false;

bool C3dglObject::displayInfo(int nSeverity)
{
	string name = getName();
	string severity;
	switch (nSeverity)
	{
		case 2: severity = "Warning:   "; break;
		case 3: severity = "*** ERROR: "; break;
		default: severity = ""; break;
	}
	if (name.empty())
		(m_bStatus ? cout : cerr) << severity << m_info << endl;
	else
		(m_bStatus ? cout : cerr) << severity << name << " " << m_info << endl;
	return m_bStatus; 
}

