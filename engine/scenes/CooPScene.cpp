#include "CoopScene.h"
#include <iostream>
#include <vector>
#include <cstring> 

CoopScene::CoopScene()
	: entManager(EntityManager::getInstance())
	, net(NetworkManager::getInstance())
	, sceneManager(SceneManager::getInstance())
	, rewindSystem(entManager, 300)
{
	NetworkManager& net = NetworkManager::getInstance();
	font = AssetHandler::getInstance().getFont("mainFont");
	BackButton.setSize({ 300, 60 });
	BackButton.setFillColor(sf::Color::Transparent);
	BackButton.setOrigin(0, BackButton.getSize().y / 2.f);
	BackButton.setPosition(50, 50.f);
	BackButtonText.setFont(font);
	BackButtonText.setString("BACK");
	BackButtonText.setCharacterSize(28);
	BackButtonText.setPosition(15, BackButton.getSize().y);

	if (g_Config.game.system.debugMode)
	{
		g_ImguiStyle = ImGui::GetStyle();
	}

	const int HOST_ID = 1; 
	const int CLIENT_ID = 1000000;

	if (net.isHost) 
	{
		// Host Setup
		entManager.setStartId(2); // Start spawning other stuff at 2

		// Host is Local(1), Client is Remote(1M)
		localPlayer = entManager.createEntityWithId<Player>(HOST_ID);
		remotePlayer = entManager.createEntityWithId<Player>(CLIENT_ID, true);

		localPlayer->setPos({ 200.f, 300.f });
		remotePlayer->setPos({ 400.f, 300.f });
	}
	else 
	{
		// Client Setup
		entManager.setStartId(1000001); // Start spawning other stuff at 1M+1

		// Client is Local(1M), Host is Remote(1)
		localPlayer = entManager.createEntityWithId<Player>(CLIENT_ID);
		remotePlayer = entManager.createEntityWithId<Player>(HOST_ID, true);

		localPlayer->setPos({ 400.f, 300.f });
		remotePlayer->setPos({ 200.f, 300.f });
		remotePlayer->setColor(sf::Color(255, 255, 255, 150));
		std::cout << "[Coop] Client ready. Waiting for World State...\n";
	}
	// Enemy Setup
	enemySpawnIntervalMs = static_cast<int>(g_Config.game.enemy.spawnInterval * (1000 / 60.0f));

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

CoopScene::~CoopScene()
{
	entManager.clearAll();
	rewindSystem.clearHistory();
}

void CoopScene::handleEvent(const sf::Event& event)
{
	if (g_Config.game.system.debugMode)
	{
		ImGui::SFML::ProcessEvent(event);
	}

	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
	{
		ExitPacket pkt;
		net.sendReliable(&pkt, sizeof(pkt));
		net.cleanup();
	}


	// Pause
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P && !rewindSystem.isRewinding())
	{
		PausePacket pkt;
		pkt.newPauseState = !isPaused;
		net.sendReliable(&pkt, sizeof(pkt));
		isPaused = !isPaused;
		entManager.pauseEnt();
	}


	// Rewind
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R && !rewindSystem.isRewinding() && !isPaused)
	{
		RewindPacket pkt;
		pkt.isRewinding = true;
		net.sendReliable(&pkt, sizeof(pkt));
		rewindSystem.triggerRewind();
	}

	// Mouse Clicked Events
	if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left && !isPaused && !rewindSystem.isRewinding())
	{
		sf::Vector2i mousePixel = sf::Mouse::getPosition(*sceneManager.getRenderWindow());
		sf::Vector2f mouseWorld = sceneManager.getRenderWindow()->mapPixelToCoords(mousePixel);
		vec2 mousePos(mouseWorld.x, mouseWorld.y);


		// Back Button Logic
		if (BackButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePixel)))
		{
			sceneManager.loadScene(SceneID::MainMenu);
			ExitPacket pkt;
			net.sendReliable(&pkt, sizeof(pkt));
			net.cleanup();
		}

		//Bullet Fire Logic
		if (localPlayer->getisAlive()) // Only local player sends the bullet info so the remote player can spawn it.
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

void CoopScene::spawnEnemies(NetworkManager& net)
{
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

					auto newEnemy = entManager.createEntity<Enemy>(randomSpeed, enemyRadius, static_cast<float>(randomSides), EntityType::Enemy, angle);
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

					net.sendReliable(&pkt, sizeof(pkt)); // Spawn pkt is needs to reliable
				}
			}
			enemySpawnClock.restart();
		}
	}
}

