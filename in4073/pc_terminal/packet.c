/*------------------------------------------------------------------
 *  packet.c
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include "packet.h"
#include "queue.h"
#include "../command_types.h"
#include "../crc.h"

/*------------------------------------------------------------------
 *  void sendKeyPacket
 *
 *  Reads the keyboard values and queues it to be sent to the drone
 *
 *  Author : Evelyn Rashmi Jeyachandra
 *------------------------------------------------------------------
 */
void sendKeyPacket(char key)
{
	struct packet_t p;
	int p_flag = 0;

	// Choose appropriate command and value to be sent
	switch(key)
	{
		case '0':
			p.command = MODE_TYPE;
			p.value = SAFE_MODE;
			p_flag = 1;
			break;

		case '1':
			p.command = MODE_TYPE;
			p.value = PANIC_MODE;
			p_flag = 1;
			break;

		case '2':
			p.command = MODE_TYPE;
			p.value = MANUAL_MODE;
			p_flag = 1;
			break;

		case '3':
			p.command = MODE_TYPE;
			p.value = CALIBRATION_MODE;
			p_flag = 1;
			break;

		case '4':
			p.command = MODE_TYPE;
			p.value = YAW_CONTROL_MODE;
			p_flag = 1;
			break;

		case '5':
			p.command = MODE_TYPE;
			p.value = FULL_CONTROL_MODE;
			p_flag = 1;
			break;

		case '6':
			p.command = RAW_MODE;
			p.value = 0;
			p_flag = 1;
			break;

		case '7':
			p.command = HEIGHT_CONTROL_MODE;
			p.value = 0;
			p_flag = 1;
			break;

		case '8':
			p.command = MODE_TYPE;
			p.value = WIRELESS_MODE;
			p_flag = 1;
			break;

		case 'a':
			p.command = KEY_LIFT;
			p.value = INCREASE;
			p_flag = 1;
			break;

		case 'z':
			p.command = KEY_LIFT;
			p.value = DECREASE;
			p_flag = 1;
			break;

		case 'D':
			p.command = KEY_ROLL;
			p.value = DECREASE;
			p_flag = 1;
			break;

		case 'C':
			p.command = KEY_ROLL;
			p.value = INCREASE;
			p_flag = 1;
			break;

		case 'A':
			p.command = KEY_PITCH;
			p.value = DECREASE;
			p_flag = 1;
			break;

		case 'B':
			p.command = KEY_PITCH;
			p.value = INCREASE;
			p_flag = 1;
			break;

		case 'q':
			p.command = KEY_YAW;
			p.value = INCREASE;
			p_flag = 1;
			break;

		case 'w':
			p.command = KEY_YAW;
			p.value = DECREASE;
			p_flag = 1;
			break;

		case 'u':
			p.command = KEY_CONTROL_YAW;
			p.value = INCREASE;
			p_flag = 1;
			break;

		case 'j':
			p.command = KEY_CONTROL_YAW;
			p.value = DECREASE;
			p_flag = 1;
			break;

		case 'i':
			p.command = KEY_CONTROL_P1;
			p.value = INCREASE;
			p_flag = 1;
			break;

		case 'k':
			p.command = KEY_CONTROL_P1;
			p.value = DECREASE;
			p_flag = 1;
			break;

		case 'o':
			p.command = KEY_CONTROL_P2;
			p.value = INCREASE;
			p_flag = 1;
			break;									

		case 'l':
			p.command = KEY_CONTROL_P2;
			p.value = DECREASE;
			p_flag = 1;
			break;

		// For Log
		case 'b':
			p.command = LOG;
			p.value = LOG_START;
			p_flag = 1;
			break;

		case 's':
			p.command = LOG;
			p.value = LOG_STOP;
			p_flag = 1;
			break;

		case 'g':
			p.command = LOG;
			p.value = LOG_UPLOAD;
			p_flag = 1;
			break;

		default:
			break;
	}
	// Fill CRC byte and enqueue
	if (p_flag == 1)					
	{	
		compute_crc(&p);
		enqueue(p);
	}
}

/*------------------------------------------------------------------
 *  void sendKeyEscape
 *
 *  Sends the ABORT command to the drone
 *
 *  Author : Kars Heinen
 *------------------------------------------------------------------
 */
void sendKeyEscape()
{
	struct packet_t p;

	p.command = MODE_TYPE;
	p.value = ABORT;
	
	compute_crc(&p);
	enqueue(p);
}