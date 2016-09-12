#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "joystick.h"
#include "joystick_read.h"

int fd;
struct js_event js;

void joystick_read (int axis[6])
{
	unsigned int	i;
	/* check up on JS
 	*/
	while (read(fd, &js, sizeof(struct js_event)) == 
	       			sizeof(struct js_event))  {

		/* register data
		 */
		// fprintf(stderr,".");
		switch(js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_AXIS:
				axis[js.number] = js.value;
				break;
		}
	}
	if (errno != EAGAIN) {
		perror("\njs: error reading (EAGAIN)");
		exit (1);
	}

	printf("\n");

	for (i = 0; i < 6; i++) {
		printf("%6d ",axis[i]);
	}
}
