/*------------------------------------------------------------------
 *  term_io.c
 *
 *  Declarations for terminal IO functions
 *
 *  June 2016
 *------------------------------------------------------------------
 */

#ifndef TERM_IO_H__
#define TERM_IO_H__

void term_initio();
void term_exitio();
void term_puts(char *s);
void term_putchar(char c);
int term_getchar_nb();
int term_getchar();

#endif
