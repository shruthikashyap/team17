#include "modes.h"
extern bool raw_mode_flag;
extern bool imu_init_flag;

void safe_mode()
{
	//printf("In SAFE_MODE\n");
	
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
			if(batt_low_flag == true)
			{
				//nrf_gpio_pin_toggle(RED);
				drone.stop = 1;
				//printf("Warning! Battery critically low!\n");
				return;
			}

			// Check battery level
			read_battery_level();

			// Update log and telemetry if corresponding flags are set
			check_log_tele_flags();
			
			// Change to RAW/DMP Mode
			if(raw_mode_flag == true && imu_init_flag == false)
			{
				imu_init(false, 500);
				//printf("In RAW_MODE\n");
				imu_init_flag = true;
			}
			else if(raw_mode_flag == false && imu_init_flag == false)
			{
				imu_init(true, 100);
				//printf("In DMP_MODE\n");
				imu_init_flag = true;
			}
			
			// Upload log
			if(log_upload_flag == true && telemetry_flag == false)
			{
				//printf("Inside safe mode - log upload\n");
				log_upload();
				drone.stop = 1;
			}
		}
		else
			nrf_delay_us(10);
	}
	
	//printf("Exit SAFE_MODE\n");
}