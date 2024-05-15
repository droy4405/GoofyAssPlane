
#include <EFM8LB1.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SYSCLK 72000000
#define BAUDRATE 115200L
#define TIMER_2_FREQ 3000L
#define WINGSERVOL P1_1 //aileron servo
#define WINGSERVOR P1_2 
// #define ELEVSERVOL P1_3 //elevator left
// #define ELEVSERVOR P1_4 //elevator right servo

volatile unsigned int servo_counter=0;
volatile unsigned char servo1=150, servo2=150;

idata char buff[22];
volatile unsigned int pwm_reload;

volatile unsigned char timer5_elevL_pwm_Dcycle = 150;
volatile unsigned char timer5_elevR_pwm_Dcycle = 150;

volatile unsigned char pwm_state = 0;
volatile unsigned char count20ms;

#define timer5_elevL_out P1_7
#define timer5_elevR_out P1_6

#define ARMINGOUT P1_5

#define RELOAD_10MS (0x10000L-(SYSCLK/(12L*100L)))
#define RELOAD_10us (0x10000L-(SYSCLK/(12L*100000L))) // 10us rate

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

	// // Initialize timer 2 for periodic interrupts
	// TMR2CN0=0x00;   // Stop Timer2; Clear TF2;
	// CKCON0|=0b_0001_0000; // Timer 2 uses the system clock
	// TMR2RL=(0x10000L-(SYSCLK/(2*TIMER_2_FREQ))); // Initialize reload value
	// TMR2=0xffff;   // Set to reload immediately
	// ET2=1;         // Enable Timer2 interrupts
	// TR2=1;         // Start Timer2 (TMR2CN is bit addressable)

	// 	// Initialize timer 4 for periodic interrupts
	// 	SFRPAGE=0x10;
	// 	TMR4CN0=0x00;   // Stop Timer4; Clear TF4; WARNING: lives in SFR page 0x10
	// 	CKCON1|=0b_0000_0001; // Timer 4 uses the system clock
	// 	TMR4RL=(0x10000L-(SYSCLK/(2*TIMER_4_FREQ))); // Initialize reload value
	// 	TMR4=0xffff;   // Set to reload immediately
	// 	EIE2|=0b_0000_0100;     // Enable Timer4 interrupts
	// 	TR4=1;

  	// Initialize timer 5 for periodic interrupts
	SFRPAGE=0x10;
	TMR5CN0=0x00;
	// timer5_elevL_pwm_reload=0x10000L-(SYSCLK*1.5e-3)/12.0; // 1.5 miliseconds pulse is the center of the servo
	TMR5=0xffff;   // Set to reload immediately
	EIE2|=0b_0000_1000; // Enable Timer5 interrupts
	TR5=1;         // Start Timer5 (TMR5CN0 is bit addressable)
	
	EA=1;
	
	SFRPAGE=0x00;

	return 0;
}

// controlling PWM for servo motors on the receiver chip
void Timer5_ISR (void) interrupt INTERRUPT_TIMER5
{
	// SFRPAGE=0x10;
	// TF5H = 0; // Clear Timer5 interrupt flag
	// // Since the maximum time we can achieve with this timer in the
	// // configuration above is about 10ms, implement a simple state
	// // machine to produce the required 20ms period.
	// switch (pwm_state)
	// {
	// 	// case 10:
	// 	// 	//in this case the PWM is turned off
	// 	// 	ESCOUT = 0;
			
	// 	case 0:
	// 		timer5_elevL_out=1;
	// 		TMR5RL=RELOAD_10MS;
	// 		pwm_state=1;
	// 		count20ms++;
	// 	break;

	// 	case 1:
	// 		timer5_elevL_out=0;
	// 		TMR5RL=RELOAD_10MS-timer5_elevL_pwm_reload;
	// 		pwm_state=2;
	// 	break;

	// 	default:
	// 		timer5_elevL_out=0;
	// 		timer5_elevR_out=0;
	// 		TMR5RL=timer5_elevL_pwm_reload;
	// 		pwm_state=0;
	// 	break;
	// }

	SFRPAGE=0x10;
	TF5H = 0; // Clear Timer5 interrupt flag
	TMR5RL=RELOAD_10us;
	servo_counter++;
	if(servo_counter==2000)
	{
		servo_counter=0;
	}

	if(timer5_elevL_pwm_Dcycle>=servo_counter)
	{
		timer5_elevL_out=1;
	}
	else
	{
		timer5_elevL_out=0;
	}
	if(timer5_elevR_pwm_Dcycle>=servo_counter)
	{
		timer5_elevR_out=1;
	}
	else
	{
		timer5_elevR_out=0;
	}
}

// timer 2 for controlling servos on the wings

//pseudo code
// on the right wing pwmWingR. paired with  left aileron
// max deflection degree = 25
// right joystick, left right dir
// if joystick is all the way to left -> roll left -> right aileron move down 25 degree, left move up 25
// 
// void Timer2_ISR (void) interrupt INTERRUPT_TIMER5
// {
// 	SFRPAGE=0x10;
// 	TF2H = 0; // Clear Timer2 interrupt flag
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
// 			TMR2RL=RELOAD_10MS;
// 			pwm_state=1;
// 			count20ms++;
// 		break;

