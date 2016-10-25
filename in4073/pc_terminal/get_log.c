/*------------------------------------------------------------------
 *  get_log.c
 *
 *  Downloads the log from the board
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include "../command_types.h"

int log_start_flag = 0;
int log_byte_count = 0;
struct log log_line;

char filename[100] = "log.txt";
FILE *fp;

void get_log(uint8_t ch)
{	
	unsigned char uch = (unsigned char)ch;
	uint32_t shift_32;
	int16_t shift_16;
	//uint16_t shift_u16;

	//printf("ch: %d\n", ch);
		
		switch (log_byte_count)
		{
			case 0:
				if ((unsigned char)ch == LOG_LINE_START)
				{
					//Initialize the log_line struct
					log_line.current_time = 0;
					log_line.current_mode = 0;
					log_line.joy_lift = 0;
					log_line.joy_roll = 0;
					log_line.joy_pitch = 0;
					log_line.joy_yaw = 0;
					log_line.key_lift = 0;
					log_line.key_roll = 0;
					log_line.key_pitch = 0;
					log_line.key_yaw = 0;
					log_line.ae_0 = 0;
					log_line.ae_1 = 0;
					log_line.ae_2 = 0;
					log_line.ae_3 = 0;
					log_line.sp = 0;
					log_line.sq = 0;
					log_line.sr = 0;
					log_line.sax = 0;
					log_line.say = 0;
					log_line.saz = 0;
					log_line.controlgain_yaw = 0;
					log_line.controlgain_p1 = 0;
					log_line.controlgain_p2 = 0;
					log_line.pressure = 0;
					log_line.bat_volt = 0;

					log_byte_count++;
				}
				else
				{
					log_byte_count = 0;
				}
				break;
				
			case 1:
				shift_32 = (uint32_t)uch;
				log_line.current_time = log_line.current_time|shift_32;			// bits 0-7 of current_time
				//printf("current time 0-7: %u\n", log_line.current_time);
				log_byte_count++;
				break;
			case 2:
				shift_32 = (uint32_t)uch;
				log_line.current_time = log_line.current_time|(shift_32<<8);	// bits 8-15 of current_time
				//printf("current time 8-15: %d\n", log_line.current_time);
				log_byte_count++;
				break;
			case 3:
				shift_32 = (uint32_t)uch;		
				log_line.current_time = log_line.current_time|(shift_32<<16);	// bits 16-23 of current_time
				//printf("current time 16-23: %d\n", log_line.current_time);
				log_byte_count++;
				break;
			case 4:
				shift_32 = (uint32_t)uch;
				log_line.current_time = log_line.current_time|(shift_32<<24);	// bits 24-31 of current_time
				//printf("current time: %d\n",log_line.current_time);
				log_byte_count++;
				break;	
			case 5:
				log_line.current_mode = uch;									// current_mode
				log_byte_count++;	
				break;
#if 0
			case 6:
				shift_16 = (int16_t)ch;
				log_line.joy_lift = log_line.joy_lift|shift_16;					// bits 0-7 of joy_lift
				log_byte_count++;
				break;
			case 7:
				shift_16 = (int16_t)ch;
				log_line.joy_lift = log_line.joy_lift|(shift_16<<8);			// bits 8-15 of joy_lift
				log_byte_count++;
				break;
			case 8:
				shift_16 = (int16_t)ch;
				log_line.joy_roll = log_line.joy_roll|shift_16;					// bits 0-7 of joy_roll
				log_byte_count++;
				break;
			case 9:
				shift_16 = (int16_t)ch;
				log_line.joy_roll = log_line.joy_roll|(shift_16<<8);			// bits 8-15 of joy_roll
				log_byte_count++;
				break;
			case 10:
				shift_16 = (int16_t)ch;
				log_line.joy_pitch = log_line.joy_pitch|shift_16;				// bits 0-7 of joy_pitch
				log_byte_count++;
				break;
			case 11:
				shift_16 = (int16_t)ch;
				log_line.joy_pitch = log_line.joy_pitch|(shift_16<<8);			// bits 8-15 of joy_pitch
				log_byte_count++;
				break;
			case 12:
				shift_16 = (int16_t)ch;
				log_line.joy_yaw = log_line.joy_yaw|shift_16;					// bits 0-7 of joy_yaw
				log_byte_count++;
				break;
			case 13:
				shift_16 = (int16_t)ch;
				log_line.joy_yaw = log_line.joy_yaw|(shift_16<<8);				// bits 8-15 of joy_yaw
				log_byte_count++;
				break;
			case 14:
				shift_16 = (int16_t)ch;
				log_line.key_lift = log_line.key_lift|shift_16;					// bits 0-7 of key_lift
				log_byte_count++;
				break;
			case 15:
				shift_16 = (int16_t)ch;
				log_line.key_lift = log_line.key_lift|(shift_16<<8);			// bits 8-15 of key_lift
				log_byte_count++;
				break;
			case 16:
				shift_16 = (int16_t)ch;
				log_line.key_roll = log_line.key_roll|shift_16;					// bits 0-7 of key_roll
				log_byte_count++;
				break;
			case 17:
				shift_16 = (int16_t)ch;
				log_line.key_roll = log_line.key_roll|(shift_16<<8);			// bits 8-15 of key_roll
				log_byte_count++;
				break;
			case 18:
				shift_16 = (int16_t)ch;
				log_line.key_pitch = log_line.key_pitch|shift_16;				// bits 0-7 of key_pitch
				log_byte_count++;
				break;
			case 19:
				shift_16 = (int16_t)ch;
				log_line.key_pitch = log_line.key_pitch|(shift_16<<8);			// bits 8-15 of key_pitch
				log_byte_count++;
				break;
			case 20:
				shift_16 = (int16_t)ch;
				log_line.key_yaw = log_line.key_yaw|shift_16;					// bits 0-7 of key_yaw
				log_byte_count++;
				break;
			case 21:
				shift_16 = (int16_t)ch;
				log_line.key_yaw = log_line.key_yaw|(shift_16<<8);				// bits 8-15 of key_yaw
				log_byte_count++;
				break;
			case 22:
				shift_16 = (int16_t)ch;
				log_line.ae_0 = log_line.ae_0|shift_16;							// bits 0-7 of ae_0
				log_byte_count++;
				break;
			case 23:
				shift_16 = (int16_t)ch;
				log_line.ae_0 = log_line.ae_0|(shift_16<<8);					// bits 8-15 of ae_0
				log_byte_count++;
				break;
			case 24:
				shift_16 = (int16_t)ch;
				log_line.ae_1 = log_line.ae_1|shift_16;							// bits 0-7 of ae_1
				log_byte_count++;
				break;
			case 25:
				shift_16 = (int16_t)ch;
				log_line.ae_1 = log_line.ae_1|(shift_16<<8);					// bits 8-15 of ae_1
				log_byte_count++;
				break;
			case 26:
				shift_16 = (int16_t)ch;
				log_line.ae_2 = log_line.ae_2|shift_16;							// bits 0-7 of ae_2
				log_byte_count++;
				break;
			case 27:
				shift_16 = (int16_t)ch;
				log_line.ae_2 = log_line.ae_2|(shift_16<<8);					// bits 8-15 of ae_2
				log_byte_count++;
				break;
			case 28:
				shift_16 = (int16_t)ch;
				log_line.ae_3 = log_line.ae_3|shift_16;							// bits 0-7 of ae_3
				log_byte_count++;
				break;
			case 29:
				shift_16 = (int16_t)ch;
				log_line.ae_3 = log_line.ae_3|(shift_16<<8);					// bits 8-15 of ae_3
				log_byte_count++;
				break;
#endif
			case 6:	
				shift_16 = (int16_t)ch;
				log_line.sp = log_line.sp|shift_16;								// bits 0-7 of sp
				log_byte_count++;
				//printf("sp1: %d\n",log_line.sp);
				break;
			case 7:	
				shift_16 = (int16_t)ch;		
				log_line.sp = log_line.sp|(shift_16<<8);						// bits 8-15 of sp
				log_byte_count++;
				//printf("sp: %d\n",log_line.sp);
				break;
			case 8:	
				shift_16 = (int16_t)ch;	
				log_line.sq = log_line.sq|shift_16;								// bits 0-7 of sq
				log_byte_count++;
				//printf("sq1: %d\n",log_line.sq);
				break;
			case 9:		
				shift_16 = (int16_t)ch;
				log_line.sq = log_line.sq|(shift_16<<8);						// bits 8-15 of sq
				log_byte_count++;
				//printf("sq: %d\n",log_line.sq);
				break;
			case 10:		
				shift_16 = (int16_t)ch;
				log_line.sr = log_line.sr | shift_16;							// bits 0-7 of sr
				log_byte_count++;
				break;
			case 11:
				shift_16 = (int16_t)ch;		
				log_line.sr = log_line.sr | (shift_16<<8);						// bits 8-15 of sr
				log_byte_count++;
				//printf("sr: %d\n",log_line.sr);
				break;
			case 12:
				shift_16 = (int16_t)ch;		
				log_line.sax = log_line.sax | shift_16;							// bits 0-7 of sax
				log_byte_count++;
				break;
			case 13:
				shift_16 = (int16_t)ch;		
				log_line.sax = log_line.sax | (shift_16<<8);					// bits 8-15 of sax
				log_byte_count++;
				//printf("sax: %d\n",log_line.sax);
				break;
			case 14:		
				shift_16 = (int16_t)ch;
				log_line.say = log_line.say | shift_16;							// bits 0-7 of say
				log_byte_count++;
				break;
			case 15:	
				shift_16 = (int16_t)ch;	
				log_line.say = log_line.say | (shift_16<<8);					// bits 8-15 of say
				log_byte_count++;
				//printf("say: %d\n",log_line.say);
				break;
			case 16:	
				shift_16 = (int16_t)ch;	
				log_line.saz = log_line.saz | shift_16;							// bits 0-7 of saz
				log_byte_count++;
				break;
			case 17:		
				shift_16 = (int16_t)ch;
				log_line.saz = log_line.saz | (shift_16<<8);					// bits 8-15 of saz
				log_byte_count++;
				//printf("saz: %d\n",log_line.saz);
				break;
#if 0
			case 42:
				shift_16 = (int16_t)ch;		
				log_line.controlgain_yaw = log_line.controlgain_yaw | shift_16;		// bits 0-7 of controlgain_yaw
				log_byte_count++;
				break;
			case 43:
				shift_16 = (int16_t)ch;		
				log_line.controlgain_yaw = log_line.controlgain_yaw | (shift_16<<8); // bits 8-15 of controlgain_yaw
				log_byte_count++;
				//printf("sax: %d\n",log_line.sax);
				break;
			case 44:		
				shift_16 = (int16_t)ch;
				log_line.controlgain_p1 = log_line.controlgain_p1 | shift_16;		// bits 0-7 of controlgain_p1
				log_byte_count++;
				break;
			case 45:	
				shift_16 = (int16_t)ch;	
				log_line.controlgain_p1 = log_line.controlgain_p1 | (shift_16<<8);	// bits 8-15 of controlgain_p1
				log_byte_count++;
				//printf("say: %d\n",log_line.say);
				break;
			case 46:	
				shift_16 = (int16_t)ch;	
				log_line.controlgain_p2 = log_line.controlgain_p2 | shift_16;		// bits 0-7 of controlgain_p2
				log_byte_count++;
				break;
			case 47:		
				shift_16 = (int16_t)ch;
				log_line.controlgain_p2 = log_line.controlgain_p2 | (shift_16<<8);	// bits 8-15 of controlgain_p2
				log_byte_count++;
				//printf("saz: %d\n",log_line.saz);
				break;
			case 48:
				shift_32 = (int32_t)ch;
				log_line.pressure = log_line.pressure|shift_32;					// bits 0-7 of pressure
				//printf("current time 0-7: %u\n", log_line.pressure);
				log_byte_count++;
				break;
			case 49:
				shift_32 = (int32_t)ch;
				log_line.pressure = log_line.pressure|(shift_32<<8);			// bits 8-15 of pressure
				//printf("current time 0-7: %u\n", log_line.pressure);
				log_byte_count++;
				break;
			case 50:
				shift_32 = (int32_t)ch;
				log_line.pressure = log_line.pressure|(shift_32<<16);			// bits 16-23 of pressure
				//printf("current time 0-7: %u\n", log_line.pressure);
				log_byte_count++;
				break;
			case 51:
				shift_32 = (int32_t)ch;
				log_line.pressure = log_line.pressure|(shift_32<<24);			// bits 24-31 of pressure
				//printf("current time 0-7: %u\n", log_line.pressure);
				log_byte_count++;
				break;
			case 52:	
				shift_u16 = (uint16_t)ch;	
				log_line.bat_volt = log_line.bat_volt | shift_u16;				// bits 0-7 of bat_volt
				log_byte_count++;
				break;
			case 53:	
				shift_u16 = (uint16_t)ch;
				log_line.bat_volt = log_line.bat_volt | (shift_u16<<8);			// bits 8-15 of bat_volt
				log_byte_count++;
				//printf("bat_volt: %d\n",log_line.bat_volt);
				break;
#endif
			case 18:
				if (uch == LOG_LINE_END)
				{
					//printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", 
					//	log_line.current_time, log_line.current_mode, log_line.joy_lift, log_line.joy_roll, log_line.joy_pitch, 
					//	log_line.joy_yaw, log_line.key_lift, log_line.key_roll, log_line.key_pitch, log_line.key_yaw, 
					//	log_line.ae_0, log_line.ae_1, log_line.ae_2, log_line.ae_3, log_line.sp, log_line.sq, log_line.sr, 
					//	log_line.sax, log_line.say, log_line.saz, log_line.controlgain_yaw, log_line.controlgain_p1,
					//	log_line.controlgain_p2, log_line.pressure, log_line.bat_volt);
					fp = fopen(filename, "a+");
					if (fp == NULL)
						printf("Cannot open file!\n");
					/*fprintf(fp, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
						log_line.current_time, log_line.current_mode, log_line.joy_lift, log_line.joy_roll, log_line.joy_pitch, 
						log_line.joy_yaw, log_line.key_lift, log_line.key_roll, log_line.key_pitch, log_line.key_yaw, 
						log_line.ae_0, log_line.ae_1, log_line.ae_2, log_line.ae_3, log_line.sp, log_line.sq, log_line.sr, 
						log_line.sax, log_line.say, log_line.saz, log_line.controlgain_yaw, log_line.controlgain_p1,
						log_line.controlgain_p2, log_line.pressure, log_line.bat_volt);*/
					
					fprintf(fp, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
						log_line.current_time, log_line.current_mode, log_line.sp, log_line.sq, log_line.sr, 
						log_line.sax, log_line.say, log_line.saz);
					fclose(fp);
				}
				log_byte_count = 0;
				break;
			
			default:
				break;
		}
		
		//printf("log byte count : %d\n",log_byte_count);
}