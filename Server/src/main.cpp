#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <iostream>

#define MAX_CONNECTIONS 100
SOCKET Connections[MAX_CONNECTIONS];
unsigned int ConnectionCounter = 0;

enum Packet
{
	P_ChatMessage,
	P_Test
};

bool SendInt(int ID, int _int);
bool GetInt(int ID, int& _int);
bool SendPacketType(int ID, Packet packettype);
bool GetPacketType(int ID, Packet& packettype);
bool SendString(int ID, std::string& _string);
bool GetString(int ID, std::string& _string);

bool ProcessPacket(int ID, Packet packettype);
void ClientHandlerThread(int index);

int main()
{
	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData))
	{
		std::cout << "Failed WSAStartup." << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int addr_size = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("192.168.0.102");
	addr.sin_port = htons(56070);
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, addr_size);
	listen(sListen, SOMAXCONN);

	SOCKET newConnection;
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		newConnection = accept(sListen, (SOCKADDR*)&addr, &addr_size);
		if (!newConnection)
		{
			std::cout << "Failed to accept the client's connection." << std::endl;
		}
		else
		{
			std::cout << "Client connected." << std::endl;
			Connections[i] = newConnection;
			++ConnectionCounter;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandlerThread, (LPVOID)(i), NULL, NULL);
			std::string msg = "Welcome in to the server.";
			SendString(i, msg);
		}
	}

	return 0;
}

bool SendInt(int ID, int _int)
{
	int ReturnCheck = send(Connections[ID], (char*)&_int, sizeof(int), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}
bool GetInt(int ID, int& _int)
{
	int ReturnCheck = recv(Connections[ID], (char*)&_int, sizeof(int), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}
bool SendPacketType(int ID, Packet _packettype)
{
	int ReturnCheck = send(Connections[ID], (char*)&_packettype, sizeof(Packet), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}
bool GetPacketType(int ID, Packet& _packettype)
{
	int ReturnCheck = recv(Connections[ID], (char*)&_packettype, sizeof(Packet), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}
bool SendString(int ID, std::string& _string)
{
	if (!SendPacketType(ID, P_ChatMessage))
		return false;
	_string = '\t' + _string;
	int buffer_len = _string.size();
	if (!SendInt(ID, buffer_len))
		return false;
	int ReturnCheck = send(Connections[ID], _string.c_str(), buffer_len, NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}
bool GetString(int ID, std::string& _string)
{
	int buffer_len;
	if (!GetInt(ID, buffer_len))
		return false;
	char* buffer = new char[buffer_len + 1];
	buffer[buffer_len] = '\0';
	int ReturnCheck = recv(Connections[ID], buffer, buffer_len, NULL);
	_string = buffer;
	delete[] buffer;
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}


bool ProcessPacket(int ID, Packet packettype)
{
	switch (packettype)
	{
	case P_ChatMessage:
	{/*
		int buffer_len;
		if (!GetInt(ID, buffer_len))
			break;*/
		std::string buffer;
		if (!GetString(ID, buffer))
			break;
		for (int i = 0; i < ConnectionCounter; i++)
		{
			if (i == ID)
				continue;
			if (!SendString(i, buffer))
			{
				std::cout << "Failed to send message from client ID: " << ID << std::endl;
			}
		}
		std::cout << "Processed chat message packet from user ID: " << ID << std::endl;
		break;
	}
	default:
	{
		std::cout << "Unrecognized packet: " << packettype << std::endl;
		break;
	}
	}

	return true;
}
void ClientHandlerThread(int ID)
{
	Packet packettype;
	while (true)
	{
		if (!GetPacketType(ID, packettype))
			break;
		if (!ProcessPacket(ID, packettype))
			break;
	}
	std::cout << "Lost connection to client ID: " << ID << std::endl;
	closesocket(Connections[ID]);
}
