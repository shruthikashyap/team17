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
				demo_done = 1;
				break;
		default:
				nrf_gpio_pin_toggle(RED);
	}*/
}

void send_packet_ack(int ack)
{
	// XXX: Send ack to PC
	printf("Ack value = %d\n", ack);
}

void set_mode_type(char value)
{
	printf("In set_mode_type - %d\n", value);
	
	// Return if drone is already in this mode
	if(drone.current_mode == value)
	{
		printf("Already in %d mode\n", value);
		return;
	}
	
	// Don't change mode if drone is not in safe mode
	// Accept change to safe and panic modes during other modes
	if(drone.current_mode == SAFE_MODE || (value == SAFE_MODE || value == PANIC_MODE))
	{
		switch(value)
		{
			case SAFE_MODE:
						drone.current_mode = SAFE_MODE;
						drone.change_mode = 1;
						send_packet_ack(ACK_SUCCESS);
						break;
			case PANIC_MODE:
						if(drone.current_mode == SAFE_MODE)
							break;
						drone.current_mode = PANIC_MODE;
						drone.change_mode = 1;
						send_packet_ack(ACK_SUCCESS);
						break;
			case MANUAL_MODE:
						drone.current_mode = MANUAL_MODE;
						drone.change_mode = 1;
						send_packet_ack(ACK_SUCCESS);
						break;
			case YAW_CONTROL_MODE:
						drone.current_mode = YAW_CONTROL_MODE;
						drone.change_mode = 1;
						send_packet_ack(ACK_SUCCESS);
						break;
			case FULL_CONTROL_MODE:
						drone.current_mode = FULL_CONTROL_MODE;
						drone.change_mode = 1;
						send_packet_ack(ACK_SUCCESS);
						break;
			case CALIBRATION_MODE:
						drone.current_mode = CALIBRATION_MODE;
						drone.change_mode = 1;
						send_packet_ack(ACK_SUCCESS);
						break;
			case HEIGHT_CONTROL_MODE:
						// XXX: Not considered a mode. Needs to be handled.
						break;
			case WIRELESS_MODE:
						// XXX: Not considered a mode. Needs to be handled.
						break;
			case RAW_MODE:
						// XXX: Not considered a mode. Needs to be handled.
						break;
			default:
						printf("Invalid mode\n");
						// XXX: Send NACK
						send_packet_ack(ACK_FAILURE);
		}
	}
}

void set_key_lift(char value)
{
	printf("In set_key_lift - %d\n", value);
	
	if(value == 1)
	{
		drone.key_lift += KEYBOARD_STEP;
		if(drone.key_lift > MAX_LIFT)
			drone.key_lift = MAX_LIFT;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else if(value == 0)
	{
		drone.key_lift -= KEYBOARD_STEP;
		if(drone.key_lift < MIN_LIFT)
			drone.key_lift = MIN_LIFT;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else
	{
		// XXX: Send NACK
		send_packet_ack(ACK_FAILURE);
	}
}

void set_key_roll(char value)
{
	printf("In set_key_roll - %d\n", value);
	
	if(value == 1)
	{
		drone.key_roll += KEYBOARD_STEP;
		if(drone.key_roll > MAX_ROLL)
			drone.key_roll = MAX_ROLL;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else if(value == 0)
	{
		drone.key_roll -= KEYBOARD_STEP;
		if(drone.key_roll < MIN_ROLL)
			drone.key_roll = MIN_ROLL;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else
	{
		// XXX: Send NACK
		send_packet_ack(ACK_FAILURE);
	}
}

void set_key_pitch(char value)
{
	printf("In set_key_pitch - %d\n", value);
	
	if(value == 1)
	{
		drone.key_pitch += KEYBOARD_STEP;
		if(drone.key_pitch > MAX_PITCH)
			drone.key_pitch = MAX_PITCH;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else if(value == 0)
	{
		drone.key_pitch -= KEYBOARD_STEP;
		if(drone.key_pitch < MIN_PITCH)
			drone.key_pitch = MIN_PITCH;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else
	{
		// XXX: Send NACK
		send_packet_ack(ACK_FAILURE);
	}
}

void set_key_yaw(char value)
{
	printf("In set_key_yaw - %d\n", value);
	
	if(value == 1)
	{
		drone.key_yaw += KEYBOARD_STEP;
		if(drone.key_yaw > MAX_YAW)
			drone.key_yaw = MAX_YAW;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else if(value == 0)
	{
		drone.key_yaw -= KEYBOARD_STEP;
		if(drone.key_yaw < MIN_YAW)
			drone.key_yaw = MIN_YAW;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else
	{
		// XXX: Send NACK
		send_packet_ack(ACK_FAILURE);
	}
}

void set_joy_lift(char value)
{
	printf("In set_joy_lift - %d\n", value);
	
	drone.joy_lift = value;
	
	send_packet_ack(ACK_SUCCESS);
}

void set_joy_roll(char value)
{
	printf("In set_joy_roll - %d\n", value);
	
	drone.joy_lift = value;
	
	send_packet_ack(ACK_SUCCESS);
}

void set_joy_pitch(char value)
{
	printf("In set_joy_pitch - %d\n", value);
	
	drone.joy_lift = value;
	
	send_packet_ack(ACK_SUCCESS);
}

void set_joy_yaw(char value)
{
	printf("In set_joy_yaw - %d\n", value);
	
	drone.joy_lift = value;
	
	send_packet_ack(ACK_SUCCESS);
}

void process_packet(struct packet_t packet)
{
	//printf("Received byte:%d, %d\n", packet.command, packet.value);

// XXX: Test
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
				// XXX: Send NACK
				send_packet_ack(ACK_FAILURE);
				break;
	}
}


