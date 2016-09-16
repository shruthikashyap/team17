#include <stdio.h>
#include <math.h>
#include "command_types.h"
#include "in4073.h"
#include "process_drone.h"

/* XXX: 
1. No args for process_drone().
2. Maintain a global variable for mode.
3. Maintain a global variable to indicate change of mode. Be in a mode until this variable doesn't change.
3. Have a while loop in process_drone() to handle change of modes.
4. Set defaults.
5. process_drone() should be the main blocking function. The UART interrupt should populate the global values.
6. Is a drone struct needed?
*/

void reset_drone()
{
	// XXX: Set mode as SAFE_MODE and clear all flags
	drone.current_mode = SAFE_MODE; // XXX: Test
	drone.stop = 0;
	drone.change_mode = 0;
	
	// Reset drone control variables
	drone.key_lift = 0;
	drone.key_roll = 0;
	drone.key_pitch = 0;
	drone.key_yaw = 0;
	drone.joy_lift = 0;
	drone.joy_roll = 0;
	drone.joy_pitch = 0;
	drone.joy_yaw = 0;
}

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
	
#if 0
		// XXX: Test. drone.ae[] is not being used right now. The global variable ae[] is being used to drive the motors.
		while(ae[0] || ae[1] || ae[2] || ae[3])
		{
			ae[0] = (ae[0]) < RPM_STEP? 0 : ae[0] - 10;
			ae[1] = (ae[1]) < RPM_STEP? 0 : ae[1] - 10;
			ae[2] = (ae[2]) < RPM_STEP? 0 : ae[2] - 10;
			ae[3] = (ae[3]) < RPM_STEP? 0 : ae[3] - 10;
			printf("\nDrone motor values: %3d %3d %3d %3d\n", ae[0], ae[1], ae[2], ae[3]);
			nrf_delay_ms(1000);
		}
#endif

#if 1
		// Gradually reduce RPM of the motors to 0.
		__disable_irq();
		while(drone.ae[0] || drone.ae[1] || drone.ae[2] || drone.ae[3])
		{
			drone.ae[0] = (drone.ae[0]) < RPM_STEP? 0 : drone.ae[0] - 10;
			drone.ae[1] = (drone.ae[1]) < RPM_STEP? 0 : drone.ae[1] - 10;
			drone.ae[2] = (drone.ae[2]) < RPM_STEP? 0 : drone.ae[2] - 10;
			drone.ae[3] = (drone.ae[3]) < RPM_STEP? 0 : drone.ae[3] - 10;
			//printf("\nDrone motor values: %3d %3d %3d %3d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3]);
			nrf_delay_ms(1000);
		}
		__enable_irq();
#endif

	while(drone.change_mode == 0 && drone.stop == 0)
	{
		//printf("SAFE - drone.change_mode = %d\n", drone.change_mode);
		nrf_delay_ms(1000);
	}
	printf("Exit SAFE_MODE\n");
}

