#pragma once
#include "Scene.hpp"
#include "../EntityManager.h"
#include "../NetworkManager.h"
#include "../RewindSystem.h"

class CoopScene : public Scene
{
	EntityManager& entManager;
	RewindSystem rewindSystem;

	std::shared_ptr<entity> localPlayer;
	std::shared_ptr<entity> remotePlayer;

	sf::Clock networkTick;

public:
	CoopScene();
	~CoopScene();

	void update(float deltaTime) override;
	void render(sf::RenderWindow& window) override;
	void handleEvent(const sf::Event& event) override;

private:
	void processHostLogic(float dt);
	void processClientLogic(float dt);
	void syncEntitiesFromNetwork(char* data, int bytes);
};