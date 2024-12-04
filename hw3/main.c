

/**
 * main.c
 */

// Registers for timer
#define RCGCTIMER   (* ((volatile unsigned long *) 0x400FE604)) // Clock control for timer
#define GPTMCTL     (* ((volatile unsigned long *) 0x4003000C)) // Control for timer 0
#define GPTMCFG     (* ((volatile unsigned long *) 0x40030000)) // Configuration for timer 0
#define GPTMATMR    (* ((volatile unsigned long *) 0x40030004)) // Timer A mode
#define GPTMTAPR    (* ((volatile unsigned long *) 0x40030038)) // Timer A Prescalar
#define GPTMTAILR   (* ((volatile unsigned long *) 0x40030028)) // Timer A Interval Load
#define GPTMRIS     (* ((volatile unsigned long *) 0x4003001C)) // Timer A raw interrupt status
#define GPTMIMR     (* ((volatile unsigned long *) 0x40030018)) // Timer A interrupt mask
#define GPTMICR     (* ((volatile unsigned long *) 0x40030024)) // Timer A interrupt clear

// Registers for timer interrupts
#define NVIC_EN0    (* ((volatile unsigned long *) 0xE000E100)) // NVIC EN0 register

// Registers for UART
#define RCGCUART    (* ((volatile unsigned long *) 0x400FE618)) // Clock control for UART
#define UARTCTL     (* ((volatile unsigned long *) 0x4000C030)) // UART0 control
#define UARTIBRD    (* ((volatile unsigned long *) 0x4000C024)) // UART0 integer baud rate divisor
#define UARTFBRD    (* ((volatile unsigned long *) 0x4000C028)) // UART0 fractional baud rate divisor
#define UARTLCRH    (* ((volatile unsigned long *) 0x4000C02C)) // UART0 line control
#define UARTFR      (* ((volatile unsigned long *) 0x4000C018)) // UART0 flag register
#define UARTDR      (* ((volatile unsigned long *) 0x4000C000)) // UART0 data register

// Registers for GPIO
#define RCGCGPIO    (* ((volatile unsigned long *) 0x400FE608)) // Clock control for GPIO
#define GPIODEN_A   (* ((volatile unsigned long *) 0x4000451C)) // Digital enable for Port A
#define GPIODIR_A   (* ((volatile unsigned long *) 0x40004400)) // Direction for Port A
#define GPIOAFSL_A  (* ((volatile unsigned long *) 0x40004420)) // Alternate function select for Port A
#define GPIOPCTL_A  (* ((volatile unsigned long *) 0x4000452C)) // Port control for Port A
#define GPIOPDR_A   (* ((volatile unsigned long *) 0x40004514)) // Pull-down select for Port A
#define GPIOIM_A    (* ((volatile unsigned long *) 0x40004410)) // Interrupt mask for Port A
#define GPIOIEV_A   (* ((volatile unsigned long *) 0x4000440C)) // Interrupt event for Port A
#define GPIORIS_A   (* ((volatile unsigned long *) 0x40004414)) // Raw interrupt status for Port A
#define GPIOICR_A   (* ((volatile unsigned long *) 0x4000441C)) // Interrupt clear for Port A
#define GPIOIBE_A   (* ((volatile unsigned long *) 0x40004408)) // Interrupt both edges for Port A

#define GPIODATA_B  (* ((volatile unsigned long *) 0x400053C0)) // Data register for Port B
#define GPIODIR_B   (* ((volatile unsigned long *) 0x40005400)) // Direction for Port B
#define GPIODEN_B   (* ((volatile unsigned long *) 0x4000551C)) // Digital enable for Port B
#define GPIOPDR_B   (* ((volatile unsigned long *) 0x40005514)) // Pull-down select for Port B

