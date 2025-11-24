#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

//#include "Player.h"
#include "config/config.hpp"
#include "EntityManager.h"
#include "SceneManager.h"
#include "AssetHandler.h"
#include "RewindSystem.h"
#include "entities/player.h"
#include "entities/Enemy.h"

class GameManager
{
protected:
	sf::RenderWindow g_window;
	sf::Clock g_deltaClock;
	bool g_running;
	sf::Event g_event;

	Collision collision;
	SceneManager& sceneManager;
	AssetHandler& assetHandler;
	EntityManager& entManager;

	void update(); // Main Game method called every frame

public:
	GameManager();
	void init();
	void quit();
	void run();

	unsigned int width() const;
	unsigned int height() const;
	sf::RenderWindow& getWindow();
	bool isRunning();
	void updateScoreText();
};