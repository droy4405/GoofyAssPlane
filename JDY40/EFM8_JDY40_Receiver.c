// #include <EFM8LB1.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>

// #define SYSCLK 72000000
// #define BAUDRATE 115200L

// idata char buff[20];
// volatile unsigned int pwm_reload;
// volatile unsigned char pwm_state = 0;
// volatile unsigned char count20ms;
// #define ESCOUT P1_7

// #define RELOAD_10MS (0x10000L-(SYSCLK/(12L*100L)))

// char _c51_external_startup (void)
// {
// 	// Disable Watchdog with key sequence
// 	SFRPAGE = 0x00;
// 	WDTCN = 0xDE; //First key
// 	WDTCN = 0xAD; //Second key
  
// 	VDM0CN=0x80;       // enable VDD monitor
// 	RSTSRC=0x02|0x04;  // Enable reset on missing clock detector and VDD

// 	#if (SYSCLK == 48000000L)	
// 		SFRPAGE = 0x10;
// 		PFE0CN  = 0x10; // SYSCLK < 50 MHz.
// 		SFRPAGE = 0x00;
// 	#elif (SYSCLK == 72000000L)
// 		SFRPAGE = 0x10;
// 		PFE0CN  = 0x20; // SYSCLK < 75 MHz.
// 		SFRPAGE = 0x00;
// 	#endif
	
// 	#if (SYSCLK == 12250000L)
// 		CLKSEL = 0x10;
// 		CLKSEL = 0x10;
// 		while ((CLKSEL & 0x80) == 0);
// 	#elif (SYSCLK == 24500000L)
// 		CLKSEL = 0x00;
// 		CLKSEL = 0x00;
// 		while ((CLKSEL & 0x80) == 0);
// 	#elif (SYSCLK == 48000000L)	
// 		// Before setting clock to 48 MHz, must transition to 24.5 MHz first
// 		CLKSEL = 0x00;
// 		CLKSEL = 0x00;
// 		while ((CLKSEL & 0x80) == 0);
// 		CLKSEL = 0x07;
// 		CLKSEL = 0x07;
// 		while ((CLKSEL & 0x80) == 0);
// 	#elif (SYSCLK == 72000000L)
// 		// Before setting clock to 72 MHz, must transition to 24.5 MHz first
// 		CLKSEL = 0x00;
// 		CLKSEL = 0x00;
// 		while ((CLKSEL & 0x80) == 0);
// 		CLKSEL = 0x03;
// 		CLKSEL = 0x03;
// 		while ((CLKSEL & 0x80) == 0);
// 	#else
// 		#error SYSCLK must be either 12250000L, 24500000L, 48000000L, or 72000000L
// 	#endif
	
// 	P0MDOUT |= 0x11; // Enable UART0 TX (P0.4) and UART1 TX (P0.0) as push-pull outputs
// 	P2MDOUT |= 0x01; // P2.0 in push-pull mode
// 	XBR0     = 0x01; // Enable UART0 on P0.4(TX) and P0.5(RX)                     
// 	XBR1     = 0X00;
// 	XBR2     = 0x41; // Enable crossbar and uart 1

// 	// Configure Uart 0
// 	#if (((SYSCLK/BAUDRATE)/(2L*12L))>0xFFL)
// 		#error Timer 0 reload value is incorrect because (SYSCLK/BAUDRATE)/(2L*12L) > 0xFF
// 	#endif
// 	SCON0 = 0x10;
// 	TH1 = 0x100-((SYSCLK/BAUDRATE)/(2L*12L));
// 	TL1 = TH1;      // Init Timer1
// 	TMOD &= ~0xf0;  // TMOD: timer 1 in 8-bit auto-reload
// 	TMOD |=  0x20;                       
// 	TR1 = 1; // START Timer1
// 	TI = 1;  // Indicate TX0 ready

//   	// Initialize timer 5 for periodic interrupts
// 	SFRPAGE=0x10;
// 	TMR5CN0=0x00;
// 	pwm_reload=0x10000L-(SYSCLK*1.5e-3)/12.0; // 1.5 miliseconds pulse is the center of the servo
// 	TMR5=0xffff;   // Set to reload immediately
// 	EIE2|=0b_0000_1000; // Enable Timer5 interrupts
// 	TR5=1;         // Start Timer5 (TMR5CN0 is bit addressable)
	
// 	EA=1;
	
// 	SFRPAGE=0x00;

// 	return 0;
// }

