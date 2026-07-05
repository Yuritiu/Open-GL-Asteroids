#include "DLLManager.h"

// Get Singleton Instance
DLLManager& DLLManager::getInstance() {
    static DLLManager instance;
    return instance;
}

// Load a DLL
bool DLLManager::loadDLL(const std::string& dllName) {
    if (loadedDLLs.find(dllName) != loadedDLLs.end()) {
        std::cout << "DLL already loaded: " << dllName << std::endl;
        return true;
    }

    HINSTANCE hDLL = LoadLibrary(dllName.c_str());
    if (!hDLL) {
        std::cerr << "Failed to load DLL: " << dllName << std::endl;
        return false;
    }

    loadedDLLs[dllName] = hDLL;
    std::cout << "Successfully loaded DLL: " << dllName << std::endl;
    return true;
}

// Unload a specific DLL
void DLLManager::unloadDLL(const std::string& dllName) {
    auto it = loadedDLLs.find(dllName);
    if (it != loadedDLLs.end()) {
        FreeLibrary(it->second);
        loadedDLLs.erase(it);
        std::cout << "Unloaded DLL: " << dllName << std::endl;
    }
    else {
        std::cerr << "DLL not found: " << dllName << std::endl;
    }
}

// Unload all DLLs on destruction
DLLManager::~DLLManager() {
    unloadAll();
}

// Unload all DLLs
void DLLManager::unloadAll() {
    for (auto& [dllName, hDLL] : loadedDLLs) {
        FreeLibrary(hDLL);
        std::cout << "Unloaded DLL: " << dllName << std::endl;
    }
    loadedDLLs.clear();
}
