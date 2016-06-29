
/*
** Ping Raw Socket using ICMP protocol.
**
*/

//
// Description:
//    This sample illustrates how to use raw sockets to send ICMP
//    echo requests and receive their response. This sample performs
//    both IPv4 and IPv6 ICMP echo requests. When using raw sockets,
//    the protocol value supplied to the socket API is used as the
//    protocol field (or next header field) of the IP packet. Then
//    as a part of the data submitted to sendto, we include both
//    the ICMP request and data.
//
//    For IPv4 the IP record route option is supported via the 
//    IP_OPTIONS socket option.
//
// Compile:
//      cl -o ping.exe ping.cpp resolve.cpp ws2_32.lib
//
// Command Line Options/Parameters:
//     ping.exe [-a 4|6] [-i ttl] [-l datasize] [-r] [host]
//     
//     -a       Address family (IPv4 or IPv6)
//     -i ttl   TTL value to set on socket
//     -l size  Amount of data to send as part of the ICMP request
//     -r       Use IPv4 record route
//     host     Hostname or literal address
//

#include <WinSock2.h>
#include <WS2tcpip.h>		// include IP_HDRINCL
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")		// winsock 2.2 library

#define DEFAULT_DATA_SIZE      32       // default data size
#define DEFAULT_SEND_COUNT     4        // number of ICMP requests to send
#define DEFAULT_RECV_TIMEOUT   6000     // six second
#define DEFAULT_TTL            128		// Windows OS(128 = Windows, 64 = LINUX)

// global variables
int   gAddressFamily = AF_UNSPEC,         // Address family to use
	  gProtocol = IPPROTO_ICMP,           // Protocol value
	  gTtl = DEFAULT_TTL;                 // Default TTL value
int   gDataSize = DEFAULT_DATA_SIZE;      // Amount of data to send
BOOL  bRecordRoute = FALSE;               // Use IPv4 record route?
char *gDestination = NULL;                // Destination

// define ICMP Header Structure
typedef struct _icmp_hdr_{
	unsigned char icmp_type;
	unsigned char icmp_code;
	unsigned short icmp_checksum;
	unsigned short id;
	unsigned short icmp_sequence;
	unsigned long icmp_timestamp;
}ICMP_HDR;

// define IP Header Structure
typedef struct _ip_hdr_{
	unsigned char  ip_verlen;        // 4-bit IPv4 version
	// 4-bit header length (in 32-bit words)
	unsigned char  ip_tos;           // IP type of service
	unsigned short ip_totallength;   // Total length
	unsigned short ip_id;            // Unique identifier 
	unsigned short ip_offset;        // Fragment offset field
	unsigned char  ip_ttl;           // Time to live
	unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
	unsigned short ip_checksum;      // IP checksum
	unsigned int   ip_srcaddr;       // Source address
	unsigned int   ip_destaddr;      // Source address
}IPV4_HDR;

void ValidateArgs(int argc, char **argv);

//
// Function: usage
//
// Description:
//    Print usage information.
//
void usage(char *progname)
{
	printf("usage: ping -r <host> [data size]\n");
	printf("       -a 4|6       Address family\n");
	printf("       -i ttl       Time to live\n");
	printf("       -l bytes     Amount of data to send\n");
	printf("       -r           Record route (IPv4 only)\n");
	printf("        host        Remote machine to ping\n");
	ExitProcess(-1);
}

// main function
int main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET PingSocket;

	int iResult;
	int packetlen = 0;

	ICMP_HDR *icmp_hdr = NULL;
	IPV4_HDR *ipv4_hdr = NULL;

	// Parse the command line
