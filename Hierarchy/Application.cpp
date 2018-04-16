#include "Application.h"
#include "Heightmap.h"
#include "Aeroplane.h"
#include "Robot.h"
#include "AnimController.h"

Application* Application::s_pApp = NULL;

const int CAMERA_MAP = 0;
const int CAMERA_PLANE = 1;
const int CAMERA_GUN = 2;
const int CAMERA_MAX = 5;
const int CAMERA_ROBOT = 3;
const int CAMERA_BOMB = 4;
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool Application::HandleStart()
{
	m_frameCount = 0.0f;
	s_pApp = this;
	
	this->SetWindowTitle("Hierarchy");

	m_bWireframe = false;

	//instansiates heighMap, aeroPlane and Robot
	m_pHeightMap = new HeightMap("Resources/heightmap.bmp", 2.0f);
	m_pAeroplane = new Aeroplane(0.0f, 3.5f, 0.0f, 105.0f);
	m_pRobot = new Robot(0.0f, 9.5f, -15.0f, 90.0f);

	m_cameraZ = 50.0f;
	m_rotationAngle = 0.f;
	
	//starts the shader call
	ReloadShaders();

	if(!this->CommonApp::HandleStart())
		return false;

	this->SetRasterizerState(false, m_bWireframe);

	//sets starting camera to map
	m_cameraState = CAMERA_MAP;

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleStop()
{
	//on App stop, delete allocated memory and release meshes
	delete m_pHeightMap;
	m_pAeroplane->ReleaseResources();
	
	delete m_pAeroplane;
	m_pRobot->ReleaseResources();
	delete m_pRobot;

	this->CommonApp::HandleStop();
}

void Application::ReloadShaders()
{
	//calls the shader function on heightMap to apply the textures
	if (m_pHeightMap->ReloadShader() == false)
		this->SetWindowTitle("Reload Failed - see Visual Studio output window. Press F5 to try again.");
	else
		this->SetWindowTitle("Your Shader Here. Press F5 to reload shader file.");
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleUpdate()
{
	m_rotationAngle += .01f;

	if(m_cameraState == CAMERA_MAP)
	{
		if(this->IsKeyPressed('Q'))
			m_cameraZ -= 2.0f;

		if(this->IsKeyPressed('A'))
			m_cameraZ += 2.0f;
	}

	static bool dbC = false;

	if(this->IsKeyPressed('C'))
	{
		if(!dbC)
		{
			if(++m_cameraState == CAMERA_MAX)				
				m_cameraState = CAMERA_MAP;

			dbC = true;
		}
	}
	else
	{
		dbC = false;
	}

	static bool dbW = false;
	if(this->IsKeyPressed('W'))
	{
		if(!dbW)
		{
			m_bWireframe = !m_bWireframe;
			this->SetRasterizerState(false, m_bWireframe);
			dbW = true;
		}
	}
	else
	{
		dbW = false;
	}

	if (this->IsKeyPressed('1'))
	{
		//if 1 is pressed, set the animation of robot to Idle
		m_pRobot->SetAnimation("Resources/RobotIdleAnim.dae");
	}
	if (this->IsKeyPressed('2'))
	{
		//if 2 is pressed, set the animation of robot to Attack
		m_pRobot->SetAnimation("Resources/RobotAttackAnim.dae");
	}

	//update the instantiated objects
	m_pAeroplane->Update(m_cameraState != CAMERA_MAP);
	m_pRobot->Update();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Application::HandleRender()
{
	XMFLOAT3 vUpVector(0.0f, 1.0f, 0.0f);
	XMFLOAT3 vCamera, vLookat;
	if (m_pAeroplane->bombCount[0] != nullptr)
	{
		m_cameraState = CAMERA_BOMB;
	}
	switch(m_cameraState)
	{
		case CAMERA_MAP:
			vCamera = XMFLOAT3(sin(m_rotationAngle) * m_cameraZ, m_cameraZ / 4, cos(m_rotationAngle) * m_cameraZ);
			vLookat = XMFLOAT3(0.0f, 4.0f, 0.0f);
			break;
		case CAMERA_PLANE:
			m_pAeroplane->SetGunCamera(false);		
			vCamera = XMFLOAT3(m_pAeroplane->GetCameraPosition().x, m_pAeroplane->GetCameraPosition().y, m_pAeroplane->GetCameraPosition().z);
			vLookat = XMFLOAT3(m_pAeroplane->GetFocusPosition().x, m_pAeroplane->GetFocusPosition().y, m_pAeroplane->GetFocusPosition().z);
			break;
		case CAMERA_GUN:
			m_pAeroplane->SetGunCamera(true);			
			vCamera = XMFLOAT3(m_pAeroplane->GetCameraPosition().x, m_pAeroplane->GetCameraPosition().y, m_pAeroplane->GetCameraPosition().z);
			vLookat = XMFLOAT3(m_pAeroplane->GetFocusPosition().x, m_pAeroplane->GetFocusPosition().y, m_pAeroplane->GetFocusPosition().z);
			break;
		case CAMERA_ROBOT:
			vCamera = XMFLOAT3(m_pRobot->GetCameraPosition().x, m_pRobot->GetCameraPosition().y, m_pRobot->GetCameraPosition().z);
			vLookat = XMFLOAT3(m_pRobot->GetFocusPosition().x, m_pRobot->GetFocusPosition().y, m_pRobot->GetFocusPosition().z);
			break;
		case CAMERA_BOMB:
			if (m_pAeroplane->bombCount[0] != nullptr)
			{
				//if there is a bomb, set the camera start position to plane's position and the camera's focus to the bomb's position
				vCamera = XMFLOAT3(m_pAeroplane->GetCameraPosition().x, m_pAeroplane->GetCameraPosition().y, m_pAeroplane->GetCameraPosition().z);
				vLookat = XMFLOAT3(m_pAeroplane->bombCount[0]->GetFocusPosition().x, m_pAeroplane->bombCount[0]->GetFocusPosition().y, m_pAeroplane->bombCount[0]->GetFocusPosition().z);
			}
			else
			{
				//if no bomb, set camera to look at plane
				vCamera = XMFLOAT3(m_pAeroplane->GetCameraPosition().x, m_pAeroplane->GetCameraPosition().y, m_pAeroplane->GetCameraPosition().z);
				vLookat = XMFLOAT3(m_pAeroplane->GetFocusPosition().x, m_pAeroplane->GetFocusPosition().y, m_pAeroplane->GetFocusPosition().z);
			}
			break;
	}

	XMMATRIX matView;
	matView = XMMatrixLookAtLH(XMLoadFloat3(&vCamera), XMLoadFloat3(&vLookat), XMLoadFloat3(&vUpVector));

	XMMATRIX matProj;
	matProj = XMMatrixPerspectiveFovLH(float(XM_PI / 4), 2, 1.5f, 5000.0f);

	this->SetViewMatrix(matView);
	this->SetProjectionMatrix(matProj);

	this->EnablePointLight(0, XMFLOAT3(100.0f, 100.f, -100.f), XMFLOAT3(1.f, 1.f, 1.f));
	this->SetLightAttenuation(0, 200.f, 2.f, 2.f, 2.f);
	this->EnableDirectionalLight(1, XMFLOAT3(-1.f, -1.f, -1.f), XMFLOAT3(0.55f, 0.55f, 0.65f));

	this->Clear(XMFLOAT4(.2f, .2f, .6f, 1.f));

	XMMATRIX matWorld;
	matWorld = XMMatrixIdentity();
	this->SetWorldMatrix(matWorld);

	//draw objects
	m_pHeightMap->Draw(m_frameCount);
	m_frameCount++;
	m_pAeroplane->Draw();
	m_pRobot->Draw();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	Application application;

	Run(&application);

	return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
