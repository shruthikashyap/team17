/*------------------------------------------------------------------
 *  queue.c
 *------------------------------------------------------------------
 */

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

extern int log_start_flag;
extern int log_byte_count;
extern struct log log_line;

extern int telemetry_rcv_flag;
extern struct telemetry tele_data;

extern char filename[100];
extern FILE *fp;

/*------------------------------------------------------------------
 *  struct queue_t* createQueue
 *
 *  Creates a queue, where packets can be stored, as and when they
 *	arrive, to be sent to the QR
 *
 *  Author : Evelyn Rashmi Jeyachandra
 *------------------------------------------------------------------
 */
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

/*------------------------------------------------------------------
 *  void enqueue
 *
 *  Populates the queue with packets
 *
 *  Author : Evelyn Rashmi Jeyachandra
 *------------------------------------------------------------------
 */
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
	}

	pthread_mutex_unlock(&mutex);
}

/*------------------------------------------------------------------
 *  void process_dequeue
 *
 *  Dequeues packets from the queue and sends it to the QR
 *
 *  Author : Evelyn Rashmi Jeyachandra
 *------------------------------------------------------------------
 */
void* process_dequeue(void* thread)
{
	struct packet_t *p = (struct packet_t*)malloc(sizeof(struct packet_t));

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
		}

		pthread_mutex_unlock(&mutex);
		
		usleep(10000);
	}
	
	free(p);
}

/*------------------------------------------------------------------
 *  void process_receive_packets
 *
 *  Receives packets from the QR, which will be further used for 
 *	telemetry/logging purposes
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void* process_receive_packets(void* thread)
{
	// Receive telemetry data, log data, acknowledgement, etc.
	struct packet_t p;
	uint8_t ch;
	int rcv_byte_count = 0;

	fp = fopen(filename, "w+");
	if (fp == NULL)
		printf("Cannot open file!\n");							// Opens file for logging
	fclose(fp);
	
	while(1)
	{
		if((ch = rs232_getchar_nb()) != -1)
		{
			if(log_start_flag == 1)
			{	
				get_log(ch);									// Receive log from QR
			}
			else if(telemetry_rcv_flag == 1)
			{
				receive_telemetry_data(ch);						// Receive telemetry from QR
			}
			else
			{
				if((unsigned char)ch == START_BYTE)
				{
					if(rcv_byte_count == 0)
					{
						p.start = ch;
						rcv_byte_count = 1;						// Start receiving
					}
					else
					{
						rcv_byte_count = 0;						// Reset flags
					}
				}
				else if((unsigned char)ch == STOP_BYTE)
				{
					if(rcv_byte_count == 4)
					{
						p.stop = ch;
						if (!check_crc(p))						// Process packet
						{
							if(p.command == ACK)
							{
								ack_received = p.value;			// Update a ack_received global variable
							}
							else if(p.command == LOG && p.value == LOG_START)
							{
								log_start_flag = 1;
							}
							else if(p.command == TELEMETRY_DATA)
							{
								telemetry_rcv_flag = 1;
							}
						}
						else
						{
							printf("Packet error due to incorrect CRC\n");
						}
					}
					// Reset flag
					rcv_byte_count = 0;

					usleep(100000);
				}
				else if(rcv_byte_count == 1)
				{
					p.command = ch;								// Command byte received
					rcv_byte_count++;
				}
				else if(rcv_byte_count == 2)
				{
					p.value = ch;								// Value byte received
					rcv_byte_count++;
				}
				else if(rcv_byte_count == 3)
				{
					p.crc = ch;									// CRC byte received
					rcv_byte_count++;
				}
				else
				{
					rcv_byte_count = 0;
				}
			}
		}
	}
}