void panic_mode()
{
	printf("In PANIC_MODE\n");
	
	// Disable UART interrupts
	//NVIC_DisableIRQ(UART0_IRQn);
	__disable_irq();
	
#if 0
	// XXX: Test. drone.ae[] is not being used right now. The global variable ae[] is being used to drive the motors.
	ae[0] = HOVER_RPM;
	ae[1] = HOVER_RPM;
	ae[2] = HOVER_RPM;
	ae[3] = HOVER_RPM;
#endif

#if 1
	// Set moderate RPM values to the motors for hovering
	drone.ae[0] = HOVER_RPM;
	drone.ae[1] = HOVER_RPM;
	drone.ae[2] = HOVER_RPM;
	drone.ae[3] = HOVER_RPM;
	printf("Drone motor values: %3d %3d %3d %3d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3]);
#endif
	
	// Stay in this mode for a few seconds
	nrf_delay_ms(5000);
	
	// Go to Safe mode
	drone.current_mode = SAFE_MODE;
	drone.change_mode = 1; // XXX: Is this needed?
	
	printf("Exit PANIC_MODE\n");
	
	// Enable UART interrupts
	//NVIC_EnableIRQ(UART0_IRQn);
	__enable_irq();
}

// Scales numbers from a range into another range
int scale_number(int x, int in_min, int in_max, int out_min, int out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void manual_mode()
{
	printf("In MANUAL_MODE\n");
	
	int ae_[4]; // Motor rpm values

	// Disable UART interrupts
	//NVIC_DisableIRQ(UART0_IRQn);
	__disable_irq();
	
	// joystick goes from -127 to 128. key_lift goes from -127 to 128 so they weigh the same
	int lift_force   = drone.joy_lift  + drone.key_lift;
	int roll_moment  = drone.joy_roll  + drone.key_roll;
	int pitch_moment = drone.joy_pitch + drone.key_pitch;
	int yaw_moment   = drone.joy_yaw   + drone.key_yaw;

	// Enable UART interrupts
	//NVIC_EnableIRQ(UART0_IRQn);
	__enable_irq();

	int lift  = DRONE_LIFT_CONSTANT * lift_force;
	int pitch = DRONE_PITCH_CONSTANT * pitch_moment;
	int roll  = DRONE_ROLL_CONSTANT * roll_moment;
	int yaw   = DRONE_YAW_CONSTANT * yaw_moment;

	// no need to do this, rotor speeds are getting maxed/mined a few lines later
	lift  = lift  < MIN_LIFT  ? MIN_LIFT  : lift;
	roll  = roll  < MIN_ROLL  ? MIN_ROLL  : roll;
	pitch = pitch < MIN_PITCH ? MIN_PITCH : pitch;
	yaw   = yaw   < MIN_YAW   ? MIN_YAW   : yaw;

	lift  = lift  > MAX_LIFT  ? MAX_LIFT  : lift;
	roll  = roll  > MAX_ROLL  ? MAX_ROLL  : roll;
	pitch = pitch > MAX_PITCH ? MAX_PITCH : pitch;
	yaw   = yaw   > MAX_YAW   ? MAX_YAW   : yaw;

	// Solving drone rotor dynamics equations
	ae_[0] = (int)sqrt(0.25*(lift + 2*pitch - yaw));
	ae_[1] = (int)sqrt(0.25*(lift - 2*roll  + yaw));
	ae_[2] = (int)sqrt(0.25*(lift - 2*pitch - yaw));
	ae_[3] = (int)sqrt(0.25*(lift + 2*roll  + yaw));

	// checking min/max	
	ae_[0] = ae_[0] < MIN_RPM ? MIN_RPM : ae_[0];
	ae_[1] = ae_[1] < MIN_RPM ? MIN_RPM : ae_[1];
	ae_[2] = ae_[2] < MIN_RPM ? MIN_RPM : ae_[2];
	ae_[3] = ae_[3] < MIN_RPM ? MIN_RPM : ae_[3];
	
	ae_[0] = ae_[0] > MAX_RPM ? MAX_RPM : ae_[0];
	ae_[1] = ae_[1] > MAX_RPM ? MAX_RPM : ae_[1];
	ae_[2] = ae_[2] > MAX_RPM ? MAX_RPM : ae_[2];
	ae_[3] = ae_[3] > MAX_RPM ? MAX_RPM : ae_[3];	

	// setting drone rotor speeds
	drone.ae[0] = ae_[0];
	drone.ae[1] = ae_[1];
	drone.ae[2] = ae_[2];
	drone.ae[3] = ae_[3];

	printf("%3d %3d %3d %3d \n", ae_[0], ae_[1], ae_[2], ae_[3]);
	//printf("%3d %3d %3d %3d %3d \n",ae[0], lift, roll, pitch, yaw);
}

void yaw_control_mode()
{
	printf("In YAW_CONTROL_MODE\n");
	
	while(drone.change_mode == 0 && drone.stop == 0)
	{
		//printf("YAW - drone.change_mode = %d\n", drone.change_mode);
		nrf_delay_ms(1000);
	}
	
	printf("Exit YAW_CONTROL_MODE\n");
}

void full_control_mode()
{
	printf("In FULL_CONTROL_MODE\n");
	
	while(drone.change_mode == 0 && drone.stop == 0)
	{
		//printf("FULL - drone.change_mode = %d\n", drone.change_mode);
		nrf_delay_ms(1000);
	}
	
	printf("Exit FULL_CONTROL_MODE\n");
}

void calibration_mode()
{
	printf("In CALIBRATION_MODE\n");

	while(drone.change_mode == 0 && drone.stop == 0)
	{
		//printf("CALIBRATION - drone.change_mode = %d\n", drone.change_mode);
		nrf_delay_ms(1000);
	}
	
	printf("Exit CALIBRATION_MODE\n");
}

void raw_mode()
{
	printf("In RAW_MODE\n");
}

void height_control_mode()
{
	printf("In HEIGHT_CONTROL_MODE\n");
}

void wireless_mode()
{
	printf("In WIRELESS_MODE\n");
}

void process_drone()
{
#if 0
	// XXX: Interrupt test
	while (drone.stop == 0)
	{
		printf("\nDrone motor values: %3d %3d %3d %3d, size = %d\n", ae[0], ae[1], ae[2], ae[3], sizeof(int16_t));
		nrf_delay_ms(500);
	}
#endif

#if 1
	// Set defaults for the drone
	reset_drone();
	
	while (drone.stop == 0)
	{
		drone.change_mode = 0;

		switch (drone.current_mode)
		{
			case SAFE_MODE:
					// XXX: Test
					//ae[0] = ae[1] = ae[2] = ae[3] = 100;
					safe_mode();
					break;
			case PANIC_MODE:
					// XXX: Test
					//ae[0] = ae[1] = ae[2] = ae[3] = 500;
					//printf("\nDrone motor values before: %3d %3d %3d %3d\n", ae[0], ae[1], ae[2], ae[3]);
					panic_mode();
					// XXX: Test
					//printf("\nDrone motor values after: %3d %3d %3d %3d\n", ae[0], ae[1], ae[2], ae[3]);
					break;
			case MANUAL_MODE:
					manual_mode();
					break;
			case YAW_CONTROL_MODE:
					yaw_control_mode();
					break;
			case FULL_CONTROL_MODE:
					full_control_mode();
					break;
			case CALIBRATION_MODE:
					calibration_mode();
					break;
			case RAW_MODE:
					raw_mode();
					break;
			case HEIGHT_CONTROL_MODE:
					height_control_mode();
					break;
			case WIRELESS_MODE:
					wireless_mode();
					break;
			default :
					// XXX: Needs to be handled
					break;
		}
	}
#endif
}
