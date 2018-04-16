#include "Robot.h"
#include "tinyxml2.h"
int i = 0;
Robot::Robot(float fX, float fY, float fZ, float fRotY)
{

	//sets the camera offset of robot. sets timer and lerp timer to 0 for the animations.
	m_vCamWorldPos = XMVectorZero();
	//starts at IdleAnim
	fileName = "Resources/RobotIdleAnim.dae";
	m_v4CamOff = XMFLOAT4(55.0f, 7.5f, 0.0f, 0.0f);
	timer = 0;
	lerpTimer = 0;
	//Gets the keyframe vector data from whatever file has been set 
	animControl = new AnimController(fileName);
	keyframes = animControl->GetKeyframeData();

	//populates the component array with instantiated components for every part of the robot
	componentArray[0] = new Component(nullptr, XMFLOAT4(fX, fY, fZ, 0.0F), XMFLOAT4(0, fRotY, 0, 0)); //root
	componentArray[1] = new Component("Resources/Robot/pelvis.x", XMFLOAT4(-0.250011f / 10, 15.25F / 10, -0.000005F / 10, 0.0F), XMFLOAT4(0, 0, 0, 0)); //PELVIS
	componentArray[2] = new Component("Resources/Robot/body.x", XMFLOAT4(0.500099F / 10, 43.749992F / 10, 0.00003F / 10, 0.0F), XMFLOAT4(0, 0, 0, 0)); //body
	componentArray[3] = new Component("Resources/Robot/left_shoulder.x", XMFLOAT4(46.0F / 10, 0.0F / 10, -0.009992F / 10, 0.0F), XMFLOAT4(0, 0, 0, 0)); //left shoulder
	componentArray[4] = new Component("Resources/Robot/left_elbow.x", XMFLOAT4(34.250019f / 10, -0.499817f / 10, -0.004262f / 10, 0.0F), XMFLOAT4(0, 0, 0, 0)); //left elbow
	componentArray[5] = new Component("Resources/Robot/left_wrist.x", XMFLOAT4(55.250008f / 10, -0.999710f / 10, 0.003968f / 10, 0.0F), XMFLOAT4(0, 0, 0, 0)); //left wrist
	componentArray[6] = new Component("Resources/Robot/right_shoulder.x", XMFLOAT4 (-44.500023f / 10, 0.500000f / 10, -0.000021f / 10, 0.0f), XMFLOAT4(0, 0, 0, 0)); //right shoulder
	componentArray[7] = new Component("Resources/Robot/right_elbow.x", XMFLOAT4(-33.999996f / 10, 0.250229f / 10, -0.000194f / 10, 0.0f), XMFLOAT4(0, 0, 0, 0)); //right elbow
	componentArray[8] = new Component("Resources/Robot/right_wrist.x", XMFLOAT4(-60.000381f / 10, -1.750183f / 10, 0.007156f / 10, 0.0f), XMFLOAT4(0, 0, 0, 0)); //right wrist
	componentArray[9] = new Component("Resources/Robot/neck.x", XMFLOAT4(0.249983f / 10, 36.625015f / 10, 25.999998f / 10, 0.0f), XMFLOAT4(0, 0, 0, 0)); //neck
	componentArray[10] = new Component("Resources/Robot/left_hip.x", XMFLOAT4(19.500000f / 10, -7.724991f / 10, 0.000000f / 10, 0.0f), XMFLOAT4(0, 0, 0, 0)); //left_hip
	componentArray[11] = new Component("Resources/Robot/left_knee.x", XMFLOAT4(0.000006f / 10, -22.200001f / 10, 0.000000f / 10, 0.0f), XMFLOAT4(0, 0, 0, 0)); //left_knee
	componentArray[12] = new Component("Resources/Robot/left_ankle.x", XMFLOAT4(-0.800152f / 10, -36.399994f / 10, -0.000098f / 10, 0.0f), XMFLOAT4(0, 0, 0, 0)); //left_ankle
	componentArray[13] = new Component("Resources/Robot/right_hip.x", XMFLOAT4(-19.500000f / 10, -7.724991f / 10, 0.000000f / 10, 0.0f), XMFLOAT4(0, 0, 0, 0)); //right hip
	componentArray[14] = new Component("Resources/Robot/right_knee.x", XMFLOAT4(0.000006f / 10, -22.000000f / 10, 0.000000f / 10, 0.0f), XMFLOAT4(0, 0, 0, 0)); //right knee
	componentArray[15] = new Component("Resources/Robot/right_ankle.x", XMFLOAT4(0.199911f / 10, -36.799995f / 10, 0.000039f / 10, 0.0f), XMFLOAT4(0, 0, 0, 0)); //right ankle

	//childs the appropriate components by pushing them back into the parents children vector
	componentArray[0]->ObjectChildren.push_back(componentArray[1]); //pelvis child of root
	componentArray[0]->ObjectChildren.push_back(componentArray[10]); //left hip child of root
	componentArray[0]->ObjectChildren.push_back(componentArray[13]); //right hip child of root
	componentArray[1]->ObjectChildren.push_back(componentArray[2]); //body child of pelvis
	componentArray[2]->ObjectChildren.push_back(componentArray[3]); //left shoulder child of body
	componentArray[2]->ObjectChildren.push_back(componentArray[6]); //right shoulder child of body
	componentArray[3]->ObjectChildren.push_back(componentArray[4]); //left elbow child of left shoulder
	componentArray[4]->ObjectChildren.push_back(componentArray[5]); //left wrist child of left elbow
	componentArray[6]->ObjectChildren.push_back(componentArray[7]); //right elbow child of right shoulder
	componentArray[7]->ObjectChildren.push_back(componentArray[8]); //right wrist child of right shoulder
	componentArray[2]->ObjectChildren.push_back(componentArray[9]); //neck child of body
	componentArray[10]->ObjectChildren.push_back(componentArray[11]); //left knee child of left hip
	componentArray[11]->ObjectChildren.push_back(componentArray[12]); //left ankle child of left knee
	componentArray[13]->ObjectChildren.push_back(componentArray[14]); //right knee child of right hip
	componentArray[14]->ObjectChildren.push_back(componentArray[15]); //right ankle child of right knee
	
	//sets rootComponent as the robot's root object
	rootComponent = componentArray[0];

	//recursively loads all the components meshes
	rootComponent->LoadResources();
	//plays whatever animation has been passed into fileName
	PlayAnimation(fileName);

}

