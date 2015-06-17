/*
 * avr_test.c
 *
 * Created: 6/5/2015 8:42:22 PM
 *  Author: Benji
 */ 

#ifndef F_CPU
#define F_CPU 8000000
#endif

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdbool.h>

#define INBUF_SIZE	128

void uart_putchar_raw(char c)
{
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = c;
}

int uart_getchar_raw(void)
{
	loop_until_bit_is_set(UCSRA, RXC);
	return UDR;
}

// putchar_raw(char c): Puts character c onto output (display, UART, etc). Guaranteed to not be buffered.
// TODO Is this really the right place to do LF -> CRLF conversion?? It belongs with the buffering, IMO. (Whenever that will be added...)
void putchar_raw(char c)
{
	if(c == '\n')
		uart_putchar_raw('\r');
	uart_putchar_raw(c);
}

int uartstdio_putchar(char c, FILE *stream)
{
	putchar_raw(c);
	return 0;
}

int getchar_raw(void)
{
	return uart_getchar_raw();
}

int uartstdio_getchar(FILE *stream)
{
	static bool do_fill_buffer = true;
	static unsigned char inbuf[INBUF_SIZE];
	static unsigned char idx = 0, // Index of next unused place (NOT guaranteed to be NULL)
						 empty_idx = 0; // Index of next place to return if !do_fill_buffer (i.e., index variable for looping forward through the buffer)
	unsigned char c = 0;
	bool done = false;
	if(do_fill_buffer)
	{
		while(!done)
		{
			c = getchar_raw();
			// Skip this character if there's only one space in the buffer left, and the input is not a newline
			// We must leave one space in the buffer, to put the newline in.
			if(c != '\n' && c != '\r' && idx == INBUF_SIZE - 1)
				continue;
			switch(c)
			{
				case '\b':
				case  127: // Backspace
					if(idx == 0)
						break;
					putchar_raw('\b');
					putchar_raw(' ');
					putchar_raw('\b');
					idx--;
					break;
				case '\r': // Carriage return
				case '\n': // Newline
					putchar_raw('\n');
					inbuf[idx++] = '\n';
					do_fill_buffer = false;
					done = true;
					break;
				default:
					putchar_raw(c);
					inbuf[idx++] = c;
					break;
			}
		}
	}
	char retval = inbuf[empty_idx++];
	if(empty_idx == idx)
	{
		do_fill_buffer = true;
		empty_idx = 0;
		idx = 0;
		done = false;
	}
	return retval;
}

static FILE uartstdio = FDEV_SETUP_STREAM(uartstdio_putchar, uartstdio_getchar, _FDEV_SETUP_RW);

void init_uart()
{
#ifndef BAUD
#define BAUD 9600
#endif
#include <util/setbaud.h>
	UCSRB=0b00011000; // RX/TX enabled, no UART interrupts enabled
	UBRRL=UBRRL_VALUE;
	UBRRH=UBRRH_VALUE;
#if USE_2X
	UCSRA |= (1 << U2X);
#endif
#undef UBRRL_VALUE
#undef UBRRH_VALUE
	stdout = &uartstdio;
	stdin = &uartstdio;
}

int main(void)
{
	init_uart();
	_delay_ms(500);
	printf("Hello!\n");
	printf("Baud rate: %lu\n", (unsigned long)BAUD);
	printf("Using 2X: %s\n\n", USE_2X ? "yes" : "no");
	char buf[55] = {0};
	while(1)
	{
		fgets(buf, 55, stdin);
		printf("Entered '%s'\n", buf);
	}
}