// // using timer 5 to control ESC
// void Timer5_ISR (void) interrupt INTERRUPT_TIMER5
// {
// 	SFRPAGE=0x10;
// 	TF5H = 0; // Clear Timer5 interrupt flag
// 	// Since the maximum time we can achieve with this timer in the
// 	// configuration above is about 10ms, implement a simple state
// 	// machine to produce the required 20ms period.
// 	switch (pwm_state)
// 	{
// 		// case 10:
// 		// 	//in this case the PWM is turned off
// 		// 	ESCOUT = 0;
			
// 		case 0:
// 			ESCOUT=1;
// 			TMR5RL=RELOAD_10MS;
// 			pwm_state=1;
// 			count20ms++;
// 		break;

// 		case 1:
// 			ESCOUT=0;
// 			TMR5RL=RELOAD_10MS-pwm_reload;
// 			pwm_state=2;
// 		break;

// 		default:
// 			ESCOUT=0;
// 			TMR5RL=pwm_reload;
// 			pwm_state=0;
// 		break;
// 	}
// }

// // Uses Timer3 to delay <us> micro-seconds. 
// void Timer3us(unsigned char us)
// {
// 	unsigned char i;               // usec counter
	
// 	// The input for Timer 3 is selected as SYSCLK by setting T3ML (bit 6) of CKCON0:
// 	CKCON0|=0b_0100_0000;
	
// 	TMR3RL = (-(SYSCLK)/1000000L); // Set Timer3 to overflow in 1us.
// 	TMR3 = TMR3RL;                 // Initialize Timer3 for first overflow
	
// 	TMR3CN0 = 0x04;                 // Sart Timer3 and clear overflow flag
// 	for (i = 0; i < us; i++)       // Count <us> overflows
// 	{
// 		while (!(TMR3CN0 & 0x80));  // Wait for overflow
// 		TMR3CN0 &= ~(0x80);         // Clear overflow indicator
// 	}
// 	TMR3CN0 = 0 ;                   // Stop Timer3 and clear overflow flag
// }

// void waitms (unsigned int ms)
// {
// 	unsigned int j;
// 	unsigned char k;
// 	for(j=0; j<ms; j++)
// 		for (k=0; k<4; k++) Timer3us(250);
// }

// void UART1_Init (unsigned long baudrate)
// {
//     SFRPAGE = 0x20;
// 	SMOD1 = 0x0C; // no parity, 8 data bits, 1 stop bit
// 	SCON1 = 0x10;
// 	SBCON1 =0x00;   // disable baud rate generator
// 	SBRL1 = 0x10000L-((SYSCLK/baudrate)/(12L*2L));
// 	TI1 = 1; // indicate ready for TX
// 	SBCON1 |= 0x40;   // enable baud rate generator
// 	SFRPAGE = 0x00;
// }

// void putchar1 (char c) 
// {
//     SFRPAGE = 0x20;
// 	while (!TI1);
// 	TI1=0;
// 	SBUF1 = c;
// 	SFRPAGE = 0x00;
// }

// void sendstr1 (char * s)
// {
// 	while(*s)
// 	{
// 		putchar1(*s);
// 		s++;	
// 	}
// }

// char getchar1 (void)
// {
// 	char c;
//     SFRPAGE = 0x20;
// 	while (!RI1);
// 	RI1=0;
// 	// Clear Overrun and Parity error flags 
// 	SCON1&=0b_0011_1111;
// 	c = SBUF1;
// 	SFRPAGE = 0x00;
// 	return (c);
// }

// char getchar1_with_timeout (void)
// {
// 	char c;
// 	unsigned int timeout;
//     SFRPAGE = 0x20;
//     timeout=0;
// 	while (!RI1)
// 	{
// 		SFRPAGE = 0x00;
// 		Timer3us(20);
// 		SFRPAGE = 0x20;
// 		timeout++;
// 		if(timeout==25000)
// 		{
// 			SFRPAGE = 0x00;
// 			return ('\n'); // Timeout after half second
// 		}
// 	}
// 	RI1=0;
// 	// Clear Overrun and Parity error flags 
// 	SCON1&=0b_0011_1111;
// 	c = SBUF1;
// 	SFRPAGE = 0x00;
// 	return (c);
// }

// void getstr1 (char * s)
// {
// 	char c;
	
// 	while(1)
// 	{
// 		c=getchar1_with_timeout();
// 		if(c=='\n')
// 		{
// 			*s=0;
// 			return;
// 		}
// 		*s=c;
// 		s++;
// 	}
// }

// // RXU1 returns '1' if there is a byte available in the receive buffer of UART1
// bit RXU1 (void)
// {
// 	bit mybit;
//     SFRPAGE = 0x20;
// 	mybit=RI1;
// 	SFRPAGE = 0x00;
// 	return mybit;
// }

