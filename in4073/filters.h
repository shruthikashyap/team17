/*------------------------------------------------------------------
 *  filters.h -- file defines the filter constants used in the butterworth, 
 *               kalman filter, and moving-average filter. 
 * 
 *------------------------------------------------------------------
 */

#ifndef FILTERS_H__
#define FILTERS_H__

// All in Q14
#define BUTTER_A1 969 					
#define BUTTER_A2 969
#define BUTTER_B1 16383 			
#define BUTTER_B2 -14444  		

#define KALMAN_P2PHI 265 
#define KALMAN_C1 63 
#define KALMAN_C3 2

#define MOVING_AVERAGE_SIZE 10

void butterworth();
void kalman();
void moving_average();

#endif