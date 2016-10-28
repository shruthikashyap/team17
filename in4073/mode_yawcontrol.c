/*------------------------------------------------------------------
 *  mode_yawcontrol.c
 *------------------------------------------------------------------
 */

#include "modes.h"

 /*------------------------------------------------------------------
 *  void yaw_control_mode
 *
 *  This is the yaw controlled mode for the ES. You can control the yaw-rate
 *  with this mode. Uses a P controller
 *
 *  Author : Kars Heinen 
 *------------------------------------------------------------------
 */
void yaw_control_mode()
{
	int yawrate_setpoint;
	int yaw_error;

	int yaw_moment;
	int lift_force;

	int lift, roll, pitch, yaw;
	
	int pressure_error;

	// manually set the control gain for height control for now.
	drone.controlgain_height = 1; 
	
	while(drone.change_mode == 0 && drone.stop == 0)
	{
		// check cable disconnection
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

			// height control testing in yaw mode
			if(height_control_flag == true)
			{
				// while running height mode, exit from height mode when you touch the joystick throttle
				if(drone.height_control_lift != drone.joy_lift)
					height_control_flag = false;
				else
				{
					// simple P control for height mode
					pressure_error = pressure - drone.height_control_pressure;
					lift_force = HOVER_LIFT_FORCE + ((int)drone.controlgain_height * pressure_error);
				}
			}

			// only allow control when joystick throttle is up
			if (lift_force > 10) 
			{
				yaw_error = (int)(yawrate_setpoint - (drone.sr/12));
				yaw_moment = (int)drone.controlgain_yaw*yaw_error;
			}
			else
			{
				yaw_moment = 0;
			}

			lift  = DRONE_LIFT_CONSTANT * lift_force;
			roll  = 0; // no roll or pitch in yaw mode
			pitch = 0;
			yaw   = (int)(DRONE_YAW_CONSTANT/8 * yaw_moment);

			if (drone.joy_lift < 5)
			{
				pitch = 0;
				roll  = 0;
				yaw   = 0;
			}

			// calculate rotor speeds
			calculate_rotor_speeds(lift, pitch, roll, yaw);

			// update rotor speeds
			run_filters_and_control();
			
			// Update log and telemetry if corresponding flags are set
			check_log_tele_flags();
		}
		else
			nrf_delay_us(10);
	}
}