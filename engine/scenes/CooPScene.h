#pragma once
#include "Scene.hpp"
#include "../EntityManager.h"
#include "../NetworkManager.h"
#include "../SceneManager.h"
#include "../RewindSystem.h"
#include "../Collision.h"

#include <iostream>
#include <vector>
#include <cstring> 

#include <imgui.h>
#include <imgui-SFML.h>

class CoopScene : public Scene
{
private:
	EntityManager& entManager;
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

	bool isPaused = false;
	// Enemy setup
	int enemySpawnIntervalMs;
	int maxEnemies = 10;
	int currentEnemies = 0;
	const float safeDistanceFromPlayer = 150.f;
	sf::Clock enemySpawnClock;


	// Score
	int score;
	int highScore;
	int respawnPenalty;

	void handleNetworking();
	void sendMyPosition();
	void syncEntity(const EntityState& state);

public:
	CoopScene();
	~CoopScene();

	void update(float deltaTime) override;
	void render(sf::RenderWindow& window) override;
	void handleEvent(const sf::Event& event) override;

};