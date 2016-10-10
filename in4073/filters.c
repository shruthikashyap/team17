#include "fixed.h"
#include "filters.h"
#include "in4073.h"

int x[6][2] = {{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0}};
int y[6][2] = {{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0}};

int drift_sp, drift_sq, drift_sr = 0;
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
void kalman()
{
	sp = sp - drift_sp; // real dtheta
	theta = theta + q2normal(q_mul(normal2q(sp),KALMAN_P2PHI)); // dt = P2PHI, fixed? or just calcuate time difference every time
	theta = theta - ((theta - sax)/KALMAN_C1);
	drift_sp = drift_sp + ((theta - sax)/KALMAN_C2);

	sq = sq - drift_sq;
	phi = phi + sq * KALMAN_P2PHI;
	phi = phi - ((phi - say)/KALMAN_C1);
	drift_sq = drift_sq + ((phi - say)/KALMAN_C2);

	sr = sr - drift_sr;
	psi = psi + sr * KALMAN_P2PHI;
	psi = psi - ((psi - saz)/KALMAN_C1);
	drift_sr = drift_sr + ((psi - saz)/KALMAN_C2);
}