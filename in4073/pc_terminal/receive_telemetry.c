/*------------------------------------------------------------------
 *  receive_telemetry.c
 *
 *  Receives and processes the telemetry packets and displays the
 *	data on console.
 *
 *  June 2016
 *------------------------------------------------------------------
 */


#include <stdio.h>
#include "../command_types.h"

int telemetry_rcv_flag = 0;
struct telemetry tele_data;

void receive_telemetry_data(uint8_t ch)
{
	static uint8_t count = 0;
	uint32_t shift_32;
	int16_t shift_16;
	uint16_t shift_u16;
	
	switch(count)
	{
		case 0:
			if((unsigned char)ch != TELE_START)
			{
				telemetry_rcv_flag = 0;
				count = 0;
			}
			else
			{
				tele_data.current_time = 0;
				tele_data.current_mode = 0;
				tele_data.bat_volt = 0;
				tele_data.ae[0] = 0;
				tele_data.ae[1] = 0;
				tele_data.ae[2] = 0;
				tele_data.ae[3] = 0;
				tele_data.controlgain_yaw = 0;
				tele_data.controlgain_p1 = 0;
				tele_data.controlgain_p2 = 0;
				tele_data.key_lift = 0;
				tele_data.key_roll = 0;
				tele_data.key_pitch = 0;
				tele_data.key_yaw = 0;
				count++;
			}
			break;

		case 1:
			shift_32 = (uint32_t)ch;
			tele_data.current_time = tele_data.current_time | shift_32;
			//printf("tele_data.current_time 0-7: %u\n", tele_data.current_time);
			count++;
			break;

		case 2:
			shift_32 = (uint32_t)ch;
			tele_data.current_time = tele_data.current_time | (shift_32<<8);
			//printf("tele_data.current_time 8-15: %u\n", tele_data.current_time);
			count++;
			break;

		case 3:
			shift_32 = (uint32_t)ch;
			tele_data.current_time = tele_data.current_time | (shift_32<<16);
			//printf("tele_data.current_time 16-23: %u\n", tele_data.current_time);
			count++;
			break;

		case 4:
			shift_32 = (uint32_t)ch;
			tele_data.current_time = tele_data.current_time | (shift_32<<24);
			//printf("tele_data.current_time 24-31: %u\n", tele_data.current_time);
			count++;
			break;
			
		case 5:
			tele_data.current_mode = ch;
			//printf("tele_data.current_mode: %d\n", tele_data.current_mode);
			count++;
			break;

		case 6:
			shift_u16 = (uint16_t)ch;	
			tele_data.bat_volt = tele_data.bat_volt | shift_u16;
			//printf("tele_data.bat_volt 0-7: %d\n", tele_data.bat_volt);
			count++;
			break;

		case 7:
			shift_u16 = (uint16_t)ch;	
			tele_data.bat_volt = tele_data.bat_volt | (shift_u16<<8);
			//printf("tele_data.bat_volt 8-16: %d\n", tele_data.bat_volt);
			count++;
			break;

		case 8:
			shift_16 = (int16_t)ch;
			tele_data.ae[0] = tele_data.ae[0] | shift_16;
			//printf("tele_data.ae[0] 0-7: %d\n", tele_data.ae[0]);
			count++;
			break;

		case 9:
			shift_16 = (int16_t)ch;
			tele_data.ae[0] = tele_data.ae[0] | (shift_16<<8);
			//printf("tele_data.ae[0] 8-16: %d\n", tele_data.ae[0]);
			count++;
			break;

		case 10:
			shift_16 = (int16_t)ch;
			tele_data.ae[1] = tele_data.ae[1] | shift_16;
			//printf("tele_data.ae[1] 0-7: %d\n", tele_data.ae[1]);
			count++;
			break;

		case 11:
			shift_16 = (int16_t)ch;
			tele_data.ae[1] = tele_data.ae[1] | (shift_16<<8);
			//printf("tele_data.ae[1] 8-16: %d\n", tele_data.ae[1]);
			count++;
			break;

		case 12:
			shift_16 = (int16_t)ch;
			tele_data.ae[2] = tele_data.ae[2] | shift_16;
			//printf("tele_data.ae[2] 0-7: %d\n", tele_data.ae[2]);
			count++;
			break;

		case 13:
			shift_16 = (int16_t)ch;
			tele_data.ae[2] = tele_data.ae[2] | (shift_16<<8);
			//printf("tele_data.ae[2] 8-16: %d\n", tele_data.ae[2]);
			count++;
			break;

		case 14:
			shift_16 = (int16_t)ch;
			tele_data.ae[3] = tele_data.ae[3] | shift_16;
			//printf("tele_data.ae[3] 0-7: %d\n", tele_data.ae[3]);
			count++;
			break;

		case 15:
			shift_16 = (int16_t)ch;
			tele_data.ae[3] = tele_data.ae[3] | (shift_16<<8);
			//printf("tele_data.ae[3] 8-16: %d\n", tele_data.ae[3]);
			count++;
			break;
			
		case 16:
			shift_16 = (int16_t)ch;
			tele_data.controlgain_yaw = tele_data.controlgain_yaw | (shift_16);
			//printf("tele_data.controlgain_yaw 0-7: %d\n", tele_data.controlgain_yaw);
			count++;
			break;
			
		case 17:
			shift_16 = (int16_t)ch;
			tele_data.controlgain_yaw = tele_data.controlgain_yaw | (shift_16<<8);
			//printf("tele_data.controlgain_yaw 8-16: %d\n", tele_data.controlgain_yaw);
			count++;
			break;
			
		case 18:
			shift_16 = (int16_t)ch;
			tele_data.controlgain_p1 = tele_data.controlgain_p1 | (shift_16);
			//printf("tele_data.controlgain_p1 0-7: %d\n", tele_data.controlgain_p1);
			count++;
			break;
			
		case 19:
			shift_16 = (int16_t)ch;
			tele_data.controlgain_p1 = tele_data.controlgain_p1 | (shift_16<<8);
			//printf("tele_data.controlgain_p1 8-16: %d\n", tele_data.controlgain_p1);
			count++;
			break;
			
		case 20:
			shift_16 = (int16_t)ch;
			tele_data.controlgain_p2 = tele_data.controlgain_p2 | (shift_16);
			//printf("tele_data.controlgain_p2 0-7: %d\n", tele_data.controlgain_p2);
			count++;
			break;
			
		case 21:
			shift_16 = (int16_t)ch;
			tele_data.controlgain_p2 = tele_data.controlgain_p2 | (shift_16<<8);
			//printf("tele_data.controlgain_p2 8-16: %d\n", tele_data.controlgain_p2);
			count++;
			break;
			
		case 22:
			tele_data.dmp_raw_mode = ch;
			//printf("tele_data.controlgain_p2 8-16: %d\n", tele_data.controlgain_p2);
			count++;
			break;
			
		case 23:
			shift_16 = (int16_t)ch;
			tele_data.key_lift = tele_data.key_lift | (shift_16);
			//printf("tele_data.controlgain_p2 0-7: %d\n", tele_data.controlgain_p2);
			count++;
			break;
			
		case 24:
			shift_16 = (int16_t)ch;
			tele_data.key_lift = tele_data.key_lift | (shift_16<<8);
			//printf("tele_data.controlgain_p2 0-7: %d\n", tele_data.controlgain_p2);
			count++;
			break;
			
		case 25:
			shift_16 = (int16_t)ch;
			tele_data.key_roll = tele_data.key_roll | (shift_16);
			//printf("tele_data.controlgain_p2 0-7: %d\n", tele_data.controlgain_p2);
			count++;
			break;
			
		case 26:
			shift_16 = (int16_t)ch;
			tele_data.key_roll = tele_data.key_roll | (shift_16<<8);
			//printf("tele_data.controlgain_p2 0-7: %d\n", tele_data.controlgain_p2);
			count++;
			break;
			
		case 27:
			shift_16 = (int16_t)ch;
			tele_data.key_pitch = tele_data.key_pitch | (shift_16);
			//printf("tele_data.controlgain_p2 0-7: %d\n", tele_data.controlgain_p2);
			count++;
			break;
			
		case 28:
			shift_16 = (int16_t)ch;
			tele_data.key_pitch = tele_data.key_pitch | (shift_16<<8);
			//printf("tele_data.controlgain_p2 0-7: %d\n", tele_data.controlgain_p2);
			count++;
			break;
			
		case 29:
			shift_16 = (int16_t)ch;
			tele_data.key_yaw = tele_data.key_yaw | (shift_16);
			//printf("tele_data.controlgain_p2 0-7: %d\n", tele_data.controlgain_p2);
			count++;
			break;
			
		case 30:
			shift_16 = (int16_t)ch;
			tele_data.key_yaw = tele_data.key_yaw | (shift_16<<8);
			//printf("tele_data.controlgain_p2 0-7: %d\n", tele_data.controlgain_p2);
			count++;
			break;

		case 31:
			if((unsigned char)ch == TELE_STOP)
			{
				#if 1
				printf("%d | ", tele_data.current_time);
				//printf("%d | ", tele_data.current_mode);
				
				// Print mode
				switch(tele_data.current_mode)
				{
					case SAFE_MODE: printf("SAFE_MODE | ");
						break;
					case PANIC_MODE: printf("PANIC_MODE | ");
						break;
					case MANUAL_MODE: printf("MANUAL_MODE | ");
						break;
					case YAW_CONTROL_MODE: printf("YAW_MODE | ");
						break;
					case FULL_CONTROL_MODE: printf("FULL_MODE | ");
						break;
					case CALIBRATION_MODE: printf("CALIBRATION_MODE | ");
						break;
					default: printf("ERROR | ");						
				}
				
				printf("%d %d %d | ", tele_data.controlgain_yaw, tele_data.controlgain_p1, tele_data.controlgain_p2);
				printf("%3d %3d %3d %3d | ", tele_data.ae[0], tele_data.ae[1], tele_data.ae[2], tele_data.ae[3]);
				
				// Print DMP/RAW Mode
				switch(tele_data.dmp_raw_mode)
				{
					case 0: printf("DMP_MODE | ");
						break;
					case 1: printf("RAW_MODE | ");
						break;
					default: printf("ERROR | ");						
				}
				
				printf("%d %d %d %d | ", tele_data.key_lift, tele_data.key_roll, tele_data.key_pitch, tele_data.key_yaw);
				
				printf("%d\n", tele_data.bat_volt);
				#endif
			}
			telemetry_rcv_flag = 0;
			count = 0;
			break;

		default:
			telemetry_rcv_flag = 0;
			count = 0;
			break;
	}
}
