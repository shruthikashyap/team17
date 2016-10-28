/*------------------------------------------------------------------
 *  crc.h
 *
 *  Declartions of CRC functions
 *
 *------------------------------------------------------------------
 */

#ifndef CRC_H__
#define CRC_H__

void compute_crc(struct packet_t * packet);
int check_crc(struct packet_t packet);

#endif
