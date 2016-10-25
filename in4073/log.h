/*------------------------------------------------------------------
 *  log.h
 *
 *  Declarations of logging functions
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#ifndef LOG_H__
#define LOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "command_types.h"
#include "process_drone.h"
#include "in4073.h"
#include "crc.h"

void update_log();
void log_upload();
void uart_put_16bit(int16_t value);
void uart_put_32bit(int32_t value);

struct log write_data;
struct log read_data;

#endif
