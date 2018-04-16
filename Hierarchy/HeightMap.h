#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

//**********************************************************************
// File:			HeightMap.h
// Description:		A simple class to represent a heightmap
// Module:			Real-Time 3D Techniques for Games
// Created:			Jake - 2010-2011
// Notes:
//**********************************************************************

#include "Application.h"

static const char* const g_aTextureFileNames[] = {
	"Resources/Moss.dds",
	"Resources/Grass.dds",
	"Resources/Asphalt.dds",
	"Resources/MaterialMap.dds",
};

static const size_t NUM_TEXTURE_FILES = sizeof g_aTextureFileNames / sizeof g_aTextureFileNames[0];

class HeightMap
{
public:
	HeightMap(char* filename, float gridSize);
	~HeightMap();

	void Draw(float frameCount);
	bool ReloadShader();
	void DeleteShader();

private:
	bool LoadHeightMap(char* filename, float gridSize);

	XMFLOAT3* GetFaceNormalPtr(int faceIndex, int offset);
	XMFLOAT3 GetAveragedVertexNormal(int index, int row);

	ID3D11Buffer* m_pHeightMapBuffer;

	int m_HeightMapWidth;
	int m_HeightMapLength;
	int m_HeightMapVtxCount;
	int m_HeightMapFaceCount;
	int m_FacesPerRow;
	XMFLOAT3* m_pHeightMap;
	XMFLOAT3* m_pFaceNormals;
	XMFLOAT3* m_pNormalMap;
	Vertex_Pos3fColour4ubNormal3fTex2f* m_pMapVtxs;

	Application::Shader m_shader;

	ID3D11Buffer* m_pMyAppCBuffer; // our custom buffer resource.
	int m_psMyAppCBufferSlot; // custom buffer resource binding in PS, discovered by reflection.
	int m_vsMyAppCBufferSlot; // custom buffer resource binding in VS, discovered by reflection.

	int m_frameCountOffset;

	int m_psTexture0;
	int m_psTexture1;
	int m_psTexture2;
	int m_vsMaterialMap;

	ID3D11Texture2D* m_pTextures[NUM_TEXTURE_FILES];
	ID3D11ShaderResourceView* m_pTextureViews[NUM_TEXTURE_FILES];
	ID3D11SamplerState* m_pSamplerState;
};

#endif