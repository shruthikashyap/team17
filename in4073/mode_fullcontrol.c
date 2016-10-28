/*------------------------------------------------------------------
 *  mode_fullcontrol.c
 *------------------------------------------------------------------
 */

#include "modes.h"

 /*------------------------------------------------------------------
 *  void full_control_mode
 *
 *  This function is the full mode control for the ES, using DMP 
 *  sensor values from the MPU. It controls the drone with two P controllers
 *  for the pitch and roll.
 *
 *  Author : Shruthi Kashyap (with mods by Kars and Evelyn)
 *------------------------------------------------------------------
 */
void full_control_mode()
{
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

	while(drone.change_mode == 0 && drone.stop == 0)
	{	
		// check for cable disconnection
		if (cable_disconnect_flag == 2)
		{
			cable_disconnect_flag = 1;
			drone.change_mode = 1;
			drone.current_mode = PANIC_MODE;
			return;
		}

		// only control if the loop flag is enabled to be sure we are controlling at a certain frequency
		if(control_loop_flag == true && batt_low_flag == false)
		{
			// Clear sensor flag
			control_loop_flag = false;

			// Read sensor data
			read_sensor();
			
			// disable sensor interrupts and read sensor values
			__disable_irq();
			lift_force = drone.joy_lift;
			roll_s  = (signed char)drone.joy_roll;
			pitch_s = (signed char)drone.joy_pitch;
			yawrate_setpoint = (signed char)drone.joy_yaw;
			__enable_irq();

			// only control if the joystick throttle is up
			if (lift_force > 10)
			{
				// P control for the angle, and a P controller for the angle rate
				// The control outputs are then added up and fed intro the drone
				rollrate_setpoint = drone.controlgain_p1 * (roll_s - (drone.phi/15));
				rollrate_setpoint2 = drone.controlgain_p2 * (-drone.sp/6);
				roll_moment = rollrate_setpoint + rollrate_setpoint2;

				pitchrate_setpoint = drone.controlgain_p1 * (pitch_s - (drone.theta/15));
				pitchrate_setpoint2 = drone.controlgain_p2 * (drone.sq/6);
				pitch_moment = pitchrate_setpoint + pitchrate_setpoint2;
				
				// simple yaw rate P control
				yaw_error = (int)(yawrate_setpoint - (drone.sr/12));
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
			yaw   = (int)(DRONE_YAW_CONSTANT/16 * yaw_moment);
			
			// only allow control when the joystick throttle is up
			if(drone.joy_lift < 5)
			{
				pitch = 0;
				roll  = 0;
				yaw   = 0;
			}

			// calculate the rotor speeds
			calculate_rotor_speeds(lift, pitch, roll, yaw);

			// update calculated rotor speeds.
			run_filters_and_control();
			
			// Update log and telemetry if corresponding flags are set
			check_log_tele_flags();
		}
		else
			nrf_delay_us(10);
	}
}


 /*------------------------------------------------------------------
 *  void full_control_mode
 *
 *  This function is the full mode control for the ES, using RAW 
 *  sensor values from the MPU. Pitch and roll are controlled using 
 *  cascaded P controllers.
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void full_control_mode_raw()
{
	nrf_gpio_pin_toggle(YELLOW);

	int rollrate_setpoint;
	int roll_s;
	int pitchrate_setpoint;
	int pitch_s;
	int roll_moment;
	int pitch_moment;
	int yawrate_setpoint;
	int yaw_error;
	int yaw_moment;
	int lift_force;
	int lift, roll, pitch, yaw;
		
	while(drone.change_mode == 0 && drone.stop == 0)
	{

		// check for cable disconnection
		if (cable_disconnect_flag == 2)
		{
			cable_disconnect_flag = 1;
			drone.change_mode = 1;
			drone.current_mode = PANIC_MODE;
			return;
		}

		if(control_loop_flag_raw == true && batt_low_flag == false)
		{
			control_loop_flag_raw = false;

			// Read sensor data
			read_sensor();

			// disable sensor interrupts and read sensor values			
			__disable_irq();
			lift_force = drone.joy_lift;
			roll_s  = (signed char)drone.joy_roll;
			pitch_s = (signed char)drone.joy_pitch;
			yawrate_setpoint = (signed char)drone.joy_yaw;			
			__enable_irq();

			// only control if the joystick throttle is up
			if (lift_force > 10) 
			{
				// cascaded P control for roll/phi
				rollrate_setpoint = drone.controlgain_p1 * (roll_s - (drone.phi/15));
				roll_moment = drone.controlgain_p2 * (rollrate_setpoint - drone.sp/6);

				// cascaded P control for pitch/theta
				pitchrate_setpoint = drone.controlgain_p1 * (pitch_s - (drone.theta/15));
				pitch_moment = drone.controlgain_p2 * (pitchrate_setpoint + drone.sq/6);
				
				// simple P control for yaw/sr
				yaw_error = (int)(yawrate_setpoint - (drone.sr/12));
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
			yaw   = (int)(DRONE_YAW_CONSTANT/16 * yaw_moment);

			// only allow control when the joystick throttle is up			
			if(drone.joy_lift < 5)
			{
				pitch = 0;
				roll  = 0;
				yaw   = 0;
			}

			// calculate rotor speeds
			calculate_rotor_speeds(lift, pitch, roll, yaw);			

			// update calculated rotor speeds
			run_filters_and_control();
			
			// Update log and telemetry if corresponding flags are set
			check_log_tele_flags();
		}
		else
			nrf_delay_us(10);
	}

	nrf_gpio_pin_toggle(YELLOW);
}
