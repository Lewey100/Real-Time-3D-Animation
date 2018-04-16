#ifndef Component_H
#define Component_H

#include "Application.h"
#include <vector>

__declspec(align(16)) class Component
{
public:
	Component(char* meshName, XMFLOAT4 pos, XMFLOAT4 rot);
	~Component();

	void LoadResources(void);

	void ReleaseResources(void);

	void Update(XMMATRIX* parentMatrix, bool aero);
	void Draw(void);

	XMFLOAT4 m_v4Rot; // Euler rotation angles
	XMFLOAT4 m_v4Pos; // World position

	XMMATRIX m_mWorldMatrix;
	std::vector<Component*> ObjectChildren; //vector of components to give components children

private:
	void UpdateMatrices(XMMATRIX* parentMatrix);
	char* mMeshName;
	CommonMesh* mesh;
	bool isAeroplane = false;
};

#endif // !Component_H