#pragma once
#include "SFML//Graphics.hpp"
#include "../config/vec2.h"
#include "../config/config.hpp"
#include "entity.h"

#include "../InputHandler.h"

class Player : public entity
{
protected:
	bool justRespawned = false;
	bool isRemote = false;
	bool isDying = false;
	float deathTimer = 0.f;
	float deathDuration = 1.0f;

public:
	sf::CircleShape player;
	InputHandler& input;

	Player(int id);
	Player(int id, bool isRemote);
	void update(float deltaTime);
	void draw(sf::RenderWindow& window);
	vec2 getPos() const;
	void die();
	void rotate();
	void setColor(sf::Color color);
};