// void waitms_or_RI1 (unsigned int ms)
// {
// 	unsigned int j;
// 	unsigned char k;
// 	for(j=0; j<ms; j++)
// 	{
// 		for (k=0; k<4; k++)
// 		{
// 			if(RXU1()) return;
// 			Timer3us(250);
// 		}
// 	}
// }

// void SendATCommand (char * s)
// {
// 	printf("Command: %s", s);
// 	P2_0=0; // 'set' pin to 0 is 'AT' mode.
// 	waitms(5);
// 	sendstr1(s);
// 	waitms(10);
// 	P2_0=1; // 'set' pin to 1 is normal operation mode.
// }

// void main (void)
// {
// 	char sXAngle[4];
// 	char sYAngle[4];
// 	float iXAngle = 0;
// 	float iYAngle = 0;
// 	int i;
// 	int j;

// 	float motor_PWM_DutyCycleWidth = 1;

// 	// this variable is only for testing purposes
// 	int motor_on = 0;

// 	count20ms=0; // Count20ms is an atomic variable, so no problem sharing with timer 5 ISR
// 	waitms(500);
// 	printf("\r\nJDY-40 test\r\n");
// 	UART1_Init(9600);

// 	// To configure the device (shown here using default values).
// 	// For some changes to take effect, the JDY-40 needs to be power cycled.
// 	// Communication can only happen between devices with the
// 	// same RFID and DVID in the same channel.
	
// 	//SendATCommand("AT+BAUD4\r\n");
// 	//SendATCommand("AT+RFID8899\r\n");
// 	//SendATCommand("AT+DVID1122\r\n"); // Default device ID.
// 	//SendATCommand("AT+RFC001\r\n");
// 	//SendATCommand("AT+POWE9\r\n");
// 	//SendATCommand("AT+CLSSA0\r\n");
	
// 	// We should select an unique device ID.  The device ID can be a hex
// 	// number from 0x0000 to 0xFFFF.  In this case is set to 0xABBA
// 	SendATCommand("AT+DVID2385\r\n");  

// 	// To check configuration
// 	SendATCommand("AT+VER\r\n");
// 	SendATCommand("AT+BAUD\r\n");
// 	SendATCommand("AT+RFID\r\n");
// 	SendATCommand("AT+DVID\r\n");
// 	SendATCommand("AT+RFC\r\n");
// 	SendATCommand("AT+POWE\r\n");
// 	SendATCommand("AT+CLSS\r\n");

// 	while(1)
// 	{

// 		// determing if the motor needs to be turned on
// 		if(motor_on){
// 			// if motor needs to be turned on change the duty cycle to 1.3ms
// 			motor_PWM_DutyCycleWidth = 1.3;
			
// 		}else if(!motor_on){
// 			// otherwise writing 1ms to PWM pulse width for 0% throttle
// 			motor_PWM_DutyCycleWidth = 1;
// 		}

// 		// writing to the motor throttle
// 		pwm_reload=0x10000L-(SYSCLK*motor_PWM_DutyCycleWidth*1.0e-3)/12.0;

// 		if(RXU1())
// 		{
// 			getstr1(buff);
			
// 			for(i = 0; i < 5; i++){
// 				sXAngle[i] = buff[i];
// 			}
// 			for(j = 7; j < 11; j++){
// 				sYAngle[j-7] = buff[j];
// 			}

// 			iXAngle = atof(sXAngle);
// 			iYAngle = atof(sYAngle);

// 		}

// 		// determining if the BLDC needs to be turned on
// 		// for testing purposes, the motor will alsways run on min rpm
// 		// with a pulse width on 1ms

// 		// this following code is for testing, if the 
// 		if(P3_7 == 0 && !motor_on){
// 			motor_on = 1;
// 		}else if(P3_7 == 0 && motor_on){
// 			motor_on = 0;
// 		}

// 		printf("LX: %0.4f, RY: %0.4f\n, Motor State: %d", iXAngle, iYAngle, motor_on);

// 		waitms_or_RI1(100);
// 	}
// }

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

void putchar1 (char c) 
{
    SFRPAGE = 0x20;
	while (!TI1);
	TI1=0;
	SBUF1 = c;
	SFRPAGE = 0x00;
}

void sendstr1 (char * s)
{
	while(*s)
	{
		putchar1(*s);
		s++;	
	}
}

char getchar1 (void)
{
	char c;
    SFRPAGE = 0x20;
	while (!RI1);
	RI1=0;
	// Clear Overrun and Parity error flags 
	SCON1&=0b_0011_1111;
	c = SBUF1;
	SFRPAGE = 0x00;
	return (c);
}

