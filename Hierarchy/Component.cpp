#include "Component.h"

Component::Component(char* meshName, XMFLOAT4 pos, XMFLOAT4 rot)
{
	m_mWorldMatrix = XMMatrixIdentity();
	m_v4Pos = pos;
	m_v4Rot = rot;
	mMeshName = meshName;

}

Component::~Component()
{
}

void Component::LoadResources(void)
{
	//Loads the corresponding mesh to name if it is not empty and then recursively calls the function for each child so they can load their corresponding meshes.
	if (mMeshName != "")
	{
		mesh = CommonMesh::LoadFromXFile(Application::s_pApp, mMeshName);
	}
	for each (Component* child in ObjectChildren)
	{
		child->LoadResources();
	}
}

void Component::ReleaseResources(void)
{
	//deletes meshes recursively for all children
	delete mesh;
	for each (Component* child in ObjectChildren)
	{
		child->ReleaseResources();
	}
}

void Component::Update(XMMATRIX* parentMatrix, bool aeroPlane)
{
	//checks if the component is part of the aeroplane gameobject and then applies the matrix manipulation to the object and all of its children.
	isAeroplane = aeroPlane;
	UpdateMatrices(parentMatrix);
	for each (Component* child in ObjectChildren)
	{
		if (aeroPlane)
			child->Update(&m_mWorldMatrix, true);
		else
			child->Update(&m_mWorldMatrix, false);
	}
}

void Component::Draw(void)
{
	//sets the world matrix and draws the meshes and all of the child meshes
	Application::s_pApp->SetWorldMatrix(m_mWorldMatrix);
	if (mMeshName != nullptr)
	{
		mesh->Draw();
	}

	for each ( Component* child in ObjectChildren)
	{		
		child->Draw();
	}
}

void Component::UpdateMatrices(XMMATRIX* parentMatrix)
{
	//gets the transform matrices, checks if the object has a parent, if not goes straight to applying the transform matrices to the world matrix.
	//if the component has a parent, it applies the transform matrices along with the parents matrix so it is always in the correct position relative the parent
	//checks if its an aeroplane and orders the matrix application correctly.
	XMMATRIX mRotX, mRotY, mRotZ, mTrans;

	mRotX = XMMatrixRotationX(XMConvertToRadians(m_v4Rot.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(m_v4Rot.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(m_v4Rot.z));
	mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&m_v4Pos));
	if (parentMatrix != nullptr)
	{
		if (!isAeroplane)
			m_mWorldMatrix = mRotX * mRotY * mRotZ * mTrans * *parentMatrix;
		else
			m_mWorldMatrix = mRotZ * mRotX * mRotY * mTrans * *parentMatrix;
	}
	else
	{
		if (!isAeroplane)
			m_mWorldMatrix = mRotX * mRotY * mRotZ * mTrans;
		else
			m_mWorldMatrix = mRotZ * mRotX * mRotY * mTrans;
	}
	
}
