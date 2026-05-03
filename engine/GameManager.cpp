#include "GameManager.h"

Config g_Config;

GameManager::GameManager()
	:g_running(true)
	,m_headlessMode(g_Config.game.system.headlessMode)
	,entManager(EntityManager::getInstance())
	,assetHandler(AssetHandler::getInstance())
	,sceneManager(SceneManager::getInstance())
	,inputHandler(InputHandler::getInstance())
{
	assetHandler.loadFont("mainFont", g_Config.game.font.path);
	init();
}

void GameManager::init()
{
	if (!m_headlessMode)
	{
		if (g_Config.game.system.debugMode)
		{
			ImGui::SFML::Init(g_window);
		}

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
		sceneManager.setRenderWindow(&g_window);
		sceneManager.loadScene(SceneID::MainMenu);
	}
	sceneManager.loadScene(SceneID::MainMenu);
}


void GameManager::updateLogic(float dt)
{
	sceneManager.update(dt);
}

void GameManager::processEvents()
{
	if (m_headlessMode || !g_window.isOpen())
	{
		return;
	}

	while (g_window.pollEvent(g_event))
	{
		if (g_event.type == sf::Event::Closed)
		{
			quit();
			g_window.close();
		}

		if (g_event.type == sf::Event::KeyPressed && g_event.key.code == sf::Keyboard::Escape)
		{
			g_window.close();
		}
		inputHandler.processEvent(g_event); // Updates the input struct before handleing the events
		sceneManager.handleEvent(g_event);
	}
}

void GameManager::updateWithRendering()
{
	if (!isRunning())
		return;

	inputHandler.updateHardwareMouse(g_window);
	processEvents();
	float dt = g_deltaClock.restart().asSeconds();
	updateLogic(dt);

	g_window.clear();
	sceneManager.render(g_window);
	g_window.display();
	inputHandler.endFrame();
}

void GameManager::update()
{
	if (m_headlessMode)
	{
		float fixedDeltaTime = 1.0f / g_Config.game.window.frameLimit;
		updateLogic(fixedDeltaTime);
		inputHandler.endFrame();
	}
	else
	{
		updateWithRendering();
	}
}

void GameManager::step(float dt)
{
	updateLogic(dt);
	inputHandler.endFrame();
}

void GameManager::runHeadless(int maxSteps)
{
	m_headlessMode = true;
	float fixedDeltaTime = 1.0f / g_Config.game.window.frameLimit;
	int stepCount = 0;

	while (g_running && (maxSteps < 0 || stepCount < maxSteps))
	{
		updateLogic(fixedDeltaTime);
		inputHandler.endFrame();
		stepCount++;
	}
}

void GameManager::setHeadlessMode(bool headless)
{
	m_headlessMode = headless;
}

void GameManager::reset()
{
	entManager.clearAll();
	sceneManager.loadScene(SceneID::SinglePLayer);
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

const RawGameState& GameManager::getRawGameState()
{
	auto scene = dynamic_cast<SinglePlayerScene*>(sceneManager.getCurrentScene());
	if (scene)
	{
		scene->populateGameState(GameState);
	}
	return GameState;
}
