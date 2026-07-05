#pragma once

#include "GameObject.h"

class CollisionSystem
{
public:
	static void updateAABB(GameObject& object);

	static bool checkAABBCollision(GameObject& a, GameObject& b);
	static bool checkNarrowPhaseCollision(GameObject& a, GameObject& b);
	static bool checkFullCollision(GameObject& a, GameObject& b);
};