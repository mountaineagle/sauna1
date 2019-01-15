/*
 * SPI.c
 *
 * Created: 2016-06-22 14:22:17
 *  Author: 1
 */ 
#include <avr/io.h>
#include "SPI.h"
//-------------------------inicjalizacja SPI-------------------------------------------------------
void SPI_Init(void){
	DDRB |= (1<<PB4) | (1<<PB5) | (1<<PB7); //MOSI, SCK, SS jako wyjscia
	DDRB &= ~(1<<PB6); //MISO jako wejscie
	PORTB &= ~(1<<PB4);  //SS w stan niski
	SPCR |= (1<<SPE) | (1<<MSTR); //|(1<<SPR1) | (1<<SPR0);//SPE- spi enable, MSTR- , SPR1 i SPR0 - ustalene preskalera dla zegara
}
//-------------------------wysylanie przez SPI-----------------------------------------------------
void SPI_Send(char Sdata){
	SPDR = Sdata;
	while(! (SPSR & (1<<SPIF) ));
	PORTB |= (1<<PB4);	//opcjonalnie przy wysylce bajtu do niektórych scalaków
	PORTB &= ~(1<<PB4); //opcjonalnie przy wysylce bajtu do niektórych scalaków
}