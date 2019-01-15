/*
 * RS485Fn.c
 *
 * Created: 2016-06-22 14:01:36
 *  Author: 1
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include "RS_Functions.h"
#include "RS485Fn.h"

//-------------------------------transmisja 3 ramek przez Usart 485 Mastera------------------------------------------------------
void USART_SendTab(unsigned char RS_SendTab[3]){//zrobic w obsludze przerwania co 150ms
	RS_SendM;//zmiana na nadawanie
	for (int i=0; i<=2; i++){
		USART_Send(RS_SendTab[i]);
	}
	while (!(UCSRA & (1<<TXC))); //- rozwiazanie z TXC nie dziala
	UCSRA |= (1<<TXC);//bit TXC ma nietypow¹ w³aœciwoœc - po jego ustawieniu(na 1) w chwili wys³ania bajtu, nale¿y programowo wpisaæ do niego jedynkê , aby go skasowaæ(ustawiæ na zero)!!!!!!
	RS_RecM ;//zmiana na odbiór !!!!!!!!!!!!!! tu zbadaæ czy nie za szybko
	RS_SendTab[1]=0x00;
	RS_SendTab[2]=0x00;
}

//-------------------------------transmisja 1 ramki przez Usart 485 Mastera ze sterowaniem kierunku przep³ywu------------------------------------------------------
void USART_SendByteM(unsigned char Byte){
	
	RS_SendM;//kontrola przepywu danych na MAX485 - zmiana na nadawanie
	USART_Send(Byte);
	while (!(UCSRA & (1<<TXC)));
	UCSRA |= (1<<TXC);//bit TXC ma nietypow¹ w³aœciwoœc - po jego ustawieniu(na 1) w chwili wys³ania bajtu, nale¿y programowo wpisaæ do niego jedynkê , aby go skasowaæ(ustawiæ na zero)!!!!!!
	RS_RecM ;//kontrola przepywu danych na MAX485 - zmiana na odbiór po zbadaniu bitu TXC
}


//-------------------------------transmisja 1 ramki przez Usart 485 Slavea ze sterowaniem kierunku przep³ywu------------------------------------------------------
void USART_SendByteS(unsigned char Byte){
	
	RS_SendS;//kontrola przepywu danych na MAX485 - zmiana na nadawanie
	USART_Send(Byte);
	
	while (!(UCSRA & (1<<TXC)));
	UCSRA |= (1<<TXC);//bit TXC ma nietypow¹ w³aœciwoœc - po jego ustawieniu(na 1) w chwili wys³ania bajtu, nale¿y programowo wpisaæ do niego jedynkê , aby go skasowaæ(ustawiæ na zero)!!!!!!
	//_delay_us(100);
	RS_RecS ;//kontrola przepywu danych na MAX485 - zmiana na odbiór po zbadaniu bitu TXC
}

//-------------------------------transmisja 1 ramki przez Usart 485 Mastera------------------------------------------------------
/*Funkcja do wysy³ania bajtu danych w chwili gdy zadzieje siê dane zdarzenie,
wyposa¿ona w opcje sprawdzania stanu linii od slavea (linia PD3), funkcje wystawienia "1" jako znacznika nadawania dla Slave'a na porcie PD2, oraz zerowanie flafi flaga
przyjmuje argumenty Byte - bajt danych;
flag - flaga zezwolenia na wysylke danych ustawiana od konkretnego zdarzenia
funkcja wywolywana w przerwaniu od timera najlepej z f=1000 lub wieksza*/

int USART_SendDataM(unsigned char Byte,  int flag){//zrobic w obsludze przerwania
	if (flag==1){
		if (!(PIND & (1<<PD3))){
			PORTD |= (1<<PD2);
			USART_SendByteM(Byte);
			PORTD &= ~(1<<PD2);
			flag=0;
		}
	}
	return flag;
}

/*Funkcja do wysy³ania bajtu danych w chwili gdy zadzieje siê dane zdarzenie,
wyposa¿ona w opcje sprawdzania stanu linii od slavea (linia PD3), funkcje wystawienia "1" jako znacznika nadawania dla Slave'a na porcie PD2, oraz zerowanie flafi flaga
przyjmuje argumenty Byte - bajt danych;
flag - flaga zezwolenia na wysylke danych ustawiana od konkretnego zdarzenia
funkcja wywolywana w przerwaniu od timera najlepej z f=1000 lub wieksza*/

int USART_SendDataS(unsigned char Byte, int flag){//zrobic w obsludze przerwania
	if (flag==1){
		if (!(PIND & (1<<PD3))){
			PORTD |= (1<<PD4);
			USART_SendByteS(Byte);
			PORTD &= ~(1<<PD4);
			flag=0;
		}
	}
	return flag;
}

//-------------------------------odbior 3 ramek przez Usart 485 Master'a------------------------------------------------------
unsigned char* USART_RecTabM(){         //w przerwaniu od nadania usart przerwanie od wektora USART_RXC_vect
	static int rsWordCount=0;
	unsigned char RS_RecTab[3]={0};
	if (rsWordCount==0){
		RS_RecTab[0]=UDR;
	}
	else if(RS_RecTab[0]==0x01){
		RS_RecTab[rsWordCount]=UDR;
	}
	rsWordCount++;
	if (rsWordCount==3){
		rsWordCount=0;
	}
	return RS_RecTab;
}


//-------------------------------odbior 3 ramek przez Usart Slave'a i nadanie 3 ramek------------------------------------------------------
unsigned char* USART_RecTabS(unsigned char RS_SendTab[]){	//przerwanie od wektora USART_RXC_vect
	static int rsWordCount=0;
	unsigned char RS_RecTab[3]={0};
	if (rsWordCount==0){
		RS_RecTab[0]=UDR;
	}
	else if(RS_RecTab[0]==0x02){
		RS_RecTab[rsWordCount]=UDR;
	}
	rsWordCount++;
	if (rsWordCount==3){
		rsWordCount=0;
		RS_SendS;
		USART_SendTab(RS_SendTab);
		RS_RecS; //!!!!!!!!!!!!!! tu zbadaæ czy nie za szybko
	}
	return RS_RecTab;
}
