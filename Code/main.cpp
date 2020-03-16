/*
 * Motor_Driver_Rev-2.cpp
 *
 * Created: 3/4/2020 3:40:42 PM
 * Author : RHITVIK
 */ 

/*
 * Motor_Driver_Code_Rev-1.cpp
 *
 * Created: 3/4/2020 3:37:33 AM
 * Author : RHITVIK
 */ 

/*
Program Memory Usage 	:	500 bytes/1024 bytes   48.8 % Full
Data Memory Usage 		:	7 bytes/32 bytes	   21.9 % Full
*/

#define F_CPU	8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

//		        +====+
//	PB0(PWM Pin-1) |*     | PB3 (RESET)
//	           GND |      | VCC
//	PB1(PWM Pin-2) |      | PB2
//	 	        +====+
//	           Attiny10 Pin out

volatile uint8_t mode = 1;
volatile uint8_t ovfCnt = 0;
volatile uint8_t ovfCnt1 = 0;
volatile uint8_t SecElp = 0;
volatile bool shiftDuty = 0;
volatile bool FiveSecElp = false;

// - Ramp up the motor from 0% to 100% forward over 5 seconds.
// - Ramp down the motor from 100% forward to 0% over 5 seconds.
// - Ramp up the motor from 0% to 100% reverse over 5 seconds.
// - Ramp down the motor from 100% reverse to 0% over 5 seconds.
void Ramp_Maneuver(void);
void Initialize_PWM_params(void);	// will initialize PWM Params
void turnON_PWMpin1(void);			// turn on PWM pin1
void turnON_PWMpin2(void);			// turn on PWM pin2
void turnOFF_PWMpin1(void);			// turn off PWM pin1
void turnOFF_PWMpin2(void);			// turn off PWM pin2

int main(void)
{
	CCP = 0xD8;		// Unlock special consideration register
	CLKPSR = 0x00;	// Set the system clock to 8 MHz
	
	// sei();	// Enable Global Interrupts
	// may or may not have been enabled in the first place 
	
	Initialize_PWM_params(); 
	// Initializes general PWM parameters
	// but does not make PWM pins to start modulate right away
	// interrupts overflow has started
	
	Ramp_Maneuver();
	
	while(1);

}	//int main()

void Ramp_Maneuver(void){
	bool latch = 0;
	bool fifty_ms_del = 0; // we will compare the value of this with shiftduty in ISR to change duty value
	// mode 1 -->  Ramp up the motor from 0% to 100% forward over 5 seconds.
	// mode 2 -->  Ramp down the motor from 100% forward to 0% over 5 seconds.
	// mode 3 -->  Ramp up the motor from 0% to 100% reverse over 5 seconds.
	// mode 4 -->  Ramp down the motor from 100% reverse to 0% over 5 seconds.
	while(2){
		if (FiveSecElp == true)
		{
			FiveSecElp = false;
			mode = mode+1;
			latch = (latch == true)? false:latch; // set latch to false
			if (mode > 4)
			{
				turnOFF_PWMpin1();
				turnOFF_PWMpin2();
				break;
			}
		}
		else if ((mode == 1) && (fifty_ms_del != shiftDuty))
		{
			fifty_ms_del = shiftDuty; // make the values same so that it has to wait another 20 ms before entering the loop
			// turn off PWM pin 2 to ensure safety
			// turn on PWM pins 1 after PWM 2 is off
			// compare fifty_ms_del with shiftDuty
			if (latch == false)
			{
				latch = true;
				turnOFF_PWMpin2();
				turnON_PWMpin1();
			}
			OCR0A = OCR0A + 100; //this will increase the duty of PWM pin 1 by 1%
		}
		else if ((mode == 2) && (fifty_ms_del != shiftDuty))
		{
			fifty_ms_del = shiftDuty; // make the values same so that it has to wait another 20 ms before entering the loop
			// turn off PWM pin 2 to ensure safety
			// turn on PWM pins 1 after PWM 2 is off
			// compare fifty_ms_del with shiftDuty
			if (latch == false)
			{
				latch = true;
				turnOFF_PWMpin2();
				turnON_PWMpin1();
			}
			OCR0A = OCR0A - 100; //this will increase the duty of PWM pin 1 by 1%
		}
		else if ((mode == 3) && (fifty_ms_del != shiftDuty))
		{
			fifty_ms_del = shiftDuty; // make the values same so that it has to wait another 20 ms before entering the loop
			// turn off PWM pin 1 to ensure safety
			// turn on PWM pins 2 after PWM 1 is off
			// compare fifty_ms_del with shiftDuty
			if (latch == false)
			{
				latch = true;
				turnOFF_PWMpin1();
				turnON_PWMpin2();
			}
			OCR0B = OCR0B + 100; //this will increase the duty of PWM pin 2 by 1%
		}
		else if ((mode == 4) && (fifty_ms_del != shiftDuty))
		{
			fifty_ms_del = shiftDuty; // make the values same so that it has to wait another 20 ms before entering the loop
			// turn off PWM pin 1 to ensure safety
			// turn on PWM pins 2 after PWM 1 is off
			// compare fifty_ms_del with shiftDuty
			if (latch == false)
			{
				latch = true;
				turnOFF_PWMpin1();
				turnON_PWMpin2();
			}
			OCR0B = OCR0B - 100; //this will decrease the duty of PWM pin 2 by 1% 
		}
	}
	
}

