#pragma once
#include "model.h"

class Level {

public:
	GLuint vertexArray = 0;
	GLuint vertexBufferObject = 0;
	GLuint indexBufferObject = 0;

	GLuint uniformBufferObject = 0;

	UBO ubo;

	std::vector<VERTEX> allVert;
	std::vector<unsigned> allIndicies;
	std::vector<Model> toyBox;

	GLuint& getVertexArray() //Do not initilaze directly
	{
		return vertexArray;
	}
	GLuint& getVertexBufferObject() 
	{
		return vertexBufferObject;
	}
	GLuint& getIndexBufferObject() 
	{
		return indexBufferObject;
	}
	GLuint& getUniformBufferObject() 
	{
		return uniformBufferObject;
	}

	// Mutator
	void addVertices(std::vector<VERTEX> vertices) // Add Vertex
	{
		for (auto & curr : vertices)
		{
			allVert.push_back(curr);
		}
	}
	void addIndices(std::vector<unsigned> indices) // Add Index
	{
		int offset = allVert.size();
		for (auto& curr : indices)
		{
			allIndicies.push_back(curr + offset);
		}
	}
	int getMaterialOffset() {
		return allIndicies.size();
	}
	void addModel(Model noob) // Add a Model
	{
		toyBox.push_back(noob);
	}
};