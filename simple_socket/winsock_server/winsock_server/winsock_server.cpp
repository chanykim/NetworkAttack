
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{
	SOCKET socket_fd;
	SOCKET client_socket;
	int iResult;
	char message[256] = {0,};

	// Initialize Windows Socket
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	// Create Socket
	socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_fd == INVALID_SOCKET)
	{
		printf("Socket failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	// bind
	SOCKADDR_IN serverAddr, clientAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(8000);
	iResult = bind(socket_fd, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
	if (iResult == SOCKET_ERROR)
	{
		printf("Socket bind failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	// listen
	iResult = listen(socket_fd, 10);
	if (iResult == SOCKET_ERROR)
	{
		printf("Socket listen failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	// accept
	int clientAddrSize = sizeof(clientAddr);
	client_socket = accept(socket_fd, (SOCKADDR *)&clientAddr, &clientAddrSize);
	if (client_socket == INVALID_SOCKET)
	{
		printf("Socket failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	// Send/Recv Data
	iResult = recv(client_socket, message, sizeof(message), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("Socket recv failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	printf("Server Recv : %s\n", message);

	closesocket(socket_fd);
	closesocket(client_socket);
	WSACleanup();

	system("pause");

	return 0;
}