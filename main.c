//************************************************************************
// MSP430G2231 Demo - Timer_A, PWM TA1, Up Mode, SMCLK
//
// Description: This program generates one PWM output on P1.2 using
// Timer_A configured for up mode. The value in CCR0, 1000-1, defines the
// PWM period and the value in CCR1 the PWM duty cycles. Using SMCLK,
// the timer frequenciy is about 1.1kHz with a 25% duty cycle on P1.2.
// Normal operating mode is LPM0.
// MCLK = SMCLK = default DCO (about 1.1MHz).
//
//        MSP430G2231
//        -----------------
//   /|\ |                 |
//    |  |                 |
//     --|RST      P1.2/TA1|--> CCR1 - 75% PWM
//       |                 |
//       |                 |
//
// M.Buccini / L. Westlund
// Texas Instruments, Inc
// October 2005
// Built with CCE Version: 3.2.0 and IAR Embedded Workbench Version: 3.40A
//
// Modified by NJC for MSP430LaunchPad.com - July 2010
//
//************************************************************************
#define BAT_THRESHOLD_LOW 300
#define BAT_THRESHOLD_HIGH 1023

#include "msp430G2452.h"

unsigned int i, j, a, Temp, VBat, b, s;

const unsigned int pwmtable16[64] =
{
		1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 6, 7, 8, 9, 10,
		    11, 12, 13, 15, 17, 19, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55,
		    61, 68, 76, 85, 94, 105, 117, 131, 146, 162, 181, 202, 225, 250,
		    279, 311, 346, 386, 430, 479, 534, 595, 663, 739, 824, 918, 1023
};

void TempMessen(void)
{

	/* Configure ADC  Channel */
	  ADC10CTL1 = INCH_1 + ADC10DIV_7 ;         // Channel 1, ADC10CLK/4
	  ADC10CTL0 = SREF_1 + REFON + ADC10SHT_3 + ADC10ON + ADC10IE;  //1,5V & Vss as reference
	  ADC10AE0 |= BIT1;                         //P1.1 ADC option

	  __enable_interrupt();                     // Enable interrupts.
	  __delay_cycles(1000);                   	// Wait for ADC Ref to settle

	a = 0;
			   	  for(i=10;i>0;i--)
			   	  {
			   		  ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
			   		  __bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled
			   		  a = ADC10MEM + a;
			   	  }
			   	  Temp = a/10;
			   	ADC10CTL0 &= ~ENC;			// ADC10 abschalten um Eingangspin wechseln zu können
}

void TempInterpretieren(void)
	{
		if (Temp>1000)
		{
			//CCR1 = 1;
		}
	}


void SpannungMessen(void)
{

	/* Configure ADC  Channel */
	  ADC10CTL1 = INCH_4 + ADC10DIV_7 ;         // Channel 4, ADC10CLK/4
	  ADC10CTL0 = SREF_1 + REFON + ADC10SHT_3 + ADC10ON + ADC10IE;  //1,5V & Vss as reference
	  ADC10AE0 |= BIT4;                         //P1.4 ADC option

	  __enable_interrupt();                     // Enable interrupts.
	  __delay_cycles(1000);                   	// Wait for ADC Ref to settle

	a = 0;
			   	  for(i=10;i>0;i--)
			   	  {
			   		  ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
			   		  __bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled
			   		  a = ADC10MEM + a;
			   	  }
			   	  VBat = a/10;
			   	ADC10CTL0 &= ~ENC;			// ADC10 abschalten um Eingangspin wechseln zu können
}

void SpannungInterpretieren(void)
{
	if (VBat>BAT_THRESHOLD_HIGH)				// 993 = 13,8V
	{
		b = 1;					// betriebsbereit
		s = 1;					// Überspannung
	}

	else if (VBat<BAT_THRESHOLD_LOW)			// 864 = 12V
	{
		CCR1 = 0;					// Lampe abschalten

		P1OUT &= ~BIT6;
		__bis_SR_register(CPUOFF);


	}
	else
	{
		s = 0;					// keine Überspannung
		b = 1;					// betriebsbereit
	}

}


void main(void)
{
WDTCTL = WDT_ADLY_250;              // WDT as interval timer (period 0,5 ms)
IE1 |= WDTIE;                       // Enable WDT interrupt

 P1DIR |= BIT6;             // P1.6 to output
 P1DIR |= BIT0;
 P1SEL |= BIT6;             // P1.6 to TA0.1
 P1SEL |= BIT1;                            //ADC Input pin P1.1
 P1SEL |= BIT4;							// ADC Input pin P1.4
 P1DIR &= ~BIT3;
 P1REN |= BIT3;
 P1IE |= BIT3;                             // P1.3 interrupt enabled
 P1IES |= BIT3;                            // P1.3 Hi/lo edge
 P1IFG &= ~BIT3;                       // P1.3 IFG cleared

 CCR0 = 1024;             // PWM Period
 CCTL1 = OUTMOD_7;          // CCR1 reset/set
 CCR1 = 0;                // CCR1 PWM duty cycle
 TACTL = TASSEL_2 + MC_1;   // SMCLK, up mode

 _BIS_SR(CPUOFF + GIE);                    // Enter LPM0 w/ interrupt

 while(1)
 {}

}
// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	__delay_cycles(10000);
	if(P1IN & BIT3)
	{
		P1IFG &= ~BIT3;
		return;
	}

if(CCR1==0)
{
	CCR1 = 1024;

	i = 0;
	j = 64;
	while (!(P1IN & BIT3))
	{

		__delay_cycles(30000);

		if(++i>20 && j>0 && j<256)
		{
			CCR1 = pwmtable16 [--j];
		}

	}
}
else
{
	CCR1 = 0;
}



	P1IFG &= ~BIT3;                     // P1.3 IFG cleared
	_BIS_SR(CPUOFF + GIE);                    // Enter LPM0 w/ interrupt


}

// Watchdog Interval Timer interrupt service
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
	P1OUT |= BIT0;
	TempMessen();
	TempInterpretieren();
	SpannungMessen();
	SpannungInterpretieren();

	P1OUT &= ~BIT0;
}

#pragma vector=ADC10_VECTOR

__interrupt void ADC10_ISR (void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Return to active mode
}
