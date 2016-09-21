#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <unistd.h>
#include "packet.h"
#include "queue.h"
#include "../command_types.h"
#include "rs232_com.h"
#include "../crc.h"

int ack_received = 2;

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
			if(p->value == PANIC_MODE)
			{
				rs232_putchar(p->start);
				rs232_putchar(p->command);
				rs232_putchar(p->value);
				rs232_putchar(10);
				rs232_putchar(p->stop);
			}
			else
			{
				rs232_putchar(p->start);
				rs232_putchar(p->command);
				rs232_putchar(p->value);
				rs232_putchar(p->crc);
				rs232_putchar(p->stop);	
			}
			
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
		
		//usleep(1000);
	}
	
	free(p);
}

void* process_receive_packets(void* thread)
{
	#if 1
	// XXX: Receive telemetry data, log data, acknowledgement, etc.
	struct packet_t p;
	char ch;
	int rcv_byte_count = 0;
	
	while(1)
	{
		if ((ch = rs232_getchar_nb()) != -1)
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
		
		usleep(100); //(10000);
	}
	#endif
}
