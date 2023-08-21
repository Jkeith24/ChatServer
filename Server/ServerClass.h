#pragma once

#include "platform.h"


class ServerClass
{

public:
	
	//std::vector<Clients> clientSockets;
	std::vector<std::string> messagesSent;
	SOCKET TCPServerSocket;
	SOCKET UDPserverSocket;


	const int MAX_CLIENTS = 10;
	const int BUFFER_SIZE = 1024;

	//Commands
	std::string registerCommand = "$register";


	int TCPInit(uint16_t port);
	int UDPInit(uint16_t port);

	bool parseRegisterCommand(const std::string& message, std::string& username);
	bool parseGetListCommand(const std::string& message);
	bool parseGetLogCommand(const std::string& message);
	bool runSockets = true;

	int readMessage(SOCKET _socket, char* buffer, int32_t size);
	int sendMessage(SOCKET _socket, char* data, int32_t length);

	int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length);
	
	int tcp_recv_whole(SOCKET s, char* buf, int len);

};