Robot::~Robot(void)
{
}



void Robot::ReleaseResources(void)
{
	//recursively calls ReleaseResources()
	rootComponent->ReleaseResources();
}

void Robot::Update()
{	
	//increases timer by 1/60
	timer += 0.016f;
	//if timer is past the 1st keyframe time, play animation
	if (timer >= (20 / keyframes.size()))
	{
		lerpTimer = 0.f;
		PlayAnimation(fileName);
		timer = 0.f;
	}
	//call update on each component, telling it is not part of aeroplane to apply the correct order of matrices
	rootComponent->Update(nullptr, false);
	UpdateMatrices();
}

void Robot::Draw(void)
{
	//recursively draws all the components	
	rootComponent->Draw();
}

void Robot::HandleAnimation(char* fileName)
{
	

}

void Robot::SetAnimation(char* newAnim)
{
	//sets the animation to whatever dae has been passed in the function and refreshers the keyframes vector to match that animation
	fileName = newAnim;
	animControl = new AnimController(fileName);
	keyframes = animControl->GetKeyframeData();
	
}

void Robot::PlayAnimation(char * fileName)
{
	//if at the last keyframe of the anim
	if (i >= keyframes.size() - 1)
	{
		//reset the keyframe iterator
		i = 0;
	}
	//for every component at keyframe[i]
	//Get Vectors of the components keyframe position and NEXT keyframe position
	for (size_t j = 0; j < keyframes[i].size(); j++)
	{
		XMVECTOR v1, v2;
		v1 = XMLoadFloat4(&keyframes[i].at(j).pos);
		v2 = XMLoadFloat4(&keyframes[i + 1].at(j).pos);

		//Get Vectors of the components keyframe rotation and NEXT keyframe rotation
		XMVECTOR r1, r2;
		r1 = XMLoadFloat4(&keyframes[i].at(j).rot);
		r2 = XMLoadFloat4(&keyframes[i + 1].at(j).rot);

		//set the components position to a lerped value using the vectors I pulled from the keyframes
		XMStoreFloat4(&componentArray[j]->m_v4Pos, XMVectorLerp(v1, v2, lerpTimer));
		//set the components rotation to a lerped value using the vectors I pulled from the keyframes
		XMStoreFloat4(&componentArray[j]->m_v4Rot, XMVectorLerp(r1, r2, lerpTimer));
		//increase lerp timer to smooth the transform between the two keyframes
		lerpTimer += 0.24f;
	}
	//increase the keyframe iterator
	i++;
	//reset lerpTimer for the next keyframe loop
	lerpTimer = 0.f;

}

//attempted lerp function before i realised XMMath had one 
float lerp(float time, float start, float end) {
	return(1 - time)*start + time*end;
}

void Robot::UpdateMatrices(void)
{
	//Sets the robots camPos based off the offset
	XMVECTOR vCamPos = XMLoadFloat4(&m_v4CamOff);
	//applys the offset and the robots root pos to the camWorldMatrix
	m_mCamWorldMatrix = XMMatrixTranslationFromVector(vCamPos) * XMMatrixTranslationFromVector(XMLoadFloat4(&rootComponent->m_v4Pos));
	//sets the cameras world position to be equal to the camera's world matrix pos value
	m_vCamWorldPos = m_mCamWorldMatrix.r[3];
}


