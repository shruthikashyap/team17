#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "command_types.h"
#include "in4073.h"
#include "process_drone.h"

void panic_mode();
void safe_mode();
void manual_mode();
void yaw_control_mode();
void full_control_mode();
void calibration_mode();