// 		case 1:
// 			ESCOUT=0;
// 			TMR2RL=RELOAD_10MS-pwm_reload;
// 			pwm_state=2;
// 		break;

// 		default:
// 			ESCOUT=0;
// 			TMR2RL=pwm_reload;
// 			pwm_state=0;
// 		break;
// 	}
// }

// void Timer2_ISR (void) interrupt INTERRUPT_TIMER2
// {
// 	SFRPAGE=0x0;
// 	TF2H = 0; // Clear Timer2 interrupt flag
//    	TMR2RL=TMR2RL=0x10000L-(SYSCLK/(2*TIMER_2_FREQ));;

	
// 	servo_counter++;

// 	if(servo_counter==2000)
// 	{
// 		servo_counter=0;
// 	}
// 	if(servo1>=servo_counter)
// 	{
// 		SERVO1=1;
// 	}
// 	else
// 	{
// 		SERVO1=0;
// 	}
// 	if(servo2>=servo_counter)
// 	{
// 		SERVO2=1;
// 	}
// 	else
// 	{
// 		SERVO2=0;
// 	}
//     // TIMER_OUT_2=!TIMER_OUT_2;
// }


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

// take the joystick input and map it to pwm val
// for aileron
// void map_angle_deflection(int x)
// {
// 	if(x ==)
// }


void main (void)
{	
	char sYawL[5]; // rudder, yaw, left joystick, left and right
	char sPitchR[5]; // elevator, pitch, right joystick, forward backward
	char sRollR[5]; // aliron, roll, right joystick, left right
	char sThrottle[5]; // throttle, left joystick, forward backwoard, control
	char sParachute[2];
	char sThrottle_level2[2];
	char sThrottle_level1[2];
	char sThrottle_level0[2];

	float pitch = 0;
	float roll = 0;
	float yaw = 0;
	float fThrottle = 0;

	int i,j,k,l,m;
	int parachute_deploy = 0;

	int throttle_level2 = 0;
	int throttle_level1 = 0;
	int throttle_level0 = 0;

	//PWM variables
	float ThrottlePWM;

	//float motor_PWM_DutyCycleWidth = 1;

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

	// initialllzing throttle duty cycke
	// pwm_reload=0x10000L-(SYSCLK*motor_PWM_DutyCycleWidth*1.0e-3)/12.0;
	// ARMINGOUT = 0;

	while(1)
	{
		//PWM receiving signals from the potentiometer
		if(RXU1()){
			getstr1(buff);
			// checking the length of buff to determine if there is data loss
			if(strlen(buff) == 22){
				//printf("%s\n", buff);
				for(i = 0; i < 4; i++){
					sYawL[i] = buff[i];
				}
				sYawL[4] = '\0';
				yaw = atoi(sYawL)/1000.0;

				for(j = 5; j < 9; j++){
					sRollR[j-5] = buff[j];
				}
				sRollR[4] = '\0';
				roll = atoi(sRollR)/1000.0;

				for(k = 10; k < 14; k++){
					sPitchR[k-10] = buff[k];
				}
				sPitchR[4] = '\0';
				pitch = atoi(sPitchR)/1000.0;

				// for(l = 15; l < 19; l++){ 
				// 	sThrottle[l-15] = buff[l];
				// }
				// sThrottle[4] = '\0';
				// ThrottlePWM = atoi(sThrottle)/1000.0;

				for(l = 15; l < 16; l++){ 
					sParachute[l-15] = buff[l];
				}
				sParachute[1] = '\0';
				parachute_deploy = atoi(sParachute);
				
				sThrottle_level2[0] = buff[17];
				sThrottle_level2[1] = '\0';
				sThrottle_level1[0] = buff[19];
				sThrottle_level1[1] = '\0';
				sThrottle_level0[0] = buff[21];
				sThrottle_level0[1] = '\0';

				throttle_level2 = atoi(sThrottle_level2);
				throttle_level1 = atoi(sThrottle_level1);
				throttle_level0 = atoi(sThrottle_level0);
				// printf("X: %0.4f, ",fXAngle);
				// printf("Y: %0.4f, ",fYAngle);
				// printf("T: %0.4f\n",potentiometerReading);

				//printf("yaw: %0.4f, roll: %0.4f, pitch: %0.4f, parachute: %d throttle: %d, %d, %d\n"
				//, yaw, roll, pitch, parachute_deploy, throttle_level2, throttle_level1, throttle_level0);
			}

		}

		// check if the parachute needs to be deployed
		// if yes send digital high through pin 0.6
		P0_6 = parachute_deploy;
		
		// sending the throttle level signal from RC slave to SPI slave
		P1_0 = throttle_level2;
		P1_1 = throttle_level1;
		P1_2 = throttle_level0;
		
		waitms_or_RI1(100);
	}
}
