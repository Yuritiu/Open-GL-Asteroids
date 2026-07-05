#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include "Shader.h"


class ShaderManager {
public:
    static ShaderManager& getInstance(); // Singleton instance

    std::shared_ptr<Shader> loadShader(const std::string& tag, const std::string& vertFile, const std::string& fragFile);
    std::shared_ptr<Shader> getShader(const std::string& tag);

    void clear(); // Cleanup shaders

private:
    ShaderManager() = default;  // Private constructor (Singleton)
    ~ShaderManager() = default;
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
};
 