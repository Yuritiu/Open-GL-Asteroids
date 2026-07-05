#include "Mesh.h"
#include <iostream>

Mesh::Mesh() {
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}

void Mesh::init(Vertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices) {
    IndexedModel model;

    for (unsigned int i = 0; i < numVertices; i++) {
        model.positions.push_back(*vertices[i].GetPos());
        model.texCoords.push_back(*vertices[i].GetTexCoord());
        model.normals.push_back(*vertices[i].GetNormal());
    }

    for (unsigned int i = 0; i < numIndices; i++) {
        model.indices.push_back(indices[i]);
    }

    initModel(model);
}

void Mesh::initModel(const IndexedModel& model) {
    drawCount = model.indices.size();

    if (model.positions.empty() || model.texCoords.empty() || model.normals.empty()) {
        std::cerr << "ERROR: Model data is missing!" << std::endl;
        return;
    }

    // Create Buffers
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // Create Interleaved Data
    std::vector<float> interleavedData;
    for (size_t i = 0; i < model.positions.size(); i++) {
        interleavedData.push_back(model.positions[i].x);
        interleavedData.push_back(model.positions[i].y);
        interleavedData.push_back(model.positions[i].z);

        interleavedData.push_back(model.normals[i].x);
        interleavedData.push_back(model.normals[i].y);
        interleavedData.push_back(model.normals[i].z);

        interleavedData.push_back(model.texCoords[i].x);
        interleavedData.push_back(model.texCoords[i].y);
    }

    glBindVertexArray(vao);

    // Interleaved Data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, interleavedData.size() * sizeof(float), interleavedData.data(), GL_STATIC_DRAW);

    // Index Data to EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(unsigned int), model.indices.data(), GL_STATIC_DRAW);

    // DVertex Attributes
    size_t stride = sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2);

    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    glEnableVertexAttribArray(1); // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3)));

    glEnableVertexAttribArray(2); // Texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(glm::vec3) + sizeof(glm::vec3)));

    glBindVertexArray(0);
}

void Mesh::loadModel(const std::string& filename) {
    IndexedModel model = OBJModel(filename).ToIndexedModel();
    initModel(model);
} 

void Mesh::draw() {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, drawCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