#define GPIODIR_F   (* ((volatile unsigned long *) 0x40025400)) // Direction for Port F
#define GPIODEN_F   (* ((volatile unsigned long *) 0x4002551C)) // Digital enable for Port F
#define GPIODATA_F  (* ((volatile unsigned long *) 0x40025040)) // Data register for Port F

#define GPIODATA_E  (* ((volatile unsigned long *) 0x4002403C)) // Data register for Port E
#define GPIODIR_E   (* ((volatile unsigned long *) 0x40024400)) // Direction for Port E
#define GPIODEN_E   (* ((volatile unsigned long *) 0x4002451C)) // Digital enable for Port E

volatile int currPressed = 0;

void delay(int time) {
    while(time > 0) time--;
}

void UART_Tx(unsigned char data) {
    while ((UARTFR & 0x20) != 0);
    UARTDR = data;
}

// Timer 0A initialization
void timerInit() {
    RCGCTIMER = 0x1;   // Enable timer 0 clock
    GPTMCTL = 0;       // Disable timer 0A
    GPTMCFG = 0x4;     // Select 16-bit configuration
    GPTMATMR = 0x2;    // Set to periodic mode
    GPTMTAPR = 245;    // Prescalar = delay / maxDelay = 1s / 0.004096s = 244.1 = 245
    GPTMTAILR = 65308; // InitCnt = delay / (period * prescalar) = 1 / 0.0000153 = 65,308
    NVIC_EN0 |= 0x80000;// Enable interrupt for timer 0A
    GPTMIMR = 0x1;     // Unmask time-out interrupt for timer 0A
    GPTMICR = 0x1;     // Clear timer 0A raw output flag
    GPTMCTL = 0x1;     // Enable timer 0
}

// Handler for timer time-out
void timerExpiredHandler() {
    GPTMICR = 0x1;      // Clear timer 0A raw output flag
    // Invert state of LED
    if (GPIODATA_F == 0x10) GPIODATA_F &= ~0x10;
    else GPIODATA_F = 0x10;
    GPTMCTL = 0x1;      // Re-enable timer 0A
}

// Handler for GPIO Port A interrupt
void portAHandler() {
    if (!currPressed) {
        //NVIC_EN0 &= ~0x1;    // Disable interrupts for GPIO Port A
        currPressed = 1;
        UART_Tx('$');
        delay(9999);
        //NVIC_EN0 |= 0x1;    // Enable interrupt for GPIO Port A
    }
    currPressed = 0;
    GPIOICR_A = 0x40;
}

// UART0 initialization
void UARTInit() {
    RCGCUART = 0x1;     // Enable UART0 clock
    RCGCGPIO |= 0x1;    // Enable GPIO Port A clock
    GPIOAFSL_A |= 0x3;  // Enable alternate function select for PA0 and PA1
    GPIODEN_A |= 0x3;   // Enable PA0 and PA1
    GPIOPCTL_A |= 0x11; // Select UART functions for PA0 and PA1

    UARTCTL = 0;        // Disable UART0
    UARTIBRD = 8;       // Set integer baud rate
    UARTFBRD = 43;      // Set fractional baud rate
    UARTLCRH = 0x60;    // Set UART0 line control
    UARTCTL |= 0x301;   // Enable UART0
}


// Port E pin initialization (COLUMN OUTPUT)
void portEInit() {
    RCGCGPIO |= 0x10;   // Enable GPIO Port E clock
    GPIODIR_E = 0x0F;   // Set PE0-3 to output (drive to columns)
    GPIODEN_E = 0x0F;   // Enable digital function for PE0-3
}

// Port B pin initialization (ROW INPUT)
void portBInit() {
    RCGCGPIO |= 0x02;   // Enable GPIO Port B clock
    GPIODIR_B &= ~0xF0; // Set PB4-7 to input (read from rows)
    GPIODEN_B = 0xF0;   // Enable digital function for PB4-7
    GPIOPDR_B = 0xF0;   // Enable pull-down resistance for PB4-7
}

