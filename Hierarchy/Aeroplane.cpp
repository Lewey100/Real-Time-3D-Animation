//*********************************************************************************************
// File:			Aeroplane.cpp
// Description:		A very simple class to represent an aeroplane as one object with all the
//					hierarchical components stored internally within the class.
// Module:			Real-Time 3D Techniques for Games
// Created:			Jake - 2010-2011
// Notes:
//*********************************************************************************************

#include "Aeroplane.h"


bool Aeroplane::s_bResourcesReady = false;
float fireRate = 0.f;
float counter = 0.0f;

Aeroplane::Aeroplane(float fX, float fY, float fZ, float fRotY)
{
	Bullet::LoadResources();
	m_mWorldMatrix = XMMatrixIdentity();
	m_mCamWorldMatrix = XMMatrixIdentity();

	//initialises each individual component of the plane, puts it into an array of components and pushes the children into their relative parents
	Component* componentArray[5];
	componentArray[0] = new Component("Resources/Plane/plane.x", XMFLOAT4(fX, fY, fZ, 0.0f), XMFLOAT4(0.0f, fRotY, 0.0f, 0.0f));
	componentArray[1] = new Component("Resources/Plane/prop.x", XMFLOAT4(0.0f, 0.0f, 1.9f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	componentArray[2] = new Component("Resources/Plane/turret.x", XMFLOAT4(0.0f, 1.05f, -1.3f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	componentArray[3] = new Component("Resources/Plane/gun.x", XMFLOAT4(0.0f, 0.5f, 0.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f));
	componentArray[4] = new Component("Resources/Plane/bullet.x", XMFLOAT4(0.0f, -2.05f, 0.0f, 0.0f), XMFLOAT4(0, 0, 0, 0));
	componentArray[0]->ObjectChildren.push_back(componentArray[1]);
	componentArray[0]->ObjectChildren.push_back(componentArray[2]);
	componentArray[0]->ObjectChildren.push_back(componentArray[4]);
	componentArray[2]->ObjectChildren.push_back(componentArray[3]);
	
	//sets the root component of the plane
	rootComponent = componentArray[0];

	//calls the recursive LoadResources() function
	rootComponent->LoadResources();

	//sets the planes camera offset and rotation
	m_v4CamOff = XMFLOAT4(0.0f, 4.5f, -15.0f, 0.0f);
	m_v4CamRot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	m_vCamWorldPos = XMVectorZero();
	m_vForwardVector = XMVectorZero();
	m_vGunForwardVector = XMVectorZero();
	m_vDownVector = XMVectorZero();

	m_fSpeed = 0.0f;

	//sets the different camera checks to false
	m_bGunCam = false;
	m_BombCam = false;
}

Aeroplane::~Aeroplane(void)
{
}

void Aeroplane::Fire(void) {

	//fires a maximum of 10 bullets. Checks if there is already bullet, if not, checks if allowed to fire based on fireRate.
	for (size_t i = 0; i < 10; i++)
	{
		if (bulletCount[i] == nullptr)
		{
			fireRate++;
			if (fireRate > 50.f) 
			{
				//Gets the guns matrix by going through the hierarchy system. Decomposes into vectors. Gets the guns forward vector through the hierarchy system.
				XMMATRIX gunMat = rootComponent->ObjectChildren[1]->ObjectChildren[0]->m_mWorldMatrix;
				XMVECTOR rot, scale, pos;
				XMMatrixDecompose(&scale, &rot, &pos, gunMat);
				XMFLOAT4 posStore;
				DirectX::XMStoreFloat4(&posStore, pos);
				XMVECTOR gunVec = rootComponent->ObjectChildren[1]->ObjectChildren[0]->m_mWorldMatrix.r[2];
				//instantiates a new bullet with guns position, rotation values. The planes forward vector and speed and the guns forward vector.
				bulletCount[i] = new Bullet(posStore, rot,	m_vForwardVector, m_fSpeed, gunVec);
				//Resets the fireRate to be able to shoot again.
				fireRate = 0.f;
				//break;
			}
		}	}
	
}

void Aeroplane::BombsAway(void) {
	//Drops 1 bomb at a time
	for (size_t i = 0; i < 1; i++)
	{
		//of there isnt already a bomb instantiated
		if (bombCount[i] == nullptr)
		{
			//Gets planes matrix and decomposes it into appropriate vectors.
			XMMATRIX planeMat = rootComponent->m_mWorldMatrix;
			XMVECTOR rot, scale, pos;
			XMMatrixDecompose(&scale, &rot, &pos, planeMat);
			XMFLOAT4 posStore;
			DirectX::XMStoreFloat4(&posStore, pos);
			XMFLOAT4 rotStore;
			DirectX::XMStoreFloat4(&rotStore, rot);
			//sets rotation of X to 90 so the bomb spawns vertically
			rotStore.x = 90.f;
			//instantiates a new bomb
			bombCount[i] = new Bomb(posStore, rotStore, m_vForwardVector, m_vDownVector, m_fSpeed);
			//sets bombCam to true so the camera will follow the bomb
			m_BombCam = true;
			break;
		}
	}
}

void Aeroplane::UpdateBomb(void)
{
	//for the maximum number of bombs, update that object.
	for (size_t i = 0; i < 1; i++)
	{
		if (bombCount[i] != nullptr)
		{
			bombCount[i]->Update();
			
			//if the bomb has gone through the map, delete that bomb and stop the bombCam
			if (bombCount[i]->m_v4Pos.y < 0)
			{
				m_BombCam = false;
				bombCount[i] = nullptr;

			}
		}
	}
}

void Aeroplane::UpdateBullets(void)
{
	//for the maximum number of bullets, update the bullet
	for (size_t i = 0; i < 10; i++)
	{
		if (bulletCount[i] != nullptr)
		{
			bulletCount[i]->Update();
			//if the bullet has ran through its life, delete that bullet.
			if (bulletCount[i]->bulletLife > bulletCount[i]->bulletMaxLife)
			{
				bulletCount[i] = nullptr;

			}
		}
	}
}


void Aeroplane::UpdateMatrices(void)
{
	XMMATRIX mRotX, mRotY, mRotZ, mTrans, rot;
	XMMATRIX mPlaneCameraRot, mForwardMatrix;

	//Gets the planes down, forward vectors and the guns forward vector
	XMVECTOR down = XMVectorSet(0, -1, 0, 0);

	m_vDownVector = XMVector3TransformNormal(down, rootComponent->m_mWorldMatrix);;
	m_vForwardVector = rootComponent->m_mWorldMatrix.r[2];
	m_vGunForwardVector = rootComponent->ObjectChildren[1]->ObjectChildren[0]->m_mWorldMatrix.r[2];

	//sets the camera rotation to match the planes Y rotation
	mPlaneCameraRot = XMMatrixRotationY(XMConvertToRadians(rootComponent->m_v4Rot.y));
	//Gets the camera offset and applies it along with the rotation and planes position to the camera's world matrix
	XMVECTOR vCamPos = XMLoadFloat4(&m_v4CamOff);
	m_mCamWorldMatrix = XMMatrixTranslationFromVector(vCamPos) * mPlaneCameraRot * XMMatrixTranslationFromVector(XMLoadFloat4(&rootComponent->m_v4Pos));
	XMVECTOR vBombCamPos;
	if (m_bGunCam)
	{
		//if gunCam is enabled, apply the camera offset and guns world matrix to the camera's world matrix
		m_mCamWorldMatrix = XMMatrixTranslationFromVector(vCamPos) * rootComponent->ObjectChildren[1]->ObjectChildren[0]->m_mWorldMatrix;
	}

	if (m_BombCam)
	{
		//if bombCam is enabled, get the bombs camera offset and rotations. Apply the offset, rotations and bombs world matrix to the camera's world matrix to follow the bomb.
		for (size_t i = 0; i < 1; i++)
		{
			if (bombCount[i] != nullptr)
				vBombCamPos = XMLoadFloat4(&bombCount[i]->m_v4CamOffset);
			mPlaneCameraRot = XMMatrixRotationX(XMConvertToRadians(bombCount[i]->m_v4CamRot.x));
			XMMATRIX mPlaneCameraRotY = XMMatrixRotationY(XMConvertToRadians(bombCount[i]->m_v4CamRot.y));
			m_mCamWorldMatrix = XMMatrixTranslationFromVector(vBombCamPos) * mPlaneCameraRotY * mPlaneCameraRot* bombCount[0]->m_mWorldMatrix;

		}

	}
	//Camera's world pos is always equal to camera's world matrix position value
	m_vCamWorldPos = m_mCamWorldMatrix.r[3];
}

void Aeroplane::Update(bool bPlayerControl)
{
	//increase fireRate and update the objects handled by the aeroplane class
	fireRate++;
	UpdateBullets();
	UpdateBomb();
	//start recursive update of hierarchy, passing in a bool to tell the hierarchy this is an aeroplane and to apply the matrices differently.
	rootComponent->Update(nullptr, true);
	
	//if the player is on a camera in which they control the plane
	if(bPlayerControl)
	{
		
		//if Q is pressed, increase the planes xRot until it hits 60
		if (Application::s_pApp->IsKeyPressed('Q')) {
			if (rootComponent->m_v4Rot.x < 60)
			{
				rootComponent->m_v4Rot.x += 2.5f;
			}
		}
		else {
			//on key release, move rotation back to 0
			if (rootComponent->m_v4Rot.x >= 0)
			{
				rootComponent->m_v4Rot.x -= 1.0f;
			}
		}
		
		if (Application::s_pApp->IsKeyPressed('A')) {
			//if A is pressed, decrease the planes xRot until it hits 60
			if (rootComponent->m_v4Rot.x > -60)
			{
				rootComponent->m_v4Rot.x -= 2.5f;
			}
		}
		else {
			//on key release, level the plane's xRot
			if (rootComponent->m_v4Rot.x <= 0)
			{
				rootComponent->m_v4Rot.x += 1.0f;
			}
		}
		
		if (Application::s_pApp->IsKeyPressed('O'))
		{
			//if O is pressed, increase the planes zRot to roll and decrease the yRot to move in the direction the plane is rolling
			if (rootComponent->m_v4Rot.z < 20)
			{
				rootComponent->m_v4Rot.z += 1;
				
			}
			rootComponent->m_v4Rot.y -= 1;
		

		}
		else {
			//on release, level plane's roll
			if (rootComponent->m_v4Rot.z > 0)
			{
				rootComponent->m_v4Rot.z -= 1;
			}
		}
		if (Application::s_pApp->IsKeyPressed('P'))
		{
			//if P is pressed, decreased zRot to roll and increase yRot to turn in direction of roll
			if (rootComponent->m_v4Rot.z > -20)
			{
				
				rootComponent->m_v4Rot.z -= 1;
			}
			rootComponent->m_v4Rot.y += 1;
		}
		else
		{
			//on release, level plane's roll
			if (rootComponent->m_v4Rot.z < 0)
			{
				rootComponent->m_v4Rot.z += 1;
			}
		}

		if (Application::s_pApp->IsKeyPressed(' ')) {
			//when space is pressed, shoot a bullet
			Fire();
		}
		if (Application::s_pApp->IsKeyPressed('B')) {
			//when B is pressed, drop a bomb
			BombsAway();
		
		}
	} // End of if player control

	// Apply a forward thrust and limit to a maximum speed of 1
	m_fSpeed += 0.001f;

	if(m_fSpeed > 1)
		m_fSpeed = 1;

	// Rotate propeller and turret children, going through the hierarchy
	rootComponent->ObjectChildren[0]->m_v4Rot.z += 100 * m_fSpeed;
	rootComponent->ObjectChildren[1]->m_v4Rot.y += 0.1f;

	// Tilt gun up and down as turret rotates, going through the hierarchy system
	rootComponent->ObjectChildren[1]->ObjectChildren[0]->m_v4Rot.x = (sin((float)XMConvertToRadians(rootComponent->ObjectChildren[1]->m_v4Rot.y * 4.0f)) * 10.0f) - 10.0f;
	if (Application::s_pApp->IsKeyPressed('X')) { m_v4Rot.y++; }
	UpdateMatrices();

	m_vForwardVector = rootComponent->m_mWorldMatrix.r[2];
	// Move Forward
	XMVECTOR vCurrPos = XMLoadFloat4(&rootComponent->m_v4Pos);
	vCurrPos += m_vForwardVector * m_fSpeed;
	XMStoreFloat4(&rootComponent->m_v4Pos, vCurrPos);
}


void Aeroplane::ReleaseResources(void)
{
	//calls the recusrive ReleaseResources() function
	rootComponent->ReleaseResources();
}

void Aeroplane::Draw(void)
{
	//calls the recursive Draw() function
	rootComponent->Draw();

	//draws all bullets and bombs that are instansiated
	for (size_t i = 0; i < 10; i++)
	{
		if (bulletCount[i] != nullptr)
			bulletCount[i]->Draw();
	}

	for (size_t i = 0; i < 1; i++)
	{
		if (bombCount[i] != nullptr)
			bombCount[i]->Draw();
	}


}
