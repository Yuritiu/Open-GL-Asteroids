#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Particle
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 velocity = glm::vec3(0.0f);

	float lifetime = 1.0f;
	float maxLifetime = 1.0f;
	float size = 0.3f;

	bool isActive = true;
};

class ParticleSystem
{
public:
	void spawnExplosion(const glm::vec3& position, int amount);
	void update(float deltaTime);

	const std::vector<Particle>& getParticles() const;

private:
	std::vector<Particle> particles;
};