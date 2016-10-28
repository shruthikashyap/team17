/*------------------------------------------------------------------
 *  crc.c
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include "command_types.h"
#include "crc.h"

/*------------------------------------------------------------------
 *  void compute_crc
 *
 *  This function computes the CRC value for the command and the value
 *  byte using a 5th degree polynomial
 *
 *  Author : Evelyn Rashmi Jeyachandra
 *------------------------------------------------------------------
 */
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

	// Four bit-remainder from the command byte is concatenated with
	// four bit-remainder from the value byte and is assigned to the 
	// CRC byte of the packet

	crc = (crc_command & MASK) | ((crc_value & MASK)>>4) ;
	packet->crc = crc;
}

/*------------------------------------------------------------------
 *  void check_crc
 *
 *  This function recalculates the CRC for command byte and the value byte
 *  to see if it matches the CRC that was sent with the packet.
 *
 *  Author : Evelyn Rashmi Jeyachandra
 *------------------------------------------------------------------
 */
int check_crc(struct packet_t check_packet)
{
	struct packet_t p;
	p = check_packet;
	compute_crc(&check_packet);
	return (check_packet.crc == p.crc)? 0 : 1; 
}