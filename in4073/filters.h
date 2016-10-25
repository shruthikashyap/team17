/*------------------------------------------------------------------
 *  filters.c -- file defines the filter constants used in the butterworth
 *               and kalman filter. 
 *  October 2016
 *------------------------------------------------------------------
 */

#ifndef FILTERS_H__
#define FILTERS_H__

// All in Q14
#define BUTTER_A1 2239 // 0.1367
#define BUTTER_A2 2239
#define BUTTER_B1 16383 // 1
#define BUTTER_B2 -11903  // -0.7265

#define KALMAN_P2PHI 132 // 0.081 in Q14
#define KALMAN_C1 63 // 1/256
#define KALMAN_C3 2 // 1.234567901234568e-04

void butterworth();
void kalman();

#endif
