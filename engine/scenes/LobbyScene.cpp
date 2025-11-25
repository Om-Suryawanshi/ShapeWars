#include "LobbyScene.h"

LobbyScene::LobbyScene()
{

	NetworkManager& net = NetworkManager::getInstance();
	net.init();
	myIP = net.getLocalIP();
	addLog("Network Initialized.");
	addLog("Local IP detected: " + myIP);
}

LobbyScene::~LobbyScene()
{

}

void LobbyScene::addLog(const std::string& msg)
{
	debugLogs.push_back(msg);
	if (debugLogs.size() > 50)
	{
		debugLogs.pop_front();
	}
	std::cout << "[Lobby]" << msg << std::endl;
}

void LobbyScene::handleEvent(const sf::Event& event)
{
	ImGui::SFML::ProcessEvent(event);
}

void LobbyScene::update(float deltatime)
{
	sf::RenderWindow* win = SceneManager::getInstance().getRenderWindow(); 
	ImGui::SFML::Update(*win, sf::seconds(deltatime));
	NetworkManager& net = NetworkManager::getInstance();

	if (isHosting)
	{
		updateHostLogic(deltatime);
	}
	else if (isSearching)
	{
		updateClientLogic(deltatime);
	}
	ImGui::Begin("Co-op Lobby", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::TextColored(ImVec4(0, 1, 0, 1), "MY IP: %s", myIP.c_str());
	ImGui::Text("Status: %s", statusMessage.c_str());
	ImGui::Separator();

	if (!isHosting && !isSearching)
	{
		if (ImGui::Button("Host Game", ImVec2(200, 50)))
		{
			isHosting = true;
			statusMessage = "Broadcasting beacon...";
			net.startHosting(5000, myIP);
			addLog("Started Hosting on Port 5000");
		}

		if (ImGui::Button("Find Game", ImVec2(200, 50)))
		{
			isSearching = true;
			statusMessage = "Scanning for hosts...";
			net.startClient(0, myIP);
			addLog("Client Mode Started. Listening for Beacons...");
		}

		if (ImGui::Button("Back", ImVec2(200, 30)))
		{
			SceneManager::getInstance().loadScene(SceneID::MainMenu);
		}
	}
	else if (isHosting)
	{
		ImGui::Text("Hosting on port 5000");
		ImGui::Text("Waiting for player to join...");

		if (ImGui::Button("Stop Hosting"))
		{
			isHosting = false;
			net.reset();
			statusMessage = "Stopped.";
			addLog("Hosting stopped.");
		}
	}
	else if (isSearching)
	{
		if (foundServers.empty())
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Searching... (No beacons yet)");
		}
		else
		{
			ImGui::Text("Found Servers:");
			for (const auto& server : foundServers)
			{
				std::string label = "Join " + server.ip;
				if (ImGui::Button(label.c_str()))
				{
					net.destAddr = server.addr;

					PacketHeader pkt;
					pkt.type = JOIN_REQ;
					net.sendPacket(&pkt, sizeof(pkt));

					statusMessage = "Request sent to " + server.ip + "...";
					addLog("Sent JOIN_REQ to " + server.ip);
				}
			}
		}
		if (ImGui::Button("Cancel"))
		{
			isSearching = false;
			net.reset();
			foundServers.clear();
			addLog("Cancelled search.");
		}
	}
	ImGui::Separator();

	ImGui::Text("Network Logs:");
	ImGui::BeginChild("LogRegion", ImVec2(400, 150), true); // Scrollable region
	for (const auto& log : debugLogs) {
		ImGui::TextWrapped("%s", log.c_str());
	}
	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		ImGui::SetScrollHereY(1.0f); // Auto-scroll to bottom
	ImGui::EndChild();
	ImGui::End();
}

void LobbyScene::updateHostLogic(float dt)
{
	NetworkManager& net = NetworkManager::getInstance();
	
	char buffer[256];
	sockaddr_in sender;
	int bytes = net.receivePacket(buffer, 256, sender);

	if (bytes > 0)
	{
		// Debug
		char ipStr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(sender.sin_addr), ipStr, INET_ADDRSTRLEN);
		std::string senderIP(ipStr);
		// debug
		PacketHeader* h = (PacketHeader*)buffer;
		//debug
		std::stringstream ss;
		ss << "Recv Packet Type " << h->type << " from " << senderIP;
		addLog(ss.str());
		//debug 

		if (h->type == SEARCH_REQ)
		{
			PacketHeader resp;
			resp.type = SEARCH_RES;
			net.sendTo(&resp, sizeof(resp), sender);
			addLog("Replied to Search Req from " + senderIP);
		}
		else if (h->type == JOIN_REQ)
		{
			addLog("--> JOIN REQUEST FROM " + senderIP);
			net.destAddr = sender;
			net.connected = true;

			PacketHeader ack; ack.type = JOIN_ACK;
			net.sendPacket(&ack, sizeof(ack));
			addLog("Sent JOIN_ACK. Starting Game...");

			SceneManager::getInstance().loadScene(SceneID::COOP);
		}
	}
}

void LobbyScene::updateClientLogic(float dt)
{
	NetworkManager& net = NetworkManager::getInstance();

	//Client Broadcasts "IS ANYONE THERE?" every 1 second
	beaconTimer += dt;
	if (beaconTimer >= 1.0f)
	{
		net.sendSearchReq(); // Broadcasts to Port 5000
		beaconTimer = 0.0f;
		 addLog("Sent Search Request...");
	}

	char buffer[256];
	sockaddr_in sender;
	int bytes = net.receivePacket(buffer, 256, sender);

	if (bytes > 0)
	{
		char ipStr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(sender.sin_addr), ipStr, INET_ADDRSTRLEN);
		std::string senderIP(ipStr);

		PacketHeader* h = (PacketHeader*)buffer;

		if (h->type == SEARCH_RES)
		{
			bool exists = false;
			for (auto& s : foundServers) {
				if (s.ip == senderIP) {
					s.lastSeen.restart();
					exists = true;
					break;
				}
			}
			if (!exists) {
				FoundServer newServer;
				newServer.ip = senderIP;
				newServer.addr = sender;
				foundServers.push_back(newServer);
				addLog("Found Host: " + senderIP);
			}
		}
		else if (h->type == JOIN_ACK)
		{
			addLog("Received JOIN_ACK! Starting Game...");
			net.connected = true;
			SceneManager::getInstance().loadScene(SceneID::COOP);
		}
	}
}

void LobbyScene::render(sf::RenderWindow& window)
{
	ImGui::SFML::Render(window);
}