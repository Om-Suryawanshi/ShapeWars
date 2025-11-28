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
public:
    MainScene();
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;

private:
    EntityManager& entManager;
    SceneManager& sceneManager;
    AssetHandler& assetHandler;

    sf::Font font;
    sf::Text titleText;

    // modernized buttons
    ModernButton btnSinglePlayer;
    ModernButton btnCoop;
    ModernButton btnExit;
};