#include "command_types.h"

// XXX: Need to change the values
#define MAX_LIFT	1000
#define MAX_PITCH	1000
#define MAX_ROLL	1000
#define MAX_YAW		1000
#define MIN_LIFT	120
#define MIN_PITCH	120
#define MIN_ROLL	120
#define MIN_YAW		120

struct store_packet_t
{
	char start;
	char command;
	char value;
	char stop;
};

extern struct store_packet_t st_p;

void process_packet(struct store_packet_t packet);
void process_key(struct packet_t packet);
void set_mode_type(char value);
void set_key_lift(char value);
void set_key_roll(char value);
void set_key_pitch(char value);
void set_key_yaw(char value);
void set_joy_lift(char value);
void set_joy_roll(char value);
void set_joy_pitch(char value);
void set_joy_yaw(char value);
