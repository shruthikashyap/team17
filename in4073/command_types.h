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
#define START_BYTE	0xFA    //250
#define	STOP_BYTE	0xF0 	//240

/* ACK */
#define	ACK		0x50

/* Acknowledgement type */
#define	ACK_SUCCESS	0
#define	ACK_FAILURE	1
#define ACK_INVALID	2

/* RPMs */
#define RPM_STEP	10
#define HOVER_RPM	250 // XXX: Find out the right value

/* Increase or Decrease */
#define	INCREASE	1
#define DECREASE	0

/* Cyclic Redundancy Check */
#define MSB 		0x80
#define CRC_POLY	0xC8 
#define MASK		0xF0

struct packet_t
{
	// XXX: Add check bytes
	unsigned char start;
	char command;
	char value;
	char crc;
	unsigned char stop;
};
