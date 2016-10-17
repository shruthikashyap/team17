#include <stdio.h>
#include <math.h>
#include "command_types.h"
#include "in4073.h"
#include "process_drone.h"
#include <string.h>
#include <stdlib.h>
#include "crc.h"
//#include "log.h"

void send_packet_to_pc(struct packet_t p);

bool log_flag = false;
bool control_loop_flag = false;
bool telemetry_flag = false;
bool batt_low_flag = false;
uint32_t counter = 0;
uint32_t ae_[4];

void read_sensor()
{
	drone.sp = 0;
	drone.sq = 0;
	drone.sr = 0;
	drone.sax = 0;
	drone.say = 0;
	drone.saz = 0;
	//drone.pressure = 0;
	
	// Read sensor data
	if(check_timer_flag()) 
	{
		if (counter++%20 == 0) nrf_gpio_pin_toggle(BLUE);

		adc_request_sample();
		//read_baro();
		nrf_delay_ms(1);

		clear_timer_flag();
	}

	if(check_sensor_int_flag()) 
	{
		get_dmp_data();
		//get_raw_sensor_data();
		clear_sensor_int_flag();
	}
	
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
	//drone.pressure = pressure - drone.offset_pressure ;
	//drone.pressure = pressure;
	__enable_irq();
	
	// Battery voltage check
	if(bat_volt <= BATT_THRESHOLD && drone.current_mode != SAFE_MODE)
	{
		//printf("Battery low!\n");
		drone.current_mode = PANIC_MODE;
		drone.change_mode = 1;
		batt_low_flag = true;
		return;
	}
	
	//printf("%6d %6d %6d | ", drone.sp, drone.sq, drone.sr);
	//printf("%6d %6d %6d | ", drone.sax, drone.say, drone.saz);
	//printf("%6d %6d %6d\n", drone.phi, drone.theta, drone.psi);
	//printf("%ld\n", drone.pressure);
}

void read_battery_level()
{
	if(check_timer_flag()) 
	{
		if (counter++%20 == 0) nrf_gpio_pin_toggle(BLUE);

		adc_request_sample();
		nrf_delay_ms(1);

		clear_timer_flag();
	}
	
	//printf("%d\n", bat_volt);
	
	// Battery voltage check
	if(bat_volt <= BATT_THRESHOLD && drone.current_mode != SAFE_MODE)
	{
		//printf("Battery low!\n");
		drone.current_mode = PANIC_MODE;
		drone.change_mode = 1;
		batt_low_flag = true;
		return;
	}
}

void send_telemetry_data()
{
	// Send telemetry command to PC
	struct packet_t p;
	p.start = START_BYTE;
	p.command = TELEMETRY_DATA;
	compute_crc(&p);
	p.stop = STOP_BYTE;
	
	send_packet_to_pc(p);
	
	uart_put(TELE_START);
	//uart_put_32bit(4057748);
	uart_put_32bit(get_time_us());
	//uart_put(0x20);
	uart_put(drone.current_mode);
	//uart_put_16bit(574);
	uart_put_16bit(bat_volt);
	//uart_put_16bit(710);
	uart_put_16bit(drone.ae[0]);
	//uart_put_16bit(510);
	uart_put_16bit(drone.ae[1]);
	//uart_put_16bit(1000);
	uart_put_16bit(drone.ae[2]);
	//uart_put_16bit(555);
	uart_put_16bit(drone.ae[3]);
	//uart_put_16bit(2);
	uart_put_16bit(drone.controlgain_yaw);
	//uart_put_16bit(15);
	uart_put_16bit(drone.controlgain_p1);
	//uart_put_16bit(25);
	uart_put_16bit(drone.controlgain_p2);
	uart_put(TELE_STOP);
}

void check_log_tele_flags()
{
	// Update log
	if(log_flag == true && batt_low_flag == false && log_active_flag == true)
	{
		//printf("Start log %10ld\n", get_time_us());
		// Clear log flag
		log_flag = false;

		// Log sensor data
		update_log();
		//printf("Stop log %10ld\n", get_time_us());
	}

	// Send telemetry
	if(telemetry_flag == true && batt_low_flag == false && log_upload_flag == false)
	{
		//printf("Start tele %10ld\n", get_time_us());
		// Clear telemetry flag
		telemetry_flag = false;

		// Send telemetry data
		send_telemetry_data();
		//printf("Stop tele %10ld\n", get_time_us());
	}
}

