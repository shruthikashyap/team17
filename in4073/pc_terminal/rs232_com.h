/*------------------------------------------------------------------
 *  rs232_com.h
 *
 *  Declarations for rs232 communication functions
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#ifndef RS232_COM_H__
#define RS232_COM_H__

extern int serial_device;
extern int fd_RS232;

void rs232_open(void);
void rs232_close(void);
int	rs232_getchar_nb();
int rs232_getchar();
int rs232_putchar(char c);

#endif
