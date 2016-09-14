#include <stdio.h>
#include "process_packet.h"
#include "in4073.h"

/*------------------------------------------------------------------
 * process_key -- process command keys
 *------------------------------------------------------------------
 */
void process_key(struct packet_t p) 
{
	printf("Received byte:%d, %d\n", p.command, p.value);
	/*if(drone.current_mode == SAFE_MODE)
	{
		printf("\nIgnoring drone control commands in SAFE_MODE");
		return;
	}
	else
		printf("\nIn process_key");*/
	
	/*switch (c) 
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
	}*/
}

void set_mode_type()
{
}

void set_key_lift()
{
}

void set_key_roll()
{
}

void set_key_pitch()
{
}

void set_key_yaw()
{
}

void set_joy_lift()
{
}

void set_joy_roll()
{
}

void set_joy_pitch()
{
}

void set_joy_yaw()
{
}

void process_packet(struct packet_t packet)
{
	if(drone.current_mode == SAFE_MODE && packet.command != MODE_TYPE)
	{
		printf("\nIgnoring drone control commands in SAFE_MODE");
		return;
	}
		
	switch (packet.command)
	{
		case MODE_TYPE:
				set_mode_type();
				break;
		case KEY_LIFT:
				set_key_lift();
				break;
		case KEY_ROLL:
				set_key_roll();
				break;
		case KEY_PITCH:
				set_key_pitch();
				break;
		case KEY_YAW:
				set_key_yaw();
				break;
		case JOY_LIFT:
				set_joy_lift();
				break;
		case JOY_ROLL:
				set_joy_roll();
				break;
		case JOY_PITCH:
				set_joy_pitch();
				break;
		case JOY_YAW:
				set_joy_yaw();
				break;
		default :
				// XXX: Packet error. Needs to be handled
				break;
	}
}


