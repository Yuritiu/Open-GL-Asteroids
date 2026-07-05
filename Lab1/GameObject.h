#pragma once

#include "Mesh.h"
#include "Transform.h"
#include "Shader.h"
#include <string>

enum class GameObjectType
{
    Unknown,
    Player,
    Asteroid,
    Bullet
};

struct GameObject {
    int asteroidLevel = 0;
    float collisionRadius = 1.0f;
    bool isActive = true;
    
    glm::vec3 aabbMin = glm::vec3(0.0f);
    glm::vec3 aabbMax = glm::vec3(0.0f);

    GameObjectType type = GameObjectType::Unknown;

    Mesh* mesh = nullptr;            // Pointer to a shared mesh
    Transform* transform = nullptr;  // Pointer to a shared transform 
    Shader* shader = nullptr;        // Pointer to a shared shader

    std::string transformTag = "";

    glm::vec3 forwardDirection = glm::vec3(0, 0, 1);  // Default forward (Z-axis)
    glm::vec3 velocity = glm::vec3(0);               // Initial velocity
    glm::vec3 acceleration = glm::vec3(0.0f);

    // Default constructor allows empty object creation
    GameObject() = default;

    // Constructor-based initialization
    GameObject(Mesh* m, Transform* t, Shader* s)
        : mesh(m), transform(t), shader(s) {}

    // Flexible init function (can be called manually)
    void init(Mesh* m, Transform* t, Shader* s) {
        if (!mesh) mesh = m;
        if (!transform) transform = t;
        if (!shader) shader = s;
    }
};
