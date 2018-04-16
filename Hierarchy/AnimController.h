#pragma once
#include "tinyxml2.h"
#include "Application.h"
#include <string>
#include <vector>
#include <sstream>
using namespace tinyxml2;


struct frameValues {
	//holds animation component raw data, inputArray is keyframe timings and output is the translation/rotation at that keyframe
	std::string componentAnim;
	std::vector<float> inputArray;
	std::vector<float> outputArray;
};

struct transformData {
	//holds the transform data, positions and rotations correctly aligned
	XMFLOAT4 pos;
	XMFLOAT4 rot;
};

__declspec(align(16)) class AnimController
{
public:
	AnimController(char* fileName);
	~AnimController();
	void LoadXMLFile(void);
	std::vector <float> GetFloatVals(const char* rawData);
	void GetValues(XMLNode* node, int level);
	XMFLOAT4 PairPosValues(std::vector<frameValues> val, int i, int j, int iterator);
	XMFLOAT4 PairRotValues(std::vector<frameValues> val, int i, int j, int iterator);
	void GetTransformData(std::vector<frameValues> val);
	std::vector<std::vector<transformData>> GetKeyframeData();

private:
	char* mFileName;
	std::vector<frameValues> val; //vector of frameValues to hold all of the raw data
	frameValues currentVal;		//used to push back data into the frameValues vector
	std::vector<transformData> trans;	//vector of transformData to populate each component
	std::vector<std::vector<transformData>> trans2; //vector of transformData to hold the components data at each keyframe

	transformData currentTrans; //used to push back data into the transformData vector
};