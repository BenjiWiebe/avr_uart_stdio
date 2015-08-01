#include <avr/io.h>
#include "uart_stdio.h"

// ---Public function(s)---

// Is set to true if using 2X, false otherwise
bool uart_stdio_using_2x;

// Sets up UART, stdin, and stdout.
void init_uart_stdio(void)
{
#include <util/setbaud.h>
	UCSRB=0b00011000; // RX/TX enabled, no UART interrupts enabled
	UBRRL=UBRRL_VALUE;
	UBRRH=UBRRH_VALUE;
#if USE_2X
	UCSRA |= (1 << U2X);
	uart_stdio_using_2x = true;
#else
	uart_stdio_using_2x = false;
#endif
	stdin=stdout=uart_stdio_stream;
}

// Put character directly to UART
void raw_putchar(unsigned char c)
{
	if(c == '\n')
		raw_putchar('\r');
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = c;
}

// Get character directly from UART
unsigned char raw_getchar(void)
{
	loop_until_bit_is_set(UCSRA, RXC);
	return UDR;
}


// ---Static function(s)---

// stdio wrapper functions, to be passed to FDEV_SETUP_STREAM
static int stdio_putchar_wrapper(char,FILE*);
static int stdio_getchar_wrapper(FILE*);

// getchar()-like function, using raw_getchar/raw_putchar and buffering input
static unsigned char buffered_getchar(void);


static int stdio_putchar_wrapper(char c, FILE *stream)
{
	raw_putchar(c);
	return 0;
}

static int stdio_getchar_wrapper(FILE *stream)
{
	return buffered_getchar();
}

static FILE _uart_stdio_stream = FDEV_SETUP_STREAM(stdio_putchar_wrapper, stdio_getchar_wrapper, _FDEV_SETUP_RW);
FILE *uart_stdio_stream = &_uart_stdio_stream;

static unsigned char buffered_getchar(void)
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
			c = raw_getchar();
			// Skip this character if there's only one space in the buffer left, and the inraw_putchar is not a newline
			// We must leave one space in the buffer, to raw_putchar the newline in.
			if(c != '\n' && c != '\r' && idx == INBUF_SIZE - 1)
				continue;
			switch(c)
			{
				case '\b':
				case  127: // Backspace
					if(idx == 0)
						break;
					raw_putchar('\b');
					raw_putchar(' ');
					raw_putchar('\b');
					idx--;
					break;
				case '\r': // Carriage return
				case '\n': // Newline
					raw_putchar('\n');
					inbuf[idx++] = '\n';
					do_fill_buffer = false;
					done = true;
					break;
				default:
					raw_putchar(c);
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
