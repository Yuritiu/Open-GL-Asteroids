#pragma once
#include "../deps/include/glm/glm.hpp"
#include "../deps/include/GL/glew.h"
#include <string>
#include "obj_loader.h"
#include <iostream>
#include <vector>

struct Vertex
{
public:
	Vertex(const glm::vec3& pos, const glm::vec2& texCoord)
	{
		this->pos = pos;
		this->texCoord = texCoord;
		this->normal = glm::vec3(0.0f, 0.0f, 1.0f);
	}

	glm::vec3* GetPos() { return &pos; }
	glm::vec2* GetTexCoord() { return &texCoord; }
	glm::vec3* GetNormal() { return &normal; }

private:
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
};

class Mesh {
public:
	Mesh();
	~Mesh();

	void draw();
	void init(Vertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices);
	void loadModel(const std::string& filename);
	void initModel(const IndexedModel& model);

private:
	GLuint vao = 0;  // Vertex Array Object
	GLuint vbo = 0;  // Interleaved Vertex Buffer Object
	GLuint ebo = 0;  // Index Buffer Object
	unsigned int drawCount = 0; // Number of indices
};
