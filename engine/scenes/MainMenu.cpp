#include "MainMenu.h"

MainScene::MainScene()
    : entManager(EntityManager::getInstance())
    , sceneManager(SceneManager::getInstance())
    , assetHandler(AssetHandler::getInstance())
{
    font = assetHandler.getFont("mainFont");

    // Safety check for window dimensions
    float width = 1600;
    float height = 800;
    if (sceneManager.getRenderWindow()) {
        width = (float)sceneManager.getRenderWindow()->getSize().x;
        height = (float)sceneManager.getRenderWindow()->getSize().y;
    }

    // 1. Title
    titleText.setFont(font);
    titleText.setString("SHAPE WARS");
    titleText.setCharacterSize(80);
    titleText.setFillColor(sf::Color::White);
    titleText.setOutlineColor(sf::Color(0, 100, 100)); // Cyan dark outline
    titleText.setOutlineThickness(4);

    sf::FloatRect tRect = titleText.getLocalBounds();
    titleText.setOrigin(tRect.left + tRect.width / 2.0f, tRect.top);
    titleText.setPosition(width / 2.0f, 100);

    // 2. Buttons
    float centerX = width / 2.0f;
    float startY = 350;

    btnSinglePlayer.init(font, "SINGLE PLAYER", { 300, 60 }, { centerX, startY });
    btnCoop.init(font, "CO-OP LOBBY", { 300, 60 }, { centerX, startY + 80 });
    btnExit.init(font, "EXIT", { 300, 60 }, { centerX, startY + 160 });
}

void MainScene::handleEvent(const sf::Event& event)
{
    sf::RenderWindow* win = sceneManager.getRenderWindow();
    if (!win) return;

    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(*win);

    if (btnSinglePlayer.isClicked(event, mousePos)) {
        sceneManager.loadScene(SceneID::SinglePLayer);
    }
    else if (btnCoop.isClicked(event, mousePos)) {
        sceneManager.loadScene(SceneID::CoopLobby);
    }
    else if (btnExit.isClicked(event, mousePos)) {
        win->close();
    }
}

void MainScene::update(float deltaTime)
{
    sf::RenderWindow* win = sceneManager.getRenderWindow();
    if (!win) return;

    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(*win);

    // Update hover effects
    btnSinglePlayer.update(mousePos);
    btnCoop.update(mousePos);
    btnExit.update(mousePos);
}

void MainScene::render(sf::RenderWindow& window)
{
    window.draw(titleText);
    btnSinglePlayer.render(window);
    btnCoop.render(window);
    btnExit.render(window);
}