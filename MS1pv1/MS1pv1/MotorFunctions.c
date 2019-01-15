/*
 * CFile1.c
 *
 * Created: 2016-06-22 09:12:10
 *  Author: 1
 */ 
#include <avr/io.h>
#include <stdio.h>
#include "SW_Dv2.h"
#include "MotorFunctions.h"

//------------------------inicjalizacja przerwan INT0 potrzebna do miekkiego startu------------------------------
void initINT0(){
	DDRD &= ~(1<<PD2);
	PORTD |= (1<<PD2);
	MCUCR|=(1<<ISC01) | (1<<ISC00); //reakcja na zbocze narastaj¹ce
	//MCUCR&=~(1<<ISC00);
	//GICR|=(1<<INT0);
}
//obsluga silnika
void onSExt(){
	PORTB &= ~(1<<PB1);
}
void offSExt(){
	PORTB |= (1<<PB1);

}
void onS1(){
	PORTC |= (1<<PC6);
}
void offS1(){
	PORTC &= ~(1<<PC6);
}
void onS2(){
	PORTC |= (1<<PC7);
}
void offS2(){
	PORTC &= ~(1<<PC7);
}

void Engine1Work(uint8_t kindOfWork,volatile uint8_t *softStDisable){
	static uint8_t softS=0;
	switch (kindOfWork)
	{
		case 0:
		offS1();
		softS=0;
		*softStDisable=1;
		break;
		case 1:
		onS1();
		softS=0;
		*softStDisable=1;
		break;
		case 2:
		switch(softS){
			case 0:
			*softStDisable=0;
			softS=1;
			case 1:
			if (*softStDisable==1){onS1();}
		}
		break;
	}
}

void Engine2Work(uint8_t kindOfWork, volatile uint8_t *softStDisable){
	static uint8_t softS=0;
	switch (kindOfWork)
	{
		case 0:
		offS2();
		softS=0;
		*softStDisable=1;
		break;
		case 1:
		onS2();
		softS=0;
		*softStDisable=1;
		break;
		case 2:
		switch(softS){
			case 0:
			*softStDisable=0;
			softS=1;
			case 1:
			if (*softStDisable==1){onS2();}
		}
		break;
	}
}


/*funkcja ma za zadanie sterowaæ zewnêtrznym przekaŸnikiem od zmiany kierunku obrotu silnika. W zale¿noœci od informacji o naciœniêciu przycisku "handF" wprowadza odpowiednie opóŸnienia
miêdzy za³¹czeniem przekaŸnika a zalaczeniem triaka (gdy pojawi sie handF=1), lub miedzy wylaczeniem triaka i wylaczeniem przekaznika (gdy pojawi sie fandF=0) i zwraca informacje,
ze zalaczenie (changeMDirectInfo=1), lub wylaczenie (changeMDirectInfo=0)nastapilo 

przyjmowane argumenty:
- handF  - informacja o nacisnieciu przycisku

przyjmowane  i modyfikowane argumenty:
-changeMDirectInfo - info o aktualnym stanie przekaznika (czy ma zmieniony kierunek=1, czy nie=0);
-*timerETow - licznik/zegar sluzacy do odliczania czasow zwloki przy kolejnych etapach dzialania funkcji;
-*engineTimer licznik/zegar sluzacy do badania czy silnik pracowal w chwili nacisniecia przycisku zmiany kierunku, czy znajdowal sie w spoczynku, licznik ustawiany po to by wymusic prace silnika
jest to uwarunkowane sposobem dzialania funkcji od zalaczania silnika(silnik zalacza sie przez podanie czasu jego dzialania)

zwracane argumenty:
-changeMDirectInfo - info o wylaczeniu lub zalaczeniu przekaznika (po uplynieciu odpowiednich czasow)
*/
uint8_t MotorChangDirect(uint8_t changeMDirectInfo, volatile uint16_t *timerETow, volatile uint16_t *engineTimer, uint8_t handF)
{
	if (/*engine2Timer<5 &&*/ handF==1){
		if (changeMDirectInfo==0) {
			if (*engineTimer>0) {*engineTimer=0;*timerETow=350;}
			else {*timerETow=80;}
			changeMDirectInfo=1;
		}
		else if (*timerETow==40){onSExt();}
		else if (*timerETow==0){*engineTimer=50;}
	}else if(handF==0 && changeMDirectInfo==1){
		if (*timerETow==0){*timerETow=350;*engineTimer=0;}
		if (*timerETow<10){changeMDirectInfo=0;*timerETow=0;}
		if (*timerETow<30){offSExt();}
	}
	return changeMDirectInfo;
}


