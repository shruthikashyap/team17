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

void set_mode_type(char value)
{
	printf("In set_mode_type - %d\n", (unsigned char)value);
	
	// Return if drone is already in this mode
	if(drone.current_mode == (unsigned char)value)
	{
		printf("Already in %d mode\n", value);
		return;
	}
	
	// Don't change mode if drone is not in safe mode
	// Accept change to safe and panic modes during other modes
	if(drone.current_mode == SAFE_MODE || (value == SAFE_MODE || value == PANIC_MODE))
	{
		drone.current_mode = (unsigned char)value;
		drone.change_mode = true;
	}
}

void set_key_lift(char value)
{
	printf("In set_key_lift - %d\n", (unsigned char)value);
	
	if((unsigned char)value == 1)
	{
		drone.key_lift += RPM_STEP;
		if(drone.key_lift > MAX_LIFT)
			drone.key_lift = MAX_LIFT;
	}
	else if((unsigned char)value == 0)
	{
		drone.key_lift -= RPM_STEP;
		if(drone.key_lift < MIN_LIFT)
			drone.key_lift = MIN_LIFT;
	}
}

void set_key_roll(char value)
{
	printf("In set_key_roll - %d\n", (unsigned char)value);
	
	if((unsigned char)value == 1)
	{
		drone.key_roll += RPM_STEP;
		if(drone.key_roll > MAX_ROLL)
			drone.key_roll = MAX_ROLL;
	}
	else if((unsigned char)value == 0)
	{
		drone.key_roll -= RPM_STEP;
		if(drone.key_roll < MIN_ROLL)
			drone.key_roll = MIN_ROLL;
	}
}

void set_key_pitch(char value)
{
	printf("In set_key_pitch - %d\n", (unsigned char)value);
	
	if((unsigned char)value == 1)
	{
		drone.key_pitch += RPM_STEP;
		if(drone.key_pitch > MAX_PITCH)
			drone.key_pitch = MAX_PITCH;
	}
	else if((unsigned char)value == 0)
	{
		drone.key_pitch -= RPM_STEP;
		if(drone.key_pitch < MIN_PITCH)
			drone.key_pitch = MIN_PITCH;
	}
}

void set_key_yaw(char value)
{
	printf("In set_key_yaw - %d\n", (unsigned char)value);
	
	if((unsigned char)value == 1)
	{
		drone.key_yaw += RPM_STEP;
		if(drone.key_yaw > MAX_YAW)
			drone.key_yaw = MAX_YAW;
	}
	else if((unsigned char)value == 0)
	{
		drone.key_yaw -= RPM_STEP;
		if(drone.key_yaw < MIN_YAW);
			drone.key_yaw = MIN_YAW;
	}
}

void set_joy_lift(char value)
{
	printf("In set_joy_lift - %d\n", (unsigned char)value);
	
	drone.joy_lift = ((unsigned char)value > MAX_LIFT) ? MAX_LIFT : ((unsigned char)value < MIN_LIFT) ? MIN_LIFT : (unsigned char)value;
}

void set_joy_roll(char value)
{
	printf("In set_joy_roll - %d\n", (unsigned char)value);
	
	drone.joy_lift = ((unsigned char)value > MAX_ROLL) ? MAX_ROLL : ((unsigned char)value < MIN_ROLL) ? MIN_ROLL : (unsigned char)value;
}

void set_joy_pitch(char value)
{
	printf("In set_joy_pitch - %d\n", (unsigned char)value);
	
	drone.joy_lift = ((unsigned char)value > MAX_PITCH) ? MAX_PITCH : ((unsigned char)value < MIN_PITCH) ? MIN_PITCH : (unsigned char)value;
}

void set_joy_yaw(char value)
{
	printf("In set_joy_yaw - %d\n", (unsigned char)value);
	
	drone.joy_lift = ((unsigned char)value > MAX_YAW) ? MAX_YAW : ((unsigned char)value < MIN_YAW) ? MIN_YAW : (unsigned char)value;
}

void process_packet(struct store_packet_t packet)
{
	printf("Received byte:%d, %d\n", packet.command, packet.value);
	
#if 0
	if(drone.current_mode == SAFE_MODE && packet.command != MODE_TYPE)
	{
		printf("\nIgnoring drone control commands in SAFE_MODE");
		return;
	}
#endif
	
	switch (packet.command)
	{
		case MODE_TYPE:
				//printf("\ndrone.current_mode = %d\n", drone.current_mode);
				set_mode_type(packet.value);
				break;
		case KEY_LIFT:
				set_key_lift(packet.value);
				break;
		case KEY_ROLL:
				set_key_roll(packet.value);
				break;
		case KEY_PITCH:
				set_key_pitch(packet.value);
				break;
		case KEY_YAW:
				set_key_yaw(packet.value);
				break;
		case JOY_LIFT:
				set_joy_lift(packet.value);
				break;
		case JOY_ROLL:
				set_joy_roll(packet.value);
				break;
		case JOY_PITCH:
				set_joy_pitch(packet.value);
				break;
		case JOY_YAW:
				set_joy_yaw(packet.value);
				break;
		default :
				// XXX: Packet error. Needs to be handled
				break;
	}
}


