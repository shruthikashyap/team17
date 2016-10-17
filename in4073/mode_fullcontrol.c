#include "modes.h"

void full_control_mode()
{
	//printf("In FULL_CONTROL_MODE\n");

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
	
	drone.joy_lift = 100; // XXX: For testing
	//int count = 0;
	
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
			roll_s = (signed char)drone.joy_roll;
			pitch_s = (signed char)drone.joy_pitch;
			yawrate_setpoint = (signed char)drone.joy_yaw;
			__enable_irq();

			if (lift_force > 10) 
			{
				// XXX: Find range
				rollrate_setpoint = drone.controlgain_p1/5 * (roll_s - (drone.phi/30));
				roll_moment = drone.controlgain_p2 * (rollrate_setpoint - (drone.sp/6));

				pitchrate_setpoint = drone.controlgain_p1/5 * (pitch_s - (drone.theta/30));
				pitch_moment = drone.controlgain_p2 * (pitchrate_setpoint + (drone.sq/6));
				
				yaw_error = -(int)(yawrate_setpoint - (drone.sr/24));
				yaw_moment = (int)drone.controlgain_yaw*yaw_error;
			}
			else
			{
				roll_moment = 0;
				pitch_moment = 0;
				yaw_moment = 0;
			}

			lift  = DRONE_LIFT_CONSTANT * lift_force;
			roll  = (int)(DRONE_ROLL_CONSTANT/100 * roll_moment);
			pitch = (int)(DRONE_PITCH_CONSTANT/100 * pitch_moment);
			yaw   = (int)(DRONE_YAW_CONSTANT/8 * yaw_moment);

			calculate_rotor_speeds(lift, pitch, roll, yaw);

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
			nrf_delay_ms(1);
	}

	//printf("Exit FULL_CONTROL_MODE\n");
}