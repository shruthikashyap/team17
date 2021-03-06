/*------------------------------------------------------------------
 *  mode_safe.c
 *------------------------------------------------------------------
 */

#include "modes.h"
extern bool raw_mode_flag;
extern bool imu_init_flag;

/*------------------------------------------------------------------
 *  void safe_mode
 *
 *  This is the safe state for the drone (zero rotor RPM). When the
 *  drone is in this mode, all commands from the PC terminal are 
 *  ignored. Only in this mode you can enable RAW mode and upload the log.
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void safe_mode()
{	
	// Don't read lift/roll/pitch/yaw data from PC link.
	// Reset drone control variables
	drone.key_lift = 0;
	drone.key_roll = 0;
	drone.key_pitch = 0;
	drone.key_yaw = 0;
	drone.joy_lift = 0;
	drone.joy_roll = 0;
	drone.joy_pitch = 0;
	drone.joy_yaw = 0;
	
	while(drone.change_mode == 0 && drone.stop == 0)
	{
		if(control_loop_flag == true)
		{	
			control_loop_flag = false;
			
			// Check ABORT flag
			if(abort_flag == true)
			{
				drone.stop = 1;
				abort_flag = false;
				return;
			}
			
			// Check battery level
			read_battery_level();
			if(batt_low_flag == true)
			{
				drone.stop = 1;
				return;
			}

			// Update log and telemetry if corresponding flags are set
			check_log_tele_flags();
			
			// Change to RAW/DMP Mode
			if(raw_mode_flag == true && imu_init_flag == false)
			{
				imu_init(false, 500);
				imu_init_flag = true;
			}
			else if(raw_mode_flag == false && imu_init_flag == false)
			{
				imu_init(true, 100);
				imu_init_flag = true;
			}
			
			// Upload log
			if(log_upload_flag == true && telemetry_flag == false)
			{
				log_upload();
				drone.stop = 1;
				return;
			}
		}
		else
			nrf_delay_us(10);
	}
}