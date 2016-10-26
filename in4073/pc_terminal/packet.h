/*------------------------------------------------------------------
 *  packet.h
 *
 *  Declaration for packet.c
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#ifndef PACKET_H__
#define PACKET_H__

#define KEY_ESCAPE 27

void sendKeyPacket(char);
void sendKeyEscape();

#endif
