#pragma once
#include <string>
#include <vector>
#include "platform.h"


class Clients
{
public:

	//I think each client has its own server socket

	SOCKET clientSocket;

	
	std::string Username;

	bool isRegistered;

	Clients(SOCKET sock) : clientSocket(sock), isRegistered(false) {}
	

};

