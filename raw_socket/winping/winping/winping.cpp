
#include <stdio.h>
#include <WinSock2.h>
#include "winping.h"
#include "ip_checksum.h"

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_PACKET_SIZE	32
#define MAX_PING_DATA_SIZE	1024
#define MAX_PING_PACKET_SIZE (MAX_PING_DATA_SIZE + sizeof(IPV4_HDR))

int main(int argc, char **argv)
{
	SOCKET PingSocket;
	ICMP_HDR *SendBuf = NULL;
	IPV4_HDR *RecvBuf = NULL;		// ICMP + IP buffer

	// 입력 값 확인
	if (argc < 2)
	{
		printf("usage:\n");
		printf("%s <host> [data_size]", argv[0]);
		printf("\tdata_size can be up to %d bytes. Default is %d\n", 
			MAX_PING_DATA_SIZE, DEFAULT_PACKET_SIZE);

		return 1;
	}

	// Initialize Winsock 
	WSADATA wsaData;
	int result;

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return -1;
	}

	//
	SOCKADDR_IN srcAddr, destAddr;
	int packet_size = DEFAULT_PACKET_SIZE;
	setup_for_ping(argv[1], PingSocket, destAddr);

	packet_size = max(sizeof(ICMP_HDR), 
						min(MAX_PING_DATA_SIZE, (unsigned int)packet_size));
	// SendBuf / RecvBuf Memory allocation
 	allocate_buffers(SendBuf, RecvBuf, packet_size);

	// set up ping packet
	init_ping_packet(SendBuf, packet_size);

	// Send the ping request and Receive the reply
	send_ping(PingSocket, destAddr, SendBuf, packet_size);
	while (1)
	{
		if (recv_ping(PingSocket, srcAddr, RecvBuf, MAX_PING_PACKET_SIZE) < 0)
		{
			unsigned short header_len = RecvBuf->ip_header_length * 4;
			ICMP_HDR* icmphdr = (ICMP_HDR *)((char *)RecvBuf + header_len);

		}
		else
		{
			break;
		}
	}

	// Cleanup
	WSACleanup();
	free(SendBuf);
	free(RecvBuf);

	return 0;
}

/*

Parameters :
		SOCKET& RawSocekt : reference로 정의
*/
int setup_for_ping(char* host, SOCKET& RawSocket, SOCKADDR_IN &dest)
{
	// Create Raw Socket 
	RawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (RawSocket == INVALID_SOCKET)
	{
		printf("socket creation is failed with error: %d\n", WSAGetLastError());
		return -1;
	}

	// Initialize destination host info
	memset(&dest, 0x0, sizeof(SOCKADDR_IN));

	// IP address for ping
	unsigned int addr = inet_addr(host);
	if (addr != INADDR_NONE)
	{
		dest.sin_addr.s_addr = addr;
		dest.sin_family = AF_INET;
		dest.sin_port = 0;
	}
	else
	{
		hostent *phost = gethostbyname(host);
		if (phost != 0)
		{
			memcpy(&(dest.sin_addr), phost->h_addr_list[0], phost->h_length);
			dest.sin_family = phost->h_addrtype;
		}
		else
		{
			printf("Failed to resolve : %s\n", host);
			return -1;
		}
	}
	
	return 0;
}

int init_ping_packet(ICMP_HDR *icmp_hdr, int packet_size)
{	
	char* datapart;

	// configure out ICMP Header
	icmp_hdr->icmp_type = ICMP_MESSAGE_ECHO_REQUEST;
	icmp_hdr->icmp_code = 0;
	icmp_hdr->icmp_checksum = 0;
	icmp_hdr->icmp_id = (USHORT)GetCurrentProcessId();
	icmp_hdr->icmp_sequence = 1;
	icmp_hdr->icmp_timestamp = GetTickCount();

	// 데이터 부분을 임의의 문자로 채움.
	const unsigned long int deadmeat = 0xDEADBEEF;
	datapart = (char *)icmp_hdr + sizeof(ICMP_HDR);
	int bytes_left = packet_size - sizeof(ICMP_HDR);
	while (bytes_left > 0)
	{
		memcpy(datapart, &deadmeat, min(int(sizeof(deadmeat)), bytes_left));
		bytes_left -= sizeof(deadmeat);
		datapart += sizeof(deadmeat);
	}

	// 체크섬 계산
	icmp_hdr->icmp_checksum = ip_checksum((USHORT *)icmp_hdr, packet_size);

	return 0;
}

int allocate_buffers(ICMP_HDR*& send_buf , IPV4_HDR*& recv_buf, int packet_size)
{
	// send_buf 메모리 할당
	send_buf = (ICMP_HDR *)malloc(packet_size);
	if (send_buf == NULL)
	{
		printf("send_buf failed with error: %d\n", WSAGetLastError());
		return -1;
	}
	memset(send_buf, 0x0, packet_size);

	recv_buf = (IPV4_HDR *)malloc(MAX_PING_DATA_SIZE);
	if (recv_buf == NULL)
	{
		printf("recv_buf failed with error: %d\n", WSAGetLastError());
		return -1;
	}
	memset(recv_buf, 0x0, MAX_PING_DATA_SIZE);

	return 0;
}

int send_ping(SOCKET socket, const sockaddr_in& dest, ICMP_HDR* send_buf, int packet_size)
{
	int bwroute;
	bwroute = sendto(socket, (char *)send_buf, packet_size, 0, (sockaddr *)&dest, sizeof(dest));
	if (bwroute == SOCKET_ERROR)
	{
		printf("send_ping failed with error: %d\n", bwroute);
		return -1;
	}
	else if (bwroute < packet_size)
	{
		printf("sent %d bytes\n", bwroute);
	}

	return 0;
}

int recv_ping(SOCKET socket, const sockaddr_in& source, IPV4_HDR* recv_buf, int packet_size)
{
	int fromlen = sizeof(source);
	recvfrom(socket, (char *)recv_buf, packet_size, 0, (sockaddr *)&source, &fromlen);

	return 0;
}