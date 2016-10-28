/*------------------------------------------------------------------
 *  process_packet.c
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include "process_packet.h"
#include "in4073.h"
#include "../crc.h"

bool log_upload_flag 		= false;
bool log_active_flag 		= false;
bool height_control_flag 	= false;
bool raw_mode_flag 			= false;
bool imu_init_flag 			= true;
bool abort_flag 			= false;

/*------------------------------------------------------------------
 *  void send_packet_to_pc
 *
 *  Sends packet from QR to the PC
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void send_packet_to_pc(struct packet_t p)
{
	uart_put(p.start);
	uart_put(p.command);
	uart_put(p.value);
	uart_put(p.crc);
	uart_put(p.stop);
}

/*------------------------------------------------------------------
 *  void send_packet_ack
 *
 *  Sends ACK (acknowledgement for receiving a packet from PC to QR 
 *	in good order) from QR to the PC
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void send_packet_ack(int ack)
{	
	struct packet_t p;
	p.start = START_BYTE;
	p.command = ACK;
	p.value = ack;
	compute_crc(&p);
	p.stop = STOP_BYTE;
}

/*------------------------------------------------------------------
 *  void set_mode_type
 *
 *  Update global variable for the current mode, drone.current_mode
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void set_mode_type(char value)
{
	// Return if drone is already in this mode
	if(drone.current_mode == value)
	{
		return;
	}
	
	// Don't change mode if drone is not in safe mode
	// Accept change to safe and panic modes during other modes
	if(drone.current_mode == SAFE_MODE || value == PANIC_MODE || value == ABORT || (value == SAFE_MODE && drone.ae[0] == 0 && drone.ae[1] == 0 && drone.ae[2] == 0 && drone.ae[3] == 0))
	{
		switch(value)
		{
			case ABORT:
						abort_flag = true;
						if(drone.current_mode == SAFE_MODE)
						{
							send_packet_ack(ACK_SUCCESS);
							break;
						}
				
						drone.current_mode = PANIC_MODE;
						drone.change_mode = 1;
						send_packet_ack(ACK_SUCCESS);
						break;
			case SAFE_MODE:
						drone.current_mode = SAFE_MODE;
						drone.change_mode = 1;
						send_packet_ack(ACK_SUCCESS);
						break;
			case PANIC_MODE:
						if(drone.current_mode == SAFE_MODE)
						{
							send_packet_ack(ACK_SUCCESS);
							break;
						}
				
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
			default:
						send_packet_ack(ACK_FAILURE);
						break;
		}
	}
}

/*------------------------------------------------------------------
 *  void set_key_lift
 *
 *  Update global variable for lift value from keyboard, 
 *	drone.key_lift
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void set_key_lift(char value)
{	
	if(value == 1)
	{
		drone.key_lift += KEYBOARD_STEP;
		if(drone.key_lift > MAX_TRIM_LIFT)
			drone.key_lift = MAX_TRIM_LIFT;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else if(value == 0)
	{
		drone.key_lift -= KEYBOARD_STEP;
		if(drone.key_lift < MIN_TRIM_LIFT)
			drone.key_lift = MIN_TRIM_LIFT;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else
	{
		send_packet_ack(ACK_FAILURE);
	}
}

/*------------------------------------------------------------------
 *  void set_key_roll
 *
 *  Update global variable for roll value from keyboard,
 *	drone.key_roll
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void set_key_roll(char value)
{	
	if(value == 1)
	{
		drone.key_roll += KEYBOARD_STEP;
		if(drone.key_roll > MAX_TRIM_ROLL)
			drone.key_roll = MAX_TRIM_ROLL;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else if(value == 0)
	{
		drone.key_roll -= KEYBOARD_STEP;
		if(drone.key_roll < MIN_TRIM_ROLL)
			drone.key_roll = MIN_TRIM_ROLL;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else
	{
		send_packet_ack(ACK_FAILURE);
	}
}

/*------------------------------------------------------------------
 *  void set_key_pitch
 *
 *  Update global variable for pitch value from keyboard,
 *	drone.key_pitch
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void set_key_pitch(char value)
{
	if(value == 1)
	{
		drone.key_pitch += KEYBOARD_STEP;
		if(drone.key_pitch > MAX_TRIM_PITCH)
			drone.key_pitch = MAX_TRIM_PITCH;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else if(value == 0)
	{
		drone.key_pitch -= KEYBOARD_STEP;
		if(drone.key_pitch < MIN_TRIM_PITCH)
			drone.key_pitch = MIN_TRIM_PITCH;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else
	{
		send_packet_ack(ACK_FAILURE);
	}
}

/*------------------------------------------------------------------
 *  void set_key_yaw
 *
 *  Update global variable for yaw value from keyboard,
 *	drone.key_yaw
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void set_key_yaw(char value)
{
	if(value == 1)
	{
		drone.key_yaw += KEYBOARD_STEP;
		if(drone.key_yaw > MAX_TRIM_YAW)
			drone.key_yaw = MAX_TRIM_YAW;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else if(value == 0)
	{
		drone.key_yaw -= KEYBOARD_STEP;
		if(drone.key_yaw < MIN_TRIM_YAW)
			drone.key_yaw = MIN_TRIM_YAW;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else
	{
		send_packet_ack(ACK_FAILURE);
	}
}

/*------------------------------------------------------------------
 *  void set_key_control_yaw
 *
 *  Update global variable for yaw control gain, drone.controlgain_yaw
 *
 *  Author : Kars Heinen
 *------------------------------------------------------------------
 */
