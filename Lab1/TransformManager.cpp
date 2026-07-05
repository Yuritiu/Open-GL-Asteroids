#include "TransformManager.h"
#include <iostream>

TransformManager& TransformManager::getInstance() {
    static TransformManager instance;
    return instance;
}

void TransformManager::addTransform(const std::string& tag, const Transform& transform) {
    if (transforms.find(tag) != transforms.end()) {
        std::cerr << "Transform already exists for tag: " << tag << std::endl;
        return;
    }
    transforms[tag] = transform;
}

Transform& TransformManager::getTransform(const std::string& tag) {
    if (transforms.find(tag) == transforms.end()) {
        std::cerr << "Error: Transform not found for tag: " << tag << std::endl;
        throw std::runtime_error("Transform not found");
    }
    return transforms[tag];
}

bool TransformManager::hasTransform(const std::string& tag) {
    return transforms.find(tag) != transforms.end();
}

void TransformManager::updateTransform(const std::string& tag, const Transform& transform) {
    if (transforms.find(tag) == transforms.end()) {
        std::cerr << "Warning: Trying to update a non-existent transform: " << tag << std::endl;
        return;
    }
    transforms[tag] = transform;
}

void TransformManager::removeTransform(const std::string& tag) {
    if (transforms.erase(tag) == 0) {
        std::cerr << "Warning: Attempted to remove non-existent transform: " << tag << std::endl;
    }
}

void TransformManager::clear() {
    transforms.clear();
}
