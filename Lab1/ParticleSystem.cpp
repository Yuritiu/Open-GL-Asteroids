#include "ParticleSystem.h"

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <glm/gtc/constants.hpp>

void ParticleSystem::spawnExplosion(const glm::vec3& position, int amount)
{
	for (int i = 0; i < amount; i++)
	{
		Particle particle;

		particle.position = position;

		float angle = static_cast<float>(rand()) / RAND_MAX * glm::two_pi<float>();
		float speed = 4.0f + static_cast<float>(rand()) / RAND_MAX * 6.0f;

		particle.velocity = glm::vec3(
			cos(angle) * speed,
			sin(angle) * speed,
			0.0f
		);

		particle.maxLifetime = 0.5f + static_cast<float>(rand()) / RAND_MAX * 0.6f;
		particle.lifetime = particle.maxLifetime;

		particle.size = 0.3f + static_cast<float>(rand()) / RAND_MAX * 0.12f;
		particle.isActive = true;

		particles.push_back(particle);
	}
}

void ParticleSystem::update(float deltaTime)
{
	for (auto& particle : particles)
	{
		if (!particle.isActive) continue;

		particle.lifetime -= deltaTime;

		if (particle.lifetime <= 0.0f)
		{
			particle.isActive = false;
			continue;
		}

		particle.position += particle.velocity * deltaTime;

		// slow particles down slightly
		particle.velocity *= pow(0.92f, deltaTime * 60.0f);
	}

	particles.erase(
		std::remove_if(particles.begin(), particles.end(),
			[](const Particle& particle)
			{
				return !particle.isActive;
			}),
		particles.end()
	);
}

const std::vector<Particle>& ParticleSystem::getParticles() const
{
	return particles;
}
