#include "modes.h"
uint32_t ae_[4];

void full_control_mode()
{
	//printf("In FULL_CONTROL_MODE\n");

	uint32_t ae_[4];
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
	
	drone.joy_lift = 100; // XXX: For testing
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