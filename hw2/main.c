

/**
 * main.c
 */

#include <stdio.h>

#define RCGCUART    (* ((volatile unsigned long *) 0x400FE618))
#define RCGCGPIO    (* ((volatile unsigned long *) 0x400FE608))

#define GPIODEN_A   (* ((volatile unsigned long *) 0x4000451C))
#define GPIOAFSL_A  (* ((volatile unsigned long *) 0x40004420))
#define GPIOPCTL_A  (* ((volatile unsigned long *) 0x4000452C))

#define UARTCTL     (* ((volatile unsigned long *) 0x4000C030))
#define UARTIBRD    (* ((volatile unsigned long *) 0x4000C024))
#define UARTFBRD    (* ((volatile unsigned long *) 0x4000C028))
#define UARTLCRH    (* ((volatile unsigned long *) 0x4000C02C))
#define UARTFR      (* ((volatile unsigned long *) 0x4000C018))
#define UARTDR      (* ((volatile unsigned long *) 0x4000C000))

#define GPIODIR_B   (* ((volatile unsigned long *) 0x40005400))
#define GPIODEN_B   (* ((volatile unsigned long *) 0x4000551C))
#define GPIODATA_PB2 (* ((volatile unsigned long *) 0x40005010))
#define GPIODATA_PB3 (* ((volatile unsigned long *) 0x40005020))
#define GPIOPUR_B   (* ((volatile unsigned long *) 0x40005510))

#define PB2_HIGH 0x04
#define PB3_HIGH 0x08

void setLED(int state) {
    if (state) {
        GPIODATA_PB2 |= 0xff;
    } else {
        GPIODATA_PB2 &= ~0xff;
    }
}


void UART_Tx(unsigned char data) {
    while ((UARTFR&0x20) != 0);
    UARTDR = data;
}

unsigned char UART_Rx() {
    while ((UARTFR&0x10) != 0);
    return ((unsigned char)UARTDR&0xFF);
}


int main(void)
{
	RCGCUART |= 0x01;    // Enable clock for UART module 0
	RCGCGPIO |= 0x03;    // Enable clock for Port A and Port B

	GPIOAFSL_A |= 0x03;  // Alternate function select for PA0 and PA1
	GPIODEN_A |= 0x03;   // Enable PA0, PA1
	GPIOPCTL_A = (GPIOPCTL_A & 0xFFFFFF00) | 0x00000011;    // Select UART functions for PA0 and PA1
	UARTCTL &= ~0x00;     // Disable UART before configuring
	// 16 MHz / (16*115,200 baud) = 8.6806
	UARTIBRD = 8;
	UARTFBRD = 43;      // integer(0.6806*64) = 43
	// Set UART Line Control with desired configuration
	// In this case: parity not enabled, one stop bit, and 8 bit word length
	UARTLCRH = 0x60;
	UARTCTL |= 0x301;    // Enable UART


	GPIODIR_B |= 0x0C;  // set PB2 and PB3 to output
	GPIODEN_B |= 0x0C;  // enable PB2 and PB3

    // positive default state
	GPIODATA_PB2 |= 0x04;
	GPIODATA_PB3 &= ~0x08;
	// Enter running loop
	while (1) {
	    char data = UART_Rx();
	    if (data == 'n') {
	        UART_Tx('N');
	        GPIODATA_PB3 |= 0x08;
	        GPIODATA_PB2 &= ~0x04;
	    }

	    else if (data == 'p') {
	        UART_Tx('P');
	        GPIODATA_PB2 |= 0x04;
	        GPIODATA_PB3 &= ~0x08;
	    }


	}


}
