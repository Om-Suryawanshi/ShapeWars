#include "LobbyScene.h"

LobbyScene::LobbyScene()
    : assetHandler(AssetHandler::getInstance())
    , sceneManager(SceneManager::getInstance())
{
    font = assetHandler.getFont("mainFont");
    NetworkManager& net = NetworkManager::getInstance();
    net.init();
    myIP = net.getLocalIP();

    addLog("Network Initialized.");
    addLog("Local IP detected: " + myIP);

    initUI();
}

LobbyScene::~LobbyScene()
{
}

void LobbyScene::initUI()
{
    // Use safe defaults if window isn't ready
    float width = 1600;
    float height = 800;
    if (sceneManager.getRenderWindow()) {
        width = (float)sceneManager.getRenderWindow()->getSize().x;
        height = (float)sceneManager.getRenderWindow()->getSize().y;
    }
    float centerX = width / 2.0f;

    // Title
    titleText.setFont(font);
    titleText.setString("CO-OP LOBBY");
    titleText.setCharacterSize(60);
    titleText.setFillColor(sf::Color::White);
    titleText.setOutlineColor(sf::Color(0, 100, 100));
    titleText.setOutlineThickness(3);

    sf::FloatRect tRect = titleText.getLocalBounds();
    titleText.setOrigin(tRect.left + tRect.width / 2.0f, tRect.top);
    titleText.setPosition(centerX, 50);

    // Status Text
    statusText.setFont(font);
    statusText.setCharacterSize(24);
    statusText.setFillColor(sf::Color(0, 255, 255)); // Cyan

    // Buttons
    float startY = 400;
    btnHost.init(font, "HOST GAME", { 300, 60 }, { centerX, startY });
    btnJoin.init(font, "FIND GAME", { 300, 60 }, { centerX, startY + 80 });
    btnBack.init(font, "BACK", { 150, 40 }, { 100, 50 });

    // Cancel button (Positioned at bottom)
    btnCancel.init(font, "CANCEL / STOP", { 300, 60 }, { centerX, height - 100 });
}

void LobbyScene::addLog(const std::string& msg)
{
    debugLogs.push_back(msg);
    if (debugLogs.size() > 50) {
        debugLogs.pop_front();
    }
    std::cout << "[Lobby]" << msg << std::endl;
}

void LobbyScene::handleEvent(const sf::Event& event)
{
    // 1. Debug UI
    if (g_Config.game.system.debugMode) {
        ImGui::SFML::ProcessEvent(event);
    }

    // CRASH FIX: Check window validity
    sf::RenderWindow* win = sceneManager.getRenderWindow();
    if (!win) return;

    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(*win);
    NetworkManager& net = NetworkManager::getInstance();

    // 2. Button Inputs
    if (!isHosting && !isSearching)
    {
        if (btnHost.isClicked(event, mousePos)) {
            isHosting = true;
            statusMessage = "Broadcasting beacon...";
            net.startHosting(5000, myIP);
            addLog("Started Hosting on Port 5000");
        }
        else if (btnJoin.isClicked(event, mousePos)) {
            isSearching = true;
            statusMessage = "Scanning for hosts...";
            net.startClient(0, myIP);
            addLog("Client Mode Started. Listening for Beacons...");
        }
        else if (btnBack.isClicked(event, mousePos)) {
            sceneManager.loadScene(SceneID::MainMenu);
            return; // CRASH FIX: Stop executing immediately after loadScene
        }
    }
    else // If Hosting or Searching
    {
        if (btnCancel.isClicked(event, mousePos)) {
            isHosting = false;
            isSearching = false;
            net.reset();
            foundServers.clear();
            statusMessage = "IDLE";
            addLog("Cancelled operation.");
        }

        // Handle joining specific server (Server List Clicks)
        if (isSearching) {
            float yOffset = 250;
            float centerX = win->getSize().x / 2.0f;

            for (const auto& server : foundServers) {
                // We define a click area for the server card
                sf::FloatRect cardArea(centerX - 250, yOffset, 500, 50);

                if (cardArea.contains(mousePos) && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    net.destAddr = server.addr;
                    PacketHeader pkt;
                    pkt.type = JOIN_REQ;
                    net.sendPacket(&pkt, sizeof(pkt));
                    statusMessage = "Request sent to " + server.ip + "...";
                    addLog("Sent JOIN_REQ to " + server.ip);
                }
                yOffset += 60;
            }
        }
    }
}

