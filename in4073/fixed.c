/*------------------------------------------------------------------
 *  fixed.c
 *------------------------------------------------------------------
 */

#include <inttypes.h>
#include "fixed.h"

/*------------------------------------------------------------------
 *  q14 q_mul
 *
 *  This function multiplies two Q14 numbers with eachother
 *
 *  Author : Kars Heinen
 *------------------------------------------------------------------
 */
q14 q_mul(q14 a, q14 b)
{
	q14 result;
	int64_t temp; // needs to store 64 bit because we are multiplying two 32bit numbers

	temp = (int64_t)a*b;

	// no saturation needed since we put it in a 64 bit variable
	result = (q14)(temp >> QNUMBER);

	return result;
}


/*------------------------------------------------------------------
 *  q14 normal2q
 *
 *  This function converts a 'normal' integer to a Q14 number
 *
 *  Author : Kars Heinen
 *------------------------------------------------------------------
 */
q14 normal2q(int16_t x)
{
	return x << QNUMBER;
}


/*------------------------------------------------------------------
 *  int16_t q2normal
 *
 *  This function converts a Q14 number to a 16bit integer
 *
 *  Author : Kars Heinen
 *------------------------------------------------------------------
 */
int16_t q2normal(q14 x)
{
	return x / (1 << QNUMBER); // divide by 2^Q
}