// #include <EFM8LB1.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>

// #define SYSCLK 72000000
// #define SARCLK 18000000L
// #define BAUDRATE 115200L

// idata char buff[20];

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
  	
// 	return 0;
// }

// void InitADC (void)
// {
// 	SFRPAGE = 0x00;
// 	ADEN=0; // Disable ADC
	
// 	ADC0CN1=
// 		(0x2 << 6) | // 0x0: 10-bit, 0x1: 12-bit, 0x2: 14-bit
//         (0x0 << 3) | // 0x0: No shift. 0x1: Shift right 1 bit. 0x2: Shift right 2 bits. 0x3: Shift right 3 bits.		
// 		(0x0 << 0) ; // Accumulate n conversions: 0x0: 1, 0x1:4, 0x2:8, 0x3:16, 0x4:32
	
// 	ADC0CF0=
// 	    ((SYSCLK/SARCLK) << 3) | // SAR Clock Divider. Max is 18MHz. Fsarclk = (Fadcclk) / (ADSC + 1)
// 		(0x0 << 2); // 0:SYSCLK ADCCLK = SYSCLK. 1:HFOSC0 ADCCLK = HFOSC0.
	
// 	ADC0CF1=
// 		(0 << 7)   | // 0: Disable low power mode. 1: Enable low power mode.
// 		(0x1E << 0); // Conversion Tracking Time. Tadtk = ADTK / (Fsarclk)
	
// 	ADC0CN0 =
// 		(0x0 << 7) | // ADEN. 0: Disable ADC0. 1: Enable ADC0.
// 		(0x0 << 6) | // IPOEN. 0: Keep ADC powered on when ADEN is 1. 1: Power down when ADC is idle.
// 		(0x0 << 5) | // ADINT. Set by hardware upon completion of a data conversion. Must be cleared by firmware.
// 		(0x0 << 4) | // ADBUSY. Writing 1 to this bit initiates an ADC conversion when ADCM = 000. This bit should not be polled to indicate when a conversion is complete. Instead, the ADINT bit should be used when polling for conversion completion.
// 		(0x0 << 3) | // ADWINT. Set by hardware when the contents of ADC0H:ADC0L fall within the window specified by ADC0GTH:ADC0GTL and ADC0LTH:ADC0LTL. Can trigger an interrupt. Must be cleared by firmware.
// 		(0x0 << 2) | // ADGN (Gain Control). 0x0: PGA gain=1. 0x1: PGA gain=0.75. 0x2: PGA gain=0.5. 0x3: PGA gain=0.25.
// 		(0x0 << 0) ; // TEMPE. 0: Disable the Temperature Sensor. 1: Enable the Temperature Sensor.

// 	ADC0CF2= 
// 		(0x0 << 7) | // GNDSL. 0: reference is the GND pin. 1: reference is the AGND pin.
// 		(0x1 << 5) | // REFSL. 0x0: VREF pin (external or on-chip). 0x1: VDD pin. 0x2: 1.8V. 0x3: internal voltage reference.
// 		(0x1F << 0); // ADPWR. Power Up Delay Time. Tpwrtime = ((4 * (ADPWR + 1)) + 2) / (Fadcclk)
	
// 	ADC0CN2 =
// 		(0x0 << 7) | // PACEN. 0x0: The ADC accumulator is over-written.  0x1: The ADC accumulator adds to results.
// 		(0x0 << 0) ; // ADCM. 0x0: ADBUSY, 0x1: TIMER0, 0x2: TIMER2, 0x3: TIMER3, 0x4: CNVSTR, 0x5: CEX5, 0x6: TIMER4, 0x7: TIMER5, 0x8: CLU0, 0x9: CLU1, 0xA: CLU2, 0xB: CLU3

// 	ADEN=1; // Enable ADC
// }

// void TIMER0_Init(void)
// {
// 	TMOD&=0b_1111_0000; // Set the bits of Timer/Counter 0 to zero
// 	TMOD|=0b_0000_0001; // Timer/Counter 0 used as a 16-bit timer
// 	TR0=0; // Stop Timer/Counter 0
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

// #define VDD 5.00 // The measured value of VDD in volts

// void InitPinADC (unsigned char portno, unsigned char pinno)
// {
// 	unsigned char mask;
	
// 	mask=1<<pinno;

// 	SFRPAGE = 0x20;
// 	switch (portno)
// 	{
// 		case 0:
// 			P0MDIN &= (~mask); // Set pin as analog input
// 			P0SKIP |= mask; // Skip Crossbar decoding for this pin
// 		break;
// 		case 1:
// 			P1MDIN &= (~mask); // Set pin as analog input
// 			P1SKIP |= mask; // Skip Crossbar decoding for this pin
// 		break;
// 		case 2:
// 			P2MDIN &= (~mask); // Set pin as analog input
// 			P2SKIP |= mask; // Skip Crossbar decoding for this pin
// 		break;
// 		default:
// 		break;
// 	}
// 	SFRPAGE = 0x00;
// }

