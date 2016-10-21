#include "modes.h"
#if 0
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
			//drone.change_mode = 1;
			//drone.current_mode = PANIC_MODE;
			//return;
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

			calculate_rotor_speeds(lift, pitch, roll, yaw);
			
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
#endif