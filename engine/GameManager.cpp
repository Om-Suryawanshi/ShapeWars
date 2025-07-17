#include "GameManager.h"

Config g_Config;

GameManager::GameManager()
	:g_running(true)
{
	if (g_Config.readConfig("CONFIG.txt"))
	{
		init();
	}
}

void GameManager::init()
{
	g_window.create(sf::VideoMode(g_Config.game.window.width, g_Config.game.window.height), "ShapeWars");
	ImGui::SFML::Init(g_window);

	g_window.setFramerateLimit(g_Config.game.window.frameLimit);
	g_window.setVerticalSyncEnabled(true);
	// Full Screen Implementation

	entManager.createEntity<Player>();
	//entManager.createEntity<Enemy>();

	// Enemy Spawner

	randomSpeed = g_Config.game.enemy.minSpeed + static_cast<float>(rand()) / RAND_MAX * (g_Config.game.enemy.maxSpeed - g_Config.game.enemy.minSpeed);
	randomSides = g_Config.game.enemy.minVertices + (rand() % (g_Config.game.enemy.maxVertices - g_Config.game.enemy.minVertices + 1));

	enemySpawnIntervalMs = g_Config.game.enemy.spawnInterval * (1000 / g_Config.game.window.frameLimit);


	g_ImguiStyle = ImGui::GetStyle();
}


void GameManager::update()
{
	ImGui::SFML::Update(g_window, g_deltaClock.restart());

	if (!isRunning()) { return; }
	if (g_window.isOpen())
	{
		while (g_window.pollEvent(g_event))
		{
			ImGui::SFML::ProcessEvent(g_event);

			if (g_event.type == sf::Event::Closed)
			{
				g_window.close();
			}

			//Bullet fire logic
			if (g_event.type == sf::Event::MouseButtonPressed &&
				g_event.mouseButton.button == sf::Mouse::Left)
			{
				sf::Vector2i mousePixel = sf::Mouse::getPosition(g_window);
				sf::Vector2f mouseWorld = g_window.mapPixelToCoords(mousePixel);
				vec2 mousePos(mouseWorld.x, mouseWorld.y);

				std::shared_ptr<entity> playerEnt = entManager.getEnt(0);
				if (playerEnt && playerEnt->getType() == EntityType::Player)
				{
					vec2 playerPos = playerEnt->getPos();
					vec2 direction = mousePos - playerPos;
					entManager.createEntity<Bullet>(playerPos, direction);
				}
			}
		}
	}


	// Enemy Spawn Logic
	maxEnemies = 10;
	if (enemySpawnClock.getElapsedTime().asMilliseconds() >= enemySpawnIntervalMs)
	{
		// Spawn new enemy using random parameters from config
		currentEnemies = entManager.countByType(EntityType::Enemy);
		if (currentEnemies < maxEnemies)
		{
			float randomSpeed = g_Config.game.enemy.minSpeed +
				static_cast<float>(rand()) / RAND_MAX *
				(g_Config.game.enemy.maxSpeed - g_Config.game.enemy.minSpeed);

			int randomSides = g_Config.game.enemy.minVertices +
				(rand() % (g_Config.game.enemy.maxVertices - g_Config.game.enemy.minVertices + 1));

			float radius = static_cast<float>(g_Config.game.enemy.shapeRadius);

			entManager.createEntity<Enemy>(randomSpeed, radius, static_cast<float>(randomSides));
		}

		// Restart spawn timer
		enemySpawnClock.restart();
	}


	ImGui::Begin("Entity Manager");

	if (ImGui::CollapsingHeader("Entities", ImGuiTreeNodeFlags_DefaultOpen))
	{
		auto& allEntities = entManager.getAllEnt();
		for (const auto& [id, ent] : allEntities)
		{
			ImGui::PushID(id);

			ImGui::Text("ID: %d", id);

			// Show type
			std::string typeName = "Unknown";
			switch (ent->getType())
			{
			case EntityType::Player: typeName = "Player"; break;
			case EntityType::Enemy: typeName = "Enemy"; break;
			case EntityType::Bullet: typeName = "Bullet"; break;
			default: break;
			}
			ImGui::SameLine();
			ImGui::Text("Type: %s", typeName.c_str());
			ImGui::SameLine();
			ImGui::Text("(%f, %f)", ent->getPos().x, ent->getPos().y);

			// Delete Button
			ImGui::SameLine();
			if (ImGui::Button("D"))
			{
				ent->die();
			}

			ImGui::PopID();
		}
	}

	ImGui::End();

	g_window.clear();
	

	entManager.update();
	entManager.draw(g_window);


	ImGui::SFML::Render(g_window);

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
