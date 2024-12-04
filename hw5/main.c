#define RCC         (*((volatile unsigned int *) 0x400FE060))   // Run-Mode clock configuration
#define RCGCPWM     (*((volatile unsigned int *) 0x400FE640))   // PWM clock
#define RCGCGPIO    (*((volatile unsigned int *) 0x400FE608))   // GPIO clock
#define RCGCADC     (*((volatile unsigned int *) 0x400FE638))   // ADC clock

#define GPIOD_AFSEL (*((volatile unsigned int *) 0x40007420))   // Port D alternate function select
#define GPIOD_DIR   (*((volatile unsigned int *) 0x40007400))   // Port D direction
#define GPIOD_PCTL  (*((volatile unsigned int *) 0x4000752C))   // Port D port control
#define GPIOD_DEN   (*((volatile unsigned int *) 0x4000751C))   // Port D digital enable

#define GPIOE_AFSEL (*((volatile unsigned int *) 0x40024420))   // Port E alternate function select
#define GPIOE_AMSEL (*((volatile unsigned int *) 0x40024528))   // Port E analog mode select
#define GPIOE_DEN   (*((volatile unsigned int *) 0x4002451C))   // Port E digital enable

#define PWM1_CTL    (*((volatile unsigned int *) 0x40029040))   // PWM module 1, generator 0 control
#define PWM1_GENA   (*((volatile unsigned int *) 0x40029060))   // PWM module 1, generator 0 A control
#define PWM1_LOAD   (*((volatile unsigned int *) 0x40029050))   // PWM module 1, generator 0 load
#define PWM1_CMPA   (*((volatile unsigned int *) 0x40029058))   // PWM module 1, generator 0 compare A
#define PWM1_ENABLE (*((volatile unsigned int *) 0x40029008))   // PWM module 1 output enable

#define ADCACTSS    (*((volatile unsigned int *) 0x40038000))   // ADC0 Active Sample Sequencer
#define ADCEMUX     (*((volatile unsigned int *) 0x40038014))   // ADC0 Event Multiplexer Select
#define ADCSSMUX3   (*((volatile unsigned int *) 0x400380A0))   // ADC0 Sample Sequencer Input Multiplexer Select 3
#define ADCRIS      (*((volatile unsigned int *) 0x40038004))   // ADC0 Raw Interrupt Status
#define ADCISC      (*((volatile unsigned int *) 0x4003800C))   // ADC0 Interrupt Status and Clear
#define ADCSSFIFO3  (*((volatile unsigned int *) 0x400380A8))   // ADC0 Sample Sequence Result FIFO 3
#define ADCPSSI     (*((volatile unsigned int *) 0x40038028))   // ADC0 Sample Sequence Initiate
#define ADCSSCTL3   (*((volatile unsigned int *) 0x400380A4))   // ADC0 Sample Sequence Control 3




void delay(int ms) {
    int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 3180; j++)
        {}
    }
}


void PWMInit() {
    RCGCPWM |= 0x2;     // Enable system clock to PWM module 1
    RCC |= 0x8000;      // Enable system clock divisor function
    PWM1_CTL &= ~0x1;   // Disable generator
    PWM1_CTL &= ~0x2;   // Select count down mode
    PWM1_GENA |= 0x8C;  // Set PWM output when counter reloaded, clear when matches CMPA
    PWM1_LOAD = 1750;
    PWM1_CMPA = 1750-1;
}


void GPIODInit() {
    RCGCGPIO |= 0x8;    // Enable system clock to Port D
    GPIOD_AFSEL |= 0x1; // Enable alternate function select for PD0
    GPIOD_PCTL |= 0x5;  // Select M1PWM0 function for PD0
    GPIOD_DIR |= 0x1;   // Set PD0 to output
    GPIOD_DEN |= 0x1;   // Enable PD0
}


void GPIOEInit() {
    RCGCGPIO |= 0x10;   // Enable system clock to Port E
    GPIOE_AFSEL |= 0x8; // Enable alternate function select for PE3
    GPIOE_DEN &= ~0x8;  // Disable digital function for PE3
    GPIOE_AMSEL |= 0x8; // Enable analog function for PE3
}

void PWMEnable() {
    PWM1_CTL = 1;       // Enable generator
    PWM1_ENABLE = 1;    // Enable PWM module 1, generator 0A output
}


void ADCInit() {
    RCGCADC |= 1;       // Enable ADC module 0
    delay(10);
    ADCACTSS &= ~8;     // Disable SS3 while configuring
    ADCEMUX &= ~0xF000; // Set SS3 trigger source to processor
    ADCSSMUX3 &= ~0xF;  // Get input from AIN0 to SS3
    ADCSSCTL3 |= 0x6;   // RIS at end of conversion, end of sequence
    ADCACTSS |= 8;      // Enable SS3
}


int calculateDelay(unsigned int val) {
    if (val < 410) return 1;
    else if (val < 820) return 2;
    else if (val < 1230) return 3;
    else if (val < 1640) return 4;
    else if (val < 2050) return 5;
    else if (val < 2460) return 6;
    else if (val < 2870) return 7;
    else if (val < 3280) return 8;
    else if (val < 3690) return 9;
    else return 10;

}


void main() {
    PWMInit();      // Initialize PWM module 1 generator 0
    GPIODInit();    // Initialize PD0 with alternate function M1PWM0
    GPIOEInit();    // Initialize PE3 with analog function AIN0
    PWMEnable();    // Enable PWM timer
    ADCInit();      // Initialize and enable ADC module 0 using SS3

    int duty_cycle = 1749, increasing = 1, delay_value = 10; // 1 <= delay_value <= 10, to control pulse speed
    unsigned int adc_value = 0;

    while (1) {
        // Sample analog input from PE3
        ADCPSSI |= 0x8;                 // Start sampling from AIN0
        while ((ADCRIS & 0x8) == 0);    // Wait until sample conversion is done
        adc_value = ADCSSFIFO3;         // Read ADC conversion from FIFO 3
        ADCISC = 0x8;                   // Clear conversion flag bit
        delay_value = calculateDelay(adc_value);

        // Increase / decrease duty cycle to LED on PD0
        if (duty_cycle >= 1749) increasing = 1;
        else if (duty_cycle <= 0) increasing = 0;

        if (duty_cycle > 0 && increasing) duty_cycle -= 10;
        else if (duty_cycle <= 1749 && !increasing) duty_cycle += 10;

        PWM1_CMPA = duty_cycle;
        delay(delay_value);
    }




}
