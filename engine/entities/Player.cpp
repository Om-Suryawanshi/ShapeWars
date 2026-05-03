#include "Player.h"


Player::Player(int id)
	: entity(id, static_cast<float>(g_Config.game.player.speed), static_cast<float>(g_Config.game.player.shapeRadius), static_cast<float>(g_Config.game.player.vertices))
	, player(static_cast<float>(g_Config.game.player.shapeRadius), static_cast<size_t>(g_Config.game.player.vertices)) // sf::CircleShape init
	, isRemote(false)
	, input(InputHandler::getInstance())
{
	type = EntityType::Player;
	isAlive = true;

	player.setOrigin(size, size);
	player.setFillColor(sf::Color(g_Config.game.player.fillR, g_Config.game.player.fillG, g_Config.game.player.fillB));
	player.setOutlineColor(sf::Color(g_Config.game.player.outlineR, g_Config.game.player.outlineG, g_Config.game.player.outlineB));
	player.setOutlineThickness(static_cast<float>(g_Config.game.player.outlineThickness));

	pos.x = static_cast<float>(g_Config.game.window.width) / 2;
	pos.y = static_cast<float>(g_Config.game.window.height) / 2;
	player.setPosition(pos.x, pos.y);
}

Player::Player(int id, bool m_isRemote)
	: entity(id, static_cast<float>(g_Config.game.player.speed), static_cast<float>(g_Config.game.player.shapeRadius), static_cast<float>(g_Config.game.player.vertices))
	, player(static_cast<float>(g_Config.game.player.shapeRadius), static_cast<size_t>(g_Config.game.player.vertices)) // sf::CircleShape init
	, isRemote(m_isRemote)
	, input(InputHandler::getInstance())
{
	type = EntityType::Player;
	isAlive = true;

	player.setOrigin(size, size);
	if (!isRemote)
		player.setFillColor(sf::Color(g_Config.game.player.fillR, g_Config.game.player.fillG, g_Config.game.player.fillB));
	else
		player.setFillColor(sf::Color::Blue);
	player.setOutlineColor(sf::Color(g_Config.game.player.outlineR, g_Config.game.player.outlineG, g_Config.game.player.outlineB));
	player.setOutlineThickness(static_cast<float>(g_Config.game.player.outlineThickness));

	pos.x = static_cast<float>(g_Config.game.window.width) / 2;
	pos.y = static_cast<float>(g_Config.game.window.height) / 2;
	player.setPosition(pos.x, pos.y);
}

void Player::update(float deltaTime)
{
	if (!paused)
	{
		if (isDying)
		{
			deathTimer += deltaTime;

			float scale = 1.0f + deathTimer * 2.0f;
			float alpha = 255.0f * (1.0f - deathTimer / deathDuration);

			player.setScale(scale, scale);
			player.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));

			if (deathTimer >= deathDuration)
			{
				isAlive = false;
			}
			std::cout << "Player Died" << std::endl;
			return;
		}

		if (justRespawned)
		{
			justRespawned = false;
			return;
		}

		// Movement
		if (!isRemote)
		{
			if (input.isKeyPressed('W')) pos.y -= speed;
			if (input.isKeyPressed('S')) pos.y += speed;
			if (input.isKeyPressed('A')) pos.x -= speed;
			if (input.isKeyPressed('D')) pos.x += speed;
		}
		// Boundry Checking so the player dosent clip out
		if (pos.x < size) pos.x = size;
		if (pos.x > g_Config.game.window.width - size) pos.x = g_Config.game.window.width - size;
		if (pos.y < size) pos.y = size;
		if (pos.y > g_Config.game.window.height - size) pos.y = g_Config.game.window.height - size;

		player.setPosition(pos.x, pos.y);
		rotate();
	}
}

void Player::draw(sf::RenderWindow& window)
{
	window.draw(player);
}

vec2 Player::getPos() const
{
	return pos;
}

void Player::die()
{
	if (isDying) return;
	isDying = true;
	//isAlive = false;
}

void Player::rotate()
{
	player.rotate(2.0f);
}

void Player::setColor(sf::Color color)
{
	player.setFillColor(color);
}