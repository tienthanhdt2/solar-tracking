#include "msp430.h"
#include "stdlib.h"
#define ADC_CH 4
unsigned int LDR[ADC_CH];
volatile unsigned int avgtop,avgbot,avgleft,avgright,i,i1;
#define LDR0 BIT0
#define LDR1 BIT1
#define LDR2 BIT2
#define LDR3 BIT3

#define MCU_CLOCK 1000000
#define PWM_FREQUENCY 50
unsigned int PWM_Period	= (MCU_CLOCK / PWM_FREQUENCY);
unsigned int PWM_Duty = 0;

void ConfigureAdc(void)
{
  ADC10CTL1 = INCH_3 | ADC10DIV_0 | CONSEQ_3 | SHS_0;
  ADC10CTL0 = SREF_0 | ADC10SHT_2 | MSC | ADC10ON | ADC10IE;
  ADC10AE0 =LDR0 + LDR1 + LDR2 + LDR3 ;
  ADC10DTC1 = ADC_CH;
}

void main(void)
{
  WDTCTL = WDTPW | WDTHOLD;
  TA1CCTL1 = OUTMOD_7 ;
  TA0CCTL1 = OUTMOD_7  ;
  TA1CTL = TASSEL_2 + MC_1 ;
  TA0CTL = TASSEL_2 + MC_1 ;
  TA1CCR0 = PWM_Period-1;     // PWM Period
  TA0CCR0 = PWM_Period-1 ; 
  TA1CCR1 = 1500 ; // TACCR1 PWM Duty Cycle
  TA0CCR1 = 1500 ;
  P1DIR = 0;
  P1SEL = 0;
  P1OUT = 0;
  P1REN |= (LDR0|LDR1|LDR2|LDR3);
  P1DIR	|= BIT6;
  P2DIR |= BIT2;
  P1SEL	|= BIT6;
  P2SEL |= BIT2;
  ConfigureAdc();
  __enable_interrupt();
  while (1) 
  {
    __delay_cycles(1000);
    ADC10CTL0 &= ~ENC;
    while (ADC10CTL1 & BUSY);
    ADC10SA = (unsigned int)LDR;
    ADC10CTL0 |= ENC + ADC10SC;
    __bis_SR_register(CPUOFF + GIE);
  }
}

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
  avgtop = (LDR[0] + LDR[1])/2;//tinh trung binh cong 2 LDR tren
  avgbot = (LDR[2] + LDR[3])/2;//tinh trung binh cong 2 LDR duoi
  avgleft = (LDR[0] + LDR[2])/2;//tinh trung binh cong 2 LDR trai
  avgright = (LDR[1] + LDR[3])/2;//tinh trung binh cong 2 LDR phai
  i=abs(avgtop-avgbot);//tinh gia tri chenh lech giua tbc 2 LDR tren va tbc 2 LDR duoi
  i1=abs(avgleft-avgright);//tinh gia tri chenh lech giua tbc 2 LDR trai va tbc 2 LDR phai
  if (i>100)
  {
    if (avgtop>avgbot)
    {
      if (TA0CCR1<1800) TA0CCR1=TA0CCR1+1;
    }
    else {if (TA0CCR1>800) TA0CCR1=TA0CCR1-1;}
  }
  if (i1>100)
  {
    if (avgleft>avgright)
    {
      if (TA1CCR1<2600) TA1CCR1=TA1CCR1+1;
    }
    else {if (TA1CCR1>500) TA1CCR1=TA1CCR1-1;}
  }
  __bic_SR_register_on_exit(CPUOFF);
}