#include "msp430.h"
#include "stdlib.h"
#define ADC_CH 4
unsigned int samples[ADC_CH];
volatile unsigned int i,a,b;
volatile unsigned int i1,c,d;
#define SENS_LEFT BIT0
#define SENS_RIGHT BIT1
#define SENS_UP BIT2
#define SENS_DOWN BIT3

#define MCU_CLOCK			1000000
#define PWM_FREQUENCY		46		//ideally 50Hz.
#define SERVO_STEPS			30		// max amount of steps in degrees (180 is common)
unsigned int PWM_Period		= (MCU_CLOCK / PWM_FREQUENCY);	// PWM Period
unsigned int PWM_Duty		= 0;							//

void ConfigureAdc(void){

ADC10CTL1 = INCH_3 | ADC10DIV_0 | CONSEQ_3 | SHS_0;

ADC10CTL0 = SREF_0 | ADC10SHT_2 | MSC | ADC10ON | ADC10IE;

ADC10AE0 =SENS_LEFT + SENS_RIGHT + SENS_UP + SENS_DOWN ;

ADC10DTC1 = ADC_CH;

}

void main(void) {

WDTCTL = WDTPW | WDTHOLD;

		TA1CCTL1    = OUTMOD_7 ;
		TA0CCTL1	= OUTMOD_7  ;

		TA1CTL  = TASSEL_2 + MC_1 ;
		TA0CTL  = TASSEL_2 + MC_1 ;

		TA1CCR0	= PWM_Period-1;        // PWM Period
		TA0CCR0 = PWM_Period-1 ; 

		TA1CCR1	= 1500 ;            // TACCR1 PWM Duty Cycle
		TA0CCR1 = 1500 ;

P1DIR = 0; /* set as inputs */
P1SEL = 0; /* set as digital I/Os */
P1OUT = 0; /* set resistors as pull-downs */

P1REN |= (SENS_LEFT|SENS_RIGHT|SENS_DOWN|SENS_UP); /* enable pull-up on SENSOR */

		P1DIR	|= BIT6  ;               // P1.2 = output
		P2DIR   |= BIT2 ;

		P1SEL	|= BIT6  ;               // P1.2 = TA1 output
		P2SEL   |= BIT2  ;

ConfigureAdc();
__enable_interrupt();
while (1) {

	__delay_cycles(1000);
	ADC10CTL0 &= ~ENC;
	while (ADC10CTL1 & BUSY);
	ADC10SA = (unsigned int)samples;
	ADC10CTL0 |= ENC + ADC10SC;
	__bis_SR_register(CPUOFF + GIE);
	}
}

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR (void){
a=(samples[0]+samples[1])/2;
b=(samples[2]+samples[3])/2;
c=(samples[0]+samples[2])/2;
d=(samples[1]+samples[3])/2;
i=abs(a-b);
i1=abs(c-d);
if (i>100) 
{
  if (a<b)
  {
    if (TA0CCR1<1800) TA0CCR1=TA0CCR1+1;
    }
  else {if (TA0CCR1>800) TA0CCR1=TA0CCR1-1;}
}
if (i1>100) 
{
  if (c>d)
  {
    if (TA1CCR1<2600) TA1CCR1=TA1CCR1+1;
    }
  else {if (TA1CCR1>400) TA1CCR1=TA1CCR1-1;}
}
__bic_SR_register_on_exit(CPUOFF);
}