// unsigned int Get_ADC (void)
// {
// ADINT = 0;
// ADBUSY = 1;
// while (!ADINT); // Wait for conversion to complete
// return (ADC0);
// }

// unsigned int ADC_at_Pin(unsigned char pin)
// {
// 	ADC0MX = pin;   // Select input from pin
// 	ADINT = 0;
// 	ADBUSY = 1;     // Convert voltage at the pin
// 	while (!ADINT); // Wait for conversion to complete
// 	return (ADC0);
// }

// float Volts_at_Pin(unsigned char pin)
// {
// 	 return ((ADC_at_Pin(pin)*VDD)/16383.0);
// }

// // measure voltage of reference signal
// float voltage_ref(){
// 	float ref_voltage;
	
// 	ref_voltage = Volts_at_Pin(P0_1);

// 	return ref_voltage; 
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
// 	float X_pos_L;
// 	float Y_pos_R;
// 	char X_pos_L_string[5];
// 	int count = 0;

// 	// this important variable is used to change the PWM duty
// 	// cycle in charge of controlling the motor throttle
// 	float PWM_DutyCyclePeriod = 1; // initiallized to be 1ms for 0% throttle
	
// 	// here is the initiallization code
// 	waitms(500);
// 	printf("\r\nJDY-40 test\r\n");
// 	UART1_Init(9600);

// 	InitPinADC(2, 1); // Configure P0.1 as analog input
// 	InitPinADC(1, 2); // Configure P2.3 as analog input
// 	InitADC();

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
// 		// readng the analog voltage of left joystick X position
// 		// on pin 2.1

// 		X_pos_L = Volts_at_Pin(QFP32_MUX_P2_1);
		
// 		sprintf(buff, "%0.4f %0.4f\r\n", X_pos_L, Y_pos_R);

// 		//sprintf(buff, "test %d\n",count++);
// 		sendstr1(buff);
		
// 		waitms_or_RI1(100);
// 	}
// }

#include <EFM8LB1.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SYSCLK 72000000
#define SARCLK 18000000L
#define BAUDRATE 115200L

idata char buff[20];

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
  	
	return 0;
}

void InitADC (void)
{
	SFRPAGE = 0x00;
	ADEN=0; // Disable ADC
	
	ADC0CN1=
		(0x2 << 6) | // 0x0: 10-bit, 0x1: 12-bit, 0x2: 14-bit
        (0x0 << 3) | // 0x0: No shift. 0x1: Shift right 1 bit. 0x2: Shift right 2 bits. 0x3: Shift right 3 bits.		
		(0x0 << 0) ; // Accumulate n conversions: 0x0: 1, 0x1:4, 0x2:8, 0x3:16, 0x4:32
	
	ADC0CF0=
	    ((SYSCLK/SARCLK) << 3) | // SAR Clock Divider. Max is 18MHz. Fsarclk = (Fadcclk) / (ADSC + 1)
		(0x0 << 2); // 0:SYSCLK ADCCLK = SYSCLK. 1:HFOSC0 ADCCLK = HFOSC0.
	
	ADC0CF1=
		(0 << 7)   | // 0: Disable low power mode. 1: Enable low power mode.
		(0x1E << 0); // Conversion Tracking Time. Tadtk = ADTK / (Fsarclk)
	
	ADC0CN0 =
		(0x0 << 7) | // ADEN. 0: Disable ADC0. 1: Enable ADC0.
		(0x0 << 6) | // IPOEN. 0: Keep ADC powered on when ADEN is 1. 1: Power down when ADC is idle.
		(0x0 << 5) | // ADINT. Set by hardware upon completion of a data conversion. Must be cleared by firmware.
		(0x0 << 4) | // ADBUSY. Writing 1 to this bit initiates an ADC conversion when ADCM = 000. This bit should not be polled to indicate when a conversion is complete. Instead, the ADINT bit should be used when polling for conversion completion.
		(0x0 << 3) | // ADWINT. Set by hardware when the contents of ADC0H:ADC0L fall within the window specified by ADC0GTH:ADC0GTL and ADC0LTH:ADC0LTL. Can trigger an interrupt. Must be cleared by firmware.
		(0x0 << 2) | // ADGN (Gain Control). 0x0: PGA gain=1. 0x1: PGA gain=0.75. 0x2: PGA gain=0.5. 0x3: PGA gain=0.25.
		(0x0 << 0) ; // TEMPE. 0: Disable the Temperature Sensor. 1: Enable the Temperature Sensor.

	ADC0CF2= 
		(0x0 << 7) | // GNDSL. 0: reference is the GND pin. 1: reference is the AGND pin.
		(0x1 << 5) | // REFSL. 0x0: VREF pin (external or on-chip). 0x1: VDD pin. 0x2: 1.8V. 0x3: internal voltage reference.
		(0x1F << 0); // ADPWR. Power Up Delay Time. Tpwrtime = ((4 * (ADPWR + 1)) + 2) / (Fadcclk)
	
	ADC0CN2 =
		(0x0 << 7) | // PACEN. 0x0: The ADC accumulator is over-written.  0x1: The ADC accumulator adds to results.
		(0x0 << 0) ; // ADCM. 0x0: ADBUSY, 0x1: TIMER0, 0x2: TIMER2, 0x3: TIMER3, 0x4: CNVSTR, 0x5: CEX5, 0x6: TIMER4, 0x7: TIMER5, 0x8: CLU0, 0x9: CLU1, 0xA: CLU2, 0xB: CLU3

	ADEN=1; // Enable ADC
}

