/*------------------------------------------------------------------
 *  get_log.c
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include "../command_types.h"

int log_start_flag = 0;
int log_byte_count = 0;
struct log log_line;

char filename[100] = "log.txt";
FILE *fp;

/*------------------------------------------------------------------
 *  void get_log
 *
 *  This function obtains the run log from the drone. This is the 
 *	final step of the drone operation.
 *
 *  Author : Evelyn Rashmi Jeyachandra
 *------------------------------------------------------------------
 */
void get_log(uint8_t ch)
{	
	unsigned char uch = (unsigned char)ch;
	uint32_t shift_32;
	int16_t shift_16;
		
		switch (log_byte_count)
		{
			case 0:
				if ((unsigned char)ch == LOG_LINE_START)
				{
					//Initialize the log_line struct
					log_line.current_time = 0;
					log_line.current_mode = 0;
					log_line.sp = 0;
					log_line.sq = 0;
					log_line.sr = 0;
					log_line.sax = 0;
					log_line.say = 0;
					log_line.saz = 0;
					log_line.phi = 0;
					log_line.theta = 0;
					log_line.psi = 0;

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
				log_byte_count++;
				break;
			case 2:
				shift_32 = (uint32_t)uch;
				log_line.current_time = log_line.current_time|(shift_32<<8);	// bits 8-15 of current_time
				log_byte_count++;
				break;
			case 3:
				shift_32 = (uint32_t)uch;		
				log_line.current_time = log_line.current_time|(shift_32<<16);	// bits 16-23 of current_time
				log_byte_count++;
				break;
			case 4:
				shift_32 = (uint32_t)uch;
				log_line.current_time = log_line.current_time|(shift_32<<24);	// bits 24-31 of current_time
				log_byte_count++;
				break;	
			case 5:
				log_line.current_mode = uch;									// current_mode
				log_byte_count++;	
				break;
			case 6:	
				shift_16 = (int16_t)ch;
				log_line.sp = log_line.sp|shift_16;								// bits 0-7 of sp
				log_byte_count++;
				break;
			case 7:	
				shift_16 = (int16_t)ch;		
				log_line.sp = log_line.sp|(shift_16<<8);						// bits 8-15 of sp
				log_byte_count++;
				break;
			case 8:	
				shift_16 = (int16_t)ch;	
				log_line.sq = log_line.sq|shift_16;								// bits 0-7 of sq
				log_byte_count++;
				break;
			case 9:		
				shift_16 = (int16_t)ch;
				log_line.sq = log_line.sq|(shift_16<<8);						// bits 8-15 of sq
				log_byte_count++;
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
				break;
			case 18:
				shift_16 = (int16_t)ch;		
				log_line.phi = log_line.phi | shift_16;							// bits 0-7 of phi
				log_byte_count++;
				break;
			case 19:
				shift_16 = (int16_t)ch;		
				log_line.phi = log_line.phi | (shift_16<<8); 					// bits 8-15 of phi
				log_byte_count++;
				break;
			case 20:		
				shift_16 = (int16_t)ch;
				log_line.theta = log_line.theta | shift_16;						// bits 0-7 of theta
				log_byte_count++;
				break;
			case 21:	
				shift_16 = (int16_t)ch;	
				log_line.theta = log_line.theta | (shift_16<<8);				// bits 8-15 of theta
				log_byte_count++;
				break;
			case 22:	
				shift_16 = (int16_t)ch;	
				log_line.psi = log_line.psi | shift_16;							// bits 0-7 of psi
				log_byte_count++;
				break;
			case 23:		
				shift_16 = (int16_t)ch;
				log_line.psi = log_line.psi | (shift_16<<8);					// bits 8-15 of psi
				log_byte_count++;
				break;
			case 24:
				if (uch == LOG_LINE_END)
				{
					fp = fopen(filename, "a+");
					if (fp == NULL)
						printf("Cannot open file!\n");					
					fprintf(fp, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
						log_line.current_time, log_line.current_mode, log_line.sp, log_line.sq, log_line.sr, 
						log_line.sax, log_line.say, log_line.saz, log_line.phi, log_line.theta,
						log_line.psi);
					fclose(fp);
				}
				log_byte_count = 0;
				break;
			
			default:
				break;
		}
}