#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include "packet.h"
#include "queue.h"
#include "../command_types.h"
#include "rs232_com.h"

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
		queue->elements[queue->last].stop = STOP_BYTE;

		//printf("Enqueued Packet values : %d, %d\n", queue->elements[queue->last].command, queue->elements[queue->last].value);
	}

	pthread_mutex_unlock(&mutex);
}

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

			rs232_putchar(p->start);
			rs232_putchar(p->command);
			rs232_putchar(p->value);
			rs232_putchar(p->stop);
		
			// XXX: To be Done: Check for ACK. If NACK - Resend from beginning. Timeout if nothing is received and retry again.

		        //printf("Dequeued Packet values : %d, %d\n", p->command, p->value);
        	}

		pthread_mutex_unlock(&mutex);
	}
	free(p);
}