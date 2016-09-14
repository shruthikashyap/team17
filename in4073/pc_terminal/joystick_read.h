#define JS_DEV	"/dev/input/js0"

extern int fd;
extern struct js_event js;

void sendJsPacket();
void scale_joystick_values(int axis[6]);
