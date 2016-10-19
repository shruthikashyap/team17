#define BUTTER_A1 2239 // 0.1367
#define BUTTER_A2 2239
#define BUTTER_B1 16383 // 1
#define BUTTER_B2 -11903  // -0.7265

#define KALMAN_P2PHI 132 // 0.081 in Q14
#define KALMAN_C1 256
#define KALMAN_C2 1000000

void butterworth();
void kalman();