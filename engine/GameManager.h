#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

#include "config/config.hpp"
#include "EntityManager.h"
#include "SceneManager.h"
#include "AssetHandler.h"
#include "InputHandler.h"

#include "GameData.h" // For ai


class GameManager
{
private:
	RawGameState GameState;

protected:
	sf::RenderWindow g_window;
	sf::Clock g_deltaClock;
	bool g_running;
	sf::Event g_event;
	bool m_headlessMode;

	Collision collision;
	SceneManager& sceneManager;
	AssetHandler& assetHandler;
	EntityManager& entManager;
	InputHandler& inputHandler;

	void updateLogic(float dt);
	void updateWithRendering();
	void processEvents();
	void update();

public:
	GameManager();
	void init();
	void quit();
	void run();
	void runHeadless(int maxSteps = -1);
	void step(float dt);
	void reset();
	void setHeadlessMode(bool headless);

	unsigned int width() const;
	unsigned int height() const;
	sf::RenderWindow& getWindow();
	bool isRunning();

	const RawGameState& getRawGameState();
};