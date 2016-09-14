/*------------------------------------------------------------
 * Simple pc terminal in C
 * 
 * Arjan J.C. van Gemund (+ mods by Ioannis Protonotarios)
 *
 * read more: http://mirror.datenwolf.net/serial/
 *------------------------------------------------------------
 */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#include "command_types.h"
#include "joystick_read.h"
#include "queue.h"
#include "rs232_com.h"
#include "term_io.h"
#include "packet.h"

struct packet_t *packet;
struct queue_t *queue;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
int main(int argc, char **argv)
{
	char	c;//, key;
	//int 	axis[6];
	//int 	button[12];
	struct  packet_t *p;
	struct  packet_t packet;
	int 	ret_1=0;
	pthread_t pthread_dequeue;

	p = &packet;
/*
	int axis[6];
	if ((fd = open(JS_DEV, O_RDONLY)) < 0) {
		perror("jstest");
		exit(1);
	}
*/
	/* non-blocking mode
 	*/
//	fcntl(fd, F_SETFL, O_NONBLOCK);
	
	term_puts("\nTerminal program - Embedded Real-Time Systems\n");

	queue = createQueue(100);
	
	pthread_create(&pthread_dequeue, NULL, process_dequeue, (void*)p);
 	if (ret_1 )
 		printf("Dequeue error\n");
 	pthread_mutex_init(&mutex, NULL);

	term_initio();
	rs232_open();

	term_puts("Type ^C to exit\n");

	/* discard any incoming text
	 */
	while ((c = rs232_getchar_nb()) != -1)
		fputc(c,stderr);
	
	/* send & receive
	 */
	for (;;) 
	{
		usleep(1000);
		if ((c = term_getchar_nb()) != -1) 
		{
			sendKeyPacket(c);		
		}

		//sendJsPacket();

		if ((c = rs232_getchar_nb()) != -1) 
			term_putchar(c);

	}

	term_exitio();
	rs232_close();
	free(queue->elements);
	free(queue);
	pthread_join(pthread_dequeue, NULL);
	term_puts("\n<exit>\n");
  	
	return 0;
}


