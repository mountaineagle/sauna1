/*
 * CFile1.c
 *
 * Created: 2016-06-22 14:29:55
 *  Author: 1
 */ 
#include <avr/io.h>
#include "Timers.h"
//---------------------------inicjalizacja obs³ugi przerwañ od timera0---------------------------------
void Timer0Init(int tcnt0_value){
	//TCCR0 |= (1<<CS00) | (1<<CS01); // - ustawienie preskalera na fo/64 z taka f jest taktowany zegar
	TCCR0 |= (1<<CS02);	// - ustawienie preskalera na fo/256 z taka f jest taktowany zegar
	TCCR0 &= ~(1<<CS01);
	TCCR0 &= ~(1<<CS00);
	TCNT0 = tcnt0_value;
	TIMSK |= (1<<TOIE0);
}

//-------------------------inicjalizacja obs³ugi przerwañ od timera2-------------------------------
void Timer2Init(int tcnt2_value){
	TCCR2 &= ~(1<<CS20); // - ustawienie preskalera na ??? z taka f jest taktowany zegar
	TCCR2 |= (1<<CS21);
	TCCR2 |= (1<<CS22);
	TCCR2 &= ~(1<<WGM21);
	TCCR2 &= ~(1<<WGM20);
	TCCR2 &= ~(1<<COM21);
	TCCR2 &= ~(1<<COM20);
	TCNT2=tcnt2_value;//243
	TIMSK |= (1<<TOIE2);
}
