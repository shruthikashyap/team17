/*------------------------------------------------------------------
 *  joystick_read.c
 *
 *  Reads the joystick values. Creates a packet with the scaled
 *	values and adds it to the tranmission queue.
 *
 *  June 2016
 *------------------------------------------------------------------
 */

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
#include "packet.h"
#include "queue.h"
#include "../command_types.h"
#include "../crc.h"

int fd;
struct js_event js;
char scale_axis[6];
int axis[6];
int button[12];

void scale_joystick_values(int axis[6])
{
	//printf("In scale_joystick_values\n");
	int i;
	int OldRange = 65535;
	int NewRange = 255;
	int OldMax = 32767;
	int OldMin = -32768;
	//int NewMax = 127;
	int NewMin = -128;
	
	for(i = 0; i < 6; i++)
	{
		//scale_axis[i] = (axis[i] + 32768) * 255 / 65535;
		
		if(axis[i] >= OldMin && axis[i] <= OldMax)
		{
			if (axis[i] == 0) 
			{
				scale_axis[i] = 0;
			} 
			else 
			{
				scale_axis[i] = (((axis[i] - OldMin) * NewRange) / OldRange) + NewMin;
			}
			//printf("scale_axis[%d] is %d, axis[%d] = %d\n", i, scale_axis[i], i, axis[i]);
		}
	}
}

void sendJsPacket()
{
	unsigned int i;
	
	struct packet_t p;
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
			case JS_EVENT_BUTTON:
				button[js.number] = js.value;
				break;
		}
	}
	if (errno != EAGAIN) {
		perror("\njs: error reading (EAGAIN)");
		exit (1);
	}

	// Scale joystick axis values
	scale_joystick_values(&axis[0]);
	
	// button for PANIC_MODE
	#if 1
	if (button[0] == 1)
	{
		//printf("Panic button pressed!\n");
		p.command = MODE_TYPE;
		p.value = PANIC_MODE;
		compute_crc(&p);
		enqueue(p);
		return;
	}	
	#endif
		
	// Send joystick values
	for (i = 0; i < 4; i++ )
	{
		switch (i)
		{
			case 0:
				p.command = JOY_ROLL;
				p.value = scale_axis[i];
				compute_crc(&p);
				enqueue(p);
				break;

			case 1:
				p.command = JOY_PITCH;
				p.value = scale_axis[i];
				compute_crc(&p);
				enqueue(p);
				break;

			case 2:
				p.command = JOY_YAW;
				p.value = scale_axis[i];
				compute_crc(&p);
				enqueue(p);
				break;

			case 3:
				p.command = JOY_LIFT;
				p.value = scale_axis[i];
				compute_crc(&p);
				enqueue(p);
				break;

			default:
				break;
		}
		usleep(10000);
		//printf("Values from joystick: %d , %d\n", p.command, p.value);
	}
	//usleep(20000);
}