//	ValidateArgs(argc, argv);

	// Initialize Winsock 2.2
	printf("Initializing Winsock\n");
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup() failed with Error: %d\n", WSAGetLastError());
		return 0;
	}
	printf("Winsock initialized successfully!\n");

	// Create Raw Socket
	printf("Creating PingSocket\n");
	// ICMP에 대해서만 Raw Socket 되도록 만들 예정이어서 IPPROTO_ICMP로 설정
	PingSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (PingSocket == INVALID_SOCKET)
	{
		printf("PingSocket Not Created with Error: %d\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}
	printf("PingSocket Created!\n");

	// Initialize ICMP headers
	char *icmpbuf = NULL;
	char *datapart = NULL;

	// Figure out size of the ICMP Header and payload 
	packetlen = sizeof(ICMP_HDR) + gDataSize;
	icmpbuf = (char *)malloc(packetlen);		// ICMP 메시지 = ICMP_HDR + Packet 메모리 할당
	if (icmpbuf == NULL)
	{
		printf("icmpbuf memory allocation failed : %d\n", GetLastError());
		return 0;
	}
	memset(icmpbuf, 0x00, packetlen);
	icmp_hdr = (ICMP_HDR *)icmpbuf;
	icmp_hdr->icmp_type = 8;		// ICMP Echo Request
	icmp_hdr->icmp_code = 0;
	icmp_hdr->icmp_checksum = 0;
	icmp_hdr->icmp_sequence = 1;
	icmp_hdr->id = (unsigned short)GetCurrentProcessId();

	icmp_hdr->icmp_timestamp = GetTickCount();

	// 데이터 부분을 임의의 문자로 채운다
	datapart = icmpbuf + sizeof(ICMP_HDR);
	memset(datapart, 'E', gDataSize);
	
	// SOCKADDR 구조체에 목적지 주소를 설정
	SOCKADDR_IN destAddr, srcAddr;
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(0);		// ICMP에서 Port는 무시함.
	destAddr.sin_addr.S_un.S_addr = inet_addr("172.20.17.133");

	// 목적지 주소로 ICMP Echo Request를 Send
	printf("\nSending ICMP Packet...\n");
	iResult = sendto(PingSocket, icmpbuf, packetlen, 0, (SOCKADDR *)&destAddr, sizeof(destAddr));
	if (iResult == SOCKET_ERROR)
	{
		printf("sendto error : %d\n", WSAGetLastError());
		return 0;
	}
#if 0
	// 목적지 주소로부터 ICMP Echo Reply를 Recv
	int fromlen = sizeof(srcAddr);
	iResult = recvfrom(PingSocket, icmpbuf, packetlen, 0, (SOCKADDR *)&srcAddr, &fromlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("recvfrom error : %d\n", WSAGetLastError());
		return 0;
	}
#endif
	// Cleanup
	free(icmpbuf);
	if (PingSocket == INVALID_SOCKET)
	{
		closesocket(PingSocket);
	}	

	WSACleanup();	

	return 0;
}


//
// Function: ValidateArgs
//
// Description:
//    Parse the command line arguments.
//
void ValidateArgs(int argc, char **argv)
{
	int                i;

	for (i = 1; i < argc; i++)
	{
		if ((argv[i][0] == '-') || (argv[i][0] == '/'))
		{
			switch (tolower(argv[i][1]))
			{
			case 'a':        // address family
				if (i + 1 >= argc)
					usage(argv[0]);
				if (argv[i + 1][0] == '4')
					gAddressFamily = AF_INET;
				else if (argv[i + 1][0] == '6')
					gAddressFamily = AF_INET6;
				else
					usage(argv[0]);
				i++;
				break;
			case 'i':        // Set TTL value
				if (i + 1 >= argc)
					usage(argv[0]);
				gTtl = atoi(argv[++i]);
				break;
			case 'l':        // buffer size tos end
				if (i + 1 >= argc)
					usage(argv[0]);
				gDataSize = atoi(argv[++i]);
				break;
			case 'r':        // record route option
				bRecordRoute = TRUE;
				break;
			default:
				usage(argv[0]);
				break;
			}
		}
		else
		{
			gDestination = argv[i];
		}
	}
	return;
}
