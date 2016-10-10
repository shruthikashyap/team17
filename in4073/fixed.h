#include <inttypes.h>

#define QNUMBER 14 // can not name it Q because somewhere else?? errors...
#define K (1 << (QNUMBER - 1)) // not using this right now

typedef int32_t q14; // holds 32bit

q14 q_mul(q14 a, q14 b);
q14 normal2q (int16_t x);
int16_t q2normal(q14 x);
q14 number2neg(q14 a);