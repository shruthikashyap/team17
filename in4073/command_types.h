/*------------------------------------------------------------------
 *  command_types.h
 *
 *  Defines the command types, mode types, log and telemetry packet
 *	structures.
 *
 *------------------------------------------------------------------
 */

#ifndef COMMAND_TYPES_H__
#define COMMAND_TYPES_H__

#include <inttypes.h>

/* Command types */
#define MODE_TYPE	0x01
#define KEY_LIFT	0x02
#define KEY_ROLL	0x03
#define KEY_PITCH	0x04
#define KEY_YAW		0x05
#define JOY_ROLL	0x06
#define JOY_PITCH	0x07
#define JOY_YAW		0x08
#define JOY_LIFT	0x09
#define ABORT		0x0A
#define LOG 		0x30

#define KEY_CONTROL_YAW	0x10
#define KEY_CONTROL_P1	0x11
#define KEY_CONTROL_P2	0x12

#define TELEMETRY_DATA	0x13

/* Mode Types */
#define SAFE_MODE			0x20
#define PANIC_MODE			0x29
#define MANUAL_MODE			0x22
#define YAW_CONTROL_MODE	0x23
#define FULL_CONTROL_MODE	0x24
#define CALIBRATION_MODE	0x25
#define HEIGHT_CONTROL_MODE	0x26
#define WIRELESS_MODE		0x27
#define RAW_MODE			0x28

/* Log */
#define LOG_UPLOAD		0x31
#define LOG_START		0x32
#define LOG_STOP 		0x33
#define LOG_LINE_START	0x90
#define LOG_LINE_END	0x95
#define LOG_END    		0x99

/* Start and Stop Bytes */
#define START_BYTE	0xFA    //250
#define	STOP_BYTE	0xF5	//245

/* ACK */
#define	ACK		0x50

/* Acknowledgement type */
#define	ACK_SUCCESS	0
#define	ACK_FAILURE	1
#define ACK_INVALID	2

/* Increase or Decrease */
#define	INCREASE	1
#define DECREASE	0

/* Cyclic Redundancy Check */
#define MSB 		0x80
#define CRC_POLY	0xC8 
#define MASK		0xF0

/* Telemetry start stop bytes */
#define TELE_START	0x40
#define TELE_STOP	0x45

struct packet_t
{
	unsigned char start;
	char command;
	signed char value;
	char crc;
	unsigned char stop;
};

struct log
{
	uint32_t current_time;
	char current_mode;
	int16_t sp;
	int16_t sq;
	int16_t sr;
	int16_t sax;
	int16_t say;
	int16_t saz;
	int16_t phi;
	int16_t theta;
	int16_t psi;
};

struct telemetry
{
	uint32_t current_time;
	char current_mode;
	uint16_t bat_volt;
	int16_t ae[4];
	int16_t controlgain_yaw;
	int16_t controlgain_p1;
	int16_t controlgain_p2;
	uint8_t dmp_raw_mode;
	int16_t key_lift;
	int16_t key_roll;
	int16_t key_pitch;
	int16_t key_yaw;
};

#endif
