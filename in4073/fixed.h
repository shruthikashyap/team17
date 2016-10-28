/*------------------------------------------------------------------
 *  fixed.h -- File defines functions and constaints for fixed point 
 *			   arithmetic implementation. We're using a Q14 number 
 *			   format
 *
 *------------------------------------------------------------------
 */

#ifndef FIXED_H__
#define FIXED_H__

#include <inttypes.h>

#define QNUMBER 14

typedef int32_t q14; 				// holds 32bit

q14 q_mul(q14 a, q14 b);
q14 normal2q (int16_t x);
int16_t q2normal(q14 x);

#endif
