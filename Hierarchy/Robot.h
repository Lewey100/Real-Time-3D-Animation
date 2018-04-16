#ifndef ROBOT_H
#define ROBOT_H

#include "Application.h"
#include "Component.h"
#include "AnimController.h"

__declspec(align(16)) class Robot
{
public:
	Robot(float fX = 0.0f, float fY = 0.0f, float fZ = 0.0f, float fRotY = 0.0f);
	~Robot(void);

	void ReleaseResources(void); // Only free the resources once for all instances
	void Update(); // Player only has control of plane when flag is set
	void Draw(void);
	Component* rootComponent;
	Component* componentArray[16];
	float timer;
	float lerpTimer;
	void HandleAnimation(char* fileName);
	void SetAnimation(char* newAnim);
	void PlayAnimation(char* fileName);

private:
	void UpdateMatrices(void);
	
	static bool s_bResourcesReady;

	XMFLOAT4 m_v4Rot; // Euler rotation angles
	XMFLOAT4 m_v4Pos; // World position

	XMVECTOR m_vForwardVector; // Forward Vector for Plane

	XMMATRIX m_mWorldTrans; // World translation matrix
	XMMATRIX m_mWorldMatrix; // World transformation matrix

	XMVECTOR m_vCamWorldPos; // World position
	XMMATRIX m_mCamWorldMatrix;
	XMFLOAT4 m_v4CamOff;
	AnimController* animControl;
	std::vector<std::vector<transformData>> keyframes;
	char* fileName;

	float lerp(float, float, float);
public:
	XMFLOAT4 GetFocusPosition(void) { return GetPosition(); }
	XMFLOAT4 GetCameraPosition(void)
	{
		XMFLOAT4 v4Pos;
		XMStoreFloat4(&v4Pos, m_vCamWorldPos);
		return v4Pos;
	}

	XMFLOAT4 SetPosition(XMFLOAT4 framePos) {
		m_v4Pos = framePos;
	}
	XMFLOAT4 SetRotation(XMFLOAT4 frameRot) {
		m_v4Rot = frameRot;
	}
	XMFLOAT4 GetPosition(void) { return rootComponent->m_v4Pos; }

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