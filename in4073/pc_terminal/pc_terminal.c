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

#include "../command_types.h"
#include "joystick_read.h"
#include "queue.h"
#include "rs232_com.h"
#include "term_io.h"
#include "packet.h"

struct packet_t *packet;
struct queue_t *queue;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*----------------------------------------------------------------
 * 	main -- execute terminal
 *
 *	Creates threads to dequeue packets (to be sent to the drone), 
 *	and receive packets from the drone. Reads joystick and keyboard 
 *	values
 *
 *	Mods : Shruthi Kashyap  
 *----------------------------------------------------------------
 */
int main(int argc, char **argv)
{
	char c, d;
	int ret_1=0;
	pthread_t pthread_dequeue;
	pthread_t pthread_receive;

	if ((fd = open(JS_DEV, O_RDONLY)) < 0) {
		perror("jstest");
		exit(1);
	}

	/* non-blocking mode
 	*/
	fcntl(fd, F_SETFL, O_NONBLOCK);
	
	term_puts("\nTerminal program - Embedded Real-Time Systems\n");

	queue = createQueue(100);
	
	pthread_create(&pthread_dequeue, NULL, process_dequeue, NULL);
 	if (ret_1)
 		printf("Dequeue thread error\n");
	
 	pthread_mutex_init(&mutex, NULL);
	
	pthread_create(&pthread_receive, NULL, process_receive_packets, NULL);
 	if (ret_1)
 		printf("Receive thread error\n");

	term_initio();
	rs232_open();

	term_puts("Type ^C to exit\n");

	// discard any incoming text
	while ((c = rs232_getchar_nb()) != -1)
		fputc(c,stderr);
	
	// send & receive
	for (;;) 
	{
		if ((c = term_getchar_nb()) != -1) 
		{
			// When you press an arrow key, it is equivalent to an input from three keys simultaneously
			// The first key is an escape command, so check for the second command.
			if ((int)c == KEY_ESCAPE)
			{
				if ((d = term_getchar_nb()) == -1)
				{
					sendKeyEscape();			
				}
			}
			sendKeyPacket(c);						// Send keyboard value
		}
		sendJsPacket();								// Send joystick values
		usleep(20000);

	}

	term_exitio();
	rs232_close();
	free(queue->elements);
	free(queue);
	pthread_join(pthread_dequeue, NULL);
	pthread_join(pthread_receive, NULL);
	term_puts("\n<exit>\n");
  	
	return 0;
}
