#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "packet.h"
#include "queue.h"
#include "../command_types.h"
#include "rs232_com.h"
#include "term_io.h"
#include "../crc.h"

int ack_received = 2;
int log_start_flag = 0;
int log_byte_count = 0;
struct log log_line;

char filename[100] = "log.txt";
FILE *fp;

struct queue_t* createQueue(int numelements)
{
	struct queue_t * q = (struct queue_t *)malloc(sizeof(struct queue_t));
	q->elements = (struct packet_t *)malloc(sizeof(struct packet_t)*numelements);
	q->size = 0;
	q->capacity = numelements;
	q->first = 0;
	q->last = -1;

	return q;
}


void enqueue(struct packet_t p)
{	
	pthread_mutex_lock(&mutex);

	if(queue->size < queue->capacity)
	{
		queue->size++;
		queue->last = queue->last + 1;
		/* As we fill the queue in circular fashion */
		if(queue->last == queue->capacity)
		{
			queue->last = 0;
		}
		/* Insert the element in its rear side */
		queue->elements[queue->last].start = START_BYTE;
		queue->elements[queue->last].command = p.command;
		queue->elements[queue->last].value = p.value;
		queue->elements[queue->last].crc = p.crc;
		queue->elements[queue->last].stop = STOP_BYTE;

		//printf("Enqueued Packet values : %d, %d\n", queue->elements[queue->last].command, queue->elements[queue->last].value);
	}

	pthread_mutex_unlock(&mutex);
}

void* process_dequeue(void* thread)
{
	struct packet_t *p = (struct packet_t*)malloc(sizeof(struct packet_t));
	int retry_count = 0;
	int time_out = 0;

	while(1)
	{
		pthread_mutex_lock(&mutex);
		
		if(queue->size > 0)
		{
			(*p) = queue->elements[queue->first];
			queue->size--;
			queue->first++;
			/* As we fill elements in circular fashion */
			if(queue->first == queue->capacity)
			{
					queue->first = 0;
			}

			// Send data
			rs232_putchar(p->start);
			rs232_putchar(p->command);
			rs232_putchar(p->value);
			rs232_putchar(p->crc);
			rs232_putchar(p->stop);	
			
			// Reset ack_received
			ack_received = 2;
			
			//printf("Dequeued Packet values : %d, %d\n", p->command, p->value);

			// XXX: To be Done: Check for ACK. If NACK - Resend from beginning. Timeout if nothing is received and retry again.
			// XXX: Retry for PANIC_MODE - ack_received
			// ACK SUCCESS = 0, FAILURE = 1
			#if 1
			if(p->command == MODE_TYPE && p->value == PANIC_MODE)
			{
				//printf("Inside panic mode retry logic!\n");
				usleep(100000);
				if(ack_received != ACK_SUCCESS)
				{
					for(retry_count = 0; retry_count < 3; retry_count++)
					{
						for(time_out = 0; time_out < 10 && ack_received == ACK_INVALID; time_out++);
						{
							usleep(200);
						}
						printf("Retry Ack for PANIC command received = %d\n", ack_received);

						if(ack_received == ACK_SUCCESS)
							break;
						else
						{
							// Send data
							rs232_putchar(p->start);
							rs232_putchar(p->command);
							rs232_putchar(p->value);
							rs232_putchar(p->crc);
							rs232_putchar(p->stop);
							
							// Reset ack_received
							ack_received = 2;
						}
						usleep(100000);
					}
				}
				else
				{
					printf("Ack for PANIC command received = %d\n", ack_received);
				}
			}
			#endif
		}

		pthread_mutex_unlock(&mutex);
		
		usleep(20000);
	}
	
	free(p);
}

