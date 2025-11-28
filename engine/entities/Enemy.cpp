#include "Enemy.h"


Enemy::Enemy(int id, float speed, float size, int sides, EntityType enemyType, float a)
	: entity(id, speed, size, static_cast<float>(sides))
	, enemy(size, static_cast<size_t>(sides))	
	, angle(a)
{
	isAlive = true;
	type = enemyType;
	age = 0;

	velocity.x = std::cos(angle) * speed;
	velocity.y = std::sin(angle) * speed;

	lifetime = static_cast<float>(g_Config.game.enemy.lifespan);

	enemy.setOrigin(size, size);
	enemy.setFillColor(sf::Color::Transparent);
	enemy.setOutlineColor(sf::Color(
		g_Config.game.enemy.outlineR,
		g_Config.game.enemy.outlineG,
		g_Config.game.enemy.outlineB
	)); 
	enemy.setOutlineThickness(static_cast<float>(g_Config.game.enemy.outlineThickness));
}

Enemy::Enemy(int id, float speed, float size, int sides, EntityType enemyType)
	: entity(id, speed, size, static_cast<float>(sides))
	, enemy(size, static_cast<size_t>(sides))
{
	isAlive = true;
	type = enemyType;
	age = 0;

	velocity.x = std::cos(angle) * speed;
	velocity.y = std::sin(angle) * speed;

	lifetime = static_cast<float>(g_Config.game.enemy.lifespan);

	enemy.setOrigin(size, size);
	enemy.setFillColor(sf::Color::Transparent);
	enemy.setOutlineColor(sf::Color(
		g_Config.game.enemy.outlineR,
		g_Config.game.enemy.outlineG,
		g_Config.game.enemy.outlineB
	));
	enemy.setOutlineThickness(static_cast<float>(g_Config.game.enemy.outlineThickness));
}

void Enemy::update(float deltaTime)
{
	if (!paused)
	{
		//Age
		age += 10.0f / g_Config.game.window.frameLimit;
		if (age >= lifetime)
		{
			die();
		}
		float progress = age / lifetime;
		progress = std::clamp(progress, 0.0f, 1.0f); // Ensure it's within bounds

		sf::Uint8 alpha = static_cast<sf::Uint8>(255 * (1.0f - progress));

		// Clamp to a minimum visible value (e.g., 80)
		alpha = std::max(alpha, static_cast<sf::Uint8>(80));

		sf::Color currentOutlineColor = enemy.getOutlineColor();
		currentOutlineColor.a = alpha;
		enemy.setOutlineColor(currentOutlineColor);


		// Enemy Movement Logic
		pos += velocity;
		if (pos.x < size || pos.x > g_Config.game.window.width - size) velocity.x *= -1;
		if (pos.y < size || pos.y > g_Config.game.window.height - size) velocity.y *= -1;
		enemy.setPosition(pos.x, pos.y);
	}

	rotate();
}

void Enemy::rotate()
{
	enemy.rotate(2.0f);
}

void Enemy::draw(sf::RenderWindow& window)
{
	window.draw(enemy);
}

void Enemy::die()
{
	isAlive = false;
}

vec2 Enemy::getPos() const
{
	return pos;
}

