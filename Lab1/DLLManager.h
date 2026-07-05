#pragma once

#include <windows.h>
#include <string>
#include <iostream>
#include <unordered_map>

class DLLManager {
public:
    // Singleton pattern
    static DLLManager& getInstance();

    // Load a DLL by filename
    bool loadDLL(const std::string& dllName);

    // Retrieve a function from a loaded DLL (Template must be in header)
    template<typename T>
    T getFunction(const std::string& dllName, const std::string& functionName) {
        auto it = loadedDLLs.find(dllName);
        if (it == loadedDLLs.end()) {
            std::cerr << "DLL not loaded: " << dllName << std::endl;
            return nullptr;
        }

        FARPROC func = GetProcAddress(it->second, functionName.c_str());
        if (!func) {
            std::cerr << "Failed to retrieve function: " << functionName << " from " << dllName << std::endl;
            return nullptr;
        }

        return reinterpret_cast<T>(func);
    }

    // Unload a specific DLL
    void unloadDLL(const std::string& dllName);

    // Unload all loaded DLLs
    void unloadAll();

private:
    DLLManager() = default;
    ~DLLManager();

    std::unordered_map<std::string, HINSTANCE> loadedDLLs;  // Stores loaded DLLs

    // Prevent copy/move
    DLLManager(const DLLManager&) = delete;
    DLLManager& operator=(const DLLManager&) = delete;
};
