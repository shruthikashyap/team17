/*------------------------------------------------------------------
 *  process_drone.c
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <math.h>
#include "command_types.h"
#include "in4073.h"
#include "process_drone.h"
#include <string.h>
#include <stdlib.h>
#include "crc.h"
#include "filters.h"

void send_packet_to_pc(struct packet_t p);

bool log_flag = false;
bool control_loop_flag = false;
bool control_loop_flag_raw = false;
bool telemetry_flag = false;
bool batt_low_flag = false;
uint32_t counter = 0;
int ae_[4];

extern bool raw_mode_flag;
extern bool imu_init_flag;

 /*------------------------------------------------------------------
 *  void read_sensor
 *
 *  This function reads the sensors we need for controlling the ES.
 *  That is, the MPU (gyro+accelero), ADC (to check battery voltage) 
 *  and the barometer (pressure/height control)
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void read_sensor()
{
	drone.sp = 0;
	drone.sq = 0;
	drone.sr = 0;
	drone.sax = 0;
	drone.say = 0;
	drone.saz = 0;
	drone.pressure = 0;
	
	// Read sensor data
	if(check_timer_flag()) 
	{
		adc_request_sample();
		read_baro();
		moving_average();
		nrf_delay_ms(1);

		clear_timer_flag();
	}

	if(check_sensor_int_flag()) 
	{
		if (raw_mode_flag == true)
		{
			get_raw_sensor_data();
			butterworth();
			kalman();
		}
		else
		{
			get_dmp_data();
		}
		clear_sensor_int_flag();
	}
	
	// substract the offset for the sensor values
	__disable_irq();
	drone.sp = sp - drone.offset_sp;
	drone.sq = sq - drone.offset_sq;
	drone.sr = sr - drone.offset_sr;
	drone.sax = sax - drone.offset_sax;
	drone.say = say - drone.offset_say;
	drone.saz = saz - drone.offset_saz;
	drone.phi = phi - drone.offset_phi;
	drone.theta = theta - drone.offset_theta;
	drone.psi   = psi   - drone.offset_psi;
	drone.pressure = pressure - drone.offset_pressure;
	__enable_irq();
	
	// If battery voltage is too low, go to panic mode
	if(bat_volt <= BATT_THRESHOLD && drone.current_mode != SAFE_MODE)
	{
		drone.current_mode = PANIC_MODE;
		drone.change_mode = 1;
		batt_low_flag = true;
		return;
	}
}

 /*------------------------------------------------------------------
 *  void read_battery_level
 *
 *  This function reads the battery voltage. Used in functions where we 
 *  don't want to call read_sensor.
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void read_battery_level()
{
	if(check_timer_flag()) 
	{
		adc_request_sample();
		nrf_delay_ms(1);

		clear_timer_flag();
	}
		
	// If battery voltage is too low, go to panic mode
	if(bat_volt <= BATT_THRESHOLD && drone.current_mode != SAFE_MODE)
	{
		drone.current_mode = PANIC_MODE;
		drone.change_mode = 1;
		batt_low_flag = true;
		return;
	}
}

 /*------------------------------------------------------------------
 *  void send_telemetry_data
 *
 *  Sends the telemetry data to the PC. Send it byte by byte.
 *
 *  Author : Evelyn 
 *------------------------------------------------------------------
 */
void send_telemetry_data()
{
	// Send telemetry command to PC
	struct packet_t p;
	p.start = START_BYTE;
	p.command = TELEMETRY_DATA;
	compute_crc(&p);
	p.stop = STOP_BYTE;
	
	send_packet_to_pc(p);
	
	// send byte by byte
	uart_put(TELE_START);
	uart_put_32bit(get_time_us());
	uart_put(drone.current_mode);
	uart_put_16bit(bat_volt);
	uart_put_16bit(drone.ae[0]);
	uart_put_16bit(drone.ae[1]);
	uart_put_16bit(drone.ae[2]);
	uart_put_16bit(drone.ae[3]);
	uart_put_16bit(drone.controlgain_yaw);
	uart_put_16bit(drone.controlgain_p1);
	uart_put_16bit(drone.controlgain_p2);
	uart_put(raw_mode_flag);
	uart_put_16bit(drone.key_lift);
	uart_put_16bit(drone.key_roll);
	uart_put_16bit(drone.key_pitch);
	uart_put_16bit(drone.key_yaw);
	uart_put(TELE_STOP);
}


 /*------------------------------------------------------------------
 *  void check_log_tele_flags
 *
 *  Function checks if we need to update the log or send telemetry.
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void check_log_tele_flags()
{
	// Update log
	if(log_flag == true && batt_low_flag == false && log_active_flag == true)
	{
		// Clear log flag
		log_flag = false;

		// Log sensor data
		update_log();
	}

	// Send telemetry
	if(telemetry_flag == true && batt_low_flag == false && log_upload_flag == false)
	{
		// Clear telemetry flag
		telemetry_flag = false;

		// Send telemetry data
		send_telemetry_data();
	}
}


 /*------------------------------------------------------------------
 *  void reset_drone
 *
 *  Function resets all the drone variables.
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void reset_drone()
{
	// Set mode as SAFE_MODE and clear all flags
	drone.current_mode = SAFE_MODE;
	drone.stop = 0;
	drone.change_mode = 0;
	
	// Reset drone control variables
	drone.key_lift = 0;
	drone.key_roll = 0;
	drone.key_pitch = 0;
	drone.key_yaw = 0;
	drone.joy_lift = 0;
	drone.joy_roll = 0;
	drone.joy_pitch = 0;
	drone.joy_yaw = 0;
	drone.offset_sp = 0;
    drone.offset_sq = 0;
    drone.offset_sr = 0;
	drone.offset_sax = 0;
    drone.offset_say = 0;
    drone.offset_saz = 0;
	drone.offset_phi = 0;
    drone.offset_theta = 0;
    drone.offset_psi = 0;
	drone.offset_pressure = 0;
	
	// Other parameters
	new_addr = 0x000000;
	new_addr_write = 0x000000;
	batt_low_flag = false;
	
	if(flash_chip_erase() == true);
}


 /*------------------------------------------------------------------
 *  void calculate_rotor_speeds
 *
 *  Function calculates the corresponding rotor speeds for x amount
 *  of lift, pitch, roll and yaw. 
 *
 *  Author : Kars Heinen 
 *------------------------------------------------------------------
 */
