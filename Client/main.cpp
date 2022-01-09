#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <iostream>
#include <string>

SOCKET Connection;

enum Packet
{
	P_ChatMessage,
	P_Test
};

bool SendInt(int _int);
bool GetInt(int& _int);
bool SendPacketType(Packet _packettype);
bool GetPacketType(Packet& _packettype);
bool SendString(std::string& _string);
bool GetString(std::string& _string);

bool ProcessPacket(Packet packettype);
void ClientThread();

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData))
	{
		std::cout << "Failed WSAStartup" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int addr_size = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("134.249.116.186");
	addr.sin_port = htons(56070);
	addr.sin_family = AF_INET;

	Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(Connection, (SOCKADDR*)&addr, addr_size))
	{
		std::cout << "Failed to connect" << std::endl;
		return 0;
	}
	
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientThread, NULL, NULL, NULL);

	std::string userinput;
	while (true)
	{
		std::getline(std::cin, userinput);
		if (!SendString(userinput))
			break;
		Sleep(10);
	}

	return 0;
}

bool SendInt(int _int)
{
	int ReturnCheck = send(Connection, (char*)&_int, sizeof(int), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}
bool GetInt(int& _int)
{
	int ReturnCheck = recv(Connection, (char*)&_int, sizeof(int), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}
bool SendPacketType(Packet _packettype)
{
	int ReturnCheck = send(Connection, (char*)&_packettype, sizeof(Packet), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}
bool GetPacketType(Packet& _packettype)
{
	int ReturnCheck = recv(Connection, (char*)&_packettype, sizeof(Packet), NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}
bool SendString(std::string& _string)
{
	if (!SendPacketType(P_ChatMessage))
		return false;
	int buffer_len = _string.size();
	if (!SendInt(buffer_len))
		return false;
	int ReturnCheck = send(Connection, _string.c_str(), buffer_len, NULL);
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;;
}
bool GetString(std::string& _string)
{
	int buffer_len;
	if (!GetInt(buffer_len))
		return false;
	char* buffer = new char[buffer_len + 1];
	buffer[buffer_len] = '\0';
	int ReturnCheck = recv(Connection, buffer, buffer_len, NULL);
	_string = buffer;
	delete[] buffer;
	if (ReturnCheck == SOCKET_ERROR)
		return false;
	return true;
}

bool ProcessPacket(Packet packettype)
{
	switch (packettype)
	{
	case P_ChatMessage:
	{
		std::string buffer;
		if (!GetString(buffer))
			return false;		
		std::cout << buffer << std::endl;
		break;
	}
	default:
	{
		std::cout << "Unrecognzed packet" << packettype << std::endl;
		break;
	}
	}
	return true;
}
void ClientThread()
{
	Packet packettype;
	while (true)
	{
		if (!GetPacketType(packettype))
			break;
		if (!ProcessPacket(packettype))
			break;
	}
	std::cout << "Lost connection to the server." << std::endl;
	closesocket(Connection);
}