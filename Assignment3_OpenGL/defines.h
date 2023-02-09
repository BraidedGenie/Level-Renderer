#pragma once

#define GATEWARE_ENABLE_CORE // All libraries need this
#define GATEWARE_ENABLE_SYSTEM // Graphics libs require system level libraries
#define GATEWARE_ENABLE_GRAPHICS // Enables all Graphics Libraries
#define GATEWARE_DISABLE_GDIRECTX11SURFACE // we have another template for this
#define GATEWARE_DISABLE_GDIRECTX12SURFACE // we have another template for this
#define GATEWARE_DISABLE_GVULKANSURFACE // we have another template for this
#define GATEWARE_DISABLE_GRASTERSURFACE // we have another template for this
#define GATEWARE_ENABLE_MATH
#define GATEWARE_ENABLE_INPUT 

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "h2bParser.h"
#include "Gateware/Gateware.h"

using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;


struct VECTOR {
	float x, y, z;
};

struct VERTEX {
	VECTOR pos;
	VECTOR uvw;
	VECTOR nrm;
};

struct UBO {
	GW::MATH::GVECTORF viewPos;
	GW::MATH::GVECTORF lightColor, lightDir;
	GW::MATH::GMATRIXF world_Matrix, view_Matrix, proj_Matrix, viewproj_Matrix;
	H2B::ATTRIBUTES material;
};

std::vector<std::string> getChecks(std::string filename) {
	std::vector<std::string> hold;

	std::string term = "MESH";
	int loc = 0;

	std::string currLine;

	std::ifstream ifs(filename);
	while (getline(ifs, currLine))
	{
		size_t pos = currLine.find(term); // Find specfic type
		if (pos != std::string::npos)
		{
			getline(ifs, currLine);
			currLine.erase(std::remove(currLine.begin(), currLine.end(), ' '), currLine.end());

			hold.push_back(currLine);
		}
	}
	ifs.close();

	return hold;
}