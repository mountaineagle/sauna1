/*
 * MotorFunctions.h
 *
 * Created: 2016-06-22 09:27:42
 *  Author: 1
 */ 


#ifndef MOTORFUNCTIONS_H_
#define MOTORFUNCTIONS_H_
#define katFazVal 90
//------------------------inicjalizacja przerwan INT0 do miekkiego startu------------------------------
void initINT0();
//-------------------------funkcje obs³ugi silnika-------------------------------------------------------
void onS1();
void offS1();
void onS2();
void offS2();
//wylaczanie , zalaczanie i miekki start w zaleznosci od parametru kindOfWork
void Engine1Work(uint8_t kindOfWork, volatile uint8_t *softStDisable);
void Engine2Work(uint8_t kindOfWork, volatile uint8_t *softStDisable);

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

volatile int SoftStartF1(volatile int zeroMarkF,  volatile int *softStDisableF, int imax);
volatile int SoftStopF1(volatile int zeroMarkFF,  volatile int *softStopEnableF, int imaxF);
void SoftStartF2( int imax);
void SoftStopF2( int imaxF);
void SoftStartE1F3(volatile uint8_t *zeroMarkF,  volatile uint8_t *softStDisableF, int imax);
void SoftStartE2F3(volatile uint8_t *zeroMarkF,  volatile uint8_t *softStDisableF, int imax);


//funkcje zalaczenia i wylaczania przekaznika od zmiany kierunku silnika
void offSExt();
void onSExt();

//funkcja do zalaczania przekznika zmieniajacego kierunek pracy silnika z odpowiednimi opoznieniami
uint8_t MotorChangDirect(uint8_t changeMDirectInfo, volatile uint16_t *timerETow, volatile uint16_t *engineTimer, uint8_t handF);



#endif /* MOTORFUNCTIONS_H_ */