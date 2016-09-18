/* Command types */
#define MODE_TYPE	0x01
#define KEY_LIFT	0x02
#define KEY_ROLL	0x03
#define KEY_PITCH	0x04
#define KEY_YAW		0x05
#define JOY_LIFT	0x06
#define JOY_ROLL	0x07
#define JOY_PITCH	0x08
#define JOY_YAW		0x09

/* Mode Types */
#define SAFE_MODE		0x20
#define PANIC_MODE		0x29
#define MANUAL_MODE		0x22
#define YAW_CONTROL_MODE	0x23
#define FULL_CONTROL_MODE	0x24
#define CALIBRATION_MODE	0x25
#define HEIGHT_CONTROL_MODE	0x26
#define WIRELESS_MODE		0x27
#define RAW_MODE		0x28

/* Start and Stop Bytes */
#define START_BYTE	250
#define	STOP_BYTE	251

/* ACK */
#define	ACK	0x50

/* RPMs */
#define RPM_STEP	10
#define HOVER_RPM	250 // XXX: Find out the right value

/* Increase or Decrease */
#define	INCREASE	1
#define DECREASE	0

struct packet_t
{
	// XXX: Add check bytes
	char start;
	char command;
	char value;
	char stop;
};
