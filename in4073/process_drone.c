#include <stdio.h>
#include <math.h>
#include "command_types.h"
#include "in4073.h"
#include "process_drone.h"
#include <string.h>
#include <stdlib.h>
#include "crc.h"
#include "filters.h"
//#include "log.h"

void send_packet_to_pc(struct packet_t p);

bool log_flag = false;
bool control_loop_flag = false;
bool telemetry_flag = false;
bool batt_low_flag = false;
uint32_t counter = 0;
//uint32_t ae_[4];

extern bool raw_mode_flag;
extern bool imu_init_flag;

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
		adc_request_sample();
		//read_baro();
		nrf_delay_ms(1);

		clear_timer_flag();
	}

	if(check_sensor_int_flag()) 
	{
		if (raw_mode_flag == true)
		{
			get_raw_sensor_data();
			butterworth();
		}
		else
		{
			get_dmp_data();
		}
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
	uart_put(raw_mode_flag);
	uart_put_16bit(drone.key_lift);
	uart_put_16bit(drone.key_roll);
	uart_put_16bit(drone.key_pitch);
	uart_put_16bit(drone.key_yaw);
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

#if 0
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
	ae_[0] += (drone.key_lift + (signed char)drone.key_pitch - (signed char)drone.key_yaw);
	ae_[1] += (drone.key_lift - (signed char)drone.key_roll  + (signed char)drone.key_yaw);
	ae_[2] += (drone.key_lift - (signed char)drone.key_pitch - (signed char)drone.key_yaw);
	ae_[3] += (drone.key_lift + (signed char)drone.key_roll  + (signed char)drone.key_yaw);
	__enable_irq();

	//printf("key %d, joy %d\n", drone.key_lift, drone.joy_lift);
	
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

void manual_mode()
{
	//printf("In MANUAL_MODE\n");
	
	int ae_[4]; // Motor rpm values
	int lift_force;
	int roll_moment;
	int pitch_moment;
	int yaw_moment;
	int lift;
	int pitch;
	int roll;
	int yaw;	

	while(drone.change_mode == 0 && drone.stop == 0)
	{
		if (cable_disconnect_flag == 2)
		{	
			cable_disconnect_flag = 1;
			drone.change_mode = 1;
			drone.current_mode = PANIC_MODE;
			return;
		}

		// Disable UART interrupts
		__disable_irq();

		// joystick goes from -127 to 128. key_lift goes from -127 to 128 so they weigh the same
		lift_force   = drone.joy_lift  + drone.key_lift;
		roll_moment  = (signed char)drone.joy_roll  + (signed char)drone.key_roll;
		pitch_moment = (signed char)drone.joy_pitch + (signed char)drone.key_pitch;
		yaw_moment   = (signed char)drone.joy_yaw   + (signed char)drone.key_yaw;

		// Enable UART interrupts
		__enable_irq();

		lift  = DRONE_LIFT_CONSTANT * lift_force;
		pitch = DRONE_PITCH_CONSTANT * pitch_moment;
		roll  = DRONE_ROLL_CONSTANT * roll_moment;
		yaw   = DRONE_YAW_CONSTANT * yaw_moment;

		if ((signed char)drone.joy_lift < 5)
		{
			pitch = 0;
			roll  = 0;
			yaw   = 0;
		}
		
		ae_[0] = 0.25*(lift + 2*pitch - yaw);
		ae_[1] = 0.25*(lift - 2*roll  + yaw);
		ae_[2] = 0.25*(lift - 2*pitch - yaw);
		ae_[3] = 0.25*(lift + 2*roll  + yaw);
		
		ae_[0] = ae_[0] < 0 ? 1 : (int)sqrt(ae_[0]);
		ae_[1] = ae_[1] < 0 ? 1 : (int)sqrt(ae_[1]);
		ae_[2] = ae_[2] < 0 ? 1 : (int)sqrt(ae_[2]);
		ae_[3] = ae_[3] < 0 ? 1 : (int)sqrt(ae_[3]);
		
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

		// setting drone rotor speeds
		drone.ae[0] = ae_[0];
		drone.ae[1] = ae_[1];
		drone.ae[2] = ae_[2];
		drone.ae[3] = ae_[3];

		//printf("%3d %3d %3d %3d | %d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3], bat_volt);
		
		// Update log and telemetry if corresponding flags are set
		check_log_tele_flags();

		run_filters_and_control();

		nrf_delay_ms(1);
	}
	
	//printf("Exit MANUAL_MODE\n");
}

void yaw_control_mode()
{
	//printf("In YAW_CONTROL_MODE\n");

	int yawrate_setpoint;
	int yaw_error;

	int yaw_moment;
	int lift_force;

	int lift, roll, pitch, yaw;
	
	//int pressure_error;

	drone.controlgain_yaw = 2;
	drone.controlgain_height = 1;
	//drone.key_lift = 20; // XXX: For testing
	//drone.joy_lift = 100; // XXX: For testing
	
	while(drone.change_mode == 0 && drone.stop == 0)
	{
		if (cable_disconnect_flag == 2)
		{	
			cable_disconnect_flag = 1;
			drone.change_mode = 1;
			drone.current_mode = PANIC_MODE;
			return;
		}

		if(control_loop_flag == true && batt_low_flag == false)
		{
			// Clear sensor flag
			control_loop_flag = false;

			// Read sensor data
			read_sensor();
			
			__disable_irq();
			lift_force = drone.joy_lift;
			yawrate_setpoint = (signed char)drone.joy_yaw;
			__enable_irq();

			#if 0
			if(height_control_flag == true)
			{
				if(drone.height_control_lift != drone.joy_lift)
					height_control_flag = false;
				else
				{
					pressure_error = pressure - drone.height_control_pressure;
					lift_force = HOVER_LIFT_FORCE + ((int)drone.controlgain_height * pressure_error);
					//printf("Lift force = %d, pressure_error = %d\n", lift_force, pressure_error);
				}
			}
			#endif

			// setpoint is keyboard + joystick, so in range of -127 to 128
			// gyro is in 16 bit range with a max of 250deg/s 
			// yaw rate of max 3000 seems decent (trial and error) so max of setpoint should be 3000
			// 3000/128 = 23.43, so setpoints times 24 should be in decent range
			// dividing sr by 24 to keep yaw_error a low number, so the controlgainnumber doesnt have to be so small

			if (lift_force > 10) 
			{
				yaw_error = -(int)(yawrate_setpoint - (drone.sr/24));

				// yaw error in range of -255 to 256 (although the extremes probably wont happen)
				yaw_moment = (int)drone.controlgain_yaw*yaw_error;
				//printf("yaw moment:  %d \n", yaw_moment);
			}
			else
			{
				yaw_moment = 0;
			}

			lift  = DRONE_LIFT_CONSTANT * lift_force;
			roll  = 0; // no roll or pitch in yaw mode
			pitch = 0;
			yaw   = (int)(DRONE_YAW_CONSTANT/4 * yaw_moment); // misschien deze drone constant aanpassen voor yaw mode

			//calculate_rotor_speeds(lift, pitch, roll, yaw);
			
			if ((signed char)drone.joy_lift < 5)
			{
				pitch = 0;
				roll  = 0;
				yaw   = 0;
			}

			ae_[0] = 0.25*(lift + 2*pitch - yaw);
			ae_[1] = 0.25*(lift - 2*roll  + yaw);
			ae_[2] = 0.25*(lift - 2*pitch - yaw);
			ae_[3] = 0.25*(lift + 2*roll  + yaw);

			ae_[0] = ae_[0] < 0 ? 1 : (int)sqrt(ae_[0]);
			ae_[1] = ae_[1] < 0 ? 1 : (int)sqrt(ae_[1]);
			ae_[2] = ae_[2] < 0 ? 1 : (int)sqrt(ae_[2]);
			ae_[3] = ae_[3] < 0 ? 1 : (int)sqrt(ae_[3]);

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

			// setting drone rotor speeds
			drone.ae[0] = ae_[0];
			drone.ae[1] = ae_[1];
			drone.ae[2] = ae_[2];
			drone.ae[3] = ae_[3];

			//printf("%3d %3d %3d %3d | %d | %d %d | %d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3], drone.controlgain_yaw, drone.phi, drone.theta, bat_volt);

			run_filters_and_control();
			
			// Update log and telemetry if corresponding flags are set
			check_log_tele_flags();
		}
		else
			nrf_delay_us(10);
	}
	
	//printf("Exit YAW_CONTROL_MODE\n");
}

void full_control_mode()
{
	//printf("In FULL_CONTROL_MODE\n");

	int rollrate_setpoint;
	int rollrate_setpoint2;
	int roll_s;
	int pitchrate_setpoint;
	int pitchrate_setpoint2;
	int pitch_s;
	int roll_moment;
	int pitch_moment;
	int yawrate_setpoint;
	int yaw_error;
	int yaw_moment;
	int lift_force;
	int lift, roll, pitch, yaw;
	
	//drone.joy_lift = 100; // XXX: For testing
	//int count = 0;
	
	while(drone.change_mode == 0 && drone.stop == 0)
	{
		if (cable_disconnect_flag == 2)
		{
			cable_disconnect_flag = 1;
			drone.change_mode = 1;
			drone.current_mode = PANIC_MODE;
			return;
		}

		if(control_loop_flag == true && batt_low_flag == false)
		{
			// Clear sensor flag
			control_loop_flag = false;

			// Read sensor data
			read_sensor();
			
			__disable_irq();
			
			lift_force   = drone.joy_lift;//  + drone.key_lift;
			roll_s  = (signed char)drone.joy_roll;//  + (signed char)drone.key_roll;
			pitch_s = (signed char)drone.joy_pitch;// + (signed char)drone.key_pitch;
			yawrate_setpoint = (signed char)drone.joy_yaw;//   + (signed char)drone.key_yaw;			
			
			//lift_force = drone.joy_lift;
			//roll_s = (signed char)drone.joy_roll;
			//pitch_s = (signed char)drone.joy_pitch;
			//yawrate_setpoint = (signed char)drone.joy_yaw;
			__enable_irq();

			if (lift_force > 10) 
			{
				// XXX: Find range
				rollrate_setpoint = drone.controlgain_p1 * (roll_s - (drone.phi/15));
				rollrate_setpoint2 = drone.controlgain_p2 * (-drone.sp/6);
				roll_moment = rollrate_setpoint + rollrate_setpoint2;

				pitchrate_setpoint = drone.controlgain_p1 * (pitch_s - (drone.theta/15));
				pitchrate_setpoint2 = drone.controlgain_p2 * (drone.sq/6);
				pitch_moment = pitchrate_setpoint + pitchrate_setpoint2;
				
				yaw_error = (int)(yawrate_setpoint - (drone.sr/24));
				yaw_moment = (int)drone.controlgain_yaw*yaw_error;
			}
			else
			{
				roll_moment = 0;
				pitch_moment = 0;
				yaw_moment = 0;
			}

			lift  = DRONE_LIFT_CONSTANT * lift_force;
			roll  = (int)(DRONE_ROLL_CONSTANT/50 * roll_moment);
			pitch = (int)(DRONE_PITCH_CONSTANT/50 * pitch_moment);
			yaw   = (int)(DRONE_YAW_CONSTANT/8 * yaw_moment);

			//calculate_rotor_speeds(lift, pitch, roll, yaw);
			
			if(drone.joy_lift < 5)
			{
				pitch = 0;
				roll  = 0;
				yaw   = 0;
			}

			ae_[0] = 0.25*(lift + 2*pitch - yaw);
			ae_[1] = 0.25*(lift - 2*roll  + yaw);
			ae_[2] = 0.25*(lift - 2*pitch - yaw);
			ae_[3] = 0.25*(lift + 2*roll  + yaw);

			ae_[0] = ae_[0] < 0 ? 1 : (int)sqrt(ae_[0]);
			ae_[1] = ae_[1] < 0 ? 1 : (int)sqrt(ae_[1]);
			ae_[2] = ae_[2] < 0 ? 1 : (int)sqrt(ae_[2]);
			ae_[3] = ae_[3] < 0 ? 1 : (int)sqrt(ae_[3]);
			
			ae_[0] += ((signed char)drone.key_lift + (signed char)drone.key_pitch - (signed char)drone.key_yaw);
			ae_[1] += ((signed char)drone.key_lift - (signed char)drone.key_roll  + (signed char)drone.key_yaw);
			ae_[2] += ((signed char)drone.key_lift - (signed char)drone.key_pitch - (signed char)drone.key_yaw);
			ae_[3] += ((signed char)drone.key_lift + (signed char)drone.key_roll  + (signed char)drone.key_yaw);
			
			//printf("drone.joy_lift = %d\n", drone.joy_lift);
			
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

			ae_[0] = ae_[0] > MAX_RPM ? MAX_RPM : ae_[0];
			ae_[1] = ae_[1] > MAX_RPM ? MAX_RPM : ae_[1];
			ae_[2] = ae_[2] > MAX_RPM ? MAX_RPM : ae_[2];
			ae_[3] = ae_[3] > MAX_RPM ? MAX_RPM : ae_[3];	

			// setting drone rotor speeds
			drone.ae[0] = ae_[0];
			drone.ae[1] = ae_[1];
			drone.ae[2] = ae_[2];
			drone.ae[3] = ae_[3];

			//printf("%5d %5d %5d | %5d %5d %5d \n", drone.sp, drone.sq, drone.sr, drone.sax, drone.say, drone.saz);;
			//printf("%3d %3d %3d %3d | %d %d | %d | %d %d | %d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3], drone.controlgain_p1, drone.controlgain_p2, drone.controlgain_yaw, drone.phi, drone.theta, bat_volt);
			#if 0
			if(count%100 == 0)
			{
				//printf("%3d %3d %3d %3d | %d %d | %d | %d %d | %d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3], drone.controlgain_p1, drone.controlgain_p2, drone.controlgain_yaw, drone.phi, drone.theta, bat_volt);
				count = 0;
			}
			count++;
			#endif

			run_filters_and_control();
			
			// Update log and telemetry if corresponding flags are set
			check_log_tele_flags();
		}
		else
			nrf_delay_us(10);
	}

	//printf("Exit FULL_CONTROL_MODE\n");
}
#endif

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
			default :
					// XXX: Needs to be handled
					break;
		}
	}
}
