#include "ServerClass.h"

int ServerClass::TCPInit(uint16_t port)
{

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		printf("Socket creation error");
		WSACleanup();
		return SOCKET_ERROR;
	}

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("Bind error");
		closesocket(listenSocket);
		WSACleanup();
		return SOCKET_ERROR;
	}

	if (listen(listenSocket, MAX_CLIENTS) == SOCKET_ERROR) {
		printf("Listen error");
		closesocket(listenSocket);
		WSACleanup();
		return SOCKET_ERROR;
	}

	
	
	return listenSocket;

}

int ServerClass::UDPInit(uint16_t port)
{
	SOCKET UDPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	

	if (UDPSocket == INVALID_SOCKET)
	{
		WSACleanup();		//releases memory associated with failed socket
		return SETUP_ERROR;
	}

	printf("\nUDP Init: Socket setup correctly! \n");

	bool broadcast = true;
	setsockopt(UDPSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast));

	sockaddr_in broadcastAddr;
	broadcastAddr.sin_family = AF_INET;
	broadcastAddr.sin_port = htons(port);
	broadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	
	//We don't need to bind to anything becuause we are not listening or wanting any information. Only output

	UDPserverSocket = UDPSocket;

	std::string messageToSend = "127.0.0.1%45613";		// IP ADDRESS%PORT	
	while (runSockets)
	{
		
		int bytesSent = sendto(UDPSocket, messageToSend.c_str(), static_cast<int>(messageToSend.length()), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
		//std::this_thread::sleep_for(std::chrono::seconds(1));		
	}
		
	shutdown(UDPSocket, 2);
	shutdown(UDPserverSocket, 2);

	closesocket(UDPSocket);
	
	if (UDPserverSocket != NULL)
	{
		closesocket(UDPserverSocket);
		
	}


	return SUCCESS;
}


bool ServerClass::parseRegisterCommand(const std::string& message, std::string &_username) {
	
	std::string prefix = "$register ";

	if (message.find(prefix) == 0)
	{
		//extract username
		std::string username = message.substr(prefix.length());
		_username = username;
		return true;
	}


	return false;
}


int ServerClass::readMessage(SOCKET _socket ,char* buffer, int32_t size)
{
	int error;

	if (buffer == NULL)
	{
		return PARAMETER_ERROR;
	}

	uint8_t messageSize = 0;

	int result = tcp_recv_whole(_socket, (char*)&messageSize, 1);

	if (messageSize > size)
	{
		return PARAMETER_ERROR;
	}

	if (result == 0)
	{
		return SHUTDOWN;
	}

	if (result == SOCKET_ERROR)
	{
		error = WSAGetLastError();

		if (error >= 6 && error <= 11031) //just doing a range of errors due to not knowing what error codes could be possible
		{
			return DISCONNECT;

		}
		else
		{
			return SHUTDOWN;
		}


	}
	result = tcp_recv_whole(_socket, buffer, messageSize);

	if (result == 0)
	{
		return SHUTDOWN;
	}

	if (result == SOCKET_ERROR)
	{
		error = WSAGetLastError();

		if (error >= 6 && error <= 11031) //just doing a range of errors due to not knowing what error codes could be possible
		{
			return DISCONNECT;

		}
		else
		{
			return SHUTDOWN;
		}


	}

	return SUCCESS;
}
int ServerClass::sendMessage(SOCKET _socket, char* data, int32_t length)
{
	int error = 0;
	if (data == nullptr)
	{
		return PARAMETER_ERROR;
	}
	if (length < 0 || length > 255)
	{
		return PARAMETER_ERROR;
	}

	uint8_t size = length;
	char* sendBuffer = new char[size];

	memset(sendBuffer, 0, length);
	strcpy(sendBuffer, data);

	int result = tcp_send_whole(_socket, (char*)&size, 1);

	if (result == SOCKET_ERROR)
	{
		error = WSAGetLastError();

		if (error >= 6 && error <= 11031) //just doing a range of errors due to not knowing what error codes could be possible
		{
			return DISCONNECT;

		}
		else
		{
			return SHUTDOWN;
		}


	}

	 result = tcp_send_whole(_socket, data, size);


	 if (result == SOCKET_ERROR)
	 {
		 error = WSAGetLastError();

		 if (error >= 6 && error <= 11031) //just doing a range of errors due to not knowing what error codes could be possible
		 {
			 return DISCONNECT;

		 }
		 else
		 {
			 return SHUTDOWN;
		 }


	 }

}


int ServerClass::tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
{
	int result;
	int bytesSent = 0;

	while (bytesSent < length)
	{
		result = send(skSocket, (const char*)data + bytesSent, length - bytesSent, 0);

		if (result <= 0)
			return result;

		bytesSent += result;
	}

	return bytesSent;
}
int ServerClass::tcp_recv_whole(SOCKET s, char* buf, int len)
{
	int total = 0;

	do
	{
		int ret = recv(s, buf + total, len - total, 0);
		if (ret < 1)
			return ret;
		else
			total += ret;

	} while (total < len);

	return total;
}

