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

void NetworkManager::startHosting(int port, const std::string& interfaceIP)
{
	reset();
	isHost = true;
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	//addr.sin_addr.s_addr = INADDR_ANY; //Broadcastring on Virtual Adapter
	inet_pton(AF_INET, interfaceIP.c_str(), &addr.sin_addr);

	if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
	}
}

void NetworkManager::startClient(int port, const std::string& interfaceIP)
{
	reset();
	isHost = false;
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	//addr.sin_addr.s_addr = INADDR_ANY;
	// FORCE BIND TO SPECIFIC IP
	inet_pton(AF_INET, interfaceIP.c_str(), &addr.sin_addr);

	if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
	}
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
	if (connected || destAddr.sin_port != 0)
		sendto(sock, (char*)data, size, 0, (sockaddr*)&destAddr, sizeof(destAddr));
}

void NetworkManager::sendTo(void* data, int size, sockaddr_in& target) {
	sendto(sock, (char*)data, size, 0, (sockaddr*)&target, sizeof(target));
}

int NetworkManager::receivePacket(char* buffer, int size, sockaddr_in& sender)
{
	int len = sizeof(sender);
	return recvfrom(sock, buffer, size, 0, (sockaddr*)&sender, &len);
}

void NetworkManager::reset()
{
	if (sock != INVALID_SOCKET)
	{
		cleanup();
	}
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	char broadcast = '1';
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

	u_long mode = 1;
	ioctlsocket(sock, FIONBIO, &mode);

	connected = false;
	isHost = false;
}

void NetworkManager::cleanup() const
{
	closesocket(sock);
	WSACleanup();
}

std::string NetworkManager::getLocalIP()
{
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
	{
		return "Error getting the hostname";
	}
	struct addrinfo hints = { 0 } , * info = nullptr;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(hostname, nullptr, &hints, &info) != 0)
	{
		return "Error resolving hostname";
	}

	char ipStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &((struct sockaddr_in*)info->ai_addr)->sin_addr, ipStr, INET_ADDRSTRLEN);

	freeaddrinfo(info);
	return std::string(ipStr);
}