#include "CoopScene.h"
#include <iostream>
#include <vector>
#include <cstring> // for memcpy

CoopScene::CoopScene()
	: entManager(EntityManager::getInstance())
	, rewindSystem(entManager, 300)
{
	NetworkManager& net = NetworkManager::getInstance();

		// --- HOST SETUP ---
		// 1. Create Host Player (Red)
		localPlayer = entManager.createEntity<Player>();
		remotePlayer = entManager.createEntity<Player>();
		if (net.isHost) {
			localPlayer->setPos({ 200.f, 300.f });
			remotePlayer->setPos({ 400.f, 300.f });
			// Host is responsible for spawning enemies, so we don't do anything else yet
		}
		else {
			// Client logic: Swap positions so we don't spawn on top of each other
			localPlayer->setPos({ 400.f, 300.f });
			remotePlayer->setPos({ 200.f, 300.f });
			remotePlayer->setColor(sf::Color(255, 255, 255, 150));
			std::cout << "[Coop] Client ready. Waiting for World State...\n";
		}
}

CoopScene::~CoopScene()
{
	entManager.clearAll();
	rewindSystem.clearHistory();
}

void CoopScene::handleEvent(const sf::Event& event)
{
	// Pause menu logic here if needed
}

void CoopScene::update(float deltaTime)
{
	NetworkManager& net = NetworkManager::getInstance();
	handleNetworking();
	if (rewindSystem.isRewinding()) {
		rewindSystem.update();
		return;
	}

	if (net.isHost) {
		// Run your normal enemy spawner logic here.
		// When you create an enemy, you must Send a Packet!
		// Example:
		// auto enemy = entManager.createEntity<Enemy>(...);
		// sendSpawnPacket(enemy);
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
			else 
			{

			}
		}

		else if (h->type == SPAWN_ENTITY && !net.isHost)
		{
			// Client reads packet: "Spawn Enemy at 100,100"
			// entManager.createEntity<Enemy>(... set pos to 100,100 ...);
		}

		else if (h->type == WORLD_STATE && !net.isHost) {
			// Snap enemies to correct pos
		}
	}
}