#include "CooPScene.h"

CoopScene::CoopScene()
	:entManager(EntityManager::getInstance())
	, rewindSystem(entManager, 300)
{
	NetworkManager& net = NetworkManager::getInstance();

	if (net.isHost)
	{
		localPlayer = entManager.createEntity<Player>(); // Id = 9
		remotePlayer = entManager.createEntity<Player>(); // id = 1
	}
	else
	{
		// Wait for the host to temm em where things are
	}
}

CoopScene::~CoopScene()
{
	entManager.clearAll();
	rewindSystem.clearHistory();
}

void CoopScene::handleEvent(const sf::Event& event)
{
	// Basic input logic (pause etc)
}

void CoopScene::update(float deltaTime)
{
	NetworkManager& net = NetworkManager::getInstance();

	if (net.isHost)
	{
		processHostLogic(deltaTime);
	}
	else
	{
		processClientLogic(deltaTime);
	}
}

// Host Logic
void CoopScene::processHostLogic(float dt)
{
	NetworkManager& net = NetworkManager::getInstance();

	// Recive Input
	char buffer[1024];
	sockaddr_in sender;
	int bytes = net.receivePacket(buffer, 1024, sender);
	if (bytes > 0)
	{
		PacketHeader* h = (PacketHeader*)buffer;
		if (h->type == PKT_INPUT && remotePlayer)
		{
			InputPacket* pkt = (InputPacket*)buffer;

			// Apply input to remote player
			// Helper fn like remotePlayer->applyInput(pkt->w, pkt>a)

			if (pkt->rewind) rewindSystem.triggerRewind(); // No timer checks for now
		}
	}

	// Normal game logic like collision spawning etc

	entManager.update(dt);
	rewindSystem.update();

	if (networkTick.getElapsedTime().asMilliseconds() > 33)
	{
		std::vector<char> packet;
		PacketHeader head{ WORLD_STATE };
		// append header
		// append count
		// append all ent states

		net.sendPacket(packet.data(), packet.size());
		networkTick.restart();
	}
}

// Client logic
void CoopScene::processClientLogic(float dt)
{
	NetworkManager& net = NetworkManager::getInstance();

	// Send Client input
	InputPacket pkt;
	pkt.w = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
	pkt.a = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
	pkt.s = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
	pkt.d = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
	pkt.space = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
	pkt.rewind = sf::Keyboard::isKeyPressed(sf::Keyboard::R);

	net.sendPacket(&pkt, sizeof(pkt));

	// Recv world state
	char buffer[4096];
	sockaddr_in sender;
	int bytes = net.receivePacket(buffer, 4096, sender);

	if (bytes > 0)
	{
		PacketHeader* h = (PacketHeader*)buffer;
		if (h->type == WORLD_STATE)
		{
			// This function wipes current positions and applies the host's data
			// syncEntitiesFromNetwork(buffer, bytes);
		}
	}
	// Client does NOT run entManager.update(dt) for physics!
	// Client only runs simple visual updates (particles, etc)
}

void CoopScene::render(sf::RenderWindow& window)
{
	entManager.draw(window);
}