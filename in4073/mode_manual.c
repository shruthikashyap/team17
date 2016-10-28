/*------------------------------------------------------------------
 *  mode_manual.c
 *------------------------------------------------------------------
 */

#include "modes.h"


 /*------------------------------------------------------------------
 *  void manual_mode
 *
 *  This function is the manual mode for the ES. There is no sensor
 *  feedback.
 *
 *  Author : Kars Heinen
 *------------------------------------------------------------------
 */
void manual_mode()
{	
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
		// check for cable disconnection
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
			
			// Disable UART interrupts to read sensor values
			__disable_irq();
			lift_force   = drone.joy_lift;
			roll_moment  = (signed char)drone.joy_roll;
			pitch_moment = (signed char)drone.joy_pitch;
			yaw_moment   = (signed char)drone.joy_yaw;
			// Enable UART interrupts again
			__enable_irq();

			lift  = DRONE_LIFT_CONSTANT * lift_force;
			pitch = DRONE_PITCH_CONSTANT * pitch_moment;
			roll  = DRONE_ROLL_CONSTANT * roll_moment;
			yaw   = DRONE_YAW_CONSTANT * yaw_moment;

			// only allow control when the joystick throttle is up
			if ((signed char)drone.joy_lift < 5)
			{
				pitch = 0;
				roll  = 0;
				yaw   = 0;
			}

			// calculate rotor speeds
			calculate_rotor_speeds(lift, pitch, roll, yaw);

			// Check battery level
			read_battery_level();

			// Update log and telemetry if corresponding flags are set
			check_log_tele_flags();

			// update the rotor speeds
			run_filters_and_control();
		}
		else
			nrf_delay_us(10);
	}
}