//------------------------------funkcja obslugi SoftStartu------------------------------------
/*Funkcja powinna byæ wywo³ywana w przerwaniu od timera z czestotliwoscia 10000Hz 
jako argumenty przyjmuje:
- zeroMarkF - informacja o przejsciu przez zero sinusoidy, argument ustawiany na 1 najlepiej w obsludze od przerwania INT zboczem opadajacym
- softStDisableF - zmienna globalna zezwalajaca "zerem" na start obslugi softStartu, ustawiana na 1 przez funkcje SoftStartF() po wykonaniu softstartu, 
- imax - argument pozwalajacy wydluzyc czas trwania softStartu;
Funkcja zwraca argument:
- zeroMarkF, ktory zaraz po wykryciu przejscia przez zero jest w funkcji SoftStartF ustawiany na zero.
Pozostale zmienne:
katFazValF=90- ilosc stopni regulacji k¹towej -  w teorii przy f=10000 mozna przyjac zakres od 0 d0 100, ale testy wykazaly ze najlepiej przyjmowac od 21 - 90*/
/*
volatile int SoftStartF1(volatile int zeroMarkF,  volatile int *softStDisableF, int imax){
	const int katFazValF=90;
	static int i;
	static int countPhaz;
	static int katFazF;
	if(*softStDisableF==0) {
		if (countPhaz==0){offS1();}
		if (zeroMarkF==1){countPhaz=1;zeroMarkF=0;}
		if(countPhaz>0){
			countPhaz++;
			if(countPhaz==katFazF){
				onS1();					
				countPhaz=0;
				i++;		
				if (i==imax){
					i=0;
					katFazF--;
					if(katFazF<=25){*softStDisableF=1;}
				}	
				//for(int j=0; j<=20; j++){} //opoznienie niezbedne dla wylaczenia triaka...nalezy zwiekszyc przy zwiekszeniu zegara uC
				//if (*softStDisableF==0 && countPhaz==0){offS1();}//umieszczenie tu instrukcji uzale¿nione jest mocno od czêstotliwoœci rezonatora, przy wiêkszych f mo¿e wyst¹piæ problem soft startu ze wzglêdu na zbyt krótki czas za³¹czenia triaka (min 5us)
			}
			
			}
			}else{countPhaz=0;katFazF=katFazValF;zeroMarkF=0;i=0;}
			return zeroMarkF;
	}
	
	volatile int SoftStopF1(volatile int zeroMarkFF,  volatile int *softStopEnableF, int imaxF){
		const int katFazValFF=25;
		static int iF;
		static int countPhazF;
		static int katFazFF;
		if(*softStopEnableF==1) {
			//	if (countPhazF==0){offS1();}
			if (zeroMarkFF==1){countPhazF=1;zeroMarkFF=0;}
			if(countPhazF>0){
				countPhazF++;
				if(countPhazF==katFazFF){
					onS1();
					countPhazF=0;
					iF++;
					if (iF==imaxF){
						iF=0;
						katFazFF++;
						if(katFazFF<=95){*softStopEnableF=0;}
					}
					//for(int j=0; j<=20; j++){} //opoznienie niezbedne dla wylaczenia triaka...nalezy zwiekszyc przy zwiekszeniu zegara uC
					if (*softStopEnableF==0 && countPhazF==0){offS1();}//umieszczenie tu instrukcji uzale¿nione jest mocno od czêstotliwoœci rezonatora, przy wiêkszych f mo¿e wyst¹piæ problem soft startu ze wzglêdu na zbyt krótki czas za³¹czenia triaka (min 5us)
				}
				
			}
			}else{countPhazF=0;katFazFF=katFazValFF;zeroMarkFF=0;iF=0;}
			return zeroMarkFF;
		}
	
	
	


void SoftStartF2( int imax){
	const int katFazValF=90;
	static int i;
	static int countPhaz;
	static int katFazF;
	if(softStDisable==0) {
		if (countPhaz==0){offS1();}
		if (zeroMark==1){countPhaz=1;zeroMark=0;}
		if(countPhaz>0){
			countPhaz++;
			if(countPhaz==katFazF){
				onS1();
				countPhaz=0;
				i++;
				if (i==imax){
					i=0;
					katFazF--;
					if(katFazF<=25){softStDisable=1;}
				}
				//for(int j=0; j<=20; j++){} //opoznienie niezbedne dla wylaczenia triaka...nalezy zwiekszyc przy zwiekszeniu zegara uC
				//if (*softStDisableF==0 && countPhaz==0){offS1();}//umieszczenie tu instrukcji uzale¿nione jest mocno od czêstotliwoœci rezonatora, przy wiêkszych f mo¿e wyst¹piæ problem soft startu ze wzglêdu na zbyt krótki czas za³¹czenia triaka (min 5us)
			}
			
		}
		}else{countPhaz=0;katFazF=katFazValF;zeroMark=0;i=0;}
		//return zeroMark;
	}
	
	void SoftStopF2( int imaxF){
		const int katFazValFF=50;
		static int iF;
		static int countPhazF;
		static int katFazFF;
		if(softStopEnable==1) {
			if (countPhazF==0){offS1();}
			if (zeroMark1==1){countPhazF=1;zeroMark1=0;}
			if(countPhazF>0){
				countPhazF++;
				if(countPhazF==katFazFF){
					onS1();
					countPhazF=0;
					iF++;
					if (iF==imaxF){
						iF=0;
						katFazFF++;
						if(katFazFF<=95){softStopEnable=0;}
					}
					//for(int j=0; j<=20; j++){} //opoznienie niezbedne dla wylaczenia triaka...nalezy zwiekszyc przy zwiekszeniu zegara uC
					//if (softStopEnable==0 && countPhazF==0){offS1();}//umieszczenie tu instrukcji uzale¿nione jest mocno od czêstotliwoœci rezonatora, przy wiêkszych f mo¿e wyst¹piæ problem soft startu ze wzglêdu na zbyt krótki czas za³¹czenia triaka (min 5us)
				}
				
			}
			}else{countPhazF=0;katFazFF=katFazValFF;zeroMark1=0;iF=0;}
			//return zeroMark1;
		}
		*/
		void SoftStartE1F3(volatile uint8_t *zeroMarkF,  volatile uint8_t *softStDisableF, int imax){
			const int katFazValF=70;
			static int i;
			static int countPhaz;
			static int katFazF;
			if(*softStDisableF==0) {
				GICR |=(1<<INT0);
				if (countPhaz==0){offS1();}
				if (*zeroMarkF==1){countPhaz=1;*zeroMarkF=0;}
				if(countPhaz>0){
					countPhaz++;
					if(countPhaz==katFazF){
						onS1();
						countPhaz=0;
						i++;
						if (i==imax){
							i=0;
							katFazF--;
							if(katFazF<=30){*softStDisableF=1;GICR &=~(1<<INT0);}
						}
						//for(int j=0; j<=20; j++){} //opoznienie niezbedne dla wylaczenia triaka...nalezy zwiekszyc przy zwiekszeniu zegara uC
						//if (*softStDisableF==0 && countPhaz==0){offS1();}//umieszczenie tu instrukcji uzale¿nione jest mocno od czêstotliwoœci rezonatora, przy wiêkszych f mo¿e wyst¹piæ problem soft startu ze wzglêdu na zbyt krótki czas za³¹czenia triaka (min 5us)
					}
					
				}
				}else{countPhaz=0;katFazF=katFazValF;*zeroMarkF=0;i=0;}
				//return zeroMarkF;
			}
			
			
			void SoftStartE2F3(volatile uint8_t *zeroMarkF,  volatile uint8_t *softStDisableF, int imax){
				const int katFazValF=70;
				static int i;
				static int countPhaz;
				static int katFazF;
				if(*softStDisableF==0) {
					GICR |=(1<<INT0);
					if (countPhaz==0){offS2();}
					if (*zeroMarkF==1){countPhaz=1;*zeroMarkF=0;}
					if(countPhaz>0){
						countPhaz++;
						if(countPhaz==katFazF){
							onS2();
							countPhaz=0;
							i++;
							if (i==imax){
								i=0;
								katFazF--;
								if(katFazF<=30){*softStDisableF=1;GICR &=~(1<<INT0);}
							}
							//for(int j=0; j<=20; j++){} //opoznienie niezbedne dla wylaczenia triaka...nalezy zwiekszyc przy zwiekszeniu zegara uC
							//if (*softStDisableF==0 && countPhaz==0){offS1();}//umieszczenie tu instrukcji uzale¿nione jest mocno od czêstotliwoœci rezonatora, przy wiêkszych f mo¿e wyst¹piæ problem soft startu ze wzglêdu na zbyt krótki czas za³¹czenia triaka (min 5us)
						}
						
					}
					}else{countPhaz=0;katFazF=katFazValF;*zeroMarkF=0;i=0;}
					//return zeroMarkF;
				}

