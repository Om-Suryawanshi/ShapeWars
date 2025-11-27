#include "CoopScene.h"
#include <iostream>
#include <vector>
#include <cstring> // for memcpy

CoopScene::CoopScene()
	: entManager(EntityManager::getInstance())
	, rewindSystem(entManager, 300)
{
	NetworkManager& net = NetworkManager::getInstance();
	g_ImguiStyle = ImGui::GetStyle();
	localPlayer = entManager.createEntity<Player>();
	remotePlayer = entManager.createEntity<Player>(true); // True for remotePlayer
	if (net.isHost) {
		localPlayer->setPos({ 200.f, 300.f });
		remotePlayer->setPos({ 400.f, 300.f });
	}
	else {
		localPlayer->setPos({ 400.f, 300.f });
		remotePlayer->setPos({ 200.f, 300.f });
		remotePlayer->setColor(sf::Color(255, 255, 255, 150));
		std::cout << "[Coop] Client ready. Waiting for World State...\n";
	}

	// Enemy Setup
	enemySpawnIntervalMs = g_Config.game.enemy.spawnInterval * (1000 / g_Config.game.window.frameLimit);

	// Score Setup
}

CoopScene::~CoopScene()
{
	entManager.clearAll();
	rewindSystem.clearHistory();
}

void CoopScene::handleEvent(const sf::Event& event)
{
	ImGui::SFML::ProcessEvent(event);
	NetworkManager& net = NetworkManager::getInstance();
	SceneManager& sceneManager = SceneManager::getInstance();

	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P && !rewindSystem.isRewinding())
	{
		PausePacket pkt;
		pkt.newPauseState = !isPaused;
		net.sendPacket(&pkt, sizeof(pkt));
		isPaused = !isPaused;
		entManager.pauseEnt();
	}

	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R && !rewindSystem.isRewinding() && !isPaused)
	{
		RewindPacket pkt;
		pkt.isRewinding = true;
		net.sendPacket(&pkt, sizeof(pkt));
		rewindSystem.triggerRewind();
	}

	//Bullet Fire Logic
	if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left && !isPaused && !rewindSystem.isRewinding())
	{
		sf::Vector2i mousePixel = sf::Mouse::getPosition(*sceneManager.getRenderWindow());
		sf::Vector2f mouseWorld = sceneManager.getRenderWindow()->mapPixelToCoords(mousePixel);
		vec2 mousePos(mouseWorld.x, mouseWorld.y);

		if (localPlayer) // Only local player sends the bullet info so the remote player can spawn it.
		{
			vec2 playerPos = localPlayer->getPos();
			vec2 direction = mousePos - playerPos;
			auto entity = entManager.createEntity<Bullet>(playerPos, direction);
			SpawnPacket pkt;
			pkt.header.type = SPAWN_ENTITY;
			pkt.type = (int)EntityType::Bullet;

			pkt.data.bullet.id = entity->getId();
			pkt.data.bullet.x = entity->getPos().x;
			pkt.data.bullet.y = entity->getPos().y;
			pkt.data.bullet.dx = direction.x;
			pkt.data.bullet.dy = direction.y;

			net.sendPacket(&pkt, sizeof(pkt));
		}
	}
}

