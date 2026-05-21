#pragma once
#include "Scene.hpp"
#include "../EntityManager.h"
#include "../NetworkManager.h"
#include "../SceneManager.h"
#include "../RewindSystem.h"
#include "../Collision.h"
#include "../InputHandler.h"

#include <iostream>
#include <vector>
#include <cstring> 

#include <imgui.h>
#include <imgui-SFML.h>

class CoopScene : public Scene
{
private:
	uint32_t localTick = 0;

	EntityManager& entManager;
	NetworkManager& net;
	SceneManager& sceneManager;
	InputHandler& inputHandler;

	Collision collision;
	RewindSystem rewindSystem;


	std::shared_ptr<entity> localPlayer;
	std::shared_ptr<entity> remotePlayer;

	sf::Clock networkTick; // For sending Pos updates
	sf::Clock worldSyncTick; // For Host to send Enemy corrections
	ImGuiStyle g_ImguiStyle;

	sf::Font font;
	sf::Text scoreText;

	sf::Text BackButtonText;
	sf::RectangleShape BackButton;

	sf::Text RestartText;
	sf::Text ReadyText;

	bool isPaused = false;
	// Enemy setup
	int enemySpawnIntervalMs;
	int maxEnemies = 10;
	int currentEnemies = 0;
	const float safeDistanceFromPlayer = 150.f;
	sf::Clock enemySpawnClock;

	bool isGameOver = false;
	bool isPlayerDead = false;
	bool isRemotePlayerDead = false;
	bool isPlayerReadyToReplay = false;
	bool isRemotePlayerReadyToPlay = false;

	// Score
	int score;
	int highScore;
	int respawnPenalty;

	const int HOST_ID = 1;
	const int CLIENT_ID = 1000000;

	void handleNetworking();
	void sendMyPosition();
	void restartGame();
	void spawnEnemies(NetworkManager& net);

	float centerX = g_Config.game.window.width / 2.0f;
	float centerY = g_Config.game.window.height / 2.0f;
	float spacing = 100.0f;

public:
	CoopScene();
	~CoopScene();

	void update(float deltaTime) override;
	void render(sf::RenderWindow& window) override;
	void handleEvent(const sf::Event& event) override;

};