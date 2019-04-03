#ifndef SW_D
#define SW_D
//obs³uga przyciskow
#define IDLE   0
#define SHORT  1
#define LONG   2
#define REPEAT 3
#define SHORT1 4
#define SHORT2 5

#define idleSW 0
#define shortSW 1
#define longSW 2
#define repeatSW 3

#define tabSize 2
#define tabSize_opk 14
#define tabSize_sen 2

#define TEMP 0
#define TIMER_ON 1
#define FAN 2
#define PROG 3
#define TIMER_OFF 4
#define START 4
#define CTRL 2

//do obslugi brzeczyka bez generatora:
#define OFF_BUZ 0
#define ON_BUZ 1
//do obslugi brzeczyka z generatorem:
#define buzzOn PORTB |=(1<<PB0);
#define buzzOff PORTB &=~(1<<PB0);

//makra dla obslugi wyswietlacza LED
#define DigL0 PD4
#define DigL1 PD5
#define DigL2 PD6
#define DigL3 PD7
#define SW1 PB0
#define SW2 PB1


//-------------------------funkcje obs³ugi przycisków-------------------------------------------------------
uint8_t obslugaPrzycisku(/*int size*/int NrSW, int SW, int Pozition);
uint8_t obslugaPrzyciskuKrotkiego( int NrSW, int SW, int Pozition, int Rep);
uint8_t obslugaPrzyciskuKrotkiego2( int NrSW, int SW, int Pozition, int Rep);
uint8_t obslugaPrzyciskuKrotkiego3(int NrSW, int SW, int Pozition, int Rep, int Rep2);
uint8_t obslugaPrzyciskuKrotkiego4(int NrSW, int SW, int Pozition, int Rep, int Rep2);

//-------------------------funkcje obs³ugi czujnika indukcyjnego-------------------------------------------------------
int obslugaCzujnikaInd(int NrSW, int SW, int Pozition, int Rep);

//-------------------------funkcje ods³ugi diod LED-------------------------------------------------------
void setL(int i);	//zapala poszczegolne diody

void clrL(int i);	//wygasza poszczegolne diody

//-----------------------------------funkcja do mrugania diodami z wypa³nieniem zaleznym od ustawien-------------------------------------
void BlinkLed(int nrLed, volatile int counterL, int timeCycle);


//brzeczyk bez generatora
void buz(int i); //KPM-G1205B resonant frequency 2200-2500Hz

//-------------------------funkcje obs³ugi lamp-------------------------------------------------------
void onLamp1();
void offLamp1();
void onLamp2();
void offLamp2();
void onLamp3();
void offLamp3();




//odczyt stanu peda³a obs³uguj¹cy silnik i uruchomienie silnika w zaleznosci od stanu

int statePed();
int statePed1(uint8_t kindOfWork,volatile uint8_t *softStDisable);
int statePed2();
int statePed1T(int timer,uint8_t kindOfWork,volatile uint8_t *softStDisable);
int statePed2T(int timer);
int statePed2T2(int timer);

// funkcja do multipleksowania diod
void multiplexFunction(/*int mTimer,*/ int mDL, int mDC, int mDP, int mDS/*, int mLamp*/);

// funkcja do ustawiania diod
void setLedFunction(int mDL, int mDC, int mDP, int mDS);

//do obslugi brzeczyka z generatorem:
void BuzzerWork(int timerBuz, int permission);


//-----------------------odbsluga wyswietacz LED----------------------------------------------
//zalaczanie i wylaczanie poszczegolnych cyfr
void onDigL0();
void onDigL1();
void onDigL2();
void onDigL3();
void offDigL0();
void offDigL1();
void offDigL2();
void offDigL3();
//funkcja wylaczajaca segmenty wyswietlacza
void DsLedOff();
//funkcja wysylajaca znaki na dana pozycje
void DsLedSend(char dig0, char dig1, char dig2, char dig3);



	
	
	void RsDataTabRec(unsigned char byte, volatile unsigned char tab[]);
	void RsDataTab(unsigned char byte, volatile unsigned char tab[]);
	void RsShiftTab(volatile unsigned char tab[]);

#endif
