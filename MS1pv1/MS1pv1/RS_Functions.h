/*
 * RS_Functions.h
 *
 * Created: 2016-06-22 13:03:37
 *  Author: 1
 */ 


#ifndef RS_FUNCTIONS_H_
#define RS_FUNCTIONS_H_
//-----------------------------konfiguracja przerwania od odbioru USART---------------------------------------------
void USART_InitInterrupt(void);

//-------------------------inicjalizacja USART-----------------------------------------------------
void USART_Init(unsigned long int baud, long int f_osc);

//-------------------------transmisja danych przez USART-------------------------------------------
void USART_Send(unsigned char data);
unsigned char USART_Receive(void);



#endif /* RS_FUNCTIONS_H_ */