void reset_drone()
{
	// XXX: Set mode as SAFE_MODE and clear all flags
	drone.current_mode = SAFE_MODE; // XXX: Test
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
	drone.offset_pressure = 0;
	
	// Other parameters
	new_addr = 0x000000;
	new_addr_write = 0x000000;
	batt_low_flag = false;
	
	if(flash_chip_erase() == true);
		//printf("Memory erase successful\n");
}


void calculate_rotor_speeds(int lift, int pitch, int roll, int yaw)
{
	// Solving drone rotor dynamics equations
	ae_[0] = 0.25*(lift + 2*pitch - yaw);
	ae_[1] = 0.25*(lift - 2*roll  + yaw);
	ae_[2] = 0.25*(lift - 2*pitch - yaw);
	ae_[3] = 0.25*(lift + 2*roll  + yaw);

	// no negative number for sqrt
	ae_[0] = ae_[0] < 0 ? 1 : (int)sqrt(ae_[0]);
	ae_[1] = ae_[1] < 0 ? 1 : (int)sqrt(ae_[1]);
	ae_[2] = ae_[2] < 0 ? 1 : (int)sqrt(ae_[2]);
	ae_[3] = ae_[3] < 0 ? 1 : (int)sqrt(ae_[3]);

	// adjusting trimming, do we need disable irq?
	__disable_irq();
	ae_[0] += (drone.key_lift + drone.key_pitch - drone.key_yaw);
	ae_[1] += (drone.key_lift - drone.key_roll  + drone.key_yaw);
	ae_[2] += (drone.key_lift - drone.key_pitch - drone.key_yaw);
	ae_[3] += (drone.key_lift + drone.key_roll  + drone.key_yaw);
	__enable_irq();

	if (drone.joy_lift > 5)
	{
		ae_[0] = ae_[0] < MIN_RPM ? MIN_RPM : ae_[0];
		ae_[1] = ae_[1] < MIN_RPM ? MIN_RPM : ae_[1];
		ae_[2] = ae_[2] < MIN_RPM ? MIN_RPM : ae_[2];
		ae_[3] = ae_[3] < MIN_RPM ? MIN_RPM : ae_[3];
	}

	ae_[0] = ae_[0] > MAX_RPM ? MAX_RPM : ae_[0];
	ae_[1] = ae_[1] > MAX_RPM ? MAX_RPM : ae_[1];
	ae_[2] = ae_[2] > MAX_RPM ? MAX_RPM : ae_[2];
	ae_[3] = ae_[3] > MAX_RPM ? MAX_RPM : ae_[3];

	// Setting drone rotor speeds
	drone.ae[0] = ae_[0];
	drone.ae[1] = ae_[1];
	drone.ae[2] = ae_[2];
	drone.ae[3] = ae_[3];	
}

void raw_mode()
{
	//printf("In RAW_MODE\n");
}

void wireless_mode()
{
	//printf("In WIRELESS_MODE\n");
}

void process_drone()
{
	// Set defaults for the drone
	reset_drone();
	
	while (drone.stop == 0)
	{
		drone.change_mode = 0;

		if((drone.joy_lift > 10) && !(drone.current_mode == SAFE_MODE || drone.current_mode == PANIC_MODE))
		{
			drone.current_mode = SAFE_MODE;
			//printf("Make lift 0 before changing mode %d :)\n", drone.joy_lift);
			continue;
		}

		switch (drone.current_mode)
		{
			case SAFE_MODE:
					// XXX: Test
					//ae[0] = ae[1] = ae[2] = ae[3] = 100;
					safe_mode();
					break;
			case PANIC_MODE:
					// XXX: Test
					//ae[0] = ae[1] = ae[2] = ae[3] = 500;
					//printf("\nDrone motor values before: %3d %3d %3d %3d\n", ae[0], ae[1], ae[2], ae[3]);
					panic_mode();
					// XXX: Test
					//printf("\nDrone motor values after: %3d %3d %3d %3d\n", ae[0], ae[1], ae[2], ae[3]);
					break;
			case MANUAL_MODE:
					manual_mode();
					break;
			case YAW_CONTROL_MODE:
					yaw_control_mode();
					break;
			case FULL_CONTROL_MODE:
					full_control_mode();
					break;
			case CALIBRATION_MODE:
					calibration_mode();
					break;
			case RAW_MODE:
					raw_mode();
					break;
			case WIRELESS_MODE:
					wireless_mode();
					break;
			default :
					// XXX: Needs to be handled
					break;
		}
	}
}
