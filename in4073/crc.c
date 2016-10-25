/*------------------------------------------------------------------
 *  crc.c
 *
 *  Computes and checks CRC values of packets
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include "command_types.h"
#include "crc.h"

void compute_crc(struct packet_t * packet)
{
	int i;
	char crc;
	char crc_command = 0;
	char crc_value = 0;

	crc_command ^= packet->command;
	crc_value ^= packet->value;

	for (i=0; i<8; i++)
	{								
		if (crc_command & MSB)
			crc_command ^= CRC_POLY;
		crc_command <<= 1;

		if (crc_value & MSB)
			crc_value ^= CRC_POLY;
		crc_value <<= 1;
	}

	crc = (crc_command & MASK) | ((crc_value & MASK)>>4) ;
	packet->crc = crc;

}


int check_crc(struct packet_t check_packet)
{
	struct packet_t p;
	p = check_packet;
	compute_crc(&check_packet);
	//printf("Input CRC : %d, Check CRC: %d\n", p.crc, check_packet.crc );
	return (check_packet.crc == p.crc)? 0 : 1; 
}