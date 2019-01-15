/*
 * RS485Fn1.h
 *
 * Created: 2016-06-22 14:01:54
 *  Author: 1
 */ 


#ifndef RS485FN1_H_
#define RS485FN1_H_

//makra dla RS485
#define RS_SendM PORTB |= (1<<PB1)
#define RS_SendS PORTD |= (1<<PD5)
#define RS_RecM PORTB &= ~(1<<PB1)
#define RS_RecS PORTD &= ~(1<<PD5)
#define MasterAddr	0x01
#define Slave1Addr 0x02


//-------------------------------transmisja 1 ramki przez Usart 485 Mastera ze sterowaniem kierunku przep³ywu------------------------------------------------------
void USART_SendByteM( unsigned char Byte);
//-------------------------------transmisja 1 ramki przez Usart 485 Slavea ze sterowaniem kierunku przep³ywu------------------------------------------------------
void USART_SendByteS(unsigned char Byte);
//-------------------------------transmisja 1 ramki przez Usart 485 Mastera w zaleznosci od kontrolowanych stanów linii------------------------------------------------------
int USART_SendDataM(unsigned char Byte, int flag);
//-------------------------------transmisja 1 ramki przez Usart 485 Slavea w zaleznosci od kontrolowanych stanów linii------------------------------------------------------
int USART_SendDataS(unsigned char Byte, int flag);


//-------------------------------transmisja 3 ramek przez Usart Mastera------------------------------------------------------
void USART_SendTab(unsigned char RS_SendTab[3]);

//-------------------------------odbior 3 ramek przez Usart Master'a------------------------------------------------------
unsigned char* USART_RecTabM();

//-------------------------------odbior 3 ramek przez Usart Slave'a i nadanie 3 ramek------------------------------------------------------
unsigned char* USART_RecTabS(unsigned char RS_SendTab[]);


#endif /* RS485FN1_H_ */