void LobbyScene::update(float deltatime)
{
    sf::RenderWindow* win = sceneManager.getRenderWindow();
    if (!win) return;

    // Debug UI
    if (g_Config.game.system.debugMode) {
        ImGui::SFML::Update(*win, sf::seconds(deltatime));

        ImGui::Begin("Debug Network Logs", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("My IP: %s", myIP.c_str());
        ImGui::Separator();
        ImGui::BeginChild("LogRegion", ImVec2(400, 150), true);
        for (const auto& log : debugLogs) ImGui::TextWrapped("%s", log.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();
        ImGui::End();
    }

    // Button Hover Updates
    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(*win);
    if (!isHosting && !isSearching) {
        btnHost.update(mousePos);
        btnJoin.update(mousePos);
        btnBack.update(mousePos);
    }
    else {
        btnCancel.update(mousePos);
    }

    // Logic Updates (Using your original logic)
    if (isHosting)
    {
        if (updateHostLogic(deltatime)) return; // CRASH FIX: Return if scene changed
    }
    else if (isSearching)
    {
        if (updateClientLogic(deltatime)) return; // CRASH FIX: Return if scene changed
    }

    // Update Status Text position
    statusText.setString("Status: " + statusMessage);
    sf::FloatRect sRect = statusText.getLocalBounds();
    statusText.setOrigin(sRect.left + sRect.width / 2.0f, sRect.top);
    statusText.setPosition(win->getSize().x / 2.0f, 150);
}

void LobbyScene::render(sf::RenderWindow& window)
{
    window.draw(titleText);
    window.draw(statusText);

    if (!isHosting && !isSearching) {
        btnHost.render(window);
        btnJoin.render(window);
        btnBack.render(window);
    }
    else {
        btnCancel.render(window);

        // Render Server List as Cards
        if (isSearching) {
            float yOffset = 250;
            float centerX = window.getSize().x / 2.0f;
            sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);

            for (const auto& server : foundServers) {
                // Card Shape
                sf::RectangleShape panel({ 500, 50 });
                panel.setOrigin(250, 0);
                panel.setPosition(centerX, yOffset);

                // Hover Effect
                if (panel.getGlobalBounds().contains(mousePos)) {
                    panel.setFillColor(sf::Color(50, 50, 70, 255));
                    panel.setOutlineColor(sf::Color::Cyan);
                }
                else {
                    panel.setFillColor(sf::Color(20, 20, 30, 200));
                    panel.setOutlineColor(sf::Color::Green);
                }
                panel.setOutlineThickness(2);

                // Text
                sf::Text serverTxt;
                serverTxt.setFont(font);
                serverTxt.setString("HOST: " + server.ip + "  [CLICK TO JOIN]");
                serverTxt.setCharacterSize(20);
                serverTxt.setFillColor(sf::Color::White);

                sf::FloatRect tr = serverTxt.getLocalBounds();
                serverTxt.setOrigin(tr.left + tr.width / 2.0f, tr.top + tr.height / 2.0f);
                serverTxt.setPosition(centerX, yOffset + 25);

                window.draw(panel);
                window.draw(serverTxt);
                yOffset += 60;
            }
        }
    }

    if (g_Config.game.system.debugMode) {
        ImGui::SFML::Render(window);
    }
}

bool LobbyScene::updateHostLogic(float dt)
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
        if (g_Config.game.system.debugMode) addLog(ss.str());
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
            return true;
        }
    }
    return false;
}

bool LobbyScene::updateClientLogic(float dt)
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
            return true;
        }
    }
    return false;
}