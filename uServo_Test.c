//  EFM8_Servo.c: Uses timer 5 interrupt to generate a servo motor control signal.
//
//  Copyright (c) 2010-2018 Jesus Calvino-Fraga
//  ~C51~

#include <EFM8LB1.h>
#include <stdlib.h>
#include <stdio.h>

volatile unsigned int servo_counter=0;
volatile unsigned char servo1=150, servo2=150;

#define COLPITTS P1_5
#define SERVO1   P1_3
#define ESCOUT P1_7
#define EMAGNET  P1_6

volatile unsigned int pwm_reload;
volatile unsigned int pwm_motor_reload;
volatile unsigned char pwm_state=0;
volatile unsigned char pwm_state1=0;
volatile unsigned char count20ms;

#define SYSCLK 72000000L // SYSCLK frequency in Hz
#define BAUDRATE 115200L
#define RELOAD_10us (0x10000L-(SYSCLK/(12L*100000L))) // 10us rate
#define RELOAD_10MS (0x10000L-(SYSCLK/(12L*100L)))

char _c51_external_startup (void)
{
	// Disable Watchdog with key sequence
	SFRPAGE = 0x00;
	WDTCN = 0xDE; //First key
	WDTCN = 0xAD; //Second key
  
	VDM0CN |= 0x80;
	RSTSRC = 0x02;

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
	
	// Configure the pins used for square output
	P2MDOUT|=0b_0000_0011;
	P0MDOUT |= 0x10; // Enable UART0 TX as push-pull output
	XBR0     = 0x01; // Enable UART0 on P0.4(TX) and P0.5(RX)                     
	XBR1     = 0X10; // Enable T0 on P0.0
	XBR2     = 0x40; // Enable crossbar and weak pull-ups

	#if (((SYSCLK/BAUDRATE)/(2L*12L))>0xFFL)
		#error Timer 0 reload value is incorrect because (SYSCLK/BAUDRATE)/(2L*12L) > 0xFF
	#endif
	// Configure Uart 0
	SCON0 = 0x10;
	CKCON0 |= 0b_0000_0000 ; // Timer 1 uses the system clock divided by 12.
	TH1 = 0x100-((SYSCLK/BAUDRATE)/(2L*12L));
	TL1 = TH1;      // Init Timer1
	TMOD &= ~0xf0;  // TMOD: timer 1 in 8-bit auto-reload
	TMOD |=  0x20;                       
	TR1 = 1; // START Timer1
	TI = 1;  // Indicate TX0 ready

	// Initialize timer 4 for periodic interrupts
	SFRPAGE=0x10;
	TMR4CN0=0x00;   // Stop Timer4; Clear TF4; WARNING: lives in SFR page 0x10
	//CKCON1|=0b_0000_0001; // Timer 4 uses the system clock
	//TMR4RL=(0x10000L-(SYSCLK/(2*TIMER_4_FREQ))); // Initialize reload value
	pwm_motor_reload=0x10000L-(SYSCLK*1.0e-3)/12.0;
	TMR4=0xffff;   // Set to reload immediately
	EIE2|=0b_0000_0100;     // Enable Timer4 interrupts
	TR4=1;
	
	EA=1; 
	
	SFRPAGE=0x00;
	
	return 0;
}

void Timer4_ISR (void) interrupt INTERRUPT_TIMER4
{
	SFRPAGE=0x10;
	TF4H = 0; // Clear Timer4 interrupt flag
	
	switch (pwm_state1)
	{
	   case 0:
	      ESCOUT=1;
	      TMR4RL=RELOAD_10MS;
	      pwm_state1=1;
	      count20ms++;
	   break;
	   case 1:
	      ESCOUT=0;
	      TMR4RL=RELOAD_10MS-pwm_motor_reload;
	      pwm_state1=2;
	   break;
	   default:
	      ESCOUT=0;
	      TMR4RL=pwm_motor_reload;
	      pwm_state1=0;
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
	for(j=ms; j!=0; j--)
	{
		Timer3us(249);
		Timer3us(249);
		Timer3us(249);
		Timer3us(250);
	}
}

void main (void)
{
	unsigned char j, k;
    
    EMAGNET=0; // turn off electromagnet (if present!)
    
    waitms(1000); // Wait a second to give PuTTy a chance to start
    
	printf("\x1b[2J\x1b[1;1H"); // Clear screen using ANSI escape sequence.

	printf ("Micro servo tester.\n"
	        "File: %s\n"
	        "Compiled: %s, %s\n\n",
	        __FILE__, __DATE__, __TIME__);
	
	count20ms=0; // Count20ms is an atomic variable, so no problem sharing with timer 5 ISR
    while((1000/20)>count20ms); // Wait a second to give PuTTy a chance to start
		
	while(1) {
		pwm_motor_reload=0x10000L-(SYSCLK*1.5*1.0e-3)/12.0;
		waitms(20);
	}

}
