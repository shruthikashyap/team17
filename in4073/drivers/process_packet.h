/*------------------------------------------------------------------
 *  process_packet.h
 *
 *  Defines the max and min trimming values of drone attributes like lift,
 *	roll, pitch, yaw, control gains and keyboard step values, which
 *	are set by the user.
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#ifndef PROCESS_PACKET_H__
#define PROCESS_PACKET_H__

#include "../command_types.h"

#define MAX_TRIM_LIFT	128
#define MAX_TRIM_PITCH	128
#define MAX_TRIM_ROLL	128
#define MAX_TRIM_YAW	128
#define MIN_TRIM_LIFT	-127
#define MIN_TRIM_PITCH	-127
#define MIN_TRIM_ROLL	-127
#define MIN_TRIM_YAW	-127

// control gains
#define MAX_CONTROLGAIN_YAW 50
#define MAX_CONTROLGAIN_P1 50
#define MAX_CONTROLGAIN_P2 50
#define MIN_CONTROLGAIN_YAW 0
#define MIN_CONTROLGAIN_P1 0
#define MIN_CONTROLGAIN_P2 0

#define KEYBOARD_STEP 1
#define KEYBOARD_CONTROL_STEP 1

void process_packet(struct packet_t packet);
void process_key(struct packet_t packet);
void set_mode_type(char value);
void set_key_lift(char value);
void set_key_roll(char value);
void set_key_pitch(char value);
void set_key_yaw(char value);
void set_key_control_yaw(char value);
void set_key_control_p1(char value);
void set_key_control_p2(char value);
void set_joy_lift(signed char value);
void set_joy_roll(char value);
void set_joy_pitch(char value);
void set_joy_yaw(char value);

/* Send acknowledgement */
void send_packet_ack(int);
//void send_packet_to_pc(struct packet_t p);

#endif
