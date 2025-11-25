#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

// Link library
#pragma comment(lib, "ws2_32.lib")

// --- PACKET STRUCTURES ---
// We use 'push' and 'pop' to ensure data is sent byte-by-byte without padding
#pragma pack(push, 1) 

// Renamed INPUT to PKT_INPUT to fix conflict with Windows API
enum PacketType {
    SEARCH_REQ = 0, // Client asks: "Any hosts?"
    SEARCH_RES = 1, // Host replies: "I am here!"
    JOIN_REQ = 2,
    JOIN_ACK = 3,
    PKT_INPUT = 4,
    WORLD_STATE = 5
};

struct PacketHeader {
    int type;
};

struct InputPacket {
    PacketHeader header = { PKT_INPUT };
    bool w, a, s, d, space, rewind;
};

// Simplified Entity State for network sync
struct EntityState {
    int id;
    int type;
    float x, y;
    float vx, vy;
    bool isAlive;
};
#pragma pack(pop)

class NetworkManager {
public:
    static NetworkManager& getInstance();

    SOCKET sock = INVALID_SOCKET;;
    sockaddr_in destAddr; // The other player's address
    bool connected = false;
    bool isHost = false;

    void init();
    void startHosting(int port, const std::string& interfaceIP);
    void startClient(int port, const std::string& interfaceIP);
    void sendSearchReq();
    void sendPacket(void* data, int size);
    void sendTo(void* data, int size, sockaddr_in& target); // Helper to reply to a specific sender (for Discovery)
    int receivePacket(char* buffer, int size, sockaddr_in& sender);
    void cleanup() const;
    void reset();
    std::string getLocalIP();

private:
    NetworkManager() {} // Private constructor for Singleton
};