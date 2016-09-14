#include <stdio.h>
#include "command_types.h"
#include "in4073.h"

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
	drone.stop = false;
	drone.change_mode = false;
	
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
	printf("\nIn SAFE_MODE");
	
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
		while(drone.ae[0] || drone.ae[1] || drone.ae[2] || drone.ae[3])
		{
			drone.ae[0] = (drone.ae[0]) < RPM_STEP? 0 : drone.ae[0] - 10;
			drone.ae[1] = (drone.ae[1]) < RPM_STEP? 0 : drone.ae[1] - 10;
			drone.ae[2] = (drone.ae[2]) < RPM_STEP? 0 : drone.ae[2] - 10;
			drone.ae[3] = (drone.ae[3]) < RPM_STEP? 0 : drone.ae[3] - 10;
			printf("\nDrone motor values: %3d %3d %3d %3d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3]);
			nrf_delay_ms(1000);
		}
#endif

	while(drone.change_mode == false && drone.stop == false);
}

void panic_mode()
{
	printf("\nIn PANIC_MODE");
	
	// Disable UART interrupts
	NVIC_DisableIRQ(UART0_IRQn);
	
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
	printf("\nDrone motor values: %3d %3d %3d %3d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3]);
#endif
	
	// Stay in this mode for a few seconds
	nrf_delay_ms(5000);
	
	// Go to Safe mode
	drone.current_mode = SAFE_MODE;
	drone.change_mode = true; // XXX: Is this needed?
	
	// Enable UART interrupts
	NVIC_EnableIRQ(UART0_IRQn);
}

void manual_mode()
{
}

void yaw_control_mode()
{
}

void full_control_mode()
{
}

void calibration_mode()
{
}

void raw_mode()
{
}

void height_control_mode()
{
}

void wireless_mode()
{
}

void process_drone()
{
#if 0
	// XXX: Interrupt test
	while (drone.stop == false)
	{
		printf("\nDrone motor values: %3d %3d %3d %3d, size = %d\n", ae[0], ae[1], ae[2], ae[3], sizeof(int16_t));
		nrf_delay_ms(500);
	}
#endif

#if 1
	// Set defaults for the drone
	reset_drone();
	
	while (drone.stop == false)
	{
		drone.change_mode = false;
		
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
