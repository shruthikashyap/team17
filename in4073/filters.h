#define BUTTER_A1 4017 // 0.245
#define BUTTER_A2 4017
#define BUTTER_B1 16383 // 1
#define BUTTER_B2 -8348  // -0.509

#define KALMAN_P2PHI 132 // 0.081 in Q14
#define KALMAN_C1 256
#define KALMAN_C2 1000000

void butterworth();
void kalman();