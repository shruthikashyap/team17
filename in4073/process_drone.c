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
	drone.current_mode = SAFE_MODE;
	drone.stop = false;
	drone.change_mode = false;
}

void safe_mode()
{
}

void panic_mode()
{
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
		printf("\nDrone motor values: %3d %3d %3d %3d\n", ae[0], ae[1], ae[2], ae[3]);
		nrf_delay_ms(1000);
	}
#endif

	// Set defaults for the drone
	reset_drone();
	
	while (drone.stop == false)
	{
		drone.change_mode = false;
		
		switch (drone.current_mode)
		{
			case SAFE_MODE:
					safe_mode();
					break;
			case PANIC_MODE:
					panic_mode();
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
}
