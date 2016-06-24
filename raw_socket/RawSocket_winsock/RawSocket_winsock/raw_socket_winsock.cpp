
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#pragma lib(lib, "ws2_32.lib")

/*---------------------------------------------------------------------------
Raw Socket에 의해 Packet이 만들어지는 과정
1. socket() 함수를 이용하여 RAW Socket을 생성한다.
2. setsockopt() 함수를 이용하여 TCP/IP 헤더값을 빅접 변경할 수 있도록
   이 소켓의 옵션을 변경한다.
3. IP Header 구조체의 값을 직접 채운다.
4. TCP Header 구조체의 값을 직접 채운다.
5. 전송하려고 하는 Packet Header가 완성됨.
6. 특정 서버로 Packet을 전송.
7. Wireshark 같은 스니핑 프로그램으로 전송한 Packet이 제대로 전송되었는지 확인.
-----------------------------------------------------------------------------*/

// IP HEADER
typedef struct ip_hdr{	
	unsigned char ip_version : 4;
	unsigned char ip_header_len : 4;	// 4-bit header length
	unsigned char ip_tos;				// IP type of service
	unsigned short ip_total_length;
	unsigned short ip_id;				// unique identifier
	
	unsigned char ip_reserved_zero : 1;	
	unsigned char ip_dont_fragment : 1;
	unsigned char ip_more_fragment : 1;
	
	unsigned char ip_fragment_offset : 13;	// Fragment offset field
	//unsigned char ip_fragment_offset_1 : 5;	// Fragment offset field
	//unsigned char ip_fragment_offset_2;	// Fragment offset field

	unsigned char ip_ttl;
	unsigned char ip_protocol;			// Protocol(TCP, UDP, etc)
	unsigned short ip_checksum;
	unsigned int ip_src_addr;
	unsigned int ip_dest_addr;	
}IPV4_HDR;

// TCP HEADER
typedef struct tcp_hdr{
	unsigned short src_port;
	unsigned short dest_port;
	unsigned int sequence;
	unsigned int acknowledge;

	unsigned char ns : 1;
	unsigned char reserved_part1 : 3;
}TCP_HDR;
int main()
{
	SOCKET raw_socket = INVALID_SOCKET;
	WSADATA wsaData;
	SOCKADDR_IN	destAddr;
	int nResult;
	int optVal;
	char hostname[256], buf[1024], source_ip[32];
	hostent *server;

	// Initialize winsock 소켓을 한다.

	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nResult != 0)
	{
		printf("Initialized failed with error: %d\n", nResult);
		return 1;
	}
	printf("Initialized Success...\n");

	// Create RAW SOCKET
	raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (raw_socket == INVALID_SOCKET)
	{
		printf("creation of raw_socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	printf("raw_socket created successfully!\n");

	// setup socket in Raw Mode
	nResult = setsockopt(raw_socket, IPPROTO_IP, IP_HDRINCL, (char *)&optVal, sizeof(optVal));
	if (nResult != 0)
	{
		printf("setsockopt failed with error: %d\n", WSAGetLastError());
		closesocket(raw_socket);
		WSACleanup();
		return 1;
	}
	printf("setsockopt success...\n");

	// input Target Hostname
	printf("Enter Hostname : ");
	gets(hostname);
	printf("Resolving Hostname...\n");
	server = gethostbyname(hostname);
	if (server == NULL)
	{
		printf("Unable to resolve.");
		return 1;
	}
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(5000);
	memcpy(&destAddr.sin_addr.S_un.S_addr, server->h_addr_list, server->h_length);
	printf("Resolved\n");

	printf("\nInput Source IP : ");
	gets(source_ip);

	// setup TCP/IP

	// cleanup
	WSACleanup();

	return 0;
}