#pragma once
#include <unordered_map>
#include <string>
#include <GL/glew.h>

class UBOManager {
public:
    static UBOManager& getInstance(); // Singleton instance

    void createUBO(const std::string& tag, size_t size, GLuint bindingPoint);
    void updateUBOData(const std::string& tag, size_t offset, const void* data, size_t dataSize);
    void bindUBOToShader(const std::string& uboTag, GLuint shaderID, const std::string& uboName);

    void clear(); // Cleanup all UBOs

private:
    UBOManager() = default;
    ~UBOManager() = default;
    UBOManager(const UBOManager&) = delete;
    UBOManager& operator=(const UBOManager&) = delete;

    struct UBOData {
        GLuint bufferID;
        size_t bufferSize;
        GLuint bindingPoint;
    };

    std::unordered_map<std::string, UBOData> ubos;
};
