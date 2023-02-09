#pragma once
#include "defines.h"

class Model {
	std::string name, type, filename;
	float matrix[4][4];

public:
	H2B::Parser source;
	GW::MATH::GMATRIXF model = GW::MATH::GIdentityMatrixF;
	int matOffset;

	Model(std::string _filename, std::string _type) 
	{
		filename = _filename;
		type = _type;

		std::string currLine;

		std::ifstream ifs(filename);
		while (getline(ifs, currLine))
		{
			size_t pos = currLine.find(type); // Find specfic type
			if (pos != std::string::npos) 
			{
				// Getting the name of Object
				//getline(ifs, currLine);
				currLine.erase(std::remove(currLine.begin(), currLine.end(), ' '), currLine.end());
				currLine = currLine.substr(0, currLine.find('.'));

				// Parse info from h2b file with name
				name = currLine;
				filename = currLine.append(".h2b");
				filename = filename.insert(0, "Assets/");
				source.Parse(filename.c_str());
				
#pragma region BuildingMatrix
				// Get the Matrix
				for (size_t i = 0; i < 4; i++)
				{
					getline(ifs, currLine);
					stringToFloat(currLine, matrix[i]);
				}

				model.row1.x = matrix[0][0];
					model.row1.data[0] = model.row1.x;
				model.row1.y = matrix[0][1];
					model.row1.data[1] = model.row1.y;
				model.row1.z = matrix[0][2];
					model.row1.data[2] = model.row1.z;
				model.row1.w = matrix[0][3];
					model.row1.data[3] = model.row1.w;

				model.row2.x = matrix[1][0];
					model.row2.data[0] = model.row2.x;
				model.row2.y = matrix[1][1];
					model.row2.data[1] = model.row2.y;
				model.row2.z = matrix[1][2];
					model.row2.data[2] = model.row2.w;
				model.row2.w = matrix[1][3];
					model.row2.data[3] = model.row2.z;

				model.row3.x = matrix[2][0];
					model.row3.data[0] = model.row3.x;
				model.row3.y = matrix[2][1];
					model.row3.data[1] = model.row3.y;
				model.row3.z = matrix[2][2];
					model.row3.data[2] = model.row3.z;
				model.row3.w = matrix[2][3];
					model.row3.data[3] = model.row3.w;

				model.row4.x = matrix[3][0];
					model.row4.data[0] = model.row4.x;
				model.row4.y = matrix[3][1];
					model.row4.data[1] = model.row4.y;
				model.row4.z = matrix[3][2];
					model.row4.data[2] = model.row4.z;
				model.row4.w = matrix[3][3];
					model.row4.data[3] = model.row4.w;

				model.data[0] = model.row1.data[0];
				model.data[1] = model.row1.data[1];
				model.data[2] = model.row1.data[2];
				model.data[3] = model.row1.data[3];
				model.data[4] = model.row2.data[0];
				model.data[5] = model.row2.data[1];
				model.data[6] = model.row2.data[2];
				model.data[7] = model.row2.data[3];
				model.data[8] = model.row3.data[0];
				model.data[9] = model.row3.data[1];
				model.data[10] = model.row3.data[2];
				model.data[11] = model.row3.data[3];
				model.data[12] = model.row4.data[0];
				model.data[13] = model.row4.data[1];
				model.data[14] = model.row4.data[2];
				model.data[15] = model.row4.data[3];

#pragma endregion
			}
		}
		ifs.close();
	}
	std::vector<VERTEX> BuildVertices() {
		std::vector<VERTEX> verts;
		int vertexCount = source.vertexCount;

		for (size_t i = 0; i < vertexCount; i++)
		{
			VERTEX curr;

			// Pos
			curr.pos.x = source.vertices[i].pos.x;
			curr.pos.y = source.vertices[i].pos.y;
			curr.pos.z = source.vertices[i].pos.z;

			// UV
			curr.uvw.x = source.vertices[i].uvw.x;
			curr.uvw.y = source.vertices[i].uvw.y;
			curr.uvw.z = source.vertices[i].uvw.z;

			// Norms
			curr.nrm.x = source.vertices[i].nrm.x;
			curr.nrm.y = source.vertices[i].nrm.y;
			curr.nrm.z = source.vertices[i].nrm.z;

			verts.push_back(curr);
		}

		return verts;
	}

	std::vector<unsigned> BuildIndicies() {
		std::vector<unsigned> indices;
		int indexCount = source.indexCount;

		for (size_t i = 0; i < indexCount; i++)
		{
			indices.push_back(source.indices[i]);
		}

		return indices;
	}

	void UpdateBatch() {
		for (auto& i : source.meshes)
		{
			i.drawInfo.indexOffset += matOffset;
		}
	}

private:
	std::string Isolate(std::string str) {

		size_t pos = str.find('(');
		str.erase(0, pos + 1);

		str.erase(std::remove(str.begin(), str.end(), ','), str.end());
		str.erase(std::remove(str.begin(), str.end(), ')'), str.end());
		str.erase(std::remove(str.begin(), str.end(), ' '), str.end());

		return str;
	}
	void stringToFloat(std::string line, float arr[4]) {

		float temp[4];

		line = Isolate(line);

		for (size_t i = 0; i < 4; i++)
		{
			if (line.front() == '-') {
				temp[i] = std::stof(line.substr(0, 7));
				line.erase(0, 7);
			}
			else {
				temp[i] = std::stof(line.substr(0, 6));
				line.erase(0, 6);
			}
			arr[i] = temp[i];
		}

	}
};