/*------------------------------------------------------------------
 *  mode_manual.c
 *
 *  Defines the manual mode of the drone
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#include "modes.h"

void manual_mode()
{
	//printf("In MANUAL_MODE\n");
	
	int ae_[4]; // Motor rpm values
	int lift_force;
	int roll_moment;
	int pitch_moment;
	int yaw_moment;
	int lift;
	int pitch;
	int roll;
	int yaw;
	
	drone.joy_lift = 100; // XXX: For testing

	while(drone.change_mode == 0 && drone.stop == 0)
	{
		if (cable_disconnect_flag == 2)
		{	
			cable_disconnect_flag = 1;
			drone.change_mode = 1;
			drone.current_mode = PANIC_MODE;
			return;
		}

		// Disable UART interrupts
		__disable_irq();

		// joystick goes from -127 to 128. key_lift goes from -127 to 128 so they weigh the same
		lift_force   = drone.joy_lift;//  + drone.key_lift;
		roll_moment  = (signed char)drone.joy_roll;//  + (signed char)drone.key_roll;
		pitch_moment = (signed char)drone.joy_pitch;// + (signed char)drone.key_pitch;
		yaw_moment   = (signed char)drone.joy_yaw;//   + (signed char)drone.key_yaw;

		// Enable UART interrupts
		__enable_irq();

		lift  = DRONE_LIFT_CONSTANT * lift_force;
		pitch = DRONE_PITCH_CONSTANT * pitch_moment;
		roll  = DRONE_ROLL_CONSTANT * roll_moment;
		yaw   = DRONE_YAW_CONSTANT * yaw_moment;

		if ((signed char)drone.joy_lift < 5)
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
		
		if (drone.joy_lift > 5)
		{
			ae_[0] = ae_[0] < MIN_RPM ? MIN_RPM : ae_[0];
			ae_[1] = ae_[1] < MIN_RPM ? MIN_RPM : ae_[1];
			ae_[2] = ae_[2] < MIN_RPM ? MIN_RPM : ae_[2];
			ae_[3] = ae_[3] < MIN_RPM ? MIN_RPM : ae_[3];
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

		//printf("%3d %3d %3d %3d | %d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3], bat_volt);
		
		// Update log and telemetry if corresponding flags are set
		check_log_tele_flags();

		run_filters_and_control();

		nrf_delay_ms(1);
	}
	
	//printf("Exit MANUAL_MODE\n");
}