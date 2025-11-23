#pragma once
#include "SFML//Graphics.hpp"
#include "../config/vec2.h"
#include "../config/config.hpp"
#include "entity.h"

class Player : public entity
{
protected:
	bool justRespawned = false;

public:
	sf::CircleShape player;

	Player(int id);
	void update();
	void draw(sf::RenderWindow& window);
	vec2 getPos() const;
	void die();
	void rotate();
};