
#include <Windows.h>
#include "ip_checksum.h"
 
unsigned short ip_checksum(unsigned short *buffer, int size)
{
	unsigned long cksum = 0;

	// Sum all the words together, adding the final byte if size is odd
	while (size > 1)
	{
		cksum += *buffer++;
		size -= sizeof(unsigned short);
	}
	if (size)
	{
		cksum += *(unsigned char *)buffer;
	}

	// Do a little shuffling
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);

	return (unsigned short) (~cksum);
}