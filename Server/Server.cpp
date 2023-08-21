// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ServerClass.h"
#pragma comment(lib, "ws2_32.lib")


std::ofstream ologFile("ServerLog.txt");

const int MAX_CLIENTS = 3;


int main() {

	WSADATA wsaData;
	if (WSAStartup(WINSOCK_VERSION, &wsaData) != 0)
	{
		printf("WSAStartup failed");
		return 1;
	}

	ServerClass server;		// Create an instance of your ServerClass

	SOCKET listenSocket = server.TCPInit(45613);	// Initialize the listening socket

	//start UDP thread
	std::thread UDPthread([&]() {server.UDPInit(45612); });


	std::vector<Clients> clientSockets;		// Vector to hold connected client sockets

	fd_set masterSet;
	//SOCKET maxSocket;

	FD_ZERO(&masterSet);

	FD_SET(listenSocket, &masterSet);		// Add the listening socket to the master set

	//maxSocket = listenSocket;

	char clBuffer[255] = { 0 };
	int msgLen[255];		// Array to store the expected message lengths for each client


		
	if (!ologFile.is_open())		//fails	
	{
		std::wcout << "ERROR: FAILED TO OPEN THE SERVERLOG.TXT";
	}


	while (server.runSockets)
	{
		// Create a copy of the master set to use with select
		fd_set readSet = masterSet;

		int numReady = select(0, &readSet, nullptr, nullptr, nullptr);

		if (numReady == SOCKET_ERROR)
		{
			printf("Select error");
			break;
		}


		// Check if the listening socket has incoming connections
		if (FD_ISSET(listenSocket, &readSet))
		{

			SOCKET newSocket = accept(listenSocket, nullptr, nullptr);


			if (clientSockets.size() < MAX_CLIENTS)		//max clients = 3
			{

				if (newSocket == INVALID_SOCKET) {
					printf("Accept error");

					shutdown(newSocket, 2);
					closesocket(newSocket);

				}
				else
				{

					char serverSuccessMessage[] = "SV_Success";

					server.sendMessage(newSocket, serverSuccessMessage, strlen(serverSuccessMessage) + 1);

					// Add the new client to the vector
					clientSockets.emplace_back(newSocket);

					// Add the new socket to the master set
					FD_SET(newSocket, &masterSet);
					
					//let users know a person has connected

					char connectMessage[] = "A user has connected!";

					//doing this after the message is sent that way it doesn't send to the person who just connected
					for (size_t i = 0; i < clientSockets.size(); i++)
					{
						server.sendMessage(clientSockets[i].clientSocket, connectMessage, strlen(connectMessage) + 1); // +1 for null terminator

					}

					if (ologFile.is_open())
					{
						ologFile << connectMessage << std::endl;
					}
				
				}
			}
			else
			{

				char serverFullMessage[] = "SV_FULL";	

				if (ologFile.is_open())
				{
					ologFile << serverFullMessage << std::endl;
				}

				server.sendMessage(newSocket, serverFullMessage, strlen(serverFullMessage) + 1);

				shutdown(newSocket, 2);
				closesocket(newSocket);

			}
		}

		// Loop through each connected client
		for (size_t i = 0; i < clientSockets.size(); ++i)
		{
			// Check if there's data to read from the client
			if (FD_ISSET(clientSockets[i].clientSocket, &readSet)) {

				int result = server.readMessage(clientSockets[i].clientSocket, &clBuffer[0], 255);

				if (result == 1)		//result 1 is SHUTDOWN
				{
					FD_CLR(clientSockets[i].clientSocket, &masterSet);
					shutdown(clientSockets[i].clientSocket, 2);			//removing and cleaning up a socket
					closesocket(clientSockets[i].clientSocket);

					char disconnectChar[] = " has disconnected!";
					int32_t removedUserLength = clientSockets[i].Username.length() + strlen(disconnectChar) + 1;

					char* combinedText = new char[removedUserLength]; // +1 for null terminator
					strcpy(combinedText, clientSockets[i].Username.c_str());
					strcat(combinedText, disconnectChar);

					//message all clients
					for (size_t j = 0; j < clientSockets.size(); ++j)
					{
						if (clientSockets[j].clientSocket != clientSockets[i].clientSocket)
						{

							result = server.sendMessage(clientSockets[j].clientSocket, combinedText, removedUserLength);
													

							if (result == SOCKET_ERROR)
							{
								FD_CLR(clientSockets[i].clientSocket, &masterSet);
								shutdown(clientSockets[i].clientSocket, 2);			//removing and cleaning up a socket
								closesocket(clientSockets[i].clientSocket);
								clientSockets.erase(clientSockets.begin() + i);
								--i;

							}
							if (result == 2)		//ECONRESET 10054
							{
								FD_CLR(clientSockets[i].clientSocket, &masterSet);
								shutdown(clientSockets[i].clientSocket, 2);			//removing and cleaning up a socket
								closesocket(clientSockets[i].clientSocket);
								clientSockets.erase(clientSockets.begin() + i);
								--i;
							}
						}
					}

					if (ologFile.is_open())
					{
						ologFile << combinedText << std::endl;
					}

					clientSockets.erase(clientSockets.begin() + i);
					--i;

				}
				else if (result == SOCKET_ERROR)
				{
					FD_CLR(clientSockets[i].clientSocket, &masterSet);
					shutdown(clientSockets[i].clientSocket, 2);			//removing and cleaning up a socket
					closesocket(clientSockets[i].clientSocket);

					char* combinedText = new char[1];

					//message all clients
					for (size_t j = 0; j < clientSockets.size(); ++j)
					{
						if (clientSockets[j].clientSocket != clientSockets[i].clientSocket)
						{
							char disconnectChar[] = " has disconnected!";
							int32_t removedUserLength = clientSockets[i].Username.length() + strlen(disconnectChar) + 1;
							delete[] combinedText;
							combinedText = new char[removedUserLength]; // +1 for null terminator

							strcpy(combinedText, clientSockets[i].Username.c_str());
							strcat(combinedText, disconnectChar);


							result = server.sendMessage(clientSockets[j].clientSocket, combinedText, removedUserLength);




							if (result == SOCKET_ERROR)
							{
								FD_CLR(clientSockets[i].clientSocket, &masterSet);
								shutdown(clientSockets[i].clientSocket, 2);			//removing and cleaning up a socket
								closesocket(clientSockets[i].clientSocket);
								clientSockets.erase(clientSockets.begin() + i);
								--i;

							}
							if (result == 2)		//ECONRESET 10054
							{
								FD_CLR(clientSockets[i].clientSocket, &masterSet);
								shutdown(clientSockets[i].clientSocket, 2);			//removing and cleaning up a socket
								closesocket(clientSockets[i].clientSocket);
								clientSockets.erase(clientSockets.begin() + i);
								--i;
							}
						}
					}

					if (clientSockets.empty())		//still need to log when users = 0.. fixes error when a person leaves and no one is on server
					{
						char disconnectChar[] = " has disconnected!";
						int32_t removedUserLength = clientSockets[i].Username.length() + strlen(disconnectChar) + 1;
						delete[] combinedText;
						combinedText = new char[removedUserLength]; // +1 for null terminator

						strcpy(combinedText, clientSockets[i].Username.c_str());
						strcat(combinedText, disconnectChar);

						if (ologFile.is_open())
						{
							ologFile << combinedText << std::endl;
						}

					}
					else
					{

						if (ologFile.is_open())
						{
							ologFile << combinedText << std::endl;
						}


					}


					clientSockets.erase(clientSockets.begin() + i);
					--i;

				}
				else if (result == 2)		//ECONRESET 10054
				{
					FD_CLR(clientSockets[i].clientSocket, &masterSet);
					shutdown(clientSockets[i].clientSocket, 2);			//removing and cleaning up a socket
					closesocket(clientSockets[i].clientSocket);

					char* combinedText = new char[1];	//temp
					//message all clients
					for (size_t j = 0; j < clientSockets.size(); ++j)
					{
						if (clientSockets[j].clientSocket != clientSockets[i].clientSocket)
						{
							char disconnectChar[] = " has disconnected!";
							int32_t removedUserLength = clientSockets[i].Username.length() + strlen(disconnectChar) + 1;
							delete[] combinedText;
							combinedText = new char[removedUserLength]; // +1 for null terminator
							strcpy(combinedText, clientSockets[i].Username.c_str());
							strcat(combinedText, disconnectChar);


							result = server.sendMessage(clientSockets[j].clientSocket, combinedText, removedUserLength);

							

							if (result == SOCKET_ERROR)
							{
								FD_CLR(clientSockets[i].clientSocket, &masterSet);
								shutdown(clientSockets[i].clientSocket, 2);			//removing and cleaning up a socket
								closesocket(clientSockets[i].clientSocket);
								clientSockets.erase(clientSockets.begin() + i);
								--i;

							}
							if (result == 2)		//ECONRESET 10054
							{
								FD_CLR(clientSockets[i].clientSocket, &masterSet);
								shutdown(clientSockets[i].clientSocket, 2);			//removing and cleaning up a socket
								closesocket(clientSockets[i].clientSocket);
								clientSockets.erase(clientSockets.begin() + i);
								--i;
							}
						}
					}

					if (clientSockets.size() == 1)
					{

						char disconnectChar[] = " has disconnected!";
						int32_t removedUserLength = clientSockets[i].Username.length() + strlen(disconnectChar) + 1;
						delete[] combinedText;
						combinedText = new char[removedUserLength]; // +1 for null terminator
						strcpy(combinedText, clientSockets[i].Username.c_str());
						strcat(combinedText, disconnectChar);

						if (ologFile.is_open())
						{
							ologFile << combinedText << std::endl;
						}
					}
					else
					{

						if (ologFile.is_open())
						{
							ologFile << combinedText << std::endl;
						}
					}
					

					

					clientSockets.erase(clientSockets.begin() + i);
					--i;
				}
				else
				{
					if (!clientSockets[i].isRegistered)
					{
						if (server.parseRegisterCommand(std::string(clBuffer), clientSockets[i].Username))
						{
							clientSockets[i].isRegistered = true;
							char successMessage[] = "you are registered and can now send messages!!";

							server.sendMessage(clientSockets[i].clientSocket, successMessage, strlen(successMessage) + 1);

							if (ologFile.is_open())
							{
								ologFile << clBuffer << std::endl;
								ologFile << successMessage << std::endl;
							}

						}
					}
					else if (server.parseGetListCommand(std::string(clBuffer)))
					{

						if (clientSockets.empty())
						{

							char listEmptyMessage[] = "The list is empty!!";

							server.sendMessage(clientSockets[i].clientSocket, listEmptyMessage, strlen(listEmptyMessage) + 1);


							if (ologFile.is_open())
							{
								ologFile << listEmptyMessage << std::endl;
							}

						}
						else
						{

							std::string listofclients;
							char listMessage[] = "Users currently connected: ";


							for (int i = 0; i < clientSockets.size(); i++)
							{
								listofclients += clientSockets[i].Username;

								if (i < clientSockets.size() - 1)
								{
									listofclients += ",";
								}
							}
							char* charList = new char[ listofclients.length() + strlen(listMessage) + 1]; // +1 for null terminator


							strcpy(charList, listMessage);
							strcat(charList, listofclients.c_str());
							
							server.sendMessage(clientSockets[i].clientSocket, charList, strlen(charList) + 1);


							if (ologFile.is_open())
							{
								ologFile << charList << std::endl;
							}

							delete[] charList;

						}

					}
					else if (server.parseGetLogCommand(std::string(clBuffer)))
					{
						std::cout << "Parse command executed by: "  << clientSockets[i].Username;
					}
					else
					{
						//parsing message
						std::string colon = ": ";
						int32_t messageLength = clientSockets[i].Username.length() + strlen(clBuffer) + 3;
						char* combinedText = new char[clientSockets[i].Username.length() + strlen(clBuffer) + 3]; // +1 for null terminator
						strcpy(combinedText, clientSockets[i].Username.c_str());
						strcat(combinedText, colon.c_str());
						strcat(combinedText, clBuffer);



						//message all clients
						for (size_t j = 0; j < clientSockets.size(); ++j)
						{
							if (clientSockets[j].clientSocket != clientSockets[i].clientSocket)
							{


								result = server.sendMessage(clientSockets[j].clientSocket, combinedText, messageLength);

								if (result == SOCKET_ERROR)
								{
									FD_CLR(clientSockets[i].clientSocket, &masterSet);
									shutdown(clientSockets[i].clientSocket, 2);			//removing and cleaning up a socket
									closesocket(clientSockets[i].clientSocket);
									clientSockets.erase(clientSockets.begin() + i);
									--i;

								}
								if (result == 2)		//ECONRESET 10054
								{
									FD_CLR(clientSockets[i].clientSocket, &masterSet);
									shutdown(clientSockets[i].clientSocket, 2);			//removing and cleaning up a socket
									closesocket(clientSockets[i].clientSocket);
									clientSockets.erase(clientSockets.begin() + i);
									--i;
								}
							}
						}

						if (ologFile.is_open())
						{
							ologFile << combinedText << std::endl;
						}

					

						delete[] combinedText;
					}


				}
			}
		}
	}

	// Close client sockets
	for (const Clients& client : clientSockets) {
		closesocket(client.clientSocket);
	}

	ologFile.close();
	std::cout << "output log closed!";

	UDPthread.join();
	// Close the listening socket
	closesocket(listenSocket);
	

	// Clean up Winsock
	WSACleanup();

	return 0;
}


