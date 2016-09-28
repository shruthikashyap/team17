#include <stdio.h>
#include <stdlib.h>
#include "packet.h"
#include "queue.h"
#include "../command_types.h"
#include "../crc.h"

void sendKeyPacket(char key)
{
	struct packet_t p;
	int p_flag = 0;

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
			p.value = YAW_CONTROL_MODE;
			p_flag = 1;
			break;

		case '4':
			p.command = MODE_TYPE;
			p.value = FULL_CONTROL_MODE;
			p_flag = 1;
			break;

		case '5':
			p.command = MODE_TYPE;
			p.value = CALIBRATION_MODE;
			p_flag = 1;
			break;

		case '6':
			p.command = MODE_TYPE;
			p.value = HEIGHT_CONTROL_MODE;
			p_flag = 1;
			break;

		case '7':
			p.command = MODE_TYPE;
			p.value = WIRELESS_MODE;
			p_flag = 1;
			break;

		case '8':
			p.command = MODE_TYPE;
			p.value = RAW_MODE;
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
			p.value = INCREASE;
			p_flag = 1;
			break;

		case 'C':
			p.command = KEY_ROLL;
			p.value = DECREASE;
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
			p.value = DECREASE;
			p_flag = 1;
			break;

		case 'w':
			p.command = KEY_YAW;
			p.value = INCREASE;
			p_flag = 1;
			break;

		case 'q':
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

		default:
			break;
	}
	if (p_flag == 1)
	{	
		//printf("\n\nPacket values from keyboard: %d, %d\n", p.command, p.value);
		compute_crc(&p);
		enqueue(p);
	}
}