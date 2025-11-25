#pragma once
#include <memory>
#include <stack>

#include "scenes/Scene.hpp"
#include "scenes/MainMenu.h"
#include "scenes/LobbyScene.h"
#include "scenes/CooPScene.h"
#include "scenes/SinglePlayer.h"
#include "EntityManager.h"

enum class SceneID
{
	MainMenu,
	CoopLobby,
	COOP,
	SinglePLayer
};

class SceneManager
{
private:
	std::stack<std::unique_ptr<Scene>> scenes;
	sf::RenderWindow* window;
	EntityManager& entManager;
	SceneManager();
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

public:
	static SceneManager& getInstance();
	void pushScene(std::unique_ptr<Scene> scene);
	void popScene();
	void changeScene(std::unique_ptr<Scene> scene);
	Scene* getCurrentScene();
	bool isEmpty() const;
	void setRenderWindow(sf::RenderWindow* win);
	sf::RenderWindow* getRenderWindow() const;
	void handleEvent(const sf::Event& event);
	void update(float deltaTime);
	void render(sf::RenderWindow& window);
	void loadScene(SceneID id);
};