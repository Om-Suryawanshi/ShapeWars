#pragma once
#include "Scene.hpp"
#include "../EntityManager.h"
#include "../NetworkManager.h"
#include "../RewindSystem.h"

#include <iostream>
#include <vector>
#include <cstring> 

class CoopScene : public Scene
{
	EntityManager& entManager;
	RewindSystem rewindSystem;

	std::shared_ptr<entity> localPlayer;
	std::shared_ptr<entity> remotePlayer;

	sf::Clock networkTick; // For sending Pos updates
	sf::Clock worldSyncTick; // For Host to send Enemy corrections

public:
	CoopScene();
	~CoopScene();

	void update(float deltaTime) override;
	void render(sf::RenderWindow& window) override;
	void handleEvent(const sf::Event& event) override;

private:
	void handleNetworking();
	void sendMyPosition();
	void syncEntity(const EntityState& state);
};