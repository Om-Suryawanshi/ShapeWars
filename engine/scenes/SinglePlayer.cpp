#include "SinglePlayer.h"

SinglePlayerScene::SinglePlayerScene()
	:entManager(EntityManager::getInstance())
	, assetHandler(AssetHandler::getInstance())
	, sceneManager(SceneManager::getInstance())
	, g_window((*sceneManager.getRenderWindow()))
	, rewindSystem(entManager, 300)
{
	font = assetHandler.getFont("mainFont");
	BackButton.setSize({ 300, 60 });
	BackButton.setFillColor(sf::Color::Transparent);
	BackButton.setOrigin(0, BackButton.getSize().y / 2.f);
	BackButton.setPosition(0, 50.0f);

	BackButtonText.setFont(font);
	BackButtonText.setString("BACK");
	BackButtonText.setCharacterSize(28);
	BackButtonText.setPosition(15, BackButton.getSize().y);

	g_ImguiStyle = ImGui::GetStyle();

	// Create Player 
	entManager.createEntity<Player>();

	// Enemy Spawner setup
	randomSpeed = static_cast<float> (g_Config.game.enemy.minSpeed) + static_cast<float>(rand()) / RAND_MAX * (static_cast<float> (g_Config.game.enemy.maxSpeed) - static_cast<float> (g_Config.game.enemy.minSpeed));
	randomSides = static_cast<float> (g_Config.game.enemy.minVertices) + (rand() % (static_cast<int>(g_Config.game.enemy.maxVertices) - static_cast<int>(g_Config.game.enemy.minVertices + 1)));
	enemySpawnIntervalMs = g_Config.game.enemy.spawnInterval * (1000 / g_Config.game.window.frameLimit);

	// Score Setup
	scoreText.setFont(font);
	scoreText.setCharacterSize(g_Config.game.font.size);
	scoreText.setFillColor(sf::Color(
		g_Config.game.font.r,
		g_Config.game.font.g,
		g_Config.game.font.b
	));

	scoreText.setPosition(10.f, 10.f);
	scoreText.setString("Score: 0");
}

void SinglePlayerScene::handleEvent(const sf::Event& event)
{
	ImGui::SFML::ProcessEvent(event);

	//Bullet fire logic
	if (event.type == sf::Event::MouseButtonPressed &&
		event.mouseButton.button == sf::Mouse::Left && !m_isPaused && !rewindSystem.isRewinding())
	{
		sf::Vector2i mousePixel = sf::Mouse::getPosition(g_window);
		sf::Vector2f mouseWorld = g_window.mapPixelToCoords(mousePixel);
		vec2 mousePos(mouseWorld.x, mouseWorld.y);

		std::shared_ptr<entity> playerEnt = entManager.getPlayer();
		if (playerEnt && playerEnt->getType() == EntityType::Player)
		{
			vec2 playerPos = playerEnt->getPos();
			vec2 direction = mousePos - playerPos;
			entManager.createEntity<Bullet>(playerPos, direction);
		}

		if (BackButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePixel)))
		{
			sceneManager.loadScene(SceneID::MainMenu);
			//rewindSystem.clearHistory();
		}
	}

	// Pause with P
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P && !rewindSystem.isRewinding())
	{
		m_isPaused = !m_isPaused;
		entManager.pauseEnt();
		rewindSystem.pauseCapture();
	}

	// Rewind
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R && !m_isPaused)
	{
		std::cerr << "Rewind triggered" << std::endl;
		rewindSystem.triggerRewind();
	}
}

