#include "UBOManager.h"
#include <iostream>

UBOManager& UBOManager::getInstance() {
    static UBOManager instance;
    return instance;
}

void UBOManager::createUBO(const std::string& tag, size_t size, GLuint bindingPoint) {
    if (ubos.find(tag) != ubos.end()) {
        std::cerr << "UBO already exists: " << tag << std::endl;
        return;
    }

    GLuint ubo; 
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    ubos[tag] = { ubo, size, bindingPoint };
}

void UBOManager::updateUBOData(const std::string& tag, size_t offset, const void* data, size_t dataSize) {
    auto it = ubos.find(tag);
    if (it == ubos.end()) {
        std::cerr << "UBO not found: " << tag << std::endl;
        return;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, it->second.bufferID);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, dataSize, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBOManager::bindUBOToShader(const std::string& uboTag, GLuint shaderID, const std::string& uboName) {
    if (ubos.find(uboTag) == ubos.end()) {
        std::cerr << "UBO not found: " << uboTag << std::endl;
        return;
    }

    GLuint blockIndex = glGetUniformBlockIndex(shaderID, uboName.c_str());
    if (blockIndex == GL_INVALID_INDEX) {
        std::cerr << "UBO block not found in shader: " << uboName << std::endl;
        return;
    }

    glUniformBlockBinding(shaderID, blockIndex, ubos[uboTag].bindingPoint);
}

void UBOManager::clear() {
    for (auto& [tag, uboData] : ubos) {
        glDeleteBuffers(1, &uboData.bufferID);
    }
    ubos.clear();
}
