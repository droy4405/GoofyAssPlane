
//  EFM8_Servo.c: Uses timer 5 interrupt to generate a servo motor control signal.
//
//  Copyright (c) 2010-2018 Jesus Calvino-Fraga
//  ~C51~

#include <EFM8LB1.h>
#include <stdlib.h>
#include <stdio.h>

volatile unsigned int pwm_reload;
volatile unsigned int pwm_motor_reload;
volatile unsigned char pwm_state=0;
volatile unsigned char pwm_state1=0;
volatile unsigned char count20ms;

#define ESCOUT P1_7
#define ARMINGOUT P1_5
#define PWMOUT P2_0

#define SYSCLK 72000000L // SYSCLK frequency in Hz
#define BAUDRATE 115200L
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
	
	// Initialize timer 5 for periodic interrupts
	SFRPAGE=0x10;
	TMR5CN0=0x00;
	pwm_reload=0x10000L-(SYSCLK*1.0e-3)/12.0; // 1.5 miliseconds pulse is the center of the servo
	TMR5=0xffff;   // Set to reload immediately
	EIE2|=0b_0000_1000; // Enable Timer5 interrupts
	TR5=1;         // Start Timer5 (TMR5CN0 is bit addressable)
	
	EA=1;
	
	SFRPAGE=0x00;
	
	return 0;
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

void Timer5_ISR (void) interrupt INTERRUPT_TIMER5
{
	SFRPAGE=0x10;
	TF5H = 0; // Clear Timer5 interrupt flag
	// Since the maximum time we can achieve with this timer in the
	// configuration above is about 10ms, implement a simple state
	// machine to produce the required 20ms period.
	switch (pwm_state)
	{
	   case 0:
	      PWMOUT=1;
	      TMR5RL=RELOAD_10MS;
	      pwm_state=1;
	      count20ms++;
	   break;
	   case 1:
	      PWMOUT=0;
	      TMR5RL=RELOAD_10MS-pwm_reload;
	      pwm_state=2;
	   break;
	   default:
	      PWMOUT=0;
	      TMR5RL=pwm_reload;
	      pwm_state=0;
	   break;
	}
}

void main (void)
{
	int throttle_level2 = 0;
	int throttle_level1 = 0;
	int throttle_level0 = 0;

	float engine_Dcycle = 1.0;

	int engine_armed = 0;
	ARMINGOUT = 1;
	
    count20ms=0; // Count20ms is an atomic variable, so no problem sharing with timer 5 ISR
    while((1000/20)>count20ms); // Wait a second to give PuTTy a chance to start
    
	pwm_reload=0x10000L-(SYSCLK*1.0e-3)/12.0;
	pwm_motor_reload=0x10000L-(SYSCLK*1.0e-3)/12.0;

    // In a HS-422 servo a pulse width between 0.6 to 2.4 ms gives about 180 deg
    // of rotation range.
	while(1)
	{
		if(P3_7 == 1){
			// writing to the parachute pwm
			pwm_reload=0x10000L-(SYSCLK*2.0*1.0e-3)/12.0;
			pwm_motor_reload=0x10000L-(SYSCLK*1.0*1.0e-3)/12.0;
			engine_armed = 0;
			ARMINGOUT = 1;
		}else{
			// this following code is for testing, if the 
			if(P0_4 == 0 && !engine_armed){
				// button debounce
				while(P0_4 == 0);
				engine_armed = 1;
				ARMINGOUT = 0;
				//printf("Motor on\n");

			}else if(P0_4 == 0 && engine_armed){
				// button debounce
				while(P0_4 == 0);
				engine_armed = 0;
				ARMINGOUT = 1;
				//printf("Motor off\n");
			}

			// read the throttle signal from the RC slave
			throttle_level2 = P3_2;
			throttle_level1 = P3_1;
			throttle_level0 = P3_0;

			//printf("%d %d %d\n", throttle_level2, throttle_level1, throttle_level0);

			// changing the throttle level depending on the three bits
			// 000 - 0%
			// 001 - 17%
			// 010 - 33%
			// 011 - 50%
			// 100 - 67%
			// 101 - 83%
			// 110 - 100%
			
			if(throttle_level2){
				if(throttle_level1){
					// this is the case for 100% power
					engine_Dcycle = 2.0;
				}else{
					if(throttle_level0){
						// this is the case for 83% power
						engine_Dcycle = 1.8333;
					}else{
						// this is the case for 67% power
						engine_Dcycle = 1.6667;
					}
				}

			}else{
				if(throttle_level1){
					if(throttle_level0){
						// this is the case for 50% power
						engine_Dcycle = 1.5;
					}else{
						// this is the case for 33% power
						engine_Dcycle = 1.3333;
					}

				}else{
					if(throttle_level0){
						// this is the case for 17% power
						engine_Dcycle = 1.1667;
					}else{
						// this is the case for 0% power
						engine_Dcycle = 1.0;
					}
				}
			}

			// determine if the engine is armed and then assign duty cycle
			if(engine_armed){
				pwm_motor_reload=0x10000L-(SYSCLK*engine_Dcycle*1.0e-3)/12.0;
			}
			
			pwm_reload=0x10000L-(SYSCLK*1.0e-3)/12.0;

		}

		
		waitms(10);
	}
}
