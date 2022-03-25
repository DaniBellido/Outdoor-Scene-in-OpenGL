/*********************************************************************************
3DGL 3D Graphics Library created by Jarek Francik for Kingston University students
Partially based on http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html
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

#ifndef __3dglModel_h_
#define __3dglModel_h_

#include "3dglObject.h"

// AssImp Scene include
#include "assimp/scene.h"
#include "assimp/postprocess.h"

// standard libraries
#include <vector>
#include <map>

#include "../glm/mat4x4.hpp"
#include "../GL/3dglMaterial.h"

namespace _3dgl
{

#define MAX_BONES_PER_VEREX 4

	enum ATTRIB_STD	{ BUF_VERTEX, BUF_NORMAL, BUF_TEXCOORD, BUF_TANGENT, BUF_BITANGENT, BUF_COLOR, BUF_BONE, BUF_INDEX, BUF_LAST };

class C3dglModel : public C3dglObject
{
public:

	struct MESH
	{
	private:
		// Owner
		C3dglModel *m_pOwner;

		// VAO (Vertex Array Object) id
		unsigned m_idVAO;

		struct BUFFER
		{
			unsigned m_id;
			void *m_pData;
			unsigned m_num, m_size;

			BUFFER()	{ m_id = (unsigned)-1; m_pData = NULL; m_size = m_num = 0; }

			void populate(unsigned size, unsigned num, const void *pData, GLenum target = GL_ARRAY_BUFFER, GLenum usage = GL_STATIC_DRAW)
			{
				glGenBuffers(1, &m_id);
				glBindBuffer(target, m_id);
				glBufferData(target, size * num, pData, usage);
			}
			void storeData(unsigned size, unsigned num, const void *pData)
			{
				if (size * num == 0 || pData == NULL) return;
				m_size = size;
				m_num = num;
				m_pData = new char[size * num];
				memcpy(m_pData, pData, m_size * m_num);
			}
			void getData(void **p, unsigned &size, unsigned &num)	{ if (p) *p = m_pData; size = m_size; num = m_num; }
			void release()		{ glDeleteBuffers(1, &m_id); if (m_pData) delete[] m_pData; m_size = m_num = 0; }
		};

		// Buffers
		BUFFER m_buf[BUF_LAST];

		// number of elements to draw (size of index buffer)
		int m_indexSize;

		// number of texture UV coords (2 or 3 implemented)
		unsigned m_nUVComponents;

		// Material Index - points to the main m_materials collection
		unsigned m_nMaterialIndex;
		
		// Bounding Box (experimental feature)
		aiVector3D bb[2];
		aiVector3D centre;

	public:
		MESH(C3dglModel *pOwner) : m_pOwner(pOwner) { }

		void create(const aiMesh *pMesh, unsigned maskEnabledBufData = 0);
		void destroy();
		void render();

		CMaterial *getMaterial()		{ return m_pOwner ? m_pOwner->getMaterial(m_nMaterialIndex) : NULL; }
		CMaterial *createNewMaterial();

		// get buffer binary data - call C3dglModel::enableBufferData before loading!
		void getBufferData(ATTRIB_STD bufId, void **p, unsigned &size, unsigned &num)	{ m_buf[bufId].getData(p, size, num); }
		
		aiVector3D *getBB()			{ return bb; }
		aiVector3D getCentre()		{ return centre; } 
	
		std::string getName()				{ return "mesh #" + std::to_string(this - &m_pOwner->m_meshes[0]); }
		bool logError(std::string info)		{ return m_pOwner->logError(getName() + " " + info);  }
		void logWarning(std::string info)	{ return m_pOwner->logWarning(getName() + " " + info); }
		void logInfo(std::string info)		{ return m_pOwner->logInfo(getName() + " " + info); }
	};

private:

	const aiScene *m_pScene;
	std::vector<MESH> m_meshes;
	std::vector<CMaterial> m_materials;
	std::string m_name;

	unsigned m_maskEnabledBufData;

	// bone & animation data
	struct ANIMATION
	{
		std::map<const aiNode*, std::pair<unsigned, unsigned> > m_lookUp;
		aiAnimation* m_pAnim;
	};
	std::vector<ANIMATION> m_animations;
	std::vector<C3dglModel*> m_auxModels;			// used by loadAnimations(pFile)
	std::map<std::string, unsigned> m_mapBones;		// map of bone names
	std::vector<std::string> m_vecBoneNames;		// vector of bone names
	std::vector<aiMatrix4x4> m_vecBoneOffsets;		// vector of bone offsets
	aiMatrix4x4 m_globInvT;

public:
	C3dglModel() : C3dglObject()			{ m_pScene = NULL; m_maskEnabledBufData = NULL;  }
	~C3dglModel()							{ destroy(); }

	const aiScene *GetScene()				{ return m_pScene; }

	// load a model from file
	bool load(const char* pFile, unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality);
	// create a model from AssImp handle - useful if you are using AssImp directly
	void create(const aiScene *pScene);
	// create material information and load textures from MTL file - must be preceded by either load or create
	void loadMaterials(const char* pDefTexPath = NULL);
	// load animations. By default loads animations from the current model. 
	// Any other model must be structurally compatible. Returns number of animations successfully loaded.
	unsigned loadAnimations();
	unsigned loadAnimations(C3dglModel* pCompatibleModel);
	unsigned loadAnimations(const char* pFile, unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality);
	// destroy the model
	void destroy();

	// call before load - to enable buffer binary data access - see MESH::getBufferData
	void enableBufData(ATTRIB_STD bufId, bool bEnable = true);

	unsigned getMeshCount()					{ return m_meshes.size(); }
	MESH *getMesh(unsigned i)				{ return (i < m_meshes.size()) ? &m_meshes[i] : NULL; }
	unsigned getMaterialCount()				{ return m_materials.size(); }
	CMaterial *getMaterial(unsigned i)		{ return (i < m_materials.size()) ? &m_materials[i] : NULL; }

	// Rendering
	void render(glm::mat4 matrix);					// render the entire model
	void render(unsigned iNode, glm::mat4 matrix);	// render one of the parent nodes
	unsigned getParentNodeCount()			{ return (m_pScene && m_pScene->mRootNode) ? m_pScene->mRootNode->mNumChildren : 0; }
	void renderNode(aiNode* pNode, glm::mat4 m);	// render a node

	// Deprecated rendering functions - used old-style OGL GL_MODELVIEW_MATRIX matrix
	//void render();									// DEPRECATED render the entire model
	//void render(unsigned iNode);					// DEPRECATED render one of the main nodes


	// Advanced functions
	// retrieves the transform associated with the given node. If (bRecursive) the transform is recursively combined with parental transform(s)
	void getNodeTransform(aiNode *pNode, float pMatrix[16], bool bRecursive = true);
	
	// Bone system functions
	unsigned getBoneCount()									{ return m_mapBones.size(); }
	std::string getBoneName(unsigned i) { return i < getBoneCount() ? m_vecBoneNames[i] : ""; }
	bool getBone(std::string boneName);						// true if bone exists in the model
	bool getBone(std::string boneName, unsigned& id);		// as above, additionally returns the bone id
	bool getOrAddBone(std::string boneName, unsigned& id);	// as above, creates a new bone if doesn't exist

	bool hasAnimations()					{ return m_animations.size() > 0;  }
	unsigned getAnimCount()					{ return m_animations.size(); }
	bool hasAnim(unsigned iAnim)			{ return iAnim < getAnimCount(); }
	std::string GetAnimName(unsigned iAnim) { return hasAnim(iAnim) ? m_animations[iAnim].m_pAnim->mName.data : NULL; }
	double GetAnimDuration(unsigned iAnim)	{ return hasAnim(iAnim) ? m_animations[iAnim].m_pAnim->mDuration : 0; }
	double GetAnimTicksPerSecond(unsigned iAnim) { return hasAnim(iAnim) ? m_animations[iAnim].m_pAnim->mTicksPerSecond : 0; }
	// retrieve bone transformations for the given time point. The vector transforms will be cleared and resized to reflect the actual number of bones
	void getAnimData(unsigned iAnim, float time, std::vector<float>& transforms);

	// Bounding Box functions
	void getBB(aiVector3D BB[2]);
	void getBB(unsigned iNode, aiVector3D BB[2]);

	std::string getName();

private:
	bool getBBNode(aiNode* pNode, aiVector3D BB[2], aiMatrix4x4* trafo);
	void readNodeHierarchy(ANIMATION &animation, float time, const aiNode* pNode, const aiMatrix4x4 &t, std::vector<float> &transforms);
};

}; // namespace _3dgl

#endif