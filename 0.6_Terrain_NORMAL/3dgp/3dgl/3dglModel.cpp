#include <iostream>
#include <fstream>
#include "../GL/glew.h"
#include "../GL/3dglModel.h"
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

bool C3dglModel::load(const char* pFile, unsigned int flags)
{
	logInfo(string("Importing file: ") + pFile);
	const aiScene* pScene = aiImportFile(pFile, flags);
	if (pScene == NULL)
	{
		logError(aiGetErrorString());
		return false;
	}
	m_name = pFile;
	size_t i = m_name.find_last_of("/\\");
	if (i != string::npos) m_name = m_name.substr(i + 1);
	i = m_name.find_last_of(".");
	if (i != string::npos) m_name = m_name.substr(0, i);
	create(pScene);
	return true;
}

void C3dglModel::MESH::create(const aiMesh* pMesh, unsigned maskEnabledBufData)
{
	if (pMesh->mFaces[0].mNumIndices != 3 && pMesh->mNumFaces && pMesh->mNumVertices && pMesh->mVertices && pMesh->mNormals)
		return;

	// find the BB (bounding box)
	bb[0] = bb[1] = pMesh->mVertices[0];
	for (aiVector3D vec : vector<aiVector3D>(pMesh->mVertices, pMesh->mVertices + pMesh->mNumVertices))
	{
		if (vec.x < bb[0].x) bb[0].x = vec.x;
		if (vec.y < bb[0].y) bb[0].y = vec.y;
		if (vec.z < bb[0].z) bb[0].z = vec.z;
		if (vec.x > bb[0].x) bb[1].x = vec.x;
		if (vec.y > bb[0].y) bb[1].y = vec.y;
		if (vec.z > bb[0].z) bb[1].z = vec.z;
	}
	centre.x = 0.5f * (bb[0].x + bb[1].x);
	centre.y = 0.5f * (bb[0].y + bb[1].y);
	centre.z = 0.5f * (bb[0].z + bb[1].z);

	// check shader parameters
	GLuint attribVertex = (GLuint)-1, attribNormal = (GLuint)-1, attribTexCoord = (GLuint)-1,
		attribTangent = (GLuint)-1, attribBitangent = (GLuint)-1, attribColor = (GLuint)-1,
		attribBoneId = (GLuint)-1, attribBoneWeight = (GLuint)-1;

	C3dglProgram* pProgram = C3dglProgram::GetCurrentProgram();
	if (pProgram)
	{
		attribVertex = pProgram->GetAttribLocation(C3dglProgram::ATTR_VERTEX);
		attribNormal = pProgram->GetAttribLocation(C3dglProgram::ATTR_NORMAL);
		attribTexCoord = pProgram->GetAttribLocation(C3dglProgram::ATTR_TEXCOORD);
		attribTangent = pProgram->GetAttribLocation(C3dglProgram::ATTR_TANGENT);
		attribBitangent = pProgram->GetAttribLocation(C3dglProgram::ATTR_BITANGENT);
		attribColor = pProgram->GetAttribLocation(C3dglProgram::ATTR_COLOR);
		attribBoneId = pProgram->GetAttribLocation(C3dglProgram::ATTR_BONE_ID);
		attribBoneWeight = pProgram->GetAttribLocation(C3dglProgram::ATTR_BONE_WEIGHT);
	}
	else
		attribVertex = attribNormal = attribTexCoord = 0;	// enabled by default if no shader used

	if (attribVertex == (GLuint)-1)
		logWarning("is providing vertex coordinates but the current shader program does not support vertices. Consider another shader program.");
	if (attribNormal == (GLuint)-1)
		logWarning("is providing normal coordinates but the current shader program does not support normals. Consider another shader program.");

	// create VAO
	glGenVertexArrays(1, &m_idVAO);
	glBindVertexArray(m_idVAO);

	// generate a vertex buffer, than bind it and send data to OpenGL
	if (attribVertex != (GLuint)-1)
		if (pMesh->mVertices)
		{
			m_buf[BUF_VERTEX].populate(sizeof(pMesh->mVertices[0]), pMesh->mNumVertices, &pMesh->mVertices[0]);
			if (maskEnabledBufData & (1 << BUF_VERTEX))
				m_buf[BUF_VERTEX].storeData(sizeof(pMesh->mVertices[0]), pMesh->mNumVertices, &pMesh->mVertices[0]);

			if (pProgram)
			{
				glEnableVertexAttribArray(attribVertex);
				glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
			else
			{
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3, GL_FLOAT, 0, 0);
			}
		}
		else
			logWarning("is missing vertex buffer information");

	// generate a normal buffer, than bind it and send data to OpenGL
	if (attribNormal != (GLuint)-1)
		if (pMesh->mNormals)
		{
			m_buf[BUF_NORMAL].populate(sizeof(pMesh->mNormals[0]), pMesh->mNumVertices, &pMesh->mNormals[0]);
			if (maskEnabledBufData & (1 << BUF_NORMAL))
				m_buf[BUF_NORMAL].storeData(sizeof(pMesh->mNormals[0]), pMesh->mNumVertices, &pMesh->mNormals[0]);

			if (pProgram)
			{
				glEnableVertexAttribArray(attribNormal);
				glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
			else
			{
				glEnableClientState(GL_NORMAL_ARRAY);
				glNormalPointer(GL_FLOAT, 0, 0);
			}
		}
		else
			logWarning("is missing normal buffer information");

	//Texture Coordinates
	if (attribTexCoord != (GLuint)-1)
	{
		m_nUVComponents = pMesh->mNumUVComponents[0];	// should be 2
		if (pMesh->mTextureCoords[0] == NULL)
			logWarning("is missing texture coordinate buffer information");
		else if (m_nUVComponents != 2 && m_nUVComponents != 3)
			logWarning("is missing compatible texture coordinates");
		else
		{
			// first, convert indices to occupy contageous memory space
			vector<GLfloat> texCoords;
			for (aiVector3D vec : vector<aiVector3D>(pMesh->mTextureCoords[0], pMesh->mTextureCoords[0] + pMesh->mNumVertices))
			{
				texCoords.push_back(vec.x);
				texCoords.push_back(vec.y);
				if (m_nUVComponents == 3)
					texCoords.push_back(vec.z);
			}

			m_buf[BUF_TEXCOORD].populate(sizeof(texCoords[0]), texCoords.size(), &texCoords[0]);
			if (maskEnabledBufData & (1 << BUF_TEXCOORD))
				m_buf[BUF_TEXCOORD].storeData(sizeof(texCoords[0]), texCoords.size(), &texCoords[0]);

			if (pProgram)
			{
				glEnableVertexAttribArray(attribTexCoord);
				glVertexAttribPointer(attribTexCoord, m_nUVComponents, GL_FLOAT, GL_FALSE, 0, 0);
			}
			else
			{
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(m_nUVComponents, GL_FLOAT, 0, 0);
			}
		}
	}

	// generate a tangent buffer, than bind it and send data to OpenGL
	if (attribTangent != (GLuint)-1)
		if (pMesh->mTangents)
		{
			m_buf[BUF_TANGENT].populate(sizeof(pMesh->mTangents[0]), pMesh->mNumVertices, &pMesh->mTangents[0]);
			if (maskEnabledBufData & (1 << BUF_TANGENT))
				m_buf[BUF_TANGENT].storeData(sizeof(pMesh->mTangents[0]), pMesh->mNumVertices, &pMesh->mTangents[0]);

			if (pProgram)
			{
				glEnableVertexAttribArray(attribTangent);
				glVertexAttribPointer(attribTangent, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
		}
		else
			logWarning("is missing tangent buffer information");

	// generate a biTangent buffer, than bind it and send data to OpenGL
	if (attribBitangent != (GLuint)-1)
		if (pMesh->mBitangents)
		{
			m_buf[BUF_BITANGENT].populate(sizeof(pMesh->mBitangents[0]), pMesh->mNumVertices, &pMesh->mBitangents[0]);
			if (maskEnabledBufData & (1 << BUF_BITANGENT))
				m_buf[BUF_BITANGENT].storeData(sizeof(pMesh->mBitangents[0]), pMesh->mNumVertices, &pMesh->mBitangents[0]);

			if (pProgram)
			{
				glEnableVertexAttribArray(attribBitangent);
				glVertexAttribPointer(attribBitangent, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
		}
		else
			logWarning("is missing bitangent buffer information");

	// generate a color buffer, than bind it and send data to OpenGL
	if (attribColor != (GLuint)-1)
		if (pMesh->mColors[0])
		{
			m_buf[BUF_COLOR].populate(sizeof(pMesh->mColors[0][0]), pMesh->mNumVertices, &pMesh->mColors[0][0]);
			if (maskEnabledBufData & (1 << BUF_COLOR))
				m_buf[BUF_COLOR].storeData(sizeof(pMesh->mColors[0][0]), pMesh->mNumVertices, &pMesh->mColors[0][0]);

			if (pProgram)
			{
				glEnableVertexAttribArray(attribColor);
				glVertexAttribPointer(attribColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
		}
		else
			logWarning("is missing color buffer information");

	// generate and convert a bone buffer, than bind it and send data to OpenGL
	if (attribBoneId != (GLuint)-1 && attribBoneWeight != (GLuint)-1)
	{
		// initislise the buffer
		struct VertexBoneData
		{
			unsigned ids[MAX_BONES_PER_VEREX];
			float weights[MAX_BONES_PER_VEREX];
		};
		vector<VertexBoneData> bones;
		bones.resize(pMesh->mNumVertices);
		memset(&bones[0], 0, sizeof(bones[0]) * bones.size());

		if (pMesh->mNumBones)
		{
			// load bone info - based on http://ogldev.atspace.co.uk/
			logInfo("bones found: " + to_string(pMesh->mNumBones));

			// for each bone:
			for (aiBone* pBone : vector<aiBone*>(pMesh->mBones, pMesh->mBones + pMesh->mNumBones))
			{
				// determine bone index from its name
				unsigned iBone;
				if (!m_pOwner->getOrAddBone(pBone->mName.data, iBone))
				{
					// only executed for new bones
					m_pOwner->m_vecBoneOffsets.push_back(pBone->mOffsetMatrix);
					m_pOwner->m_vecBoneNames.push_back(pBone->mName.data);
				}

				// collect bone weights
				for (aiVertexWeight& weight : vector<aiVertexWeight>(pBone->mWeights, pBone->mWeights + pBone->mNumWeights))
				{
					// find a free location for the id and weight within bones[iVertex]
					unsigned i = 0;
					while (i < MAX_BONES_PER_VEREX && bones[weight.mVertexId].weights[i] != 0.0)
						i++;
					if (i < MAX_BONES_PER_VEREX)
					{
						bones[weight.mVertexId].ids[i] = iBone;
						bones[weight.mVertexId].weights[i] = weight.mWeight;
					}
					else
						logWarning("Maximum number of bones per vertex exceeded");
				}
			}

			// verify (and maybe, in future, normalize)
			bool bProblem = false;
			for (VertexBoneData& bone : bones)
			{
				float total = 0.0f;
				for (float weight : bone.weights)
					total += weight;
				bProblem = bProblem || total < 0.999f || total > 1.001f;
				//cout << total << endl;
			}
			if (bProblem)
				logWarning("Some bone weights do not sum up to 1.0");
		}
		else
			logWarning("is missing bone information");

		m_buf[BUF_BONE].populate(sizeof(bones[0]), pMesh->mNumVertices, &bones[0]);
		if (maskEnabledBufData & (1 << BUF_BONE))
			m_buf[BUF_BONE].storeData(sizeof(bones[0]), pMesh->mNumVertices, &bones[0]);

		if (pProgram)
		{
			glEnableVertexAttribArray(attribBoneId);
			glVertexAttribIPointer(attribBoneId, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
			glEnableVertexAttribArray(attribBoneWeight);
			glVertexAttribPointer(attribBoneWeight, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)sizeof(bones[0].ids));
		}
	}

	// first, convert indices to occupy contageous memory space
	vector<unsigned> indices;
	for (aiFace f : vector<aiFace>(pMesh->mFaces, pMesh->mFaces + pMesh->mNumFaces))
		for (unsigned n : vector<unsigned>(f.mIndices, f.mIndices + f.mNumIndices))
			indices.push_back(n);

	// generate indices buffer, than bind it and send data to OpenGL
	m_buf[BUF_INDEX].populate(sizeof(indices[0]), indices.size(), &indices[0], GL_ELEMENT_ARRAY_BUFFER);
	if (maskEnabledBufData & (1 << BUF_INDEX))
		m_buf[BUF_INDEX].storeData(sizeof(indices[0]), indices.size(), &indices[0]);
	m_indexSize = indices.size();

	m_nMaterialIndex = pMesh->mMaterialIndex;

	// Reset VAO & buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void C3dglModel::MESH::destroy()
{
	m_buf[BUF_VERTEX].release();
	m_buf[BUF_NORMAL].release();
	m_buf[BUF_TEXCOORD].release();
	m_buf[BUF_TANGENT].release();
	m_buf[BUF_BITANGENT].release();
	m_buf[BUF_COLOR].release();
	m_buf[BUF_BONE].release();
	m_buf[BUF_INDEX].release();
}

void C3dglModel::MESH::render()
{
	glBindVertexArray(m_idVAO);
	glDrawElements(GL_TRIANGLES, m_indexSize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

CMaterial* C3dglModel::MESH::createNewMaterial()
{
	CMaterial mat;
	m_nMaterialIndex = m_pOwner->m_materials.size();
	m_pOwner->m_materials.push_back(mat);
	return getMaterial();
}

void __createMap(const aiNode* pNode, std::map<std::string, const aiNode*>& map)
{
	map[pNode->mName.data] = pNode;
	for (aiNode* pChildNode : vector<aiNode*>(pNode->mChildren, pNode->mChildren + pNode->mNumChildren))
		__createMap(pChildNode, map);
}

void C3dglModel::create(const aiScene* pScene)
{
	// create meshes
	m_pScene = pScene;
	m_meshes.resize(m_pScene->mNumMeshes, MESH(this));
	aiMesh** ppMesh = m_pScene->mMeshes;
	for (MESH& mesh : m_meshes)
		mesh.create(*ppMesh++, m_maskEnabledBufData);

	m_globInvT = m_pScene->mRootNode->mTransformation;
	m_globInvT.Inverse();
}

void C3dglModel::loadMaterials(const char* pTexRootPath)
{
	if (!m_pScene) return;

	m_materials.resize(m_pScene->mNumMaterials, CMaterial());
	aiMaterial** ppMaterial = m_pScene->mMaterials;
	for (CMaterial& material : m_materials)
		material.create(*ppMaterial++, pTexRootPath);
}

unsigned C3dglModel::loadAnimations()
{
	return loadAnimations(this);
}

unsigned C3dglModel::loadAnimations(C3dglModel* pCompatibleModel)
{
	if (pCompatibleModel == NULL)
		pCompatibleModel = this;

	const aiScene* pScene = pCompatibleModel->m_pScene;

	if (!pScene || !pScene->HasAnimations()) return 0;
	if (getBoneCount() == 0)
	{
		pCompatibleModel->logWarning("comes with animations but skinning is not implemented.");
		return 0;
	}

	// create animation look-up tables
	map<string, const aiNode*> mymap;		// map of nodes
	__createMap(GetScene()->mRootNode, mymap);

	for (unsigned iAnim = 0; iAnim < pScene->mNumAnimations; iAnim++)
	{
		aiAnimation* pAnim = pScene->mAnimations[iAnim];

		m_animations.push_back(ANIMATION());
		ANIMATION& anim = m_animations[m_animations.size() - 1];
		anim.m_pAnim = pAnim;
		map<const aiNode*, std::pair<unsigned, unsigned> > &lookUp = anim.m_lookUp;

		// create the lookUp structure
		int i = 0;
		for (aiNodeAnim* pNodeAnim : vector<aiNodeAnim*>(pAnim->mChannels, pAnim->mChannels + pAnim->mNumChannels))
		{
			string strNodeName = pNodeAnim->mNodeName.data;
			const aiNode* pNode = mymap[strNodeName];
			unsigned id = 99999;
			getBone(strNodeName, id);
			lookUp[pNode] = pair<unsigned, unsigned>(i++, id);
		}

		// rest of the nodes
		for (auto mypair : mymap)
		{
			// check if the aiNode is already known in the look-up table
			if (lookUp.find(mypair.second) == lookUp.end())
			{
				unsigned id = 99999;
				getBone(mypair.first, id);
				lookUp[mypair.second] = pair<unsigned, unsigned>(0xffff, id);
			}
		}
	}
	return pScene->mNumAnimations;
}

unsigned C3dglModel::loadAnimations(const char* pFile, unsigned int flags)
{
	C3dglModel *p = new C3dglModel();
	m_auxModels.push_back(p);
	p->load(pFile, flags);
	return loadAnimations(p);
}

void C3dglModel::destroy()
{
	if (m_pScene)
	{
		for (MESH mesh : m_meshes)
			mesh.destroy();
		for (CMaterial mat : m_materials)
			mat.destroy();
		for (C3dglModel *p : m_auxModels)
			delete p;
		aiReleaseImport(m_pScene);
		m_pScene = NULL;
	}
}

void C3dglModel::enableBufData(ATTRIB_STD bufId, bool bEnable)
{
	if (bEnable)
		m_maskEnabledBufData |= (1 << bufId);
	else
		m_maskEnabledBufData &= ~(1 << bufId);
}

void C3dglModel::renderNode(aiNode* pNode, glm::mat4 m)
{
	aiMatrix4x4 mx = pNode->mTransformation;
	aiTransposeMatrix4(&mx);
	m *= glm::make_mat4((GLfloat*)&mx);

	// check if a shading program is active
	C3dglProgram* pProgram = C3dglProgram::GetCurrentProgram();

	// send model view matrix
	if (pProgram)
		pProgram->SendStandardUniform(C3dglProgram::UNI_MODELVIEW, m);
	else
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixf((GLfloat*)&m);
	}

	for (unsigned iMesh : vector<unsigned>(pNode->mMeshes, pNode->mMeshes + pNode->mNumMeshes))
	{
		MESH* pMesh = &m_meshes[iMesh];
		CMaterial* pMaterial = pMesh->getMaterial();
		if (pMaterial) pMaterial->bind();
		pMesh->render();
	}

	// draw all children
	for (aiNode* p : vector<aiNode*>(pNode->mChildren, pNode->mChildren + pNode->mNumChildren))
		renderNode(p, m);
}

void C3dglModel::render(glm::mat4 matrix)
{
	if (m_pScene->mRootNode)
		renderNode(m_pScene->mRootNode, matrix);
}

void C3dglModel::render(unsigned iNode, glm::mat4 matrix)
{
	// update transform
	aiMatrix4x4 m = m_pScene->mRootNode->mTransformation;
	aiTransposeMatrix4(&m);
	matrix *= glm::make_mat4((GLfloat*)&m);

	if (iNode <= getParentNodeCount())
		renderNode(m_pScene->mRootNode->mChildren[iNode], matrix);
}

//void C3dglModel::render()
//{
//	glm::mat4 m;
//	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)&m);
//	render(m);
//}

//void C3dglModel::render(unsigned iNode)
//{
//	glm::mat4 m;
//	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)&m);
//	render(iNode, m);
//}

void C3dglModel::getNodeTransform(aiNode* pNode, float pMatrix[16], bool bRecursive)
{
	aiMatrix4x4 m1, m2;
	if (bRecursive && pNode->mParent)
		getNodeTransform(pNode->mParent, (float*)&m1, true);

	m2 = pNode->mTransformation;
	aiTransposeMatrix4(&m2);

	*((aiMatrix4x4*)pMatrix) = m2 * m1;
}

bool C3dglModel::getBone(std::string boneName)
{
	return m_mapBones.find(boneName) != m_mapBones.end();
}

bool C3dglModel::getBone(std::string boneName, unsigned& id)
{
	auto it = m_mapBones.find(boneName);
	if (it == m_mapBones.end())
		return false;
	id = it->second;
	return true;
}

bool C3dglModel::getOrAddBone(std::string boneName, unsigned& id)
{
	if (getBone(boneName, id))
		return true;

	id = m_mapBones.size();
	m_mapBones[boneName] = id;
	return false;
}

bool C3dglModel::getBBNode(aiNode* pNode, aiVector3D BB[2], aiMatrix4x4* trafo)
{
	aiMatrix4x4 prev = *trafo;
	aiMultiplyMatrix4(trafo, &pNode->mTransformation);

	for (unsigned iMesh : vector<unsigned>(pNode->mMeshes, pNode->mMeshes + pNode->mNumMeshes))
	{
		aiVector3D bb[2];
		memcpy(bb, m_meshes[iMesh].getBB(), sizeof(bb));
		aiTransformVecByMatrix4(&bb[0], trafo);
		aiTransformVecByMatrix4(&bb[1], trafo);
		if (bb[0].x < BB[0].x) BB[0].x = bb[0].x;
		if (bb[0].y < BB[0].y) BB[0].y = bb[0].y;
		if (bb[0].z < BB[0].z) BB[0].z = bb[0].z;
		if (bb[1].x > BB[0].x) BB[1].x = bb[1].x;
		if (bb[1].y > BB[0].y) BB[1].y = bb[1].y;
		if (bb[1].z > BB[0].z) BB[1].z = bb[1].z;
	}

	for (aiNode* pNode : vector<aiNode*>(pNode->mChildren, pNode->mChildren + pNode->mNumChildren))
		getBBNode(pNode, BB, trafo);

	*trafo = prev;
	return true;
}

void C3dglModel::getBB(aiVector3D BB[2])
{
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	BB[0].x = BB[0].y = BB[0].z = 1e10f;
	BB[1].x = BB[1].y = BB[1].z = -1e10f;
	getBBNode(m_pScene->mRootNode, BB, &trafo);
}

std::string C3dglModel::getName()
{
	if (m_name.empty())
		return "";
	else
		return "Model (" + m_name + ")";
}

//////////////////////////////////////////////////////////////////////////////////////
// Articulated Animation Functions

aiVector3D Interpolate(float AnimationTime, const aiVectorKey* pKeys, unsigned nKeys)
{
	// find a pair of keys to interpolate
	unsigned i = 0;
	while (i < nKeys - 1 && AnimationTime >= (float)pKeys[i + 1].mTime)
		i++;

	// if out of bounds, return the last key
	if (i >= nKeys - 1)
		return pKeys[nKeys - 1].mValue;

	// interpolate
	const aiVector3D& Start = pKeys[i].mValue;
	const aiVector3D& End = pKeys[i + 1].mValue;
	float f = (AnimationTime - (float)pKeys[i].mTime) / ((float)(pKeys[i + 1].mTime - pKeys[i].mTime));
	return Start + f * (End - Start);
}

aiQuaternion Interpolate(float AnimationTime, const aiQuatKey* pKeys, unsigned nKeys)
{
	// find a pair of keys to interpolate
	unsigned i = 0;
	while (i < nKeys - 1 && AnimationTime >= (float)pKeys[i + 1].mTime)
		i++;

	// if out of bounds, return the last key
	if (i >= nKeys - 1)
		return pKeys[nKeys - 1].mValue;

	const aiQuaternion& StartRotationQ = pKeys[i].mValue;
	const aiQuaternion& EndRotationQ = pKeys[i + 1].mValue;
	float f = (AnimationTime - (float)pKeys[i].mTime) / ((float)(pKeys[i + 1].mTime - pKeys[i].mTime));
	aiQuaternion q;
	aiQuaternion::Interpolate(q, StartRotationQ, EndRotationQ, f);	// spherical interpolation (SLERP)
	return q.Normalize();
}

void C3dglModel::readNodeHierarchy(ANIMATION& animation, float time, const aiNode* pNode, const aiMatrix4x4& t, vector<float>& transforms)
{
	aiMatrix4x4 transform;

	auto it = animation.m_lookUp.find(pNode);
	if (it != animation.m_lookUp.end())
	{
		unsigned iAnimInd = it->second.first;
		if (iAnimInd < animation.m_pAnim->mNumChannels)
		{
			const aiNodeAnim* pNodeAnim = animation.m_pAnim->mChannels[iAnimInd];

			// Interpolate position, rotation and scaling
			aiVector3D vecTranslate = Interpolate(time, pNodeAnim->mPositionKeys, pNodeAnim->mNumPositionKeys);
			aiQuaternion quatRotate = Interpolate(time, pNodeAnim->mRotationKeys, pNodeAnim->mNumRotationKeys);
			aiVector3D vecScale = Interpolate(time, pNodeAnim->mScalingKeys, pNodeAnim->mNumScalingKeys);

			// create matrices
			aiMatrix4x4 matTranslate, matScale;
			aiMatrix4x4::Translation(vecTranslate, matTranslate);
			aiMatrix4x4 matRotate = aiMatrix4x4(quatRotate.GetMatrix());
			aiMatrix4x4::Scaling(vecScale, matScale);

			// Combine the above transformations
			transform = t * matTranslate * matRotate * matScale;
		}
		else
			transform = t * pNode->mTransformation;


		unsigned iBone = it->second.second;
		if (iBone < m_vecBoneOffsets.size())
		{
			aiMatrix4x4 m = (m_globInvT * transform * m_vecBoneOffsets[iBone]).Transpose();
			memcpy(&transforms[iBone * 16], &m, sizeof(m));
		}
	}

	for (aiNode* pChildNode : vector<aiNode*>(pNode->mChildren, pNode->mChildren + pNode->mNumChildren))
		readNodeHierarchy(animation, time, pChildNode, transform, transforms);
}

void C3dglModel::getAnimData(unsigned iAnim, float time, vector<float>& transforms)
{
	transforms.resize(m_vecBoneOffsets.size() * 16);	// 16 floats per bone matrix
	if (!hasAnim(iAnim))
	{
		if (m_vecBoneOffsets.size() == 0) transforms.resize(16);
		aiMatrix4x4 m;
		for (unsigned iBone = 0; iBone < transforms.size() / 16; iBone++)
			memcpy(&transforms[iBone * 16], &m, sizeof(m));
	}
	else
	{
		float fTicksPerSecond = (float)GetAnimTicksPerSecond(iAnim);
		if (fTicksPerSecond == 0) fTicksPerSecond = 25.0f;
		time = fmod(time * fTicksPerSecond, (float)GetAnimDuration(iAnim));

		aiMatrix4x4 t;
		readNodeHierarchy(m_animations[iAnim], time, GetScene()->mRootNode, t, transforms);
	}
}

