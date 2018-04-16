#include "HeightMap.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

HeightMap::HeightMap(char* filename, float gridSize)
{
	for (size_t i = 0; i < NUM_TEXTURE_FILES; ++i)
	{
		m_pTextures[i] = NULL;
		m_pTextureViews[i] = NULL;
	}

	m_pSamplerState = NULL;
	m_pMyAppCBuffer = NULL;

	LoadHeightMap(filename, gridSize);

	m_pHeightMapBuffer = NULL;

	static VertexColour MAP_COLOUR(200, 255, 150, 255);

	m_HeightMapVtxCount = (m_HeightMapLength * m_HeightMapWidth * 6);
	m_pMapVtxs = new Vertex_Pos3fColour4ubNormal3fTex2f[m_HeightMapVtxCount];

	int vtxIndex = 0;
	int mapIndex = 0;

	XMVECTOR v0, v1, v2, v3;
	XMVECTOR t0, t1, t2, t3;
	XMFLOAT3 v512 = XMFLOAT3(512.0f, 0.0f, 512.0f);
	XMVECTOR vOffset = XMLoadFloat3(&v512);

	// This is the unstripped method, which you can replace with your stripped method if you wish
	for (int l = 0; l < m_HeightMapLength; ++l)
	{
		for (int w = 0; w < m_HeightMapWidth; ++w)
		{
			if (w < m_HeightMapWidth - 1 && l < m_HeightMapLength - 1)
			{
				v0 = XMLoadFloat3(&m_pHeightMap[mapIndex]);
				v1 = XMLoadFloat3(&m_pHeightMap[mapIndex + m_HeightMapWidth]);
				v2 = XMLoadFloat3(&m_pHeightMap[mapIndex + 1]);
				v3 = XMLoadFloat3(&m_pHeightMap[mapIndex + m_HeightMapWidth + 1]);

				XMVECTOR vA = v0 - v1;
				XMVECTOR vB = v1 - v2;
				XMVECTOR vC = v3 - v1;

				XMVECTOR vN1, vN2;
				vN1 = XMVector3Cross(vA, vB);
				vN1 = XMVector3Normalize(vN1);

				vN2 = XMVector3Cross(vB, vC);
				vN2 = XMVector3Normalize(vN2);

				// Spread textures evenly across landscape
				t0 = (v0 + vOffset) / 32.0f;
				t1 = (v1 + vOffset) / 32.0f;
				t2 = (v2 + vOffset) / 32.0f;
				t3 = (v3 + vOffset) / 32.0f;

				t0 = XMVectorSwizzle(t0, 0, 2, 1, 3);
				t1 = XMVectorSwizzle(t1, 0, 2, 1, 3);
				t2 = XMVectorSwizzle(t2, 0, 2, 1, 3);
				t3 = XMVectorSwizzle(t3, 0, 2, 1, 3);

				m_pMapVtxs[vtxIndex + 0] = Vertex_Pos3fColour4ubNormal3fTex2f(v0, MAP_COLOUR, vN1, t0);
				m_pMapVtxs[vtxIndex + 1] = Vertex_Pos3fColour4ubNormal3fTex2f(v1, MAP_COLOUR, vN1, t1);
				m_pMapVtxs[vtxIndex + 2] = Vertex_Pos3fColour4ubNormal3fTex2f(v2, MAP_COLOUR, vN1, t2);
				m_pMapVtxs[vtxIndex + 3] = Vertex_Pos3fColour4ubNormal3fTex2f(v2, MAP_COLOUR, vN2, t2);
				m_pMapVtxs[vtxIndex + 4] = Vertex_Pos3fColour4ubNormal3fTex2f(v1, MAP_COLOUR, vN2, t1);
				m_pMapVtxs[vtxIndex + 5] = Vertex_Pos3fColour4ubNormal3fTex2f(v3, MAP_COLOUR, vN2, t3);

				vtxIndex += 6;
			}
			mapIndex++;
		}
	}

	m_pHeightMapBuffer = CreateImmutableVertexBuffer(Application::s_pApp->GetDevice(), sizeof Vertex_Pos3fColour4ubNormal3fTex2f * m_HeightMapVtxCount, m_pMapVtxs);

	for (size_t i = 0; i < NUM_TEXTURE_FILES; ++i)
	{
		LoadTextureFromFile(Application::s_pApp->GetDevice(), g_aTextureFileNames[i], &m_pTextures[i], &m_pTextureViews[i], &m_pSamplerState); //load all textures
	}

	ReloadShader(); // This compiles the shader
}

