

#define ICMP_MESSAGE_ECHO_REQUEST	8
#define ICMP_MESSAGE_ECHO_REPLY		0

// define ICMP Header Structure
typedef struct _icmp_hdr_{
	BYTE icmp_type;
	BYTE icmp_code;
	USHORT icmp_checksum;
	USHORT icmp_id;
	USHORT icmp_sequence;
	ULONG icmp_timestamp;
}ICMP_HDR;

// define IPv4 Header Structure
typedef struct _ipv4_hdr_{
	unsigned char  ip_header_length;
	unsigned char  ip_verlen;        // 4-bit IPv4 version	
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



// define function
int init_ping_packet(ICMP_HDR* icmp_hdr, int packet_size);
int setup_for_ping(char* host,
					SOCKET& RawSocket, /*out*/
					SOCKADDR_IN &dest /*out*/
					);
int allocate_buffers(ICMP_HDR*& send_buf, /*out*/
					 IPV4_HDR*& recv_buf, /*out*/
					 int packet_size);
int send_ping(SOCKET socket, 
	const sockaddr_in& dest, 
	ICMP_HDR* send_buf, 
	int packet_size);
int recv_ping(SOCKET socket,
	const sockaddr_in& source,
	IPV4_HDR* recv_buf,
	int packet_size);