void CoopScene::update(float deltaTime)
{
	NetworkManager& net = NetworkManager::getInstance();
	handleNetworking();

	if (isPaused) return;

	// Enemy Spawnner
	if (net.isHost) 
	{
		if (enemySpawnClock.getElapsedTime().asMilliseconds() >= enemySpawnIntervalMs && !isPaused && !rewindSystem.isRewinding())
		{
			currentEnemies = entManager.countByType(EntityType::Enemy);
			if (currentEnemies < maxEnemies)
			{
				if (!remotePlayer && !localPlayer) return; // If both played died then return no spawn
				const vec2& playerPos = localPlayer->getPos();
				const float enemyRadius = static_cast<float>(g_Config.game.enemy.shapeRadius);
				const float bufferZone = safeDistanceFromPlayer + enemyRadius;

				vec2 spawnPos;
				int maxAttempts = 100;
				bool validPosFound = false;
				for (int i = 0; i < maxAttempts; i++)
				{
					// Generate spawn pos within window
					spawnPos = {
						static_cast<float>(rand() % (g_Config.game.window.width - static_cast<int>(enemyRadius * 2)) + enemyRadius),
						static_cast<float>(rand() % (g_Config.game.window.height - static_cast<int>(enemyRadius * 2)) + enemyRadius)
					};

					float dx = spawnPos.x - playerPos.x;
					float dy = spawnPos.y - playerPos.y;

					float distanceSquared = dx * dx + dy * dy;
					if (distanceSquared >= bufferZone * bufferZone)
					{
						validPosFound = true;
						break;
					}
				}
				if (validPosFound)
				{
					float randomSpeed = g_Config.game.enemy.minSpeed +
						static_cast<float>(rand()) / RAND_MAX *
						(g_Config.game.enemy.maxSpeed - g_Config.game.enemy.minSpeed);

					int randomSides = g_Config.game.enemy.minVertices +
						(rand() % (g_Config.game.enemy.maxVertices - g_Config.game.enemy.minVertices + 1));

					float angle = static_cast<float>((rand() % 360) * PI / 180.0f); // degrees to radians

					auto newEnemy = entManager.createEntity<Enemy>(randomSpeed, enemyRadius, randomSides, EntityType::Enemy, angle);
					newEnemy->setPos(spawnPos);

					SpawnPacket pkt;
					pkt.type = (int)newEnemy->getType();

					pkt.data.enemy.id = newEnemy->getId();
					pkt.data.enemy.x = spawnPos.x;
					pkt.data.enemy.y = spawnPos.y;
					pkt.data.enemy.speed = randomSpeed;
					pkt.data.enemy.radius = enemyRadius;
					pkt.data.enemy.sides = randomSides;
					pkt.data.enemy.angle = angle;

					net.sendPacket(&pkt, sizeof(pkt));
				}
			}
			enemySpawnClock.restart();
		}
	}

	// 4. UPDATE PHYSICS
	// Both sides run physics! Client predicts enemies.
	entManager.update(deltaTime);
	rewindSystem.update(); // Record history

	// 5. SEND MY POSITION (e.g. 60 times a sec or less)
	if (networkTick.getElapsedTime().asMilliseconds() > 15) {
		sendMyPosition();
		networkTick.restart();
	}

	// 6. HOST: SEND CORRECTIONS (e.g. 10 times a sec)
	// Prevents "Drift"
	if (net.isHost && worldSyncTick.getElapsedTime().asMilliseconds() > 100) {
		// Send WORLD_STATE packet containing Enemy positions only
		// (Code omitted for brevity, logic same as before)
		worldSyncTick.restart();
	}
}

void CoopScene::render(sf::RenderWindow& window)
{
	// Crash Prevention: render() handles empty lists safely
	entManager.draw(window);
}

void CoopScene::sendMyPosition()
{
	if (!localPlayer) return;

	PlayerPosPacket pkt;
	pkt.x = localPlayer->getPos().x;
	pkt.y = localPlayer->getPos().y;
	pkt.vx = localPlayer->getVelocity().x;
	pkt.vy = localPlayer->getVelocity().y;

	NetworkManager::getInstance().sendPacket(&pkt, sizeof(pkt));
}

void CoopScene::handleNetworking()
{
	NetworkManager& net = NetworkManager::getInstance();
	char buffer[4096];
	sockaddr_in sender;

	while (true)
	{
		int bytes = net.receivePacket(buffer, 4096, sender);
		if (bytes < 0) break;

		PacketHeader* h = (PacketHeader*)buffer;

		if (h->type == PLAYER_POS && remotePlayer)
		{
			PlayerPosPacket* pkt = (PlayerPosPacket*)buffer;
			remotePlayer->setPos(vec2(pkt->x, pkt->y));
			remotePlayer->setVelocity(vec2(pkt->vx, pkt->vy));
		}

		if (h->type == REWIND_EVENT)
		{
			RewindPacket* pkt = (RewindPacket*)buffer;
			if (pkt->isRewinding)
			{
				rewindSystem.triggerRewind();
			}
		}


		if (h->type == WORLD_STATE) {
			// Snap enemies to correct pos
		}

		if (h->type == PAUSE)
		{
			PausePacket* pkt = (PausePacket*)buffer;
			if (pkt->newPauseState != isPaused)
			{
				isPaused = !isPaused;
				entManager.pauseEnt();
			}
		}

		if (h->type == SPAWN_ENTITY)
		{
			SpawnPacket* pkt = (SpawnPacket*)buffer;
			if (pkt->type == (int)EntityType::Bullet)
			{
				auto entity = entManager.createEntity<Bullet>(vec2(pkt->data.bullet.x, pkt->data.bullet.y), vec2(pkt->data.bullet.dx, pkt->data.bullet.dy));
			}

			if (pkt->type == (int)EntityType::Enemy)
			{
				auto entity = entManager.createEntity<Enemy>(pkt->data.enemy.speed, pkt->data.enemy.radius, pkt->data.enemy.sides, EntityType::Enemy, pkt->data.enemy.angle);
				entity->setPos(vec2(pkt->data.enemy.x, pkt->data.enemy.y));
			}
		}
	}
}