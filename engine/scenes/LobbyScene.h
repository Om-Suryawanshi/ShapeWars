#pragma once
#include "Scene.hpp"
#include "../NetworkManager.h"
#include "../SceneManager.h"
#include <iostream>
#include <sstream>
#include "imgui.h"
#include "imgui-SFML.h"
#include <vector>
#include <string>
#include <deque> // Efficient for logs

class LobbyScene : public Scene {
    bool isHosting = false;
    bool isSearching = false;

    // IP Display
    std::string myIP;

    // Debugging
    std::deque<std::string> debugLogs;
    void addLog(const std::string& msg);

    // Hosting Vars
    float beaconTimer = 0.0f;
    std::string statusMessage = "Select an option";

    // Client Vars
    struct FoundServer {
        std::string ip;
        sockaddr_in addr;
        sf::Clock lastSeen;
    };
    std::vector<FoundServer> foundServers;

public:
    LobbyScene();
    ~LobbyScene();

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    void handleEvent(const sf::Event& event) override;

private:
    void updateHostLogic(float dt);
    void updateClientLogic(float dt);
};