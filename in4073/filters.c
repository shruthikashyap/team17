/*------------------------------------------------------------------
 *  filters.c -- file includes the butterworth filter and the kalman filter
 *
 *  October 2016
 *------------------------------------------------------------------
 */


#include "fixed.h"
#include "filters.h"
#include "in4073.h"

q14 x[6][2] = {{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0}};
q14 y[6][2] = {{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0}};

q14 drift_sp, drift_sq, drift_sr = 0;
q14 q14theta, q14phi, q14psi = 0;
q14 q14sp, q14sq, q14sr;
q14 q14sax, q14say, q14saz;
//int theta = 0;

// first order butterworth, 100hz sample freq, 10hz cut off freq
void butterworth()
{
	x[0][1] = normal2q(sp);
	x[1][1] = normal2q(sq);
	x[2][1] = normal2q(sr);
	x[3][1] = normal2q(sax);
	x[4][1] = normal2q(say);
	x[5][1] = normal2q(saz);

	for (int i=0; i<6; i++) 
	{
		y[i][1] = q_mul(BUTTER_A1,x[i][1]) + q_mul(BUTTER_A2,x[i][0]) - q_mul(BUTTER_B2,y[i][0]); 

		x[i][0] = x[i][1];
		y[i][0] = y[i][1];
	}

	sp  = q2normal(y[0][1]);
	sq  = q2normal(y[1][1]);
	sr  = q2normal(y[2][1]);
	sax = q2normal(y[3][1]);
	say = q2normal(y[4][1]);
	saz = q2normal(y[5][1]);
}


// kalman filter ENGAGED:
// calculate 'real' phi theta psi and sp sq sr
// not working yet
void kalman()
{
	q14sp = normal2q(sp);
	q14sq = normal2q(sq);
	q14sr = normal2q(sr);

	q14sax = normal2q(sax);
	q14say = normal2q(say);
	q14saz = normal2q(saz);

	q14sp = q14sp - drift_sp;
	q14theta = q14theta + q_mul(q14sp, KALMAN_P2PHI);
	q14theta = q14theta - q_mul(q14theta-q14sax, KALMAN_C1);
	drift_sp = drift_sp + q_mul(q14theta-q14sax, KALMAN_C3);

	q14sq = q14sq - drift_sq;
	q14phi = q14phi + q_mul(q14sq, KALMAN_P2PHI);
	q14phi = q14phi - q_mul(q14phi-q14say, KALMAN_C1);
	drift_sq = drift_sq + q_mul(q14phi-q14say, KALMAN_C2);

	q14sr = q14sr - drift_sr;
	q14psi = q14psi + q_mul(q14sr, KALMAN_P2PHI);
	q14psi = q14psi - q_mul(q14psi-q14saz, KALMAN_C1);
	drift_sr = drift_sr + q_mul(q14psi-q14saz, KALMAN_C2);

	sp = q2normal(q14sp);
	sq = q2normal(q14sp);
	sr = q2normal(q14sp);
	
	theta = q2normal(q14theta);
	phi   = q2normal(q14phi);
	psi   = q2normal(q14psi);
}