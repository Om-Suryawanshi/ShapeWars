#pragma once
#include "Scene.hpp"
#include <SFML/Graphics.hpp>

#include "../EntityManager.h"
#include "../AssetHandler.h"
#include "../SceneManager.h"

class SceneManager;
class EntityManager;
class AssetHandler;

class MainScene : public Scene
{
private:
	EntityManager& entManager;
	SceneManager& sceneManager;
	AssetHandler& assetHandler;

	sf::Font font;
	sf::Text titleText;
	sf::RectangleShape singlePlayerplayButton;
	sf::Text singlePlayerplayText;

	sf::RectangleShape coopPlayButton;
	sf::Text coopPlayText;

public:
	MainScene();
	void handleEvent(const sf::Event& event);
	void update(float delatTime);
	void render(sf::RenderWindow& window);
	bool isMouseOver(const sf::RectangleShape& button, const sf::RenderWindow& window);
};