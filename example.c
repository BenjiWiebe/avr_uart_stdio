#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "uart_stdio.h"

int main(void)
{
	init_uart_stdio();
	_delay_ms(500);
	printf("Hello!\n");
	printf("Baud rate: %lu\n", (unsigned long)BAUD);
	printf("Using 2X: %s\n\n", uart_stdio_using_2x ? "yes" : "no");
	char buf[55] = {0};
	while(1)
	{
		fgets(buf, 55, stdin);
		printf("Entered '%s'\n", buf);
	}
}
