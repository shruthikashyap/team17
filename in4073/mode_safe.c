#include "modes.h"

void safe_mode()
{
	printf("In SAFE_MODE\n");
	
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

			if(log_upload_flag == true && telemetry_flag == false)
			{
				//printf("Inside safe mode - log upload\n");
				log_upload();
			}
		}
		else
			nrf_delay_ms(1);
	}
	
	//printf("Exit SAFE_MODE\n");
}