void set_key_control_yaw(char value)
{	
	if(value == 1)
	{
		drone.controlgain_yaw += KEYBOARD_CONTROL_STEP;
		if(drone.controlgain_yaw > MAX_CONTROLGAIN_YAW)
			drone.controlgain_yaw = MAX_CONTROLGAIN_YAW;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else if(value == 0)
	{
		drone.controlgain_yaw -= KEYBOARD_CONTROL_STEP;
		if(drone.controlgain_yaw < MIN_CONTROLGAIN_YAW)
			drone.controlgain_yaw = MIN_CONTROLGAIN_YAW;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else
	{
		send_packet_ack(ACK_FAILURE);
	}
}

/*------------------------------------------------------------------
 *  void set_key_control_p1
 *
 *  Update global variable for control gain P1, drone.controlgain_p1
 *
 *  Author : Kars Heinen
 *------------------------------------------------------------------
 */
void set_key_control_p1(char value)
{
	if(value == 1)
	{
		drone.controlgain_p1 += KEYBOARD_CONTROL_STEP;
		if(drone.controlgain_p1 > MAX_CONTROLGAIN_P1)
			drone.controlgain_p1 = MAX_CONTROLGAIN_P1;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else if(value == 0)
	{
		drone.controlgain_p1 -= KEYBOARD_CONTROL_STEP;
		if(drone.controlgain_p1 < MIN_CONTROLGAIN_P1)
			drone.controlgain_p1 = MIN_CONTROLGAIN_P1;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else
	{
		send_packet_ack(ACK_FAILURE);
	}
}

/*------------------------------------------------------------------
 *  void set_key_control_p2
 *
 *  Update global variable for control gain P2, drone.controlgain_p2
 *
 *  Author : Kars Heinen
 *------------------------------------------------------------------
 */
void set_key_control_p2(char value)
{	
	if(value == 1)
	{
		drone.controlgain_p2 += KEYBOARD_CONTROL_STEP;
		if(drone.controlgain_p2 > MAX_CONTROLGAIN_P2)
			drone.controlgain_p2 = MAX_CONTROLGAIN_P2;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else if(value == 0)
	{
		drone.controlgain_p2 -= KEYBOARD_CONTROL_STEP;
		if(drone.controlgain_p2 < MIN_CONTROLGAIN_P2)
			drone.controlgain_p2 = MIN_CONTROLGAIN_P2;
		
		send_packet_ack(ACK_SUCCESS);
	}
	else
	{
		send_packet_ack(ACK_FAILURE);
	}
}

/*------------------------------------------------------------------
 *  void set_joy_lift
 *
 *  Update global variable for lift value from joystick,
 *	drone.joy_lift
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void set_joy_lift(signed char value)
{	
	drone.joy_lift = -value + 127;
	send_packet_ack(ACK_SUCCESS);
}

/*------------------------------------------------------------------
 *  void set_joy_roll
 *
 *  Update global variable for roll value from joystick,
 *	drone.joy_roll
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void set_joy_roll(char value)
{
	drone.joy_roll = value;
	send_packet_ack(ACK_SUCCESS);
}

/*------------------------------------------------------------------
 *  void set_joy_pitch
 *
 *  Update global variable for pitch value from joystick,
 *	drone.joy_pitch
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void set_joy_pitch(char value)
{	
	drone.joy_pitch = value;
	send_packet_ack(ACK_SUCCESS);
}

/*------------------------------------------------------------------
 *  void set_joy_yaw
 *
 *  Update global variable for yaw value from joystick,
 *	drone.joy_yaw
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void set_joy_yaw(char value)
{
	drone.joy_yaw = value;
	send_packet_ack(ACK_SUCCESS);
}

/*------------------------------------------------------------------
 *  void process_packet
 *
 *  Process the packets obtained from the PC and update the 
 *	corresponding global variables
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void process_packet(struct packet_t packet)
{
	// Ignore any incoming packet if drone is in safe mode, except commands
	// 	1) change to another mode
	//	2) change to RAW mode
	//	3) retrieve log 
	if((drone.current_mode == SAFE_MODE && !(packet.command == RAW_MODE || packet.command == MODE_TYPE || packet.command == LOG)) || (drone.current_mode != SAFE_MODE && packet.command == RAW_MODE))
	{
		return;
	}

	// Send Failure ACK if CRC check failed for the received packet
	if(check_crc(packet))
	{
		send_packet_ack(ACK_FAILURE);
		return;
	}
	
	// Assign packet value to the appropriate variable
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
		case KEY_CONTROL_YAW:
				set_key_control_yaw(packet.value);
				break;
		case KEY_CONTROL_P1:
				set_key_control_p1(packet.value);
				break;
		case KEY_CONTROL_P2:
				set_key_control_p2(packet.value);
				break;
		case LOG:
				// Start logging
				if (packet.value == LOG_START)					
				{
					log_active_flag = true;
					nrf_gpio_pin_toggle(GREEN);
				}
				// Stop logging
				else if (packet.value == LOG_STOP)
				{
					log_active_flag = false;
					nrf_gpio_pin_toggle(GREEN);
				}
				// Upload log to PC only if in Safe mode
				else if (packet.value == LOG_UPLOAD && drone.current_mode == SAFE_MODE)
				{
					log_upload_flag = true;
				}
				break;
		case HEIGHT_CONTROL_MODE:
				if(height_control_flag == false)
				{
					height_control_flag = true;
					drone.height_control_pressure = pressure;
					drone.height_control_lift = drone.joy_lift;
				}
				else
				{
					height_control_flag = false;
				}
				break;
		case RAW_MODE:
				// Change to RAW from DMP mode
				if(raw_mode_flag == false)
				{
					raw_mode_flag = true;
				}
				// Change to DMP from RAW mode
				else
				{
					raw_mode_flag = false;
				}
				imu_init_flag = false;
				break;
		default :
				send_packet_ack(ACK_FAILURE);
				break;
	}
}


