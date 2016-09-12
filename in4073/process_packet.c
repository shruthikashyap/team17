#include <stdio.h>
#include "process_packet.h"
#include "process_mode.h"
#include "in4073.h"

/*------------------------------------------------------------------
 * process_key -- process command keys
 *------------------------------------------------------------------
 */
void process_key(uint8_t c) 
{
	switch (c) 
	{
		case 'q':
			ae[0] += 10;
			break;
		case 'a':
			ae[0] -= 10;
			if (ae[0] < 0) ae[0] = 0;
			break;
		case 'w':
			ae[1] += 10;
			break;
		case 's':
			ae[1] -= 10;
			if (ae[1] < 0) ae[1] = 0;
			break;
		case 'e':
			ae[2] += 10;
			break;
		case 'd':
			ae[2] -= 10;
			if (ae[2] < 0) ae[2] = 0;
			break;
		case 'r':
			ae[3] += 10;
			break;
		case 'f':
			ae[3] -= 10;
			if (ae[3] < 0) ae[3] = 0;
			break;
		case 27:
			demo_done = true;
			break;
		default:
			nrf_gpio_pin_toggle(RED);
	}
}

void process_packet(struct packet_t packet)
{
	switch (packet.command)
	{
		case MODE_TYPE:
		      break;
		case KEY_LIFT:
		      break;
		case KEY_ROLL:
		      break;
		case KEY_PITCH:
		      break;
		case KEY_YAW:
		      break;
		case JOY_LIFT:
		      break;
		case JOY_ROLL:
		      break;
		case JOY_PITCH:
		      break;
		case JOY_YAW:
		      break;
		default :
		      // XXX: Needs to be handled
		      break;
	}
}


