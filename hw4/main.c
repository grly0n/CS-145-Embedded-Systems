

/**
 * main.c
 */

#define RCGCGPIO    (* ((volatile unsigned long *) 0x400FE608)) // Clock control for GPIO

#define GPIODATA_A  (* ((volatile unsigned long *) 0x400043C0)) // Data register for Port A (pins 4-7)
#define GPIODIR_A   (* ((volatile unsigned long *) 0x40004400)) // Direction for Port A
#define GPIODEN_A   (* ((volatile unsigned long *) 0x4000451C)) // Digital enable for Port A
#define GPIOPCTL_A  (* ((volatile unsigned long *) 0x4000452C)) // GPIO port control for Port A

#define GPIODATA_B  (* ((volatile unsigned long *) 0x400053FC)) // Data register for Port B (pins 0-7)
#define GPIODIR_B   (* ((volatile unsigned long *) 0x40005400)) // Direction for Port B
#define GPIODEN_B   (* ((volatile unsigned long *) 0x4000551C)) // Digital enable for Port B
#define GPIOPCTL_B  (* ((volatile unsigned long *) 0x4000552C)) // GPIO port control for Port B

#define GPIODATA_C  (* ((volatile unsigned long *) 0x400063FC)) // Data register for Port C (pins 4-7)
#define GPIODIR_C   (* ((volatile unsigned long *) 0x40006400)) // Direction for Port C
#define GPIODEN_C   (* ((volatile unsigned long *) 0x4000651C)) // Digital enable for Port C
#define GPIOPDR_C   (* ((volatile unsigned long *) 0x40006514)) // Pull down select for Port C

#define GPIODATA_D  (* ((volatile unsigned long *) 0x4000703C)) // Data register for Port D (pins 0-3)
#define GPIODIR_D   (* ((volatile unsigned long *) 0x40007400)) // Direction for Port D
#define GPIODEN_D   (* ((volatile unsigned long *) 0x4000751C)) // Digital enable for Port D

#define GPIODATA_E  (* ((volatile unsigned long *) 0x40024038)) // Data register for Port E (pins 1-3)
#define GPIODIR_E   (* ((volatile unsigned long *) 0x40024400)) // Direction for Port E
#define GPIODEN_E   (* ((volatile unsigned long *) 0x4002451C)) // Digital enable for Port E

volatile char currentState = 'I';
volatile int A, B;

void LCDInit();


void delay(int d) {
    while (d > 0) d--;
}


void delay_ms(int d) {
    volatile long count;
    while (d--) {
        for(count = 0; count < 3180; count++);
    }
}

// Port A pin initialization (Touch pad columns output)
void portAInit() {
    RCGCGPIO |= 0x1;    // Enable GPIO Port A clock
    GPIODIR_A |= 0xF0;  // Set PA4-7 to output
    GPIODEN_A |= 0xF0;  // Enable digital function for PA4-7
    GPIOPCTL_A = 0;      // Enable GPIO functionality for all pins
}


// Port B pin initialization (LCD data/information register)
void portBInit() {
    RCGCGPIO |= 0x2;   // Enable GPIO Port B clock
    GPIODIR_B |= 0xFF;  // Set PB0-7 to output
    GPIODEN_B |= 0xFF;   // Enable digital function for PB0-7
    GPIOPCTL_B = 0;   // Enable GPIO functionality for all pins
}


// Port C pin initialization (Touch pad rows input)
void portCInit() {
    RCGCGPIO |= 0x4;    // Enable GPIO Port C clock
    GPIODIR_C &= ~0xF0; // Set PC4-7 to input
    GPIODEN_C |= 0xF0;  // Enable digital function for PC4-7
    GPIOPDR_C |= 0xF0;  // Enable pull-down resistance for PC4-7
}


// Port D pin initialization (Touch pad columns output)
void portDInit() {
    RCGCGPIO |= 0x8;    // Enable GPIO Port D clock
    GPIODIR_D |= 0xF;   // Set PD0-3 to output
    GPIODEN_D |= 0xF;   // Enable digital function for PD0-3
}


// Port E pin initialization (LCD configuration)
void portEInit() {
    RCGCGPIO |= 0x10;   // Enable GPIO Port E clock
    GPIODIR_E |= 0xE;   // Set PE1-3 to output
    GPIODEN_E |= 0xE;   // Enable digital function for PE1-3
}


