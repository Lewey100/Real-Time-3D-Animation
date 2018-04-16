#include "AnimController.h"
using namespace std;

AnimController::AnimController(char* fileName)
{
	//gets the animations dae file name and calls the function to load the XML oc
	mFileName = fileName;
	LoadXMLFile();

}

AnimController::~AnimController()
{
}

void AnimController::LoadXMLFile(void)
{
	//loads the dae file by name, sets the root to the first useful point of data <library-animations>
	//calls a recursive function to parse the data
	//calls a function to process the parsed data into useful position and rotation data
	tinyxml2::XMLDocument doc;
	doc.LoadFile(mFileName);

	XMLHandle docHandle(&doc);

	XMLNode* root = doc.FirstChild()->NextSibling()->FirstChild()->NextSibling()->FirstChild();
	GetValues(root, 0);

	GetTransformData(val);
}

void AnimController::GetValues(XMLNode* node, int level)
{
	//loops through the nodes, getting each animation node and putting that into the current frameValue struct
	//sets the structs input and output arrays by calling GetFloatVals which turns the char value of the node into a float
	//pushes the completed struct into the frameValues vector
	//increments the level that the function starts at and recursively calls itself.
	XMLNode *cur_node = NULL;
	++level; //one level deeper in next call
	for (cur_node = node; cur_node; cur_node = cur_node->NextSibling()) {
		if (std::string(cur_node->Value()) == "animation") {
			string elementID = (cur_node->ToElement()->Attribute("id"));
			currentVal.componentAnim = elementID;
			currentVal.inputArray = GetFloatVals(cur_node->FirstChild()->FirstChild()->ToElement()->GetText());
			currentVal.outputArray = GetFloatVals(cur_node->FirstChild()->NextSibling()->FirstChild()->ToElement()->GetText());
			val.push_back(currentVal);
			dprintf(" %s\n", cur_node->ToElement()->Attribute("id"));
		}

		++level;
		GetValues(cur_node->FirstChild(), level);
	}


}

std::vector<std::vector<transformData>> AnimController::GetKeyframeData()
{
	//returns the keyFrame vector which contains keyframes which in turn contain the keyframes transform data for each object
	return trans2;
}

XMFLOAT4 AnimController::PairPosValues(std::vector<frameValues> val, int i, int j, int iterator)
{
	//sets xyz to the corresponding keyframes xyz and divides by 10 to stop the seperate components from being disconnected
	//iterator is what keyframe it is
	XMFLOAT4 xyz = XMFLOAT4(0, 0, 0, 0);

	xyz.x = val[i].outputArray[(iterator * 3)] / 10;
	xyz.y = val[i].outputArray[(iterator * 3) + 1] / 10;
	xyz.z = val[i].outputArray[(iterator * 3) + 2] / 10;


	return xyz;
}

XMFLOAT4 AnimController::PairRotValues(std::vector<frameValues> val, int i, int j, int iterator)
{
	//sets xyz to the corresponding keyframes xyz by getting next outputs value because rotation is set up like RotateX(0,0,0)
	//Runs down the X Y Z and gets the corresponding keyframe
	//	| Keyframe(1,2,3)
	//	| RotateX(0,0,0)
	//	| RotateY(0,0,0)
	//	V RotateZ(0,0,0)
	//iterator is what keyframe it is
	XMFLOAT4 xyz = XMFLOAT4(0, 0, 0, 0);
	xyz.x = val[i + 1].outputArray[j + iterator];
	xyz.y = val[i + 2].outputArray[j + iterator];
	xyz.z = val[i + 3].outputArray[j + iterator];
	return xyz;
}

void AnimController::GetTransformData(std::vector<frameValues> val)
{
	int iterator = 0;
	int j = 0;
	int maxSize = 0;
	//loops through each inputArray and sets them all to be of the biggest size so when I get the keyframe data I don't trigger an access violation
	for (size_t i = 0; i <= val[i].inputArray.size(); i+=2)
	{
		if (val[i].inputArray.size() > maxSize)
		{
			maxSize = val[i].inputArray.size();
		}
	}
	//increases the size of the arrays if they arent = to the biggest array size. increases output 3 times due to translate output holding xyz for every keyframe.
	for (size_t i = 0; i < val.size(); i++)
	{
		int killMe = 0; //this is where I started losing hope
		while (val[i].inputArray.size() < maxSize)
		{
			val[i].inputArray.push_back(val[i].inputArray[killMe]);
			val[i].outputArray.push_back(val[i].outputArray[killMe]);
			val[i].outputArray.push_back(val[i].outputArray[killMe +1]);
			val[i].outputArray.push_back(val[i].outputArray[killMe +2]);
		}
	}
	for (size_t s = 0; s < val[s].inputArray.size(); s++) //runs through every keyframe
	{
		
		for (size_t i = 0; i < val.size(); i += 4) //for how many components there are loops through
		{

			if (val[i].componentAnim.find("translate") != std::string::npos) //if its a translation
			{
				currentTrans.pos = PairPosValues(val, i, j, s); //set transform position for keyframe
			}
			if (val[i + 1].componentAnim.find("rotate") != std::string::npos) //if the component is a rotation
			{
				currentTrans.rot = PairRotValues(val, i, j, s); //set transform rotation for that keyframe
			}
			iterator++; //increase iterator for each keyframe
			trans.push_back(currentTrans); //pushes this component transformData into the vector that contains all the data for the current keyframe
		}
		trans2.push_back(trans); //pushes the vector of all components keyframe data into the vector that contrains all the keyframes
		trans.clear();	//emptys the transformData vector to be used again in the next iteration of the loop

	}
}
		

std::vector <float> AnimController::GetFloatVals(const char* rawData)
{
	//Gets the raw data of the dae file, cuts out the whitespace, converts the strings of each value to float and pushes it back into a vector
	string s = rawData;
	string buf;
	stringstream iss(s);
	std::vector <float> floatVals;

	vector<string> tokens;

	while (iss >> buf)
	{
		tokens.push_back(buf);
	}

	for (size_t i = 0; i < tokens.size(); i++)
	{
		floatVals.push_back((float)atof(tokens[i].c_str()));
	}

	return floatVals;
}






