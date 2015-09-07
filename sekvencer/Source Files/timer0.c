/*
 * timer0.c
 *
 * Created: 18. 6. 2015 21:54:09
 *  Author: Martin
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "program.h"
#include "timer0.h"


static volatile uint32_t _msToWait = 0;


ISR(TIMER0_COMP_vect)
{
	cli();
	_msToWait--;
	if (_msToWait == 0) {
		program_execute_instruction();
		if (_msToWait == 0) {
			TIMSK &= ~((1<< OCIE0) | (1 << TOIE0));
			TCCR0 = 0;
			OCR0 = 0;			
		}
	}
	
	sei();
	
	return;
}

void timer0_start(const uint32_t Miliseconds)
{
	cli();
	_msToWait = Miliseconds;
	TCCR0 = (1 << WGM01);
	OCR0 = 0xf9;
	TCNT0 = 0;
	TIMSK |= (1 << OCIE0); 
	TCCR0 |= (1 << CS02);
	sei();
	
	return;
}

void timer0_plan_next(const uint32_t Miliseconds)
{
	_msToWait = Miliseconds;
	
	return;
}

void timer0_stop(void)
{
	cli();
	TIMSK &= ~((1<< OCIE0) | (1 << TOIE0));
	TCCR0 = 0;
	OCR0 = 0;
	_msToWait = 0;
	sei();
	
	return;
}