void calculate_rotor_speeds(int lift, int pitch, int roll, int yaw)
{
	// calculate rotor speeds by the dynamics
	ae_[0] = 0.25*(lift + 2*pitch - yaw);
	ae_[1] = 0.25*(lift - 2*roll  + yaw);
	ae_[2] = 0.25*(lift - 2*pitch - yaw);
	ae_[3] = 0.25*(lift + 2*roll  + yaw);

	// dont take square root of negative values
	ae_[0] = ae_[0] < 0 ? 1 : (int)sqrt(ae_[0]);
	ae_[1] = ae_[1] < 0 ? 1 : (int)sqrt(ae_[1]);
	ae_[2] = ae_[2] < 0 ? 1 : (int)sqrt(ae_[2]);
	ae_[3] = ae_[3] < 0 ? 1 : (int)sqrt(ae_[3]);
	
	// adjust for trimming
	ae_[0] += ((signed char)drone.key_lift + (signed char)drone.key_pitch - (signed char)drone.key_yaw);
	ae_[1] += ((signed char)drone.key_lift - (signed char)drone.key_roll  + (signed char)drone.key_yaw);
	ae_[2] += ((signed char)drone.key_lift - (signed char)drone.key_pitch - (signed char)drone.key_yaw);
	ae_[3] += ((signed char)drone.key_lift + (signed char)drone.key_roll  + (signed char)drone.key_yaw);
	
	// if the joystick throttle is up, the minimum rotor speeds should be MIN_RPM to keep the rotors spinning
	if (drone.joy_lift > 5)
	{
		ae_[0] = ae_[0] < MIN_RPM ? MIN_RPM : ae_[0];
		ae_[1] = ae_[1] < MIN_RPM ? MIN_RPM : ae_[1];
		ae_[2] = ae_[2] < MIN_RPM ? MIN_RPM : ae_[2];
		ae_[3] = ae_[3] < MIN_RPM ? MIN_RPM : ae_[3];
	}
	else
	{
		ae_[0] = 0;
		ae_[1] = 0;
		ae_[2] = 0;
		ae_[3] = 0;
	}

	// don't set the rotor speeds to high.
	ae_[0] = ae_[0] > MAX_RPM ? MAX_RPM : ae_[0];
	ae_[1] = ae_[1] > MAX_RPM ? MAX_RPM : ae_[1];
	ae_[2] = ae_[2] > MAX_RPM ? MAX_RPM : ae_[2];
	ae_[3] = ae_[3] > MAX_RPM ? MAX_RPM : ae_[3];	

	// setting drone rotor speeds
	drone.ae[0] = ae_[0];
	drone.ae[1] = ae_[1];
	drone.ae[2] = ae_[2];
	drone.ae[3] = ae_[3];
}


 /*------------------------------------------------------------------
 *  void process_drone
 *
 *  Main function for the drone. Kind of the bone structure.
 *  Checks which state are we in and sets the corresponding mode.
 *  This function is called from int main();
 *
 *  Author : Shruthi Kasyap
 *------------------------------------------------------------------
 */
void process_drone()
{
	// Set defaults for the drone
	reset_drone();
	
	while (drone.stop == 0)
	{
		drone.change_mode = 0;

		// only allow mode change if we are in safe mode or panid mode and the throttle is down.
		if((drone.joy_lift > 10) && !(drone.current_mode == SAFE_MODE || drone.current_mode == PANIC_MODE))
		{
			drone.current_mode = SAFE_MODE;
			continue;
		}

		switch (drone.current_mode)
		{
			case SAFE_MODE:
					safe_mode();
					break;
			case PANIC_MODE:
					panic_mode();
					break;
			case MANUAL_MODE:
					manual_mode();
					break;
			case YAW_CONTROL_MODE:
					yaw_control_mode();
					break;
			case FULL_CONTROL_MODE:
					if(raw_mode_flag == true)
						full_control_mode_raw();
					else
						full_control_mode();
					break;
			case CALIBRATION_MODE:
					calibration_mode();
					break;
			default :
					safe_mode();
					break;
		}
	}
}
