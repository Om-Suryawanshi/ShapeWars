#pragma once
#include <SFML/Graphics.hpp>
#include "../config/config.hpp"
#include "../config/vec2.h"


enum class EntityType 
{
	None = 0,
	Player = 1,
	Enemy = 2,
	Bullet = 3,
	MiniEnemy = 4
};

class entity
{
protected:
	vec2 velocity;
	bool isAlive;
	int id;
	float lifetime;
	float age;
	vec2 pos;
	float speed;
	float size; // Radius
	int sides;
	bool paused = false;
	EntityType type = EntityType::None;

public:
	entity();
	explicit entity(int id, float speed, float size, float sides) : id(id), speed(speed), size(size), sides(static_cast<int>(sides)) {} // Used for player and enemy
	explicit entity(int id, float speed, float size, float sides, vec2& pos) : id(id), speed(speed), size(size), sides(static_cast<int>(sides)), pos(pos) {} // Used for bullet

	virtual ~entity() = default;
	virtual void update(float deltaTime) = 0;
	virtual void draw(sf::RenderWindow& window) = 0;
	void die();
	void pause();
	// Getter
	EntityType getType() const { return type; }
	int getId() const { return id; }
	vec2 getPos() const;
	float getCollisionRadius() const;
	bool getisAlive() const;
	int getVertices() const;
	float getSpeed() const;
	float getSize() const;
	float getAge() const;
	float getLifetime() const;
	vec2 getVelocity() const;
	// Setter
	void setType(EntityType type);
	void setId(int id); // Coop else auto set
	void setPos(vec2 pos);
	void setLifetime(float life);
	void setAge(float a) { age = a; }
	void setVelocity(const vec2& v) { velocity = v; }
	void rotate();
	void setColor(sf::Color color);
};

