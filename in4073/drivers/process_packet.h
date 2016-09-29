#include "../command_types.h"

// XXX: Need to change the values
#define MAX_LIFT	128
#define MAX_PITCH	128
#define MAX_ROLL	128
#define MAX_YAW		128
#define MIN_LIFT	-127
#define MIN_PITCH	-127
#define MIN_ROLL	-127
#define MIN_YAW		-127

// control gains
#define MAX_CONTROLGAIN_YAW 50
#define MAX_CONTROLGAIN_P1 50
#define MAX_CONTROLGAIN_P2 50
#define MIN_CONTROLGAIN_YAW 0
#define MIN_CONTROLGAIN_P1 0
#define MIN_CONTROLGAIN_P2 0

#define KEYBOARD_STEP 2

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
void set_joy_lift(char value);
void set_joy_roll(char value);
void set_joy_pitch(char value);
void set_joy_yaw(char value);

/* Send acknowledgement */
void send_packet_ack(int);
//void send_packet_to_pc(struct packet_t p);
