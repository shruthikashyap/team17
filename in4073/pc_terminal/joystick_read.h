/*------------------------------------------------------------------
 *  joystick_read.h
 *
 *  Declarations for joystick read functions
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#ifndef JOYSTICK_READ_H__
#define JOYSTICK_READ_H__

#define JS_DEV	"/dev/input/js0"

extern int fd;
extern struct js_event js;

void sendJsPacket();
void scale_joystick_values(int axis[6]);

#endif
