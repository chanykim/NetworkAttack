
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#pragma lib(lib, "ws2_32.lib")

/*---------------------------------------------------------------------------
Raw Socket�� ���� Packet�� ��������� ����
1. socket() �Լ��� �̿��Ͽ� RAW Socket�� �����Ѵ�.
2. setsockopt() �Լ��� �̿��Ͽ� TCP/IP ������� ���� ������ �� �ֵ���
   �� ������ �ɼ��� �����Ѵ�.
3. IP Header ����ü�� ���� ���� ä���.
4. TCP Header ����ü�� ���� ���� ä���.
5. �����Ϸ��� �ϴ� Packet Header�� �ϼ���.
6. Ư�� ������ Packet�� ����.
7. Wireshark ���� ������ ���α׷����� ������ Packet�� ����� ���۵Ǿ����� Ȯ��.
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

	// Initialize winsock ������ �Ѵ�.

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