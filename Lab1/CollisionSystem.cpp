#include "CollisionSystem.h"

#include <glm/glm.hpp>

void CollisionSystem::updateAABB(GameObject& object)
{
	if (!object.transform) return;

	glm::vec3 pos = *(object.transform->GetPos());

	object.aabbMin = glm::vec3(
		pos.x - object.collisionRadius,
		pos.y - object.collisionRadius,
		0.0f
	);

	object.aabbMax = glm::vec3(
		pos.x + object.collisionRadius,
		pos.y + object.collisionRadius,
		0.0f
	);
}

bool CollisionSystem::checkAABBCollision(GameObject& a, GameObject& b)
{
	bool overlapX = a.aabbMin.x <= b.aabbMax.x && a.aabbMax.x >= b.aabbMin.x;
	bool overlapY = a.aabbMin.y <= b.aabbMax.y && a.aabbMax.y >= b.aabbMin.y;

	return overlapX && overlapY;
}

bool CollisionSystem::checkNarrowPhaseCollision(GameObject& a, GameObject& b)
{
	if (!a.transform || !b.transform) return false;

	glm::vec3 aPos = *(a.transform->GetPos());
	glm::vec3 bPos = *(b.transform->GetPos());

	glm::vec2 difference = glm::vec2(aPos.x - bPos.x, aPos.y - bPos.y);

	float distanceSquared = glm::dot(difference, difference);
	float combinedRadius = a.collisionRadius + b.collisionRadius;
	float combinedRadiusSquared = combinedRadius * combinedRadius;

	return distanceSquared <= combinedRadiusSquared;
}

bool CollisionSystem::checkFullCollision(GameObject& a, GameObject& b)
{
	updateAABB(a);
	updateAABB(b);

	if (!checkAABBCollision(a, b))
	{
		return false;
	}

	return checkNarrowPhaseCollision(a, b);
}