// LED (Port F pin 4) initialization
void portFInit() {
    RCGCGPIO |= 0x20;   // Enable GPIO Port F clock
    GPIODIR_F = 0x10;   // Set pin 4 direction to output
    GPIODEN_F = 0x10;   // Enable digital function for pin 2
}

// Port A (pushbutton interrupt) initialization
void portAInit() {
    // RCGCGPIO will already be set from UARTInit()
    GPIODEN_A |= 0x40;  // Enable digital function for pin 6
    GPIODIR_A &= ~0x40; // Set pin 6 direction to input
    GPIOPDR_A = 0x40;   // Enable pull-down resistance for PA6
    GPIOIM_A |= 0x40;   // Set interrupt mask for pin 6
    GPIOIEV_A |= 0x40;  // Interrupt on rising edges
    NVIC_EN0 |= 0x1;    // Enable interrupt for GPIO Port A
}

int main(void)
{
    portFInit();    // Initialize Port F pin
    UARTInit();     // Initialize UART
    timerInit();    // Initialize timer
    portBInit();    // Initialize Port B pins
    portEInit();    // Initialize Port E pins
    portAInit();    // Initialize Port A pins

    int onPush1 = 1, onPush2 = 1, onPush3 = 1, onPush4 = 1;

    while(1) {
        // State 1: C1 is 1
        GPIODATA_E = 0x8;      // 1000
        delay(9999);
        if (GPIODATA_B == 0x80 && onPush1) {UART_Tx('1'); onPush1 = 0;}
        else if (GPIODATA_B == 0x40 && onPush1) {UART_Tx('4'); onPush1 = 0;}
        else if (GPIODATA_B == 0x20 && onPush1) {UART_Tx('7'); onPush1 = 0;}
        else if (GPIODATA_B == 0x10 && onPush1) {UART_Tx('*'); onPush1 = 0;}
        else if (GPIODATA_B == 0x00 && !onPush1) onPush1 = 1;


        // State 2: C2 is 1
        GPIODATA_E = 0x4;      // 0100
        delay(9999);
        if (GPIODATA_B == 0x80 && onPush2) {UART_Tx('2'); onPush2 = 0;}
        else if (GPIODATA_B == 0x40 && onPush2) {UART_Tx('5'); onPush2 = 0;}
        else if (GPIODATA_B == 0x20 && onPush2) {UART_Tx('8'); onPush2 = 0;}
        else if (GPIODATA_B == 0x10 && onPush2) {UART_Tx('0'); onPush2 = 0;}
        else if (GPIODATA_B == 0x00 && !onPush2) onPush2 = 1;

        // State 3: C3 is 1
        GPIODATA_E = 0x2;      // 0010
        delay(9999);
        if (GPIODATA_B == 0x80 && onPush3) {UART_Tx('3'); onPush3 = 0;}
        else if (GPIODATA_B == 0x40 && onPush3) {UART_Tx('6'); onPush3 = 0;}
        else if (GPIODATA_B == 0x20 && onPush3) {UART_Tx('9'); onPush3 = 0;}
        else if (GPIODATA_B == 0x10 && onPush3) {UART_Tx('#'); onPush3 = 0;}
        else if (GPIODATA_B == 0x00 && !onPush3) onPush3 = 1;

        // State 4: C4 is 1
        GPIODATA_E = 0x1;      // 0001
        delay(9999);
        if (GPIODATA_B == 0x80 && onPush4) {UART_Tx('A'); onPush4 = 0;}
        else if (GPIODATA_B == 0x40 && onPush4) {UART_Tx('B'); onPush4 = 0;}
        else if (GPIODATA_B == 0x20 && onPush4) {UART_Tx('C'); onPush4 = 0;}
        else if (GPIODATA_B == 0x10 && onPush4) {UART_Tx('D'); onPush4 = 0;}
        else if (GPIODATA_B == 0x00 && !onPush4) onPush4 = 1;

    }
}
