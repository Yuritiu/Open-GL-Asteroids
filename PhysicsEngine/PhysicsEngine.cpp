#include "PhysicsEngine.h"
#include <iostream>

// Function implementation
extern "C" PHYSICS_API void HelloWorld() {
    std::cout << "Hello from Physics Engine DLL!" << std::endl;
}

extern "C" PHYSICS_API void setForwardDirection(GameObject * obj, glm::vec3 newForward) {
    // Access the Z-axis rotation (in radians)
    float angle = obj->transform->GetRot()->z;

    // Compute 2D forward direction based on rotation
    glm::vec3 forward(cos(angle), sin(angle), 0.0f);

    obj->forwardDirection = glm::normalize(forward);
}

extern "C" PHYSICS_API void applyThrust(GameObject* obj, float thrustAmount) {
    if (!obj || !obj->transform || !obj->transform->GetRot()) {
        std::cerr << "[ERROR] applyThrust: Invalid transform or rotation pointer.\n";
        return;
    }

    float angle = -obj->transform->GetRot()->z + glm::pi<float>();

    glm::vec3 thrustDirection;
    thrustDirection.x = sin(angle);
    thrustDirection.y = cos(angle);
    thrustDirection.z = 0.0f;

    thrustDirection = glm::normalize(thrustDirection);

    // Thrust now adds acceleration, not instant velocity
    obj->acceleration -= thrustDirection * thrustAmount;
}

extern "C" PHYSICS_API void updatePhysics(GameObject* obj, float deltaTime) {
    if (!obj || !obj->transform) return;

    obj->velocity += obj->acceleration * deltaTime;

    float maxSpeed = 12.0f;
    float speed = glm::length(obj->velocity);

    if (speed > maxSpeed) {
        obj->velocity = glm::normalize(obj->velocity) * maxSpeed;
    }

    float dragFactor = 0.985f;
    obj->velocity *= pow(dragFactor, deltaTime * 60.0f);

    if (glm::length(obj->velocity) < 0.001f) {
        obj->velocity = glm::vec3(0.0f);
    }

    glm::vec3 pos = *(obj->transform->GetPos());
    pos += obj->velocity * deltaTime;
    obj->transform->SetPos(pos);

    obj->acceleration = glm::vec3(0.0f);
}