void get_log(uint8_t ch)
{	
	unsigned char uch = (unsigned char)ch;
	uint32_t shift_32;
	int16_t shift_16;
	uint16_t shift_u16;

	//printf("ch: %d\n", ch);
		
		switch (log_byte_count)
		{
			case 0:
				if ((unsigned char)ch == LOG_LINE_START)
				{
					//Initialize the log_line struct
					log_line.current_time = 0;
					log_line.sp = 0;
					log_line.sq = 0;
					log_line.sr = 0;
					log_line.sax = 0;
					log_line.say = 0;
					log_line.saz = 0;
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
				shift_16 = (int16_t)ch;
				log_line.sp = log_line.sp|shift_16;								// bits 0-7 of sp
				log_byte_count++;
				//printf("sp1: %d\n",log_line.sp);
				break;
			case 6:	
				shift_16 = (int16_t)ch;		
				log_line.sp = log_line.sp|(shift_16<<8);						// bits 8-15 of sp
				log_byte_count++;
				//printf("sp: %d\n",log_line.sp);
				break;
			case 7:	
				shift_16 = (int16_t)ch;	
				log_line.sq = log_line.sq|shift_16;								// bits 0-7 of sq
				log_byte_count++;
				//printf("sq1: %d\n",log_line.sq);
				break;
			case 8:		
				shift_16 = (int16_t)ch;
				log_line.sq = log_line.sq|(shift_16<<8);						// bits 8-15 of sq
				log_byte_count++;
				//printf("sq: %d\n",log_line.sq);
				break;
			case 9:		
				shift_16 = (int16_t)ch;
				log_line.sr = log_line.sr | shift_16;							// bits 0-7 of sr
				log_byte_count++;
				break;
			case 10:
				shift_16 = (int16_t)ch;		
				log_line.sr = log_line.sr | (shift_16<<8);						// bits 8-15 of sr
				log_byte_count++;
				//printf("sr: %d\n",log_line.sr);
				break;
			case 11:
				shift_16 = (int16_t)ch;		
				log_line.sax = log_line.sax | shift_16;							// bits 0-7 of sax
				log_byte_count++;
				break;
			case 12:
				shift_16 = (int16_t)ch;		
				log_line.sax = log_line.sax | (shift_16<<8);					// bits 8-15 of sax
				log_byte_count++;
				//printf("sax: %d\n",log_line.sax);
				break;
			case 13:		
				shift_16 = (int16_t)ch;
				log_line.say = log_line.say | shift_16;							// bits 0-7 of say
				log_byte_count++;
				break;
			case 14:	
				shift_16 = (int16_t)ch;	
				log_line.say = log_line.say | (shift_16<<8);					// bits 8-15 of say
				log_byte_count++;
				//printf("say: %d\n",log_line.say);
				break;
			case 15:	
				shift_16 = (int16_t)ch;	
				log_line.saz = log_line.saz | shift_16;							// bits 0-7 of saz
				log_byte_count++;
				break;
			case 16:		
				shift_16 = (int16_t)ch;
				log_line.saz = log_line.saz | (shift_16<<8);					// bits 8-15 of saz
				log_byte_count++;
				//printf("saz: %d\n",log_line.saz);
				break;
			case 17:	
				shift_u16 = (uint16_t)ch;	
				log_line.bat_volt = log_line.bat_volt | shift_u16;				// bits 0-7 of bat_volt
				log_byte_count++;
				break;
			case 18:	
				shift_u16 = (uint16_t)ch;
				log_line.bat_volt = log_line.bat_volt | (shift_u16<<8);			// bits 8-15 of bat_volt
				log_byte_count++;
				//printf("bat_volt: %d\n",log_line.bat_volt);
				break;
			case 19:
				if (uch == LOG_LINE_END)
				{
					//printf("%d %d %d %d %d %d %d %d \n", log_line.current_time, log_line.sp, log_line.sq, log_line.sr, log_line.sax, log_line.say, log_line.saz, log_line.bat_volt);
					fp = fopen(filename, "a+");
					if (fp == NULL)
						printf("Cannot open file!\n");
					fprintf(fp, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",log_line.current_time, log_line.sp, log_line.sq, log_line.sr, log_line.sax, log_line.say, log_line.saz, log_line.bat_volt);
					fclose(fp);
				}
				log_byte_count = 0;
				break;
			
			default:
				break;
		}
		
		//printf("log byte count : %d\n",log_byte_count);
}

void* process_receive_packets(void* thread)
{
	#if 1
	char c;
	while(1)
	{
		if((c = rs232_getchar_nb()) != -1) 
			term_putchar(c);

		usleep(250);
	}
	#endif
	
	#if 0
	// XXX: Receive telemetry data, log data, acknowledgement, etc.
	struct packet_t p;
	uint8_t ch;
	int rcv_byte_count = 0;

	fp = fopen(filename, "w+");
	if (fp == NULL)
		printf("Cannot open file!\n");
	fclose(fp);
	
	while(1)
	{
		if ((ch = rs232_getchar_nb()) != -1)
		{
			//printf("%d\n", ch);
			if (log_start_flag == 1)
			{
				get_log(ch);
			}
			else
			{
				if((unsigned char)ch == START_BYTE)
				{
					if(rcv_byte_count == 0)
					{
						p.start = ch;

						// Start receiving
						rcv_byte_count = 1;
					}
					else
					{
						// XXX: Packet error
						//printf("Packet error: %d!\n", ch);

						// Reset flags
						rcv_byte_count = 0;
					}
				}
				else if((unsigned char)ch == STOP_BYTE)
				{
					if(rcv_byte_count == 4)
					{
						p.stop = ch;

						// Process packet
						if (!check_crc(p))
						{
							printf("Packet received from QR - command = %d, value = %d\n", p.command, p.value);
							if(p.command == ACK)
							{
								// Update a ack_received global variable
								ack_received = p.value;
								//printf("Updating global ack = %d\n", ack_received);
							}
							else if (p.command == LOG && p.value == LOG_START)
							{
								log_start_flag = 1;
							}
							else
							{
								// XXX: Need to handle other packets
							}
						}
						else
						{
							printf("Packet error due to incorrect CRC\n");
						}
					}
					else
					{
						// XXX: Packet error
						//printf("Packet error: %d!\n", ch);
					}

					// Reset flag
					rcv_byte_count = 0;

					usleep(100000);
				}
				else if(rcv_byte_count == 1)
				{
					p.command = ch;
					rcv_byte_count++;
				}
				else if(rcv_byte_count == 2)
				{
					p.value = ch;
					rcv_byte_count++;
				}
				else if(rcv_byte_count == 3)
				{
					p.crc = ch;
					rcv_byte_count++;
				}
				else
				{
					// XXX: Packet error
					//printf("Packet error: %d!\n", ch);

					// Reset flag
					rcv_byte_count = 0;
				}
			}
		}
		else
		{
			usleep(100000);
		}
	}
	#endif
}
