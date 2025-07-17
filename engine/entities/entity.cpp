#include "entity.h"


entity::entity()
{

}

vec2 entity::getPos() const
{
	return pos;
}

void entity::die()
{
	isAlive = false;
}

bool entity::getisAlive() const
{
	return isAlive;
}

void entity::pause() 
{
	paused = !paused;
}

float entity::getCollisionRadius() const
{
	if (type == EntityType::Bullet)
		return static_cast<float>(g_Config.game.bullet.collisionRadius);

	if (type == EntityType::Enemy)
		return static_cast<float>(g_Config.game.enemy.collisionRadius);

	if (type == EntityType::Player)
		return static_cast<float>(g_Config.game.player.collisionRadius);

	if (type == EntityType::None)
		return 0.0f;
}