XMFLOAT3 HeightMap::GetAveragedVertexNormal(int index, int row)
{
	XMFLOAT3 ret;

	assert(index >= 0 && index < m_HeightMapVtxCount);

	int faceIndex = (index - row) * 2; // Map vertex to face

	XMVECTOR vAverage = XMLoadFloat3(GetFaceNormalPtr(faceIndex, 0)) +
		XMLoadFloat3(GetFaceNormalPtr(faceIndex, -1)) +
		XMLoadFloat3(GetFaceNormalPtr(faceIndex, -2)) +
		XMLoadFloat3(GetFaceNormalPtr(faceIndex, -m_FacesPerRow - 1)) +
		XMLoadFloat3(GetFaceNormalPtr(faceIndex, -m_FacesPerRow)) +
		XMLoadFloat3(GetFaceNormalPtr(faceIndex, -m_FacesPerRow + 1));

	vAverage /= 6;

	vAverage = XMVector3Normalize(vAverage);

	XMStoreFloat3(&ret, vAverage);

	return ret;
}

XMFLOAT3* HeightMap::GetFaceNormalPtr(int faceIndex, int offset)
{
	static XMFLOAT3 vUp(0.0f, 1.0f, 0.0f);

	if (faceIndex >= m_HeightMapFaceCount) // Last row of vertices will map off the end of the face list
		return &vUp;

	assert(faceIndex >= 0 && faceIndex < m_HeightMapFaceCount);

	int newIndex = faceIndex + offset;
	int oldRow = faceIndex / m_FacesPerRow;
	int newRow = newIndex / m_FacesPerRow;

	if (newIndex < 0 || oldRow != newRow || newIndex > m_HeightMapFaceCount)
		return &m_pFaceNormals[faceIndex];

	return &m_pFaceNormals[newIndex];
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

HeightMap::~HeightMap()
{
	Release(m_pHeightMapBuffer);

	DeleteShader();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMap::Draw(float frameCount)
{
	XMMATRIX worldMtx = XMMatrixIdentity();

	ID3D11DeviceContext* pContext = Application::s_pApp->GetDeviceContext();

	Application::s_pApp->SetWorldMatrix(worldMtx);

	// Update the data in our `MyApp' cbuffer.
	// The D3D11_MAP_WRITE_DISCARD flag is best for performance,
	// However, when mapping, the previous buffer contents are indeterminate. So the entire buffer
	// must be written.

	if (m_pMyAppCBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE map;
		if (SUCCEEDED(pContext->Map(m_pMyAppCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
		{
			// Set the buffer contents. There is only one variable to set in this case.
			// This method relies on the offset which has been found through Shader Reflection.
			SetCBufferFloat(map, m_frameCountOffset, frameCount);
			pContext->Unmap(m_pMyAppCBuffer, 0);
		}
	}

	// Bind the same constant buffer to any stages that use it.
	if (m_psMyAppCBufferSlot != -1)
	{
		pContext->PSSetConstantBuffers(m_psMyAppCBufferSlot, 1, &m_pMyAppCBuffer);
	}
	if (m_vsMyAppCBufferSlot != -1)
	{
		pContext->VSSetConstantBuffers(m_vsMyAppCBufferSlot, 1, &m_pMyAppCBuffer);
	}

	if (m_psTexture0 >= 0)
		pContext->PSSetShaderResources(m_psTexture0, 1, &m_pTextureViews[0]);

	if (m_psTexture1 >= 0)
		pContext->PSSetShaderResources(m_psTexture1, 1, &m_pTextureViews[1]);

	if (m_psTexture2 >= 0)
		pContext->PSSetShaderResources(m_psTexture2, 1, &m_pTextureViews[2]);

	if (m_vsMaterialMap >= 0)
		pContext->VSSetShaderResources(m_vsMaterialMap, 1, &m_pTextureViews[3]);

	m_pSamplerState = Application::s_pApp->GetSamplerState(true, true, true);

	Application::s_pApp->DrawWithShader(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_pHeightMapBuffer, sizeof(Vertex_Pos3fColour4ubNormal3fTex2f),
		NULL, 0, m_HeightMapVtxCount, NULL, m_pSamplerState, &m_shader);
}

bool HeightMap::ReloadShader(void)
{
	DeleteShader();

	ID3D11VertexShader* pVS = NULL;
	ID3D11PixelShader* pPS = NULL;
	ID3D11InputLayout* pIL = NULL;
	ShaderDescription vs, ps;

	ID3D11Device* pDevice = Application::s_pApp->GetDevice();

	// When the CommonApp draw functions set any of the light arrays,
	// they assume that the arrays are of CommonApp::MAX_NUM_LIGHTS
	// in size. Using a shader compiler #define is an easy way to
	// get this value to the shader.

	char maxNumLightsValue[100];
	_snprintf_s(maxNumLightsValue, sizeof maxNumLightsValue, _TRUNCATE, "%d", CommonApp::MAX_NUM_LIGHTS);

	D3D_SHADER_MACRO aMacros[] = {
		{
			"MAX_NUM_LIGHTS",
			maxNumLightsValue,
		},
		{ NULL },
	};

	if (!CompileShadersFromFile(pDevice, "./Resources/ExampleShader.hlsl", "VSMain", &pVS, &vs, g_aVertexDesc_Pos3fColour4ubNormal3fTex2f,
		g_vertexDescSize_Pos3fColour4ubNormal3fTex2f, &pIL, "PSMain", &pPS, &ps, aMacros))
	{
		return false; // false;
	}

	Application::s_pApp->CreateShaderFromCompiledShader(&m_shader, pVS, &vs, pIL, pPS, &ps);

	// Find cbuffer(s) for the globals that won't get set by the CommonApp
	// code. These are shader-specific; you have to know what you are
	// looking for, if you're going to set it...

	ps.FindCBuffer("MyApp", &m_psMyAppCBufferSlot);
	ps.FindFloat(m_psMyAppCBufferSlot, "g_frameCount", &m_frameCountOffset);

	ps.FindTexture("g_texture0", &m_psTexture0);
	ps.FindTexture("g_texture1", &m_psTexture1);
	ps.FindTexture("g_texture2", &m_psTexture2);

	// We have to find the constant buffer slot in the vertex shader too
	vs.FindCBuffer("MyApp", &m_vsMyAppCBufferSlot);
	vs.FindTexture("g_materialMap", &m_vsMaterialMap);

	// Create a cbuffer, using the shader description to find out how
	// large it needs to be.
	m_pMyAppCBuffer = CreateBuffer(pDevice, ps.GetCBufferSizeBytes(m_psMyAppCBufferSlot), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL);

	// In this example we are sharing the constant buffer between both vertex and pixel shaders.
	// This is efficient since we only update one buffer. However we could define separate constant buffers for each stage.
	// Generally constant buffers should represent groups of variables that must be updated at the same rate.
	// So : we might have 'per execution' 'per frame', 'per draw' constant buffers.

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMap::DeleteShader()
{
	Release(m_pMyAppCBuffer);

	m_shader.Reset();
}

//////////////////////////////////////////////////////////////////////
// LoadHeightMap
// Original code sourced from rastertek.com
//////////////////////////////////////////////////////////////////////
bool HeightMap::LoadHeightMap(char* filename, float gridSize)
{
	FILE* filePtr;
	int error;
	unsigned int count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize, i, j, k, index;
	unsigned char* bitmapImage;
	unsigned char height;

	// Open the height map file in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Save the dimensions of the terrain.
	m_HeightMapWidth = bitmapInfoHeader.biWidth;
	m_HeightMapLength = bitmapInfoHeader.biHeight;

	// Calculate the size of the bitmap image data.
	imageSize = m_HeightMapWidth * m_HeightMapLength * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if (!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if (count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	// Create the structure to hold the height map data.
	XMFLOAT3* pUnsmoothedMap = new XMFLOAT3[m_HeightMapWidth * m_HeightMapLength];
	m_pHeightMap = new XMFLOAT3[m_HeightMapWidth * m_HeightMapLength];

	if (!m_pHeightMap)
	{
		return false;
	}

	// Initialize the position in the image data buffer.
	k = 0;

	// Read the image data into the height map.
	for (j = 0; j < m_HeightMapLength; j++)
	{
		for (i = 0; i < m_HeightMapWidth; i++)
		{
			height = bitmapImage[k];

			index = (m_HeightMapWidth * j) + i;

			m_pHeightMap[index].x = (float)(i - (m_HeightMapWidth / 2)) * gridSize;
			m_pHeightMap[index].y = (float)height / 6 * gridSize;
			m_pHeightMap[index].z = (float)(j - (m_HeightMapLength / 2)) * gridSize;

			pUnsmoothedMap[index].y = (float)height / 6 * gridSize;

			k += 3;
		}
	}

	// Smoothing the landscape makes a big difference to the look of the shading
	for (int s = 0; s < 2; ++s)
	{
		for (j = 0; j < m_HeightMapLength; j++)
		{
			for (i = 0; i < m_HeightMapWidth; i++)
			{
				index = (m_HeightMapWidth * j) + i;

				if (j > 0 && j < m_HeightMapLength - 1 && i > 0 && i < m_HeightMapWidth - 1)
				{
					m_pHeightMap[index].y = 0.0f;
					m_pHeightMap[index].y += pUnsmoothedMap[index - m_HeightMapWidth - 1].y + pUnsmoothedMap[index - m_HeightMapWidth].y + pUnsmoothedMap[index - m_HeightMapWidth + 1].y;
					m_pHeightMap[index].y += pUnsmoothedMap[index - 1].y + pUnsmoothedMap[index].y + pUnsmoothedMap[index + 1].y;
					m_pHeightMap[index].y += pUnsmoothedMap[index + m_HeightMapWidth - 1].y + pUnsmoothedMap[index + m_HeightMapWidth].y + pUnsmoothedMap[index + m_HeightMapWidth + 1].y;
					m_pHeightMap[index].y /= 9;
				}
			}
		}

		for (j = 0; j < m_HeightMapLength; j++)
		{
			for (i = 0; i < m_HeightMapWidth; i++)
			{
				index = (m_HeightMapWidth * j) + i;
				pUnsmoothedMap[index].y = m_pHeightMap[index].y;
			}
		}
	}

	// Release the bitmap image data.
	delete[] bitmapImage;
	delete[] pUnsmoothedMap;
	bitmapImage = 0;

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
