
#ifndef BULLET_H
#define BULLET_H

//*********************************************************************************************
// File:			Aeroplane.h
// Description:		A very simple class to represent an aeroplane as one object with all the
//					hierarchical components stored internally within the class.
// Module:			Real-Time 3D Techniques for Games
// Created:			Jake - 2010-2011
// Notes:
//*********************************************************************************************

#include "Application.h"

__declspec(align(16)) class Bullet
{public:

	Bullet(XMFLOAT4 pos = XMFLOAT4(0,0,0,0), XMVECTOR rot = XMVectorZero(), XMVECTOR planeForwardVector = XMVectorZero(), float planeSpeed = 0.0f, XMVECTOR gunVec = XMVectorZero());
	~Bullet(void);

	static void LoadResources(void);
	static void ReleaseResources(void);

	void Update();
	void Draw(void);

	float bulletLife;
	float bulletMaxLife;
	

private:
	void UpdateMatrices(void);
	static CommonMesh* s_bulletMesh; //Only one bullet mesh for all instances

	static bool s_bResourcesReady;

	XMVECTOR m_v4Rot;
	XMFLOAT4 m_v4Pos;

	
	XMMATRIX m_mWorldMatrix;
	float bulletSpeed;
	XMVECTOR planeForward;
	XMVECTOR gunForward;

public:

	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}
};

#endif