void CoopScene::update(float deltaTime)
{
	localTick++;
	NetworkManager& net = NetworkManager::getInstance();
	net.update(); // Resends lost reliable pkts
	net.updateStats(); // Updates ping pkt loss
	handleNetworking(); // Checks incomming pkts

	if (isPaused) return;

	spawnEnemies(net);
	

	// Collision code between player handle only localplayer the remote player will handle himself
	for (auto& enemy : entManager.getByType(EntityType::Enemy))
	{
		if (!enemy->getisAlive()) continue;
		if (collision.checkCollision(*localPlayer, *enemy))
		{
			RewindPacket pkt;
			pkt.header.type = REWIND_CLEAR;
			pkt.isRewinding = false;
			net.sendPacket(&pkt, sizeof(pkt)); // Send Packet to clear history of rewind


			KillEntityPacket Playerkillpkt;
			Playerkillpkt.type = (int)EntityType::Player;
			net.sendReliable(&Playerkillpkt, sizeof(Playerkillpkt)); // Send packet to kill the localPlayer on remoteClient
			localPlayer->die();


			KillEntityPacket enemyKillPacket;
			enemyKillPacket.type = (int)EntityType::Enemy;
			enemyKillPacket.id = enemy->getId();
			net.sendReliable(&enemyKillPacket, sizeof(enemyKillPacket)); // send the packet to which we colided and with
			enemy->die();
			rewindSystem.clearHistory();
			localPlayer = entManager.createEntity<Player>();
			localPlayer->setPos({ 200.f, 300.f });
			score -= enemy->getVertices() * 1000;

			// Send Score Packet
			ScorePacket scrpkt;
			scrpkt.score = score;
			net.sendReliable(&scrpkt, sizeof(scrpkt));
		}
	}

	// Collision between bullet and enemy and mini Enemy and only host will calculate that
	if (net.isHost)
	{
		for (auto& bullet : entManager.getByType(EntityType::Bullet))
		{
			if (!bullet->getisAlive()) continue;
			for (auto& enemy : entManager.getByType(EntityType::Enemy))
			{
				if (!enemy->getisAlive()) continue;
				if (collision.checkCollision(*bullet, *enemy)) // Only host checks collision and sends it to client so he can do that
				{
					// Bullet remove pkt
					KillEntityPacket BulletKillPkt;
					BulletKillPkt.type = (int)EntityType::Bullet;
					BulletKillPkt.id = bullet->getId();
					net.sendPacket(&BulletKillPkt, sizeof(BulletKillPkt));
					bullet->die();

					score += enemy->getVertices() * 100;

					ScorePacket scrPkt;
					scrPkt.score = score;
					net.sendReliable(&scrPkt, sizeof(scrPkt));

					float angleIncrement = 2.0f * PI / enemy->getVertices();

					// MiniEnemy Spawnner loop through sides
					for (int i = 0; i < static_cast<int>(enemy->getVertices()); i++)
					{
						float angle = i * angleIncrement;
						vec2 dir(std::cos(angle), std::sin(angle));

						// 1. Host creates entity locally (uses nextId, e.g., 50)
						auto miniEntBase = entManager.createEntity<Enemy>(
							enemy->getSpeed(),
							enemy->getSize() / 2,
							static_cast<float>(enemy->getVertices()),
							EntityType::MiniEnemy
						);

						auto miniEnemy = std::dynamic_pointer_cast<Enemy>(miniEntBase);
						miniEnemy->setPos(enemy->getPos());
						miniEnemy->setVelocity(dir * (enemy->getSpeed()));
						miniEnemy->setLifetime(10);

						SpawnPacket spawnPkt;
						spawnPkt.header.type = SPAWN_ENTITY;
						spawnPkt.type = (int)EntityType::MiniEnemy;

						spawnPkt.data.miniEnemy.id = miniEnemy->getId();
						spawnPkt.data.miniEnemy.x = miniEnemy->getPos().x;
						spawnPkt.data.miniEnemy.y = miniEnemy->getPos().y;

						spawnPkt.data.miniEnemy.vx = miniEnemy->getVelocity().x;
						spawnPkt.data.miniEnemy.vy = miniEnemy->getVelocity().y;

						spawnPkt.data.miniEnemy.radius = miniEnemy->getSize();
						spawnPkt.data.miniEnemy.sides = miniEnemy->getVertices();

						net.sendReliable(&spawnPkt, sizeof(spawnPkt));
					}

					KillEntityPacket pkt;
					pkt.type = (int)EntityType::Enemy;
					pkt.id = enemy->getId();
					net.sendReliable(&pkt, sizeof(pkt));
					enemy->die();
					break;
				}

				// MiniEnemy and bullet
				for (auto& miniEnemy : entManager.getByType(EntityType::MiniEnemy))
				{
					if (!miniEnemy->getisAlive()) continue;
					if (collision.checkCollision(*bullet, *miniEnemy))
					{
						if (net.isHost)
						{
							score += miniEnemy->getVertices() * 300;
							ScorePacket pkt;
							pkt.score = score;
							net.sendPacket(&pkt, sizeof(pkt));
						}

						KillEntityPacket MiniEnemypkt;
						MiniEnemypkt.type = (int)EntityType::MiniEnemy;
						MiniEnemypkt.id = miniEnemy->getId();
						net.sendReliable(&MiniEnemypkt, sizeof(MiniEnemypkt));

						KillEntityPacket BulletPkt;
						BulletPkt.type = (int)EntityType::Bullet;
						BulletPkt.id = bullet->getId();
						net.sendReliable(&BulletPkt, sizeof(BulletPkt));

						bullet->die();
						miniEnemy->die();
						break;
					}
				}
			}
		}
	}

	scoreText.setString("Score: " + std::to_string(score));


	// SEND MY POSITION (e.g. 60 times a sec or less)
	if (networkTick.getElapsedTime().asMilliseconds() > 15) {
		sendMyPosition();
		networkTick.restart();
	}

	// HOST: SEND CORRECTIONS 
	if (!rewindSystem.isRewinding() && net.isHost && worldSyncTick.getElapsedTime().asMilliseconds() > 100) {
		WorldStatePacket pkt;
		pkt.enemyCount = 0;
		auto enemies = entManager.getByType(EntityType::Enemy);

		for (auto& ent : enemies) {
			if (!ent->getisAlive()) continue;

			if (pkt.enemyCount < 64) {
				pkt.enemies[pkt.enemyCount].id = ent->getId();
				pkt.enemies[pkt.enemyCount].x = ent->getPos().x;
				pkt.enemies[pkt.enemyCount].y = ent->getPos().y;

				// Cast to Enemy to get velocity if needed, or just send 0 if not used yet
				pkt.enemies[pkt.enemyCount].vx = ent->getVelocity().x;
				pkt.enemies[pkt.enemyCount].vy = ent->getVelocity().y;

				pkt.enemyCount++;
			}
		}

		if (pkt.enemyCount > 0) {
			net.sendPacket(&pkt, sizeof(pkt));
		}

		worldSyncTick.restart();
	}

	entManager.update(deltaTime);
	rewindSystem.update(); // Record history
	if (g_Config.game.system.debugMode)
	{
		sf::Time dt = sf::seconds(deltaTime);
		ImGui::SFML::Update(*SceneManager::getInstance().getRenderWindow(), dt);

		// --- WINDOW 1: ENTITY MANAGER (Your Old Block) ---
		ImGui::Begin("Entity Manager");
		if (ImGui::CollapsingHeader("Entities", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto& allEntities = entManager.getAllEnt();
			for (const auto& [id, ent] : allEntities)
			{
				ImGui::PushID(id);
				ImGui::Text("ID: %d", id);

				std::string typeName = "Unknown";
				switch (ent->getType()) {
				case EntityType::Player: typeName = "Player"; break;
				case EntityType::Enemy: typeName = "Enemy"; break;
				case EntityType::Bullet: typeName = "Bullet"; break;
				case EntityType::MiniEnemy: typeName = "MiniEnemy"; break;
				default: break;
				}
				ImGui::SameLine();
				ImGui::Text("Type: %s", typeName.c_str());
				ImGui::SameLine();
				ImGui::Text("(%.1f, %.1f)", ent->getPos().x, ent->getPos().y);

				ImGui::SameLine();
				if (ImGui::Button("D")) {
					ent->die();
				}
				ImGui::PopID();
			}
		}
		ImGui::End();

		NetworkManager& net = NetworkManager::getInstance();

		ImGui::Begin("Network Stats");

		ImGui::Text("My IP: %s", net.getLocalIP().c_str());

		ImGui::Separator();

		// --- Ping (RTT) ---
		ImVec4 pingColor = ImVec4(0, 1, 0, 1); // Green
		if (net.rtt > 60) pingColor = ImVec4(1, 1, 0, 1); // Yellow
		if (net.rtt > 150) pingColor = ImVec4(1, 0, 0, 1); // Red

		ImGui::TextColored(pingColor, "Ping (RTT): %.1f ms", net.rtt);

		// --- Bandwidth ---
		ImGui::Text("Upload:   %.2f KB/s", net.uploadRate);
		ImGui::Text("Download: %.2f KB/s", net.downloadRate);

		ImGui::Separator();

		// --- Packet Loss ---
		// Color logic: 0% = Green, <5% = Yellow, >5% = Red
		ImVec4 lossColor = ImVec4(0, 1, 0, 1); // Green
		if (net.packetLoss > 0.0f) lossColor = ImVec4(1, 1, 0, 1); // Yellow (Any loss is bad)
		if (net.packetLoss > 5.0f) lossColor = ImVec4(1, 0, 0, 1); // Red (Critical)

		ImGui::TextColored(lossColor, "Reliable Loss: %.2f %%", net.packetLoss);

		// Optional: Show raw count for debugging
		 ImGui::SameLine();
		 ImGui::TextDisabled("(%d lost)", net.getLostPackets());

		ImGui::End();
	}
	SceneManager::getInstance().getRenderWindow()->clear();
}

void CoopScene::render(sf::RenderWindow& window)
{
	entManager.draw(window);
	window.draw(scoreText);
	window.draw(BackButton);
	window.draw(BackButtonText);
	if (g_Config.game.system.debugMode)
	{
		ImGui::SFML::Render(*SceneManager::getInstance().getRenderWindow());
	}
}

void CoopScene::sendMyPosition()
{
	if (!localPlayer || rewindSystem.isRewinding()) return;

	PlayerPosPacket pkt;
	pkt.header.type = PLAYER_POS;
	pkt.header.sequenceID = 0; // debug safety will be set in sendPacket
	pkt.x = localPlayer->getPos().x;
	pkt.y = localPlayer->getPos().y;
	pkt.vx = localPlayer->getVelocity().x;
	pkt.vy = localPlayer->getVelocity().y;
	pkt.tick = localTick;

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

		if (net.handleInternalPacket(buffer))
		{
			continue;
		}

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

		if (h->type == REWIND_CLEAR)
		{
			rewindSystem.clearHistory();
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
				auto entity = entManager.createEntityWithId<Bullet>(
					pkt->data.bullet.id, 
					vec2(pkt->data.bullet.x, pkt->data.bullet.y), 
					vec2(pkt->data.bullet.dx, pkt->data.bullet.dy)
				);
			}

			if (pkt->type == (int)EntityType::Enemy)
			{
				auto entity = entManager.createEntityWithId<Enemy>(
					pkt->data.enemy.id, 
					pkt->data.enemy.speed, 
					pkt->data.enemy.radius, 
					static_cast<float>(pkt->data.enemy.sides),
					EntityType::Enemy, 
					pkt->data.enemy.angle
				);
				entity->setPos(vec2(pkt->data.enemy.x, pkt->data.enemy.y));
			}

			if (pkt->type == (int)EntityType::MiniEnemy)
			{
				auto entity = entManager.createEntityWithId<Enemy>(
					pkt->data.miniEnemy.id,
					0.f, // Speed is irrelevant we set valocity manually
					pkt->data.miniEnemy.radius,
					static_cast<float>(pkt->data.miniEnemy.sides),
					EntityType::MiniEnemy,
					0.f // Angle is irrelevant bcz velocuty determines direction
				);
				entity->setPos(vec2(pkt->data.miniEnemy.x, pkt->data.miniEnemy.y));
				entity->setVelocity(vec2(pkt->data.miniEnemy.vx, pkt->data.miniEnemy.vy));
				entity->setLifetime(10);
			}
		}

		if (h->type == KILL_ENTITY)
		{
			KillEntityPacket* pkt = (KillEntityPacket*)buffer;

			if (pkt->type == (int)EntityType::Player)
			{
				int oldID = -1;
				if (remotePlayer)
				{
					oldID = remotePlayer->getId();
					remotePlayer->die(); // If a kill pkt has come that means that the remote player has collided and has died.
				}
				rewindSystem.clearHistory();

				if (oldID != -1)
				{
					remotePlayer = entManager.createEntityWithId<Player>(oldID, true); // Spawn new remote player
					remotePlayer->setPos({ 200.f, 300.f }); // No checking of enemy for now no cooldown either
				}
			}
			else // Else handle everything else (Enemy, MiniEnemy, Bullet)
			{
				auto entity = entManager.getEnt(pkt->id);
				if (entity)
				{
					entity->die();
				}
			}
		}

		if (h->type == SCORE)
		{
			ScorePacket* pkt = (ScorePacket*)buffer;
			score = pkt->score;
		}

		if (h->type == WORLD_STATE) {
			WorldStatePacket* pkt = (WorldStatePacket*)buffer;

			for (int i = 0; i < pkt->enemyCount; i++)
			{
				int id = pkt->enemies[i].id;
				auto ent = entManager.getEnt(id);

				if (ent && ent->getisAlive())
				{
					float serverX = pkt->enemies[i].x;
					float serverY = pkt->enemies[i].y;

					float dx = ent->getPos().x - serverX;
					float dy = ent->getPos().y - serverY;

					// Use squared distance to avoid slow sqrt() calls
					float distSq = dx * dx + dy * dy;

					float threshold = 1.0f;

					if (distSq > (threshold * threshold))
					{
						ent->setPos(vec2(serverX, serverY));
						ent->setVelocity(vec2(pkt->enemies[i].vx, pkt->enemies[i].vy));
					}
				}
			}
		}

		if (h->type == EXIT)
		{
			rewindSystem.pauseCapture();
			NetworkManager::getInstance().cleanup();
			SceneManager::getInstance().loadScene(SceneID::MainMenu);
		}
	}
}