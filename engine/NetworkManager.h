#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <unordered_set>

#pragma comment(lib, "ws2_32.lib")

#pragma pack(push, 1)


enum PacketType
{
    SEARCH_REQ = 0,
    SEARCH_RES = 1,
    JOIN_REQ = 2,
    JOIN_ACK = 3,
    PLAYER_POS = 4,
    SPAWN_ENTITY = 5,
    KILL_ENTITY = 6,
    REWIND_EVENT = 7,
    REWIND_CLEAR = 8,
    WORLD_STATE = 9,
    PAUSE = 10,
    SCORE = 11,
    PING = 12,
    PONG = 13,
    ACK = 14
};

enum PacketFlags
{
    PKT_NONE = 0,
    PKT_RELIABLE = 1 << 0,
    PKT_ACK = 1 << 1
};

struct PacketHeader
{
    uint8_t  type;
    uint8_t  flags;
    uint16_t reserved;
    uint32_t sequenceID; // USED ONLY FOR RELIABLE PACKETS
};

struct PingPacket
{
    PacketHeader header;
    double timestamp;
};

struct AckPacket
{
    PacketHeader header;
    uint32_t ackSequence;
};

struct PausePacket
{
    PacketHeader header = { PAUSE };
    bool newPauseState; // true = paused, false = unpaused
};

struct PlayerPosPacket
{
    PacketHeader header = { PLAYER_POS };
    float x, y;
    float vx, vy;
    uint32_t tick; // Simulation tick
};

struct RewindPacket
{
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

// Enemy Drift fix
struct EnemyStateData {
    int id;
    float x, y;
    float vx, vy; // Velocity is helpful for smoothing, but optional
};

struct WorldStatePacket
{
    PacketHeader header = { WORLD_STATE };
    int enemyCount;
    EnemyStateData enemies[64];
};

#pragma pack(pop)


class NetworkManager
{
public:
    static NetworkManager& getInstance();

    void init();
    void reset();
    void cleanup() const;

    void startHosting(int port, const std::string& interfaceIP);
    void startClient(int port, const std::string& interfaceIP);
    void sendTo(void* data, int size, sockaddr_in& target); // Helper to reply to a specific sender (for Discovery)
    void sendSearchReq();

    void sendPacket(void* data, int size);       // Unreliable
    void sendReliable(void* data, int size);     // Reliable
    void sendAck(uint32_t seq);

    int  receivePacket(char* buffer, int size, sockaddr_in& sender);

    // Debug / Stats
    void update(); // resends reliable packets if lost in transmission
    void updateStats(); // Updates ping pkt loss etc
    void sendPingPacket();
    bool handleInternalPacket(char* buffer);

    std::string getLocalIP();
    int getLostPackets();

    float rtt = 0.0f;
    float packetLoss = 0.0f;
    float uploadRate = 0.0f;
    float downloadRate = 0.0f;

    SOCKET sock = INVALID_SOCKET;
    sockaddr_in destAddr{};
    bool connected = false;
    bool isHost = false;

private:
    NetworkManager() {}

    // Reliable send tracking
    struct ReliablePacket
    {
        std::vector<char> data;
        uint32_t sequence;
        std::chrono::steady_clock::time_point lastSent;
        int resendCount;
    };

    std::unordered_map<uint32_t, ReliablePacket> reliableOutbox;
    std::unordered_set<uint32_t> receivedReliable; // duplicate suppression

    uint32_t localReliableSeq = 0;

    // Debug stats
    int bytesSentAccumulator = 0;
    int bytesRecvAccumulator = 0;

    uint32_t lastRemoteReliableSeq = 0;
    uint32_t lostPackets = 0;
    uint32_t receivedPackets = 0;

    std::chrono::steady_clock::time_point lastStatTime;
};
