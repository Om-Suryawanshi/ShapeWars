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
    SEARCH_REQ = 0,     // Client asks: "Any hosts?"
    SEARCH_RES = 1,     // Host replies: "I am here!"
    JOIN_REQ = 2,
    JOIN_ACK = 3,
    PLAYER_POS = 4,     // "I am at X,Y"
    SPAWN_ENTITY = 5,   // "Spawn Enemy at X,Y"
    KILL_ENTITY = 6,
    REWIND_EVENT = 7,   // "Start/Stop Rewinding"
    REWIND_CLEAR = 8,
    WORLD_STATE = 9,    // "Correction data"
    PAUSE = 10,           // true = paused, false = not paused
    SCORE = 11
};

struct PacketHeader {
    int type;
};

struct PausePacket {
    PacketHeader header = { PAUSE };
    bool newPauseState; // true = paused, false = unpaused
};

struct PlayerPosPacket {
    PacketHeader header = { PLAYER_POS };
    float x, y;
    float vx, vy;
};

struct RewindPacket {
    PacketHeader header = { REWIND_EVENT };
    bool isRewinding; // true = start, false = stop
};

struct BulletData
{
    int id;
    float x, y;
    float dx, dy; // Direction in which the bullet is fired
};

struct EnemyData
{
    int id;
    float x, y;
    float speed;
    float radius;
    int sides;
    float angle;
};

struct MiniEnemyData
{
    int id;
    float x, y;
    float vx, vy;
    float radius;
    int sides;
};

struct SpawnPacket
{
    PacketHeader header = { SPAWN_ENTITY };
    int type;

    union {
        BulletData bullet;
        EnemyData enemy;
        MiniEnemyData miniEnemy;
    }data;
};

struct KillEntityPacket
{
    PacketHeader header = { KILL_ENTITY };
    int type;
    int id;
};

struct ScorePacket
{
    PacketHeader header = { SCORE };
    int score;
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