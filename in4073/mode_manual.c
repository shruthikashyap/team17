#include "modes.h"

void manual_mode()
{
	//printf("In MANUAL_MODE\n");
	
	int lift;
	int pitch;
	int roll;
	int yaw;
	drone.joy_lift = 100; // XXX: For testing

	while(drone.change_mode == 0 && drone.stop == 0)
	{
		if (cable_disconnect_flag == 2)
		{
			//printf("Going into panic mode from cable disconnection!!!\n");
			cable_disconnect_flag = 1;
			//drone.change_mode = 1;
			//drone.current_mode = PANIC_MODE;
			//return;
		}

		if(control_loop_flag == true && batt_low_flag == false)
		{
			// Clear sensor flag
			control_loop_flag = false;
			
			// Disable UART interrupts
			__disable_irq();
			lift  = DRONE_MANUAL_LIFT_CONSTANT  * drone.joy_lift;
			pitch = DRONE_MANUAL_PITCH_CONSTANT * (signed char)drone.joy_pitch;
			roll  = DRONE_MANUAL_ROLL_CONSTANT  * (signed char)drone.joy_roll;
			yaw   = DRONE_MANUAL_YAW_CONSTANT   * (signed char)drone.joy_yaw;
			__enable_irq();

			if((signed char)drone.joy_lift < 5)
			{
				pitch = 0;
				roll  = 0;
				yaw   = 0;
			}

			calculate_rotor_speeds(lift, pitch, roll, yaw);
			
			//printf("%3d %3d %3d %3d | %d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3], bat_volt);

			run_filters_and_control();
			
			// Check battery level
			read_battery_level();
			
			// Update log and telemetry if corresponding flags are set
			check_log_tele_flags();
		}
		else
			nrf_delay_ms(1);
	}
	
	//printf("Exit MANUAL_MODE\n");
}