ISR(TIM0_OVF_vect){ // Is invoked when counter reaches ICR top (as set by WGM settings)
	// Here, 10 milliseconds have elapsed	
	ovfCnt++;
	if(ovfCnt >= 5){
		// 50 milliseconds have elapsed
		ovfCnt = 0; // set the ovfCnt to 0
		shiftDuty ^= shiftDuty; // invert shiftDuty to be read outside ISR
		ovfCnt1++;		
		if (ovfCnt1 >= 20){
			ovfCnt1 = 0; // set the ovfCnt1 to 0
			// Here, 1 second has elapsed!!
			SecElp++;
			if (SecElp >= 5){
				SecElp = 0;
				//Here, 5 seconds have elapsed
				FiveSecElp = true;
			}
		}
	}
}

void Initialize_PWM_params(void){
	cli();	//clear global interrupts
	
	DDRB |= (1U<<PINB0) | (1U<<PINB1);
	//set pin B0, B1 and as outputs irrespective of their previous pin function
	
	PORTB &= 0x0C;
	// Setting pins to low without touching reset and Pin B2
	// can also be 0x04 but not recommended as sometimes Reset Pin is used as a GPIO pin
	// clear PWM pins irrespective of their previous state
	
	//DDRB &= ~(1U<<PINB3);
	//PUEB |= (1U<<PUEB3);
	// Pull up on Pin B3
	//not needed unless fuse settings altered for RST pin to be used as GPIO
	
	TCCR0A |= (1U<<WGM01);
	TCCR0B |= (1U<<WGM03) | (1U<<WGM02);
	// set the waveform mode to Fast PWM (16 bit registers are now accessible)
	
	ICR0 = 10000;
	// counter resets itself to 0 when it reaches 10000
	// will generate a PWM frequency of 100 Hz @ 1MHz via prescalar selection
	
	TCCR0B |= (1U<<CS01);
	// set pre-scalar to 8
	// the controller will count and fill the register at 1MHz
	
	//TCCR0A |= (1U<<COM0A1) | (1U<<COM0B1);
	// Clear OC0A/OC0B on compare match
	
	TIMSK0 |= (1U<<TOIE0);
	//Set Timer Overflow Interrupt Enable
	
	sei();	// enable global interrupts
}

void turnON_PWMpin1(void){
	cli();
	TCCR0A |= (1U<<COM0A1);		// Clear OC0A pin on compare match
	sei();
}
void turnON_PWMpin2(void){
	cli();
	TCCR0A |= (1U<<COM0B1);		// Clear OC0B pin on compare match
	sei();
}

void turnOFF_PWMpin1(void){
	TCCR0A &= ~(1U<<COM0A1);		// Disable PWM on OC0A pin
	PORTB &= ~(1<<PINB0);		// Clear the pinB0
}
void turnOFF_PWMpin2(void){
	TCCR0A &= ~(1U<<COM0B1);		// Disable PWM on OC0B pin
	PORTB &= ~(1<<PINB1);		// Clear the pinB1
}
