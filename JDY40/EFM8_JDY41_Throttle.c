
#include <EFM8LB1.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SYSCLK 72000000
#define BAUDRATE 115200L

idata char buff[20];
volatile unsigned int pwm_reload;
volatile unsigned char pwm_state = 0;
volatile unsigned char count20ms;
#define ESCOUT P1_7
#define ARMINGOUT P1_5

#define RELOAD_10MS (0x10000L-(SYSCLK/(12L*100L)))

char _c51_external_startup (void)
{
	// Disable Watchdog with key sequence
	SFRPAGE = 0x00;
	WDTCN = 0xDE; //First key
	WDTCN = 0xAD; //Second key
  
	VDM0CN=0x80;       // enable VDD monitor
	RSTSRC=0x02|0x04;  // Enable reset on missing clock detector and VDD

	#if (SYSCLK == 48000000L)	
		SFRPAGE = 0x10;
		PFE0CN  = 0x10; // SYSCLK < 50 MHz.
		SFRPAGE = 0x00;
	#elif (SYSCLK == 72000000L)
		SFRPAGE = 0x10;
		PFE0CN  = 0x20; // SYSCLK < 75 MHz.
		SFRPAGE = 0x00;
	#endif
	
	#if (SYSCLK == 12250000L)
		CLKSEL = 0x10;
		CLKSEL = 0x10;
		while ((CLKSEL & 0x80) == 0);
	#elif (SYSCLK == 24500000L)
		CLKSEL = 0x00;
		CLKSEL = 0x00;
		while ((CLKSEL & 0x80) == 0);
	#elif (SYSCLK == 48000000L)	
		// Before setting clock to 48 MHz, must transition to 24.5 MHz first
		CLKSEL = 0x00;
		CLKSEL = 0x00;
		while ((CLKSEL & 0x80) == 0);
		CLKSEL = 0x07;
		CLKSEL = 0x07;
		while ((CLKSEL & 0x80) == 0);
	#elif (SYSCLK == 72000000L)
		// Before setting clock to 72 MHz, must transition to 24.5 MHz first
		CLKSEL = 0x00;
		CLKSEL = 0x00;
		while ((CLKSEL & 0x80) == 0);
		CLKSEL = 0x03;
		CLKSEL = 0x03;
		while ((CLKSEL & 0x80) == 0);
	#else
		#error SYSCLK must be either 12250000L, 24500000L, 48000000L, or 72000000L
	#endif
	
	P0MDOUT |= 0x11; // Enable UART0 TX (P0.4) and UART1 TX (P0.0) as push-pull outputs
	P2MDOUT |= 0x01; // P2.0 in push-pull mode
	XBR0     = 0x01; // Enable UART0 on P0.4(TX) and P0.5(RX)                     
	XBR1     = 0X00;
	XBR2     = 0x41; // Enable crossbar and uart 1

	// Configure Uart 0
	#if (((SYSCLK/BAUDRATE)/(2L*12L))>0xFFL)
		#error Timer 0 reload value is incorrect because (SYSCLK/BAUDRATE)/(2L*12L) > 0xFF
	#endif
	SCON0 = 0x10;
	TH1 = 0x100-((SYSCLK/BAUDRATE)/(2L*12L));
	TL1 = TH1;      // Init Timer1
	TMOD &= ~0xf0;  // TMOD: timer 1 in 8-bit auto-reload
	TMOD |=  0x20;                       
	TR1 = 1; // START Timer1
	TI = 1;  // Indicate TX0 ready

  	// Initialize timer 5 for periodic interrupts
	SFRPAGE=0x10;
	TMR5CN0=0x00;
	pwm_reload=0x10000L-(SYSCLK*1.5e-3)/12.0; // 1.5 miliseconds pulse is the center of the servo
	TMR5=0xffff;   // Set to reload immediately
	EIE2|=0b_0000_1000; // Enable Timer5 interrupts
	TR5=1;         // Start Timer5 (TMR5CN0 is bit addressable)
	
	EA=1;
	
	SFRPAGE=0x00;

	return 0;
}

