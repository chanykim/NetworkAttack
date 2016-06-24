
#include <stdio.h>
#include <WinSock2.h>

// WinSock 2.2 Library
#pragma comment(lib, "ws2_32.lib")

int main()
{
	SOCKET socket_fd;
	int iResult;
	char message[256] = {"Test Client Socket!"};

	// 1. Initialize WinSock
	WSADATA wsaData;

	printf("\nInitializing Windows Socket...\n");
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		printf("WSAStartup() Failed with error: %d\n", iResult);
		return 1;
	}
	printf("Initialized Windows Socket!");

	// 2. Create a Socket for the Client
#if 0
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hInts;
	
	ZeroMemory(hInts, sizeof(hInts));
	hInts.ai_family = AF_UNSPEC;		// IPv4 or IPv6
	hInts.ai_socktype = SOCK_STREAM;
	hInts.ai_protocol = IPPROTO_TCP;
#endif

	socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_fd == INVALID_SOCKET)
	{
		printf("Creating Socket Failed");
		return 0;
	}

	// 3. Connect Socket
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(8000);

	int result = connect(socket_fd, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		printf("Connect Socket Failed");
		return 0;
	}

	// 4. Send/Recv Data
	int strLen;
	strLen = send(socket_fd, message, sizeof(message - 1), 0);
	if (strLen == -1)
	{

	}

	// 5. Close Socket
	closesocket(socket_fd);
	WSACleanup();

	system("pause");

	return 0;
}