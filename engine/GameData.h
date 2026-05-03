#pragma once
#include "config/vec2.h"
#include <vector>

struct RawGameState
{
	bool isPlayerDead;
	vec2 playerPos;

	std::vector<vec2> enemyPositions;
	std::vector<vec2> miniEnemyPositions;
	std::vector<vec2> bulletPositions;

	float currentScore;
};