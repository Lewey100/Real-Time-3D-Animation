#pragma once

#ifndef BOMB_H
#define BOMB_H

//*********************************************************************************************
// File:			Aeroplane.h
// Description:		A very simple class to represent an aeroplane as one object with all the
//					hierarchical components stored internally within the class.
// Module:			Real-Time 3D Techniques for Games
// Created:			Jake - 2010-2011
// Notes:
//*********************************************************************************************

#include "Application.h"

__declspec(align(16)) class Bomb
{
public:

	Bomb(XMFLOAT4 pos = XMFLOAT4(0, 0, 0, 0), XMFLOAT4 rot = XMFLOAT4(0,0,0,0), XMVECTOR planeForwardVector = XMVectorZero(), XMVECTOR planeDownVector = XMVectorZero(), float planeSpeed = 0.0f);
	~Bomb(void);

	static void LoadResources(void);
	static void ReleaseResources(void);

	void Update();
	void Draw(void);

	XMFLOAT4 m_v4Pos;	//bomb's position
	XMFLOAT4 m_v4CamRot; // Cam rotation
	XMFLOAT4 m_v4CamOffset; // Cam offset
	XMMATRIX m_mWorldMatrix;
private:
	void UpdateMatrices(void);
	static CommonMesh* s_bombMesh; //Only one bomb mesh for all instances

	static bool s_bResourcesReady;

	XMFLOAT4 m_v4Rot;
	

	XMVECTOR m_vForwardVector;
	float m_fSpeed;

	XMMATRIX m_mWorldTrans;
	

	float fallSpeed;
	XMVECTOR planeForward;
	XMVECTOR planeDown;
	XMVECTOR m_vCamWorldPos; // World position

public:
	//functions to pass the bombs position to the camera for the bombCam
	XMFLOAT4 GetFocusPosition(void) { return GetPosition(); }
	XMFLOAT4 GetCameraPosition(void)
	{
		XMFLOAT4 v4Pos;
		XMStoreFloat4(&v4Pos, m_vCamWorldPos);
		return v4Pos;
	}
	XMFLOAT4 GetPosition(void) { return m_v4Pos; }
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