void TIMER0_Init(void)
{
	TMOD&=0b_1111_0000; // Set the bits of Timer/Counter 0 to zero
	TMOD|=0b_0000_0001; // Timer/Counter 0 used as a 16-bit timer
	TR0=0; // Stop Timer/Counter 0
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

#define VDD 5.00 // The measured value of VDD in volts

void InitPinADC (unsigned char portno, unsigned char pinno)
{
	unsigned char mask;
	
	mask=1<<pinno;

	SFRPAGE = 0x20;
	switch (portno)
	{
		case 0:
			P0MDIN &= (~mask); // Set pin as analog input
			P0SKIP |= mask; // Skip Crossbar decoding for this pin
		break;
		case 1:
			P1MDIN &= (~mask); // Set pin as analog input
			P1SKIP |= mask; // Skip Crossbar decoding for this pin
		break;
		case 2:
			P2MDIN &= (~mask); // Set pin as analog input
			P2SKIP |= mask; // Skip Crossbar decoding for this pin
		break;
		default:
		break;
	}
	SFRPAGE = 0x00;
}

unsigned int Get_ADC (void)
{
ADINT = 0;
ADBUSY = 1;
while (!ADINT); // Wait for conversion to complete
return (ADC0);
}

unsigned int ADC_at_Pin(unsigned char pin)
{
	ADC0MX = pin;   // Select input from pin
	ADINT = 0;
	ADBUSY = 1;     // Convert voltage at the pin
	while (!ADINT); // Wait for conversion to complete
	return (ADC0);
}

float Volts_at_Pin(unsigned char pin)
{
	 return ((ADC_at_Pin(pin)*VDD)/16383.0);
}

float throttle_control_ADC(unsigned char pin)
{
	unsigned int throttle_ADC;
	throttle_ADC = ADC_at_Pin(pin);

	if(throttle_ADC < 16383/5){
		return 1.0;
	} else{
		return ((throttle_ADC*VDD)/16383.0);
	}
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
	float X_pos_L;
	float Y_pos_R;
	char X_pos_L_string[5];
	int count = 0;

	float potentiometer = 0;

	// this important variable is used to change the PWM duty
	// cycle in charge of controlling the motor throttle
	float PWM_DutyCyclePeriod = 1; // initiallized to be 1ms for 0% throttle
	
	// here is the initiallization code
	waitms(500);
	printf("\r\nJDY-40 test\r\n");
	UART1_Init(9600);

	InitPinADC(2, 2); // Configure P0.1 as analog input
	InitPinADC(1, 2); // Configure P2.3 as analog input
	InitPinADC(2, 3);
	InitPinADC(2, 1);
	InitADC();

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
		// readng the analog voltage of left joystick X position
		// on pin 2.1

		X_pos_L = Volts_at_Pin(QFP32_MUX_P2_2);
		Y_pos_R = Volts_at_Pin(QFP32_MUX_P1_2);
		
		// casting the coords to send with RC
		X_pos_L *= 1000;
		Y_pos_R *= 1000;

		//potentiometer = Volts_at_Pin(QFP32_MUX_P2_3);
		potentiometer = throttle_control_ADC(QFP32_MUX_P2_3);

		// casting throttle reading into int for radio sending
		potentiometer *= 1000;
		
		sprintf(buff, "%0*d %0*d %d\n", 4, (int)X_pos_L, 4, (int)Y_pos_R, (int)potentiometer);

		//sprintf(buff, "test %d\n",count++);
		printf("%s",buff);
		sendstr1(buff);
		
		waitms_or_RI1(100);
	}
}
