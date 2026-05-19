#pragma once
#include "Scene.hpp"
#include "../EntityManager.h"
#include "../AssetHandler.h"
#include "../SceneManager.h"
#include "../RewindSystem.h"
#include "../Collision.h"
#include "../InputHandler.h"

#include "../GameData.h"

#include <imgui.h>
#include <imgui-SFML.h>
#include <time.h>

class SceneManager;
class EntityManager;
class AssetHandler;

class SinglePlayerScene : public Scene
{
private:
	EntityManager& entManager;
	SceneManager& sceneManager;
	AssetHandler& assetHandler;
	InputHandler& inputHandler;
	RewindSystem rewindSystem;
	Collision collision;

	sf::Font font;
	sf::Text scoreText;

	sf::Text BackButtonText;
	sf::RectangleShape BackButton;

	sf::Text RestartText;

	sf::RectangleShape replayBar;
	sf::RectangleShape replayBarFill;

	ImGuiStyle g_ImguiStyle;
	sf::RenderWindow& g_window;

	bool m_isPaused = false;

	//World Config
	int score;
	int highScore;
	int respawnPenalty;

	float randomSpeed;
	float randomSides;
	sf::Clock enemySpawnClock;
	int enemySpawnIntervalMs;
	int maxEnemies = 10;
	int currentEnemies = 0;
	const float safeDistanceFromPlayer = 150.f;

	bool isGameOver = false;
	float deathTimer = 0.f;
	const float padding = 20.f;

	void restartGame();

public:
	SinglePlayerScene();
	~SinglePlayerScene();
	void handleEvent(const sf::Event& event);
	void update(float deltaTime);
	void render(sf::RenderWindow& window);
	void populateGameState(RawGameState& outData) const;
};