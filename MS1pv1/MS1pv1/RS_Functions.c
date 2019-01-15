/*
 * RS_Functions.c
 *
 * Created: 2016-06-22 13:03:17
 *  Author: 1
 */ 
#include <avr/io.h>
#include <stdio.h>
#include "SW_Dv2.h"
#include "RS_Functions.h"
//-----------------------------konfiguracja przerwania od odbioru USART---------------------------------------------
void USART_InitInterrupt(){
	UCSRB |=(1<<RXCIE); //przerwanie od wektora USART_RXC_vect
}
//-------------------------inicjalizacja USART-----------------------------------------------------
void USART_Init(unsigned long int baud, long int f_osc){
	unsigned int ubr_cont;
	//obliczanie zawartoœci UBR w zale¿noœci od wymaganej czêstotliwosci transmisji z wyeliminowaniem b³êdu (dodanie 0,5 w odpowiedni sposób):
	ubr_cont=((10*f_osc)/16/baud-5)/10;
	UBRRH = (unsigned char) (ubr_cont>>8);
	UBRRL = (unsigned char) ubr_cont;
	//zezwolenie na transmisje i odbior:
	UCSRB |= (1<<RXEN) | (1<<TXEN);
	//ustawienia ramki ursel - aby mozliwe bylo modyfikowanie ucsr, ktory ma wspolny adres z rejestrem ubrrh, usbs = 1 - 2 bity stopu , ucsz=3 - 8 bitow danych
	UCSRC |= (1<<URSEL) | (1<<USBS) | (3<<UCSZ0);
}
//-------------------------transmisja danych przez USART-------------------------------------------
void USART_Send(unsigned char data){
	while (!(UCSRA &(1<<UDRE)));
	UDR=data;
}
unsigned char USART_Receive(void){
	while(!(UCSRA &(1<<RXC)));
	return UDR;
}