// XXX: Need to change the values
#define MIN_RPM 200 // Minimal speed to let the rotor turn due to internal friction
#define MAX_RPM	600 

/* RPMs */
#define RPM_STEP	10
#define HOVER_RPM	400
#define HOVER_LIFT_FORCE	107 // 4*400^2/6000

#define MIN_LIFT 0 // negative lift does not occur
#define MIN_ROLL  -25000000
#define MIN_PITCH -25000000
#define MIN_YAW   -25000000 // 2 500 000

#define MAX_LIFT  25000000
#define MAX_ROLL  25000000
#define MAX_PITCH 25000000
#define MAX_YAW   25000000 // 2 500 000

// Drone specific constants for lift roll yaw pitch
#define DRONE_LIFT_CONSTANT 6000 // 1 280 000
#define DRONE_ROLL_CONSTANT 1000
#define DRONE_PITCH_CONSTANT 1000
#define DRONE_YAW_CONSTANT 3000 // 10000*128 = 12800000 = 12 800 000

// Battery threshold level
#define BATT_THRESHOLD 400	//1050 // XXX: Need to update this

void process_drone();
void reset_drone();
void safe_mode();
void panic_mode();
void manual_mode();
void yaw_control_mode();
void full_control_mode();
void calibration_mode();
void raw_mode();
void height_control_mode();
void wireless_mode();
void log_upload();
void uart_put_16bit(int16_t);					
void uart_put_32bit(int32_t);					
//int scale_number();