void SinglePlayerScene::update(float deltaTime)
{
	// Enemy Spawn Logic
	if (enemySpawnClock.getElapsedTime().asMilliseconds() >= enemySpawnIntervalMs && !m_isPaused && !rewindSystem.isRewinding())
	{
		currentEnemies = entManager.countByType(EntityType::Enemy);
		if (currentEnemies < maxEnemies)
		{
			const auto& player = entManager.getPlayer();
			if (!player) return; // Make sure player exists

			const vec2& playerPos = player->getPos();
			const float enemyRadius = static_cast<float>(g_Config.game.enemy.shapeRadius);
			const float bufferZone = safeDistanceFromPlayer + enemyRadius;

			vec2 spawnPos;
			int maxAttempts = 100; // Avoid infinite loops
			bool validPositionFound = false;

			for (int i = 0; i < maxAttempts; ++i)
			{
				// Generate random spawn position within window
				spawnPos = {
					static_cast<float>(rand() % (g_Config.game.window.width - static_cast<int>(enemyRadius * 2)) + enemyRadius),
					static_cast<float>(rand() % (g_Config.game.window.height - static_cast<int>(enemyRadius * 2)) + enemyRadius)
				};

				float dx = spawnPos.x - playerPos.x;
				float dy = spawnPos.y - playerPos.y;
				float distanceSquared = dx * dx + dy * dy;

				if (distanceSquared >= bufferZone * bufferZone)
				{
					validPositionFound = true;
					break;
				}
			}

			if (validPositionFound)
			{
				float randomSpeed = g_Config.game.enemy.minSpeed +
					static_cast<float>(rand()) / RAND_MAX *
					(g_Config.game.enemy.maxSpeed - g_Config.game.enemy.minSpeed);

				int randomSides = g_Config.game.enemy.minVertices +
					(rand() % (g_Config.game.enemy.maxVertices - g_Config.game.enemy.minVertices + 1));

				auto newEnemy = entManager.createEntity<Enemy>(randomSpeed, enemyRadius, static_cast<float>(randomSides), EntityType::Enemy);
				newEnemy->setPos(spawnPos);
			}
		}
		enemySpawnClock.restart();
	}

	//Collision Code between player and enemy
	std::shared_ptr<entity> player = entManager.getPlayer();
	for (auto& enemy : entManager.getByType(EntityType::Enemy))
	{
		if (!enemy->getisAlive()) continue;
		if (collision.checkCollision(*player, *enemy))
		{
			player->die();
			if (!entManager.playerExists())
			{
				rewindSystem.clearHistory();
				entManager.createEntity<Player>();
			}
			score -= enemy->getVertices() * 100;
			enemy->die();
		}

		for (auto& minienemy : entManager.getByType(EntityType::MiniEnemy))
		{
			if (!minienemy) continue;
			if (collision.checkCollision(*player, *minienemy))
			{
				// Prevents duplication of player if which happened in old logic 
				/*player->die();
				if (!entManager.playerExists())
				{
					rewindSystem.clearHistory();
					entManager.createEntity<Player>();
				}*/
				score -= enemy->getVertices() * 100;
				minienemy->die();
			}
		}
	}

	// Collision logic between bullet and enemy
	for (auto& bullet : entManager.getByType(EntityType::Bullet))
	{
		if (!bullet->getisAlive()) continue;
		for (auto& enemy : entManager.getByType(EntityType::Enemy))
		{
			if (!enemy->getisAlive()) continue;
			if (collision.checkCollision(*bullet, *enemy))
			{
				bullet->die();
				score += enemy->getVertices() * 100;

				//Enemy Split and scatter
				float angleIncrement = 2.0f * PI / enemy->getVertices();
				// Mini enemy spawnner
				for (int i = 0; i < static_cast<int>(enemy->getVertices()); ++i)
				{
					float angle = i * angleIncrement;
					vec2 dir(std::cos(angle), std::sin(angle));

					// Spawn the mini enemy
					std::shared_ptr<entity> minienemy_base = entManager.createEntity<Enemy>(
						enemy->getSpeed(),
						enemy->getSize() / 2,
						static_cast<float>(enemy->getVertices()),
						EntityType::MiniEnemy
					);

					std::shared_ptr<Enemy> minienemy = std::dynamic_pointer_cast<Enemy>(minienemy_base);

					minienemy->setPos(enemy->getPos());

					minienemy->setVelocity(dir * (enemy->getSpeed()));

					minienemy->setLifetime(10);
				}
				enemy->die();
				break;
			}
		}

		//Minienemy and bullet
		for (auto& minienemy : entManager.getByType(EntityType::MiniEnemy))
		{
			if (!minienemy->getisAlive()) continue;
			if (collision.checkCollision(*bullet, *minienemy))
			{
				score += minienemy->getVertices() * 300;
				bullet->die();
				minienemy->die();
				break;
			}
		}
	}

	scoreText.setString("Score: " + std::to_string(score));

	sf::Time dt = sf::seconds(deltaTime);
	ImGui::SFML::Update(g_window, dt);
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
			case EntityType::MiniEnemy: typeName = "MiniEnemy"; break;
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
	rewindSystem.update(); // Allways keep rewind before clear or else it will fuck it up
	g_window.clear();
	entManager.update(deltaTime);
}

void SinglePlayerScene::render(sf::RenderWindow& window)
{
	window.draw(scoreText);
	window.draw(BackButton);
	window.draw(BackButtonText);
	entManager.draw(window);
	ImGui::SFML::Render(g_window);
}

SinglePlayerScene::~SinglePlayerScene()
{
	std::cout << "SinglePlayerScene destroyed\n";
	entManager.clearAll();
	rewindSystem.clearHistory();
}