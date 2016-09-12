#define JS_DEV	"/dev/input/js0"

extern int fd;
extern struct js_event js;

void joystick_read (int axis[6]);
