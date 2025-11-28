#pragma once
#include "entity.h"


#include <random>
#include <ctime>

#include "../config/config.hpp"


class Enemy : public entity
{
protected:
	float angle;
	bool is_HitbyBullet = false;
	sf::CircleShape enemy;


public:
	Enemy(int id, float speed, float size, int sides, EntityType type, float angle); // used to set in which direction the enemy will move better for coop allows sync
	Enemy(int id, float speed, float size, int sides, EntityType type); // Better for single player no need to wory about the angle at which the enemy goes
	void update(float deltaTime);
	//bool colide(vec2& playerPos);
	void draw(sf::RenderWindow& window);
	vec2 getPos() const;
	void die();
	void rotate();
};