// Write a byte to the LCD instruction register
void WriteToIR(int c) {
    GPIODATA_B = 0;
    GPIODATA_E &= ~0x6;     // Set RS to instruction input, write mode (both low)
    delay_ms(1);
    GPIODATA_E |= 0x8;      // Set E to high
    GPIODATA_B = c;         // Read instruction into IR
    delay_ms(1);
    GPIODATA_E &= ~0x8;     // Set E to low
    delay_ms(3);
}


// Write a byte to the LCD display register
void WriteToDR(char c) {
    GPIODATA_B = 0;
    GPIODATA_E |= 0x2;      // Set RS to display input
    GPIODATA_E &= ~0x4;     // Set to write mode
    delay_ms(1);
    GPIODATA_E |= 0x8;      // Set E to high
    GPIODATA_B = c;         // Read data into DR
    delay_ms(1);
    GPIODATA_E &= ~0x8;     // Set E to low
    delay_ms(3);
}


// Converts product to a string
void ProductToString(long long prod, char *str) {
    long long temp = prod;
    int length = 0;
    if (prod == 0) {
        str[0] = '0';
        return;
    }
    while (temp != 0) {
        temp /= 10;
        length++;
    }
    while (prod > 0) {
        str[--length] = (prod % 10) + '0';
        prod /= 10;
    }
}


// Change state of system
void ChangeState(char to) {
    if (to == 'I') {        // To Initial
        LCDInit();
        A = 0; B = 0;
        currentState = 'A';
        //ChangeState('S');
    }
    else if (to == 'S') {   // To Start
        A = 0; B = 0;
        WriteToIR(0x80);
        int i = 0;
        for (; i < 8; i++) WriteToDR(' ');
        WriteToIR(0x80);
        currentState = 'A';
    }
    else if (to == 'B') {    // To B
        WriteToIR(0x80);
        int i = 0;
        for (; i < 8; i++) WriteToDR(' ');
        WriteToIR(0x80);
        currentState = 'B';
    } else if (to == 'D') {    // To Display
        WriteToIR(0xC0);
        char productString[16] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
        ProductToString(A*B, productString);
        int i = 0;
        for (; i < 16; i++) WriteToDR(productString[i]);
        ChangeState('S');
    }
}


// LCD initialization
void LCDInit() {
    WriteToIR(0x01);    // Clear display
    WriteToIR(0x38);    // Set function (8-bit mode, 2 lines, 5x8 dots per character)
    WriteToIR(0x6);     // Set entry mode (increment cursor, display shift off)
    WriteToIR(0xF);     // Set display (display on, cursor off, blink off)
}


