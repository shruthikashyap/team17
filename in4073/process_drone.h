// XXX: Need to change the values
#define MIN_RPM 120 // Minimal speed to let the rotor turn due to internal friction
#define MAX_RPM	1000 

// Drone specific constants for lift roll yaw pitch
#define DRONE_LIFT_CONSTANT 50
#define DRONE_ROLL_CONSTANT 50
#define DRONE_PITCH_CONSTANT 50
#define DRONE_YAW_CONSTANT 50

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
int scale_number();
