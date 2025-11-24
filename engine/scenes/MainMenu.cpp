#include "MainMenu.h"

MainScene::MainScene()
	:entManager(EntityManager::getInstance())
	, sceneManager(SceneManager::getInstance())
	, assetHandler(AssetHandler::getInstance())
{
	font = assetHandler.getFont("mainFont");
	float windowHeight = static_cast<float>(g_Config.game.window.height);
	float initialButtonY = 350.f;
	titleText.setFont(font);
	titleText.setString("ShapeWars");
	titleText.setCharacterSize(48);
	titleText.setFillColor(sf::Color::White);
	titleText.setPosition(50, 50);

	singlePlayerplayButton.setSize({ 300, 60 });
	singlePlayerplayButton.setFillColor(sf::Color::Transparent);
	singlePlayerplayButton.setOrigin(0, singlePlayerplayButton.getSize().y / 2.f);
	singlePlayerplayButton.setPosition(50, initialButtonY);

	singlePlayerplayText.setFont(font);
	singlePlayerplayText.setString("Play");
	singlePlayerplayText.setCharacterSize(28);
	singlePlayerplayText.setFillColor(sf::Color(128, 128, 128));
	singlePlayerplayText.setPosition(singlePlayerplayButton.getPosition().x, singlePlayerplayButton.getPosition().y - (singlePlayerplayText.getGlobalBounds().height / 2.f));

	coopPlayButton.setSize({ 300, 60 });
	coopPlayButton.setFillColor(sf::Color::Transparent);
	coopPlayButton.setOrigin(0, coopPlayButton.getSize().y / 2.f);
	coopPlayButton.setPosition(50, singlePlayerplayButton.getPosition().y + singlePlayerplayButton.getGlobalBounds().height + 30);

	coopPlayText.setFont(font);
	coopPlayText.setString("COOP");
	coopPlayText.setCharacterSize(28);
	coopPlayText.setFillColor(sf::Color(128, 128, 128));
	coopPlayText.setPosition(coopPlayButton.getPosition().x, coopPlayButton.getPosition().y - (coopPlayButton.getGlobalBounds().height / 2.f));
}

void MainScene::handleEvent(const sf::Event& event)
{
	if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
	{
		if(isMouseOver(singlePlayerplayButton, *sceneManager.getRenderWindow()))
		{
			//sceneManager.SinglePlayerMenu();
		}
		else if (isMouseOver(coopPlayButton, *sceneManager.getRenderWindow()))
		{
			sceneManager.COOPMenu();
		}
	}
}

void MainScene::update(float deltaTime)
{
	sf::RenderWindow* window = sceneManager.getRenderWindow();
	if (!window)
	{
		return;
	}
	if (isMouseOver(singlePlayerplayButton, *window))
	{
		singlePlayerplayText.setFillColor(sf::Color::White);
	}
	else
	{
		singlePlayerplayText.setFillColor(sf::Color(128, 128, 128));
	}

	if (isMouseOver(coopPlayButton, *window))
	{
		coopPlayText.setFillColor(sf::Color::White);
	}
	else
	{
		coopPlayText.setFillColor(sf::Color(128, 128, 128));
	}
}

void MainScene::render(sf::RenderWindow& window)
{
	window.draw(titleText);
	window.draw(singlePlayerplayButton);
	window.draw(singlePlayerplayText);
	window.draw(coopPlayButton);
	window.draw(coopPlayText);
}

bool MainScene::isMouseOver(const sf::RectangleShape& button, const sf::RenderWindow& window)
{
	sf::Vector2i mousePos = sf::Mouse::getPosition(window);
	return button.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}
