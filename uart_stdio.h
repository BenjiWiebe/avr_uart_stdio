#include <stdio.h>
#include <stdbool.h>

#define INBUF_SIZE		128
#define MAX_WRITE_BUF	128
#define MAX_READ_BUF	128
#ifndef BAUD
#	define BAUD	9600
#endif

extern FILE *uart_stdio_stream;
extern bool uart_stdio_using_2x;

/* Function to call to initialize UART and to set up stdin/stdout streams to point to UART.
 * Define BAUD to set baud rate.
 */
void init_uart_stdio(void);

/* Functions to put/get directly from UART, bypassing buffering
 */
void raw_putchar(unsigned char);
unsigned char raw_getchar(void);
