/*------------------------------------------------------------------
 *  process_drone.h
 *
 *  Defines the max, min and hover RPMs for the drone motors, drone
 *	constants for lift, roll, pitch and yaw.
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#ifndef PROCESS_DRONE_H__
#define PROCESS_DRONE_H__

// XXX: Check this
#define MIN_RPM 200 // Minimal speed to let the rotor turn due to internal friction
#define MAX_RPM	700 

/* RPMs */
#define RPM_STEP	10
#define HOVER_RPM	400		// XXX: Check this
#define HOVER_LIFT_FORCE	107 // 4*400^2/6000

// Drone specific constants for lift roll yaw pitch
#define DRONE_LIFT_CONSTANT 6000 // 1 280 000
#define DRONE_ROLL_CONSTANT 1000
#define DRONE_PITCH_CONSTANT 1000
#define DRONE_YAW_CONSTANT 3000 // 10000*128 = 12800000 = 12 800 000

// Drone specific constants for manual mode
#define DRONE_MANUAL_LIFT_CONSTANT 6000 // 1 280 000
#define DRONE_MANUAL_ROLL_CONSTANT 3000
#define DRONE_MANUAL_PITCH_CONSTANT 3000
#define DRONE_MANUAL_YAW_CONSTANT 3000 // 10000*128 = 12800000 = 12 800 000

// Battery threshold level
#define BATT_THRESHOLD 400//1050 // XXX: Need to update this

void process_drone();
void reset_drone();
void safe_mode();
void panic_mode();
void manual_mode();
void yaw_control_mode();
void full_control_mode();
void calibration_mode();
void raw_mode();
void log_upload();
void read_sensor();
void read_battery_level();
void check_log_tele_flags();
void update_log();
void uart_put_16bit(int16_t);
void uart_put_32bit(int32_t);
void calculate_rotor_speeds(int lift, int pitch, int roll, int yaw);
//int scale_number();

uint32_t new_addr;
uint32_t new_addr_write;

#endif