int main(void)
{
    portAInit();
    portBInit();
    portCInit();
    portEInit();
    LCDInit();

    int onPush1 = 1, onPush2 = 1, onPush3 = 1, onPush4 = 1, digitsWritten = 0;
    currentState = 'A';

    while(1) {
        // State 1: C1 is 1
        GPIODATA_A = 0x80;      // 1000
        if (GPIODATA_C == 0x4D && onPush1) {             // 1
            if (digitsWritten < 8) {
                WriteToDR('1');
                if (currentState == 'A') A = (A*10) + 1;
                else B = (B*10) + 1;
                digitsWritten++;
            } else {
                digitsWritten = 0;
                if (currentState == 'A') ChangeState('B');
                else ChangeState('D');
            }
            onPush1 = 0;
        }
        else if (GPIODATA_C == 0x8D && onPush1) {        // 4
            if (digitsWritten < 8) {
                WriteToDR('4');
                if (currentState == 'A') A = (A*10) + 4;
                else B = (B*10) + 4;
                digitsWritten++;
            } else {
                digitsWritten = 0;
                if (currentState == 'A') ChangeState('B');
                else ChangeState('D');
            }
            onPush1 = 0;
        }
        else if (GPIODATA_C == 0x2D && onPush1) {        // 7
            if (digitsWritten < 8) {
                WriteToDR('7');
                if (currentState == 'A') A = (A*10) + 7;
                else B = (B*10) + 7;
                digitsWritten++;
            } else {
                digitsWritten = 0;
                if (currentState == 'A') ChangeState('B');
                else ChangeState('D');
            }
            onPush1 = 0;
        }
        else if (GPIODATA_C == 0x1D && onPush1) {        // *
            if (currentState == 'A') {digitsWritten = 0; ChangeState('B');}
            onPush1 = 0;
        }
        else if (GPIODATA_C == 0x0D && !onPush1) {onPush1 = 1;}


        // State 2: C2 is 1
        GPIODATA_A = 0x40;      // 0100
        if (GPIODATA_C == 0x4D && onPush2) {             // 2
            if (digitsWritten < 8) {
                WriteToDR('2');
                if (currentState == 'A') A = (A*10) + 2;
                else B = (B*10) + 2;
                digitsWritten++;
            } else {
                digitsWritten = 0;
                if (currentState == 'A') ChangeState('B');
                else ChangeState('D');
            }
            onPush2 = 0;
        }
        else if (GPIODATA_C == 0x8D && onPush2) {        // 5
            if (digitsWritten < 8) { // A state
                WriteToDR('5');
                if (currentState == 'A') A = (A*10) + 5;
                else B = (B*10) + 5;
                digitsWritten++;
            } else {
                digitsWritten = 0;
                if (currentState == 'A') ChangeState('B');
                else ChangeState('D');
            }
            onPush2 = 0;
        }
        else if (GPIODATA_C == 0x2D && onPush2) {        // 8
            if (digitsWritten < 8) { // A state
                WriteToDR('8');
                if (currentState == 'A') A = (A*10) + 8;
                else B = (B*10) + 8;
                digitsWritten++;
            } else {
                digitsWritten = 0;
                if (currentState == 'A') ChangeState('B');
                else ChangeState('D');
            }
            onPush2 = 0;
        }
        else if (GPIODATA_C == 0x1D && onPush2) {        // 0
            if (digitsWritten < 8) { // A state
                WriteToDR('0');
                if (currentState == 'A') A = (A*10);
                else B = (B*10);
                digitsWritten++;
            } else {
                digitsWritten = 0;
                if (currentState == 'A') ChangeState('B');
                else ChangeState('D');
            }
            onPush2 = 0;
        }
        else if (GPIODATA_C == 0x0D && !onPush2) onPush2 = 1;


        // State 1: C3 is 1
        GPIODATA_A = 0x20;      // 0010
        if (GPIODATA_C == 0x4D && onPush3) {             // 3
            if (digitsWritten < 8) { // A state
                WriteToDR('3');
                if (currentState == 'A') A = (A*10) + 3;
                else B = (B*10) + 3;
                digitsWritten++;
            } else {
                digitsWritten = 0;
                if (currentState == 'A') ChangeState('B');
                else ChangeState('D');
            }
            onPush3 = 0;
        }
        else if (GPIODATA_C == 0x8D && onPush3) {        // 6
            if (digitsWritten < 8) { // A state
                WriteToDR('6');
                if (currentState == 'A') A = (A*10) + 6;
                else B = (B*10) + 6;
                digitsWritten++;
            } else {
                digitsWritten = 0;
                if (currentState == 'A') ChangeState('B');
                else ChangeState('D');
            }
            onPush3 = 0;
        }
        else if (GPIODATA_C == 0x2D && onPush3) {        // 9
            if (digitsWritten < 8) { // A state
                WriteToDR('9');
                if (currentState == 'A') A = (A*10) + 9;
                else B = (B*10) + 9;
                digitsWritten++;
            } else {
                digitsWritten = 0;
                if (currentState == 'A') ChangeState('B');
                else ChangeState('D');
            }
            onPush3 = 0;
        }
        else if (GPIODATA_C == 0x1D && onPush3) {        // #
            digitsWritten = 0;
            ChangeState('D');
            onPush3 = 0;
        }
        else if (GPIODATA_C == 0x0D && !onPush3) onPush3 = 1;

        // State 1: C4 is 1
        GPIODATA_A = 0x10;      // 0001
        if (GPIODATA_C == 0x2D && onPush4) {             // C
            digitsWritten = 0;
            WriteToIR(1);
            currentState = 'A';
            onPush4 = 0;
        }
        else if (GPIODATA_C == 0x0D && !onPush4) onPush4 = 1;
    }
	return 0;
}