// using timer 5 to control ESC
void Timer5_ISR (void) interrupt INTERRUPT_TIMER5
{
	SFRPAGE=0x10;
	TF5H = 0; // Clear Timer5 interrupt flag
	// Since the maximum time we can achieve with this timer in the
	// configuration above is about 10ms, implement a simple state
	// machine to produce the required 20ms period.
	switch (pwm_state)
	{
		// case 10:
		// 	//in this case the PWM is turned off
		// 	ESCOUT = 0;
			
		case 0:
			ESCOUT=1;
			TMR5RL=RELOAD_10MS;
			pwm_state=1;
			count20ms++;
		break;

		case 1:
			ESCOUT=0;
			TMR5RL=RELOAD_10MS-pwm_reload;
			pwm_state=2;
		break;

		default:
			ESCOUT=0;
			TMR5RL=pwm_reload;
			pwm_state=0;
		break;
	}
}

// Uses Timer3 to delay <us> micro-seconds. 
void Timer3us(unsigned char us)
{
	unsigned char i;               // usec counter
	
	// The input for Timer 3 is selected as SYSCLK by setting T3ML (bit 6) of CKCON0:
	CKCON0|=0b_0100_0000;
	
	TMR3RL = (-(SYSCLK)/1000000L); // Set Timer3 to overflow in 1us.
	TMR3 = TMR3RL;                 // Initialize Timer3 for first overflow
	
	TMR3CN0 = 0x04;                 // Sart Timer3 and clear overflow flag
	for (i = 0; i < us; i++)       // Count <us> overflows
	{
		while (!(TMR3CN0 & 0x80));  // Wait for overflow
		TMR3CN0 &= ~(0x80);         // Clear overflow indicator
	}
	TMR3CN0 = 0 ;                   // Stop Timer3 and clear overflow flag
}

void waitms (unsigned int ms)
{
	unsigned int j;
	unsigned char k;
	for(j=0; j<ms; j++)
		for (k=0; k<4; k++) Timer3us(250);
}

void UART1_Init (unsigned long baudrate)
{
    SFRPAGE = 0x20;
	SMOD1 = 0x0C; // no parity, 8 data bits, 1 stop bit
	SCON1 = 0x10;
	SBCON1 =0x00;   // disable baud rate generator
	SBRL1 = 0x10000L-((SYSCLK/baudrate)/(12L*2L));
	TI1 = 1; // indicate ready for TX
	SBCON1 |= 0x40;   // enable baud rate generator
	SFRPAGE = 0x00;
}



void main (void)
{
	float fThrottle = 0;
	int i;
	int j;
	int k;

	//PWM variables
	float potentiometerReading;

	float motor_PWM_DutyCycleWidth = 1;

	// this variable is only for testing purposes
	int motor_on = 0;

	count20ms=0; // Count20ms is an atomic variable, so no problem sharing with timer 5 ISR
	waitms(500);

	UART1_Init(9600);
	
	// initialllzing throttle duty cycke
	pwm_reload=0x10000L-(SYSCLK*motor_PWM_DutyCycleWidth*1.0e-3)/12.0;
	ARMINGOUT = 0;

	while(1)
	{
		// determing if the motor needs to be turned on
		if(motor_on){
		//if motor needs to be turned on change the duty cycle to 1.3ms
			motor_PWM_DutyCycleWidth = 2.0 - (potentiometerReading-1.0)/4.0;
		
		}else if(!motor_on){
		// otherwise writing 1ms to PWM pulse width for 0% throttle
			motor_PWM_DutyCycleWidth = 1;
		}

		// writing to the motor throttle
		pwm_reload=0x10000L-(SYSCLK*motor_PWM_DutyCycleWidth*1.0e-3)/12.0;
		
		// determining if the BLDC needs to be turned on
		// for testing purposes, the motor will alsways run on min rpm
		// with a pulse width on 1ms

		// this following code is for testing, if the 
		if(P0_4 == 0 && !motor_on){
			// button debounce
			while(P0_4 == 0);
			motor_on = 1;
			ARMINGOUT = 1;
			printf("Motor on\n");

		}else if(P0_4 == 0 && motor_on){
			// button debounce
			while(P0_4 == 0);
			motor_on = 0;
			ARMINGOUT = 0;
			printf("Motor off\n");
		}

		waitms(20);
	}
}
