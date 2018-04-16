#include "Bullet.h"

CommonMesh* Bullet::s_bulletMesh = NULL;

bool Bullet::s_bResourcesReady = false;

Bullet::Bullet(XMFLOAT4 pos, XMVECTOR rot, XMVECTOR planeForwardVector, float planeSpeed, XMVECTOR gunVec) {
	m_mWorldMatrix = XMMatrixIdentity();

	m_v4Rot = rot;
	m_v4Pos = pos;

	gunForward = gunVec;
	planeForward = planeForwardVector;
	LoadResources();
	bulletSpeed = 1.0f;
	bulletLife = 0.f;
	bulletMaxLife = 300.f;
}

Bullet::~Bullet(void)
{
}


void Bullet::Update(void)
{
	//Get the forward vectors of plane and gun to apply relative motion and stores it in the bullets pos. 
	//Increases bulletLife so it will delete after being instantiated for a set amount of time.
	XMVECTOR currPos = XMLoadFloat4(&m_v4Pos);
	currPos += (planeForward + gunForward) * bulletSpeed;
	XMStoreFloat4(&m_v4Pos, currPos);
	bulletLife++;
	UpdateMatrices();
}
	

void Bullet::UpdateMatrices(void)
{
	//gets the rotation quaternion of bullet, scales down the bullet to half size, gets a translation matrix and applies all to the bullets world matrix.
	XMMATRIX mRot, mTrans, mScale;

	mRot = XMMatrixRotationQuaternion(m_v4Rot);
	mScale = XMMatrixScaling(0.5f, 0.5f,0.5f);
	mTrans = XMMatrixTranslationFromVector(XMLoadFloat4(&m_v4Pos));
	m_mWorldMatrix = mScale * mRot * mTrans;

}

void Bullet::LoadResources(void)
{
	//loads the bullets mesh
	s_bulletMesh = CommonMesh::LoadFromXFile(Application::s_pApp, "Resources/Plane/bullet.x");
}

void Bullet::ReleaseResources(void)
{
	//deletes bullet mesh to free up memory
	delete s_bulletMesh;
}

void Bullet::Draw(void)
{
	//sets bullets world matrix and draws the mesh
	Application::s_pApp->SetWorldMatrix(m_mWorldMatrix);
 	s_bulletMesh->Draw();
}
