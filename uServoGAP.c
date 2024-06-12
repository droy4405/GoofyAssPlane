//  all_timers.c: Uses timer 0 to 5 overflow interrupts to generate a square
//  wave at pins P2.0, P1_7, P1_6, P1_5, P1_4, and P1_3.  Also use the PCA with interrupts to
//  generate square signals at pins P1.2, P1.1, P1.0, P0.7, and P0.6
//
//  Copyright (c) 2010-2018 Jesus Calvino-Fraga
//
//  ~C51~

#include <EFM8LB1.h>
#include <stdlib.h>
#include <stdio.h>

#define SYSCLK 72000000L // SYSCLK frequency in Hz
#define BAUDRATE 115200L
#define TIMER_2_FREQ 3000L
#define RUDDEROUT   P2_6

#define RELOAD_10us (0x10000L-(SYSCLK/(12L*100000L))) // 10us rate

// #define ELEVSERVOL P1_3 //elevator left
// #define ELEVSERVOR P1_4 //elevator right servo

volatile unsigned int servo_counter=0;
volatile unsigned int aileron_counter=0;
volatile unsigned char aileron_L=150, aileron_R=150;
volatile unsigned int rudder_counter=0;
volatile unsigned int rudder=150;


idata char buff[22];
volatile unsigned int pwm_reload;

volatile unsigned char elevator_counter=0;
volatile unsigned char elevL_pwm_Dcycle = 150;
volatile unsigned char elevR_pwm_Dcycle = 150;

volatile unsigned char pwm_state = 0;
volatile unsigned char count20ms;

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
	//P1MDOUT |= 0x01;
	P2MDOUT |= 0b_0111_1101; // P2.0 in push-pull mode
	
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

	// Initialize timer 2 for periodic interrupts
	TMR2CN0=0x00;   // Stop Timer2; Clear TF2;
	CKCON0|=0b_0001_0000; // Timer 2 uses the system clock
	TMR2RL=RELOAD_10us; // Initialize reload value
	TMR2=0xffff;   // Set to reload immediately
	ET2=1;         // Enable Timer2 interrupts
	TR2=1;         // Start Timer2 (TMR2CN is bit addressable)
	
	EA=1;
	
	SFRPAGE=0x00;
	return 0;
}

void Timer2_ISR (void) interrupt INTERRUPT_TIMER2
{
	
	SFRPAGE=0x0;
	TF2H = 0; // Clear Timer2 interrupt flag
	TMR2RL=RELOAD_10us;

	rudder_counter++;

	if(rudder_counter==2000)
	{
		rudder_counter=0;
	}
	if(rudder>=rudder_counter)
	{
		RUDDEROUT=1;
	}
	else
	{
		RUDDEROUT=0;
	}
}



void main (void)
{
    unsigned int j;
	while(1)
	{
	    printf("testing\n");
		rudder = 150;
	}
}
