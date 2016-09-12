#include <stdio.h>
#include "command_types.h"

void process_mode(char mode)
{
	switch (mode)
	{
		case SAFE_MODE:
		      break;
		case PANIC_MODE:
		      break;
		case MANUAL_MODE:
		      break;
		case YAW_CONTROL_MODE:
		      break;
		case FULL_CONTROL_MODE:
		      break;
		case CALIBRATION_MODE:
		      break;
		case HEIGHT_CONTROL_MODE:
		      break;
		case WIRELESS_MODE:
		      break;
		default :
		      // XXX: Needs to be handled
		      break;
	}
}
