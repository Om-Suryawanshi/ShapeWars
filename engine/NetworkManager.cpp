#include "NetworkManager.h"

NetworkManager& NetworkManager::getInstance()
{
    static NetworkManager instance;
    return instance;
}

void NetworkManager::init()
{
    reset();
}

void NetworkManager::reset()
{
    if (sock != INVALID_SOCKET)
        cleanup();

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa); 

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    char broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    connected = false;
    isHost = false;

    reliableOutbox.clear();
    receivedReliable.clear();
    localReliableSeq = 0;

    lastStatTime = std::chrono::steady_clock::now();
}

void NetworkManager::cleanup() const
{
    closesocket(sock);
    WSACleanup();
}


void NetworkManager::startHosting(int port, const std::string& ip)
{
    reset();
    isHost = true;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    bind(sock, (sockaddr*)&addr, sizeof(addr));
}

void NetworkManager::startClient(int port, const std::string& ip)
{
    reset();
    isHost = false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    bind(sock, (sockaddr*)&addr, sizeof(addr));
}


void NetworkManager::sendSearchReq()
{
    sockaddr_in bcast{};
    bcast.sin_family = AF_INET;
    bcast.sin_port = htons(5000); // We know Host is on 5000
    bcast.sin_addr.s_addr = INADDR_BROADCAST;
    PacketHeader p{ SEARCH_REQ };
    sendto(sock, (char*)&p, sizeof(p), 0, (sockaddr*)&bcast, sizeof(bcast));
}

void NetworkManager::sendPacket(void* data, int size)
{
    PacketHeader* h = (PacketHeader*)data;
    h->flags = PKT_NONE;
    h->sequenceID = 0;

    sendto(sock, (char*)data, size, 0,
        (sockaddr*)&destAddr, sizeof(destAddr));

    bytesSentAccumulator += size;
}

void NetworkManager::sendReliable(void* data, int size)
{
    PacketHeader* h = (PacketHeader*)data;
    h->flags = PKT_RELIABLE;
    h->sequenceID = ++localReliableSeq;

    ReliablePacket rp;
    rp.sequence = h->sequenceID;
    rp.data.assign((char*)data, (char*)data + size);
    rp.lastSent = std::chrono::steady_clock::now();
    rp.resendCount = 0;

    reliableOutbox[rp.sequence] = rp;

    sendto(sock, rp.data.data(), static_cast<int>(rp.data.size()), 0,
        (sockaddr*)&destAddr, sizeof(destAddr));

    bytesSentAccumulator += size;
}

void NetworkManager::sendAck(uint32_t seq)
{
    AckPacket ack{};
    ack.header.type = ACK;
    ack.header.flags = PKT_ACK;
    ack.header.sequenceID = 0;
    ack.ackSequence = seq;

    sendto(sock, (char*)&ack, sizeof(ack), 0,
        (sockaddr*)&destAddr, sizeof(destAddr));
}


int NetworkManager::receivePacket(char* buffer, int size, sockaddr_in& sender)
{
    int len = sizeof(sender);
    int bytes = recvfrom(sock, buffer, size, 0,
        (sockaddr*)&sender, &len);

    if (bytes > 0)
        bytesRecvAccumulator += bytes;

    return bytes;
}


bool NetworkManager::handleInternalPacket(char* buffer)
{
    PacketHeader* h = (PacketHeader*)buffer;

    // ACK handling
    if (h->flags & PKT_ACK)
    {
        AckPacket* ack = (AckPacket*)buffer;
        reliableOutbox.erase(ack->ackSequence);
        return true;
    }

    // Reliable receive
    if (h->flags & PKT_RELIABLE)
    {
        if (receivedReliable.count(h->sequenceID))
        {
            sendAck(h->sequenceID);
            return true;
        }

        receivedReliable.insert(h->sequenceID);
        sendAck(h->sequenceID);

        // Packet loss tracking
        if (h->sequenceID > lastRemoteReliableSeq)
        {
            lostPackets += (h->sequenceID - lastRemoteReliableSeq - 1);
            lastRemoteReliableSeq = h->sequenceID;
        }

        receivedPackets++;
    }

    // Ping/Pong
    if (h->type == PING)
    {
        PingPacket* in = (PingPacket*)buffer;
        PingPacket out{};
        out.header.type = PONG;
        out.timestamp = in->timestamp;
        sendPacket(&out, sizeof(out));
        return true;
    }

    if (h->type == PONG)
    {
        PingPacket* pkt = (PingPacket*)buffer;
        double now = std::chrono::duration<double>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        rtt = float((now - pkt->timestamp) * 1000.0);
        return true;
    }

    return false;
}

void NetworkManager::sendTo(void* data, int size, sockaddr_in& target) {
    sendto(sock, (char*)data, size, 0, (sockaddr*)&target, sizeof(target));
}


void NetworkManager::update()
{
    auto now = std::chrono::steady_clock::now();

    // Reliable Packet Resend Logic
    // This loops through the 'outbox' of packets we sent but haven't received an ACK for yet.
    for (auto& [seq, pkt] : reliableOutbox)
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - pkt.lastSent).count();

        // If 200ms have passed and no ACK, resend it!
        if (elapsed > 200 && pkt.resendCount < 5)
        {
            sendto(sock, pkt.data.data(), static_cast<int>(pkt.data.size()), 0,
                (sockaddr*)&destAddr, sizeof(destAddr));

            pkt.lastSent = now;
            pkt.resendCount++;

            // Optional: Count this as "extra bytes sent" for stats
            bytesSentAccumulator += static_cast<int>(pkt.data.size());
        }
    }
}

void NetworkManager::updateStats()
{
    auto now = std::chrono::steady_clock::now();

    // Only recalculate stats once every 1 second (1000ms)
    if (std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastStatTime).count() >= 1000)
    {
        // 1. Calculate Rates (KB/s)
        uploadRate = bytesSentAccumulator / 1024.0f;
        downloadRate = bytesRecvAccumulator / 1024.0f;

        // 2. Calculate Packet Loss (Reliable only)
        uint32_t total = receivedPackets + lostPackets;
        packetLoss = (total > 0) ? (lostPackets * 100.0f / total) : 0.0f;

        // 3. Reset Counters for the next second
        bytesSentAccumulator = 0;
        bytesRecvAccumulator = 0;
        receivedPackets = 0;
        lostPackets = 0;

        lastStatTime = now;

        sendPingPacket();
    }
}

void NetworkManager::sendPingPacket()
{
    PingPacket pkt{};
    pkt.header.type = PING;
    pkt.timestamp = std::chrono::duration<double>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    sendPacket(&pkt, sizeof(pkt));
}

std::string NetworkManager::getLocalIP()
{
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    addrinfo hints{}, *info = nullptr;
    hints.ai_family = AF_INET;
    if (getaddrinfo(hostname, nullptr, &hints, &info) != 0 || info == nullptr)
        return "0.0.0.0";

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &((sockaddr_in*)info->ai_addr)->sin_addr, ip, sizeof(ip));

    freeaddrinfo(info);
    return ip;
}

int NetworkManager::getLostPackets()
{
    return lostPackets;
}