char getchar1_with_timeout (void)
{
	char c;
	unsigned int timeout;
    SFRPAGE = 0x20;
    timeout=0;
	while (!RI1)
	{
		SFRPAGE = 0x00;
		Timer3us(20);
		SFRPAGE = 0x20;
		timeout++;
		if(timeout==25000)
		{
			SFRPAGE = 0x00;
			return ('\n'); // Timeout after half second
		}
	}
	RI1=0;
	// Clear Overrun and Parity error flags 
	SCON1&=0b_0011_1111;
	c = SBUF1;
	SFRPAGE = 0x00;
	return (c);
}

void getstr1 (char * s)
{
	char c;
	
	while(1)
	{
		c=getchar1_with_timeout();
		if(c=='\n')
		{
			*s=0;
			return;
		}
		*s=c;
		s++;
	}
}

// RXU1 returns '1' if there is a byte available in the receive buffer of UART1
bit RXU1 (void)
{
	bit mybit;
    SFRPAGE = 0x20;
	mybit=RI1;
	SFRPAGE = 0x00;
	return mybit;
}

void waitms_or_RI1 (unsigned int ms)
{
	unsigned int j;
	unsigned char k;
	for(j=0; j<ms; j++)
	{
		for (k=0; k<4; k++)
		{
			if(RXU1()) return;
			Timer3us(250);
		}
	}
}

void SendATCommand (char * s)
{
	printf("Command: %s", s);
	P2_0=0; // 'set' pin to 0 is 'AT' mode.
	waitms(5);
	sendstr1(s);
	waitms(10);
	P2_0=1; // 'set' pin to 1 is normal operation mode.
}

void main (void)
{
	char sXAngle[4];
	char sYAngle[4];
	char sThrottle[4];
	float fXAngle = 0;
	float fYAngle = 0;
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
	printf("\r\nJDY-40 test\r\n");
	UART1_Init(9600);

	// To configure the device (shown here using default values).
	// For some changes to take effect, the JDY-40 needs to be power cycled.
	// Communication can only happen between devices with the
	// same RFID and DVID in the same channel.
	
	//SendATCommand("AT+BAUD4\r\n");
	//SendATCommand("AT+RFID8899\r\n");
	//SendATCommand("AT+DVID1122\r\n"); // Default device ID.
	//SendATCommand("AT+RFC001\r\n");
	//SendATCommand("AT+POWE9\r\n");
	//SendATCommand("AT+CLSSA0\r\n");
	
	// We should select an unique device ID.  The device ID can be a hex
	// number from 0x0000 to 0xFFFF.  In this case is set to 0xABBA
	SendATCommand("AT+DVID2385\r\n");  

	// To check configuration
	SendATCommand("AT+VER\r\n");
	SendATCommand("AT+BAUD\r\n");
	SendATCommand("AT+RFID\r\n");
	SendATCommand("AT+DVID\r\n");
	SendATCommand("AT+RFC\r\n");
	SendATCommand("AT+POWE\r\n");
	SendATCommand("AT+CLSS\r\n");

	while(1)
	{
		//PWM receiving signals from the fucking potentiometer
		if(RXU1()){
			getstr1(buff);
			// checking the length of buff to determine if there is data loss
			if(strlen(buff) == 14){
				for(i = 0; i < 4; i++){
				sXAngle[i] = buff[i];
				}
				for(j = 5; j < 9; j++){
					sYAngle[j-5] = buff[j];
				}
				for(k = 10; k < 14; k++){
					sThrottle[k-10] = buff[k];
				}

				fXAngle = atoi(sXAngle)/1000.0;
				fYAngle = atoi(sYAngle)/1000.0;

				potentiometerReading = atoi(buff)/1000.0;

				printf("X: %0.4f, Y: %0.4f, T: %0.4f\n");
			}

			// determing if the motor needs to be turned on
			if(motor_on){
			//if motor needs to be turned on change the duty cycle to 1.3ms
				motor_PWM_DutyCycleWidth = 2.0 - potentiometerReading/5;
			
			}else if(!motor_on){
			// otherwise writing 1ms to PWM pulse width for 0% throttle
				motor_PWM_DutyCycleWidth = 1;
			}

			// writing to the motor throttle
			pwm_reload=0x10000L-(SYSCLK*motor_PWM_DutyCycleWidth*1.0e-3)/12.0;
		}

		// determining if the BLDC needs to be turned on
		// for testing purposes, the motor will alsways run on min rpm
		// with a pulse width on 1ms

		// this following code is for testing, if the 
		if(P3_7 == 0 && !motor_on){
			motor_on = 1;
		}else if(P3_7 == 0 && motor_on){
			motor_on = 0;
		}

		// printf("Duty: %0.4f, Motor State: %d, Potentiometer: %0.4f\n",
		//  motor_PWM_DutyCycleWidth, motor_on,potentiometerReading);

		waitms_or_RI1(100);
	}
}
