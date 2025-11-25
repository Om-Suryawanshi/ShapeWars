#include "GameManager.h"

Config g_Config;

GameManager::GameManager()
	:g_running(true)
	,entManager(EntityManager::getInstance())
	,assetHandler(AssetHandler::getInstance())
	,sceneManager(SceneManager::getInstance())
{
	if (g_Config.readConfig("CONFIG.txt"))
	{
		assetHandler.loadFont("mainFont", g_Config.game.font.path);
		init();
	}
}

void GameManager::init()
{
	ImGui::SFML::Init(g_window);

	// Full Screen logic
	std::cerr << g_Config.game.window.fullscreen << std::endl;
	if (g_Config.game.window.fullscreen)
	{
		sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
		g_window.create(desktop, "ShapeWars", sf::Style::Fullscreen);
	}
	else 
	{
		g_window.create(sf::VideoMode(g_Config.game.window.width, g_Config.game.window.height), "ShapeWars");
	}

	g_window.setFramerateLimit(g_Config.game.window.frameLimit);
	g_window.setVerticalSyncEnabled(true);
	// Full Screen Implementation
	// Not implemented

	sceneManager.setRenderWindow(&g_window);
	sceneManager.loadScene(SceneID::MainMenu);
}


void GameManager::update()
{
	if (!isRunning()) { return; }
	if (g_window.isOpen())
	{
		while (g_window.pollEvent(g_event))
		{
			// Close
			if (g_event.type == sf::Event::Closed)
			{
				quit();
				g_window.close();
			}
			
			// Quit Game with esc
			if (g_event.type == sf::Event::KeyPressed && g_event.key.code == sf::Keyboard::Escape)
			{
				g_window.close();
			}
			sceneManager.handleEvent(g_event);
		}
	}
	sceneManager.update(g_deltaClock.restart().asSeconds());
	g_window.clear();
	sceneManager.render(g_window);
	g_window.display();
}


void GameManager::quit()
{
	g_running = false;
}

void GameManager::run()
{
	while (isRunning())
	{
		update();
	}
}

unsigned int GameManager::width() const
{
	return g_window.getSize().x;
}

unsigned int GameManager::height() const
{
	return g_window.getSize().y;
}

sf::RenderWindow& GameManager::getWindow()
{
	return g_window;
}

bool GameManager::isRunning()
{
	return g_running && g_window.isOpen();
}
