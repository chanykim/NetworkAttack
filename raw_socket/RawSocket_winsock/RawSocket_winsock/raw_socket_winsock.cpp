
#include <WinSock2.h>
#include <WS2tcpip.h>		// IP_HDRINCL is here
#include <stdio.h>
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

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
	
	unsigned char ip_fragment_offset_1 : 5;	// Fragment offset field
	unsigned char ip_fragment_offset_2;	// Fragment offset field

	unsigned char ip_ttl;
	unsigned char ip_protocol;			// Protocol(TCP, UDP, etc)
	unsigned short ip_checksum;
	unsigned int ip_src_addr;
	unsigned int ip_dest_addr;	
}IPV4_HDR;

// TCP HEADER
typedef struct tcp_hdr{
	unsigned short src_port;		// source port
	unsigned short dest_port;		// destination port
	unsigned int sequence;			// sequence number - 32 bits
	unsigned int acknowledge;		// acknowledge number - 32 bits

	unsigned char data_offset : 4;	
	unsigned char reserved_part : 3;
	unsigned char ns : 1;				// Nonce: Not set
	unsigned char cwr : 1;				// Congestion Windows Reduced Flag
	unsigned char ecn : 1;				// ECN-Echo Flag
		
	unsigned char urg : 1;
	unsigned char ack : 1;
	unsigned char psh : 1;
	unsigned char rst : 1;
	unsigned char syn : 1;
	unsigned char fin : 1;

	unsigned short window;
	unsigned short checksum;
	unsigned short urgent_pointer;
}TCP_HDR;

int main()
{
	SOCKET raw_socket = INVALID_SOCKET;
	WSADATA wsaData;
	SOCKADDR_IN	destAddr;
	int nResult;
	int payload = 512, optVal;
	char hostname[256], buf[1024], source_ip[32];
	char *data = NULL;
	hostent *server;

	IPV4_HDR *v4hdr = NULL;
	TCP_HDR *tcphdr = NULL;

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
	memcpy(&destAddr.sin_addr.S_un.S_addr, server->h_addr_list[0], server->h_length);
	printf("Resolved\n");

	printf("\nInput Source IP : ");
	gets(source_ip);

	// setup TCP/IP
	// IP
	v4hdr = (IPV4_HDR *)buf;
	v4hdr->ip_version = 4;
	v4hdr->ip_header_len = 5;
	v4hdr->ip_tos = 0;
	v4hdr->ip_total_length = htons(sizeof(IPV4_HDR)+sizeof(TCP_HDR)+payload);
	v4hdr->ip_id = htons(2);
	v4hdr->ip_fragment_offset_1 = 0;
	v4hdr->ip_fragment_offset_2 = 0;
	v4hdr->ip_reserved_zero = 0;
	v4hdr->ip_dont_fragment = 1;
	v4hdr->ip_more_fragment = 0;
	v4hdr->ip_ttl = 8;
	v4hdr->ip_protocol = IPPROTO_TCP;
	v4hdr->ip_src_addr = inet_addr(source_ip);
	v4hdr->ip_dest_addr = inet_addr(inet_ntoa(destAddr.sin_addr));
	v4hdr->ip_checksum = 0;

	// TCP
	tcphdr = (TCP_HDR *)&buf[sizeof(IPV4_HDR)];	// get the pointer to the tcp header in the packet
	tcphdr->src_port = htons(1234);
	tcphdr->dest_port = htons(50000);
	tcphdr->ns = 1;
	tcphdr->cwr = 0;
	tcphdr->ecn = 1;
	tcphdr->urg = 0;
	tcphdr->ack = 0;
	tcphdr->psh = 0;
	tcphdr->rst = 1;
	tcphdr->syn = 0;
	tcphdr->fin = 0;
	tcphdr->checksum = 0;
	// Initialize the TCP payload to some rubbish
	data = &buf[sizeof(IPV4_HDR)+sizeof(TCP_HDR)];
	memset(data, '^', payload);

	printf("\nSending Packet...\n");

	/* keyboard  */
	int k = 1;
	while (!_kbhit())
	{
		printf(" %d packets send\r", k++);
		nResult = sendto(raw_socket, buf, sizeof(IPV4_HDR)+sizeof(TCP_HDR)+payload,
			0, (SOCKADDR *)&destAddr, sizeof(destAddr));
		if (nResult == SOCKET_ERROR)
		{
			printf("Error sending Packet : %d\n", WSAGetLastError());
			break;
		}
	}

	// cleanup
	WSACleanup();
	closesocket(raw_socket);

	return 0;
}