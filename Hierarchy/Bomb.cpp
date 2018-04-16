#include "Bomb.h"

CommonMesh* Bomb::s_bombMesh = NULL;

bool Bomb::s_bResourcesReady = false;


Bomb::Bomb(XMFLOAT4 pos, XMFLOAT4 rot, XMVECTOR planeForwardVector,XMVECTOR planeDownVector, float planeSpeed)
{
	m_mWorldMatrix = XMMatrixIdentity();

	m_v4Rot = rot;
	m_v4Pos = pos;

	//moves the camera that looks at the bomb so it looks forward and slightly down on the bomb
	m_v4CamOffset = XMFLOAT4(0, 5, -10, 0);
	m_v4CamRot = XMFLOAT4(-45, 90, 0, 0);

	//gets the planes down, forward vectors and speed to be used when applying relative motion
	planeDown = planeDownVector;
	planeForward = planeForwardVector;
	m_fSpeed = planeSpeed;
	LoadResources();
	
	m_vCamWorldPos = XMVectorZero();
	
	fallSpeed = 1.0f;
}

Bomb::~Bomb(void)
{
}


void Bomb::Update(void)
{
	//updates the bombs position based using the planes down vector and the bombs fall speed.
	//updates the cameras world position to follow the bomb down
	XMVECTOR currPos = XMLoadFloat4(&m_v4Pos);
	m_vCamWorldPos = currPos;
	currPos += (planeDown * fallSpeed);
	XMStoreFloat4(&m_v4Pos, currPos);
	UpdateMatrices();
}
	

void Bomb::UpdateMatrices(void)
{
	//applies the transform matrices to the bomb to update its world matrix.
	XMMATRIX mRotX, mRotY, mRotZ, mTrans, mScale;

	mRotX = XMMatrixRotationX(XMConvertToRadians(m_v4Rot.x));
	mRotY = XMMatrixRotationY(XMConvertToRadians(m_v4Rot.y));
	mRotZ = XMMatrixRotationZ(XMConvertToRadians(m_v4Rot.z));
	
	mScale = XMMatrixScaling(0.5f, 0.5f,0.5f);
	mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&m_v4Pos));
	m_mWorldMatrix = mScale * mRotX * mRotY * mRotY * mTrans;

}

void Bomb::LoadResources(void)
{
	//loads the bombs mesh
	s_bombMesh = CommonMesh::LoadFromXFile(Application::s_pApp, "Resources/Plane/bullet.x");
}

void Bomb::ReleaseResources(void)
{
	//deletes the bombs mesh to free up memory
	delete s_bombMesh;
}

void Bomb::Draw(void)
{
	//draws the bombs mesh in the applications world matrix
	Application::s_pApp->SetWorldMatrix(m_mWorldMatrix);
 	s_bombMesh->Draw();
}
