#pragma once
#include "Scene.hpp"
#include "../NetworkManager.h"
#include "../AssetHandler.h"
#include "../SceneManager.h"
#include <iostream>
#include <sstream>
#include <imgui.h>
#include <imgui-SFML.h>
#include <vector>
#include <string>
#include <deque> // Efficient for logs

class SceneManager;
class EntityManager;
class AssetHandler;

class LobbyScene : public Scene
{
public:
    LobbyScene();
    ~LobbyScene();

    void handleEvent(const sf::Event& event) override;
    void update(float deltatime) override;
    void render(sf::RenderWindow& window) override;

private:
    AssetHandler& assetHandler;
    SceneManager& sceneManager; // Added reference
    sf::Font font;

    bool isHosting = false;
    bool isSearching = false;
    std::string myIP;
    std::string statusMessage = "IDLE";
    float beaconTimer = 0.0f;

    struct FoundServer {
        std::string ip;
        sf::Clock lastSeen;
        sockaddr_in addr;
    };
    std::vector<FoundServer> foundServers;

    // --- Logs ---
    std::deque<std::string> debugLogs;
    void addLog(const std::string& msg);

    // --- Logic Functions ---
    bool updateHostLogic(float dt);
    bool updateClientLogic(float dt);
    void initUI();

    sf::Text titleText;
    sf::Text statusText;

    ModernButton btnHost;
    ModernButton btnJoin; // This is "Find Game"
    ModernButton btnBack;
    ModernButton btnCancel;
};