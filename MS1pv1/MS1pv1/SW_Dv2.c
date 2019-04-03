#include <avr/io.h>
#include "SW_Dv2.h"
#include "MotorFunctions.h"
#include "SPI.h"

uint8_t obslugaPrzycisku(/*int size, */int NrSW, int SW, int Pozition){	

	static int tab1[tabSize];
	static int countSwitch[tabSize];
	uint8_t status_u8 = IDLE;
	if (!(SW & Pozition))
	{
	countSwitch[NrSW]++;
	switch (tab1[NrSW])
		{
			case idleSW :
			if(countSwitch[NrSW]==50)
			{
				tab1[NrSW]=shortSW;
			}
			break;

			case shortSW:
			if(countSwitch[NrSW]==400)
			{
				tab1[NrSW]=longSW; 
			}
			break;
		
			case longSW:
			if(countSwitch[NrSW]==1000)
			{
				tab1[NrSW]=repeatSW;
				status_u8 = REPEAT;
			}
			break;

			case repeatSW:
				tab1[NrSW]=repeatSW;
				status_u8 = REPEAT;
			break;
		}
	}
	else if (tab1[NrSW]==shortSW)
	{	
		tab1[NrSW]=idleSW;
		countSwitch[NrSW]=0;
		status_u8 = SHORT;
	}

	else if (tab1[NrSW]==longSW)
	{
		tab1[NrSW]=idleSW;
		countSwitch[NrSW]=0;
		status_u8 = LONG;
	}

	else
	{	
		tab1[NrSW]=idleSW;
		countSwitch[NrSW]=0;
		status_u8 = IDLE;
	}
	
	return status_u8;

}

//obsluga krotkich przycisniec
//funkcja wyrzuca informacje o nacisnieciu przycisku przez caly okres jego przyciskania
uint8_t obslugaPrzyciskuKrotkiego(int NrSW, int SW, int Pozition, int Rep){	

//	static int tab_opk[tabSize_opk];
	static long int countSwitch_opk[tabSize_opk];
	uint8_t status_u8 = IDLE;
	if (!(SW & Pozition))
	{		
		if(countSwitch_opk[NrSW]>Rep)
		{
			//	countSwitch_opk[NrSW]=0;
			status_u8 = SHORT;
		}
		else 
		{
			countSwitch_opk[NrSW]++;
		}

	}
	else
	{	
		countSwitch_opk[NrSW]=0;
		status_u8 = IDLE;
	}
	
	return status_u8;

}
/*funkcja tylko raz wyrzuca informacje o nacisnieciu przycisku

przyjmuje nastêpuj¹ce argumenty:
NrSw - numer danego przycisku(jednoczeœni jego pozycja w tworzonej przez funkcjê tablicy przycisków)
SW - port na którym jest dany przycisk
Pozition - pozycja przycisku w wektorze portów wejœciowych mikrokontrolera
Rep - czas tzw. debouncingu
Funkcja zwraca wartoœæ IDLE (0), gdy nie zosta³ przyciœniêty przycisk, natomiast wartosc SHORT (1) , gdy przyciœniêto przycisk d³u¿ej ni¿ czas debouncingu (Rep) ,
przy czym wartosc SHORT jest zwracana tylko raz
Nale¿y ustaliæ wartoœc makra tabSize_opk na tak¹ ile jest obs³ugiwanych przycisków
funkcjê wywo³ujemy w przerwaniu i pobieramy jej wartosc przyrownujac do zmiennej globalnej 
*/
uint8_t obslugaPrzyciskuKrotkiego2(int NrSW, int SW, int Pozition, int Rep){
	uint8_t returnM=0;
	static long int countSwitch_opk[tabSize_opk];
	static int swMark[tabSize_opk];
	if (!(SW & Pozition))
	{
		
		if(countSwitch_opk[NrSW]>Rep && swMark[NrSW]==0)
		{
			swMark[NrSW]=1; //tylko raz wyrzuca infomacje o naciœniêciu
			returnM = SHORT;
			
		}
		else 
		{
			countSwitch_opk[NrSW]++;
			returnM = IDLE;
		}

	}
	else
	{
		countSwitch_opk[NrSW]=0;
		swMark[NrSW]=0;
		returnM=IDLE;
	}
	
	return returnM;////////??????????????????

}

/*funkcja po up³ywie czasu Rep cyklicznie wyrzuca informacje o nacisnieciu przycisku z czasem powtarzania równym parametrowi Rep2

przyjmuje nastêpuj¹ce argumenty:
NrSw - numer danego przycisku(jednoczeœni jego pozycja w tworzonej przez funkcjê tablicy przycisków)
SW - port na którym jest dany przycisk
Pozition - pozycja przycisku w wektorze portów wejœciowych mikrokontrolera
Rep - czas tzw. debouncingu
Rep2 - czas powtarzania
Funkcja zwraca wartoœæ IDLE (0), gdy nie zosta³ przyciœniêty przycisk, natomiast wartosc SHORT (1) , gdy przyciœniêto przycisk d³u¿ej ni¿ czas debouncingu (Rep) ,
przy czym wartosc SHORT jest zwracana co czas przejscia wartosci Rep
Nale¿y ustaliæ wartoœc makra tabSize_opk na tak¹ ile jest obs³ugiwanych przycisków
funkcjê wywo³ujemy w przerwaniu i pobieramy jej wartosc przyrownujac do zmiennej globalnej
*/
uint8_t obslugaPrzyciskuKrotkiego3(int NrSW, int SW, int Pozition, int Rep, int Rep2){
	uint8_t returnM=0;
	static long int countSwitch_opk[tabSize_opk];
	static long int countSwitch2_opk[tabSize_opk];
	if (!(SW & Pozition))
	{
		//if (countSwitch_opk[NrSW]<(Rep+2)){countSwitch_opk[NrSW]++;}
		if(countSwitch_opk[NrSW]>Rep)
		{
			if (countSwitch2_opk[NrSW]>Rep2)
			{
				countSwitch2_opk[NrSW]=0;
				returnM = SHORT;
			}
			else 
			{
				countSwitch2_opk[NrSW]++;
				returnM = IDLE;
			}
			
		}
		else 
		{
			countSwitch_opk[NrSW]++;
		}
	}
	else
	{
		countSwitch2_opk[NrSW]=0;
		countSwitch_opk[NrSW]=0;
		returnM=IDLE;
	}
	
	return returnM;////////??????????????????

}


/*funkcja, ktora przyspiesza inkrementacje w  zaleznosci od czasu trzymania, cyklicznie wyrzuca informacje o nacisnieciu przycisku z czasem powtarzania równym parametrowi Rep2

przyjmuje nastêpuj¹ce argumenty:
NrSw - numer danego przycisku(jednoczeœni jego pozycja w tworzonej przez funkcjê tablicy przycisków)
SW - port na którym jest dany przycisk
Pozition - pozycja przycisku w wektorze portów wejœciowych mikrokontrolera
Rep - czas tzw. debouncingu
Rep2 - czas powtarzania
Funkcja zwracanatepujace argument returnM ktory przyjmuje nastepujace wartosci:
- IDLE (0), gdy nie zosta³ przyciœniêty przycisk, natomiast wartosc 
- SHORT (1) , gdy przyciœniêto przycisk d³u¿ej ni¿ czas debouncingu (Rep) ,
- SHORT1 (?) , gdy trzymamy przycisk dluzej niz 10xRep2+Rep
- SHORT2 (?), gdy trzymamy przycisk dluzej niz 15xRep2+Rep
przy czym wartosc SHORT jest zwracana co czas przejscia wartosci Rep
Nale¿y ustaliæ wartoœc makra tabSize_opk na tak¹ ile jest obs³ugiwanych przycisków
funkcjê wywo³ujemy w przerwaniu i pobieramy jej wartosc przyrownujac do zmiennej globalnej
*/


uint8_t obslugaPrzyciskuKrotkiego4(int NrSW, int SW, int Pozition, int Rep, int Rep2){
	uint8_t returnM=0;
	static long int countSwitch_opk[tabSize_opk];
	static long int countSwitch2_opk[tabSize_opk];
	if (!(SW & Pozition))
	{
		//if (countSwitch_opk[NrSW]<(Rep+2)){countSwitch_opk[NrSW]++;}
		if (countSwitch_opk[NrSW]>(Rep+Rep2*17))
		{
			countSwitch_opk[NrSW]=Rep+Rep2*16;
		} //-zabezpieczenie przed przekroczeniem zakresu long int w przypadku dlugiego naciskania przycisku
		if (countSwitch_opk[NrSW]>(Rep+Rep2*15))
		{
			if (countSwitch2_opk[NrSW]>Rep2)
			{
				countSwitch2_opk[NrSW]=0;
				returnM = SHORT2;
			}
			else 
			{
				countSwitch2_opk[NrSW]++;
				returnM = IDLE;
			}
		}	
		else if (countSwitch_opk[NrSW]>(Rep+Rep2*10))
		{
			if (countSwitch2_opk[NrSW]>Rep2)
			{
				countSwitch2_opk[NrSW]=0;
				returnM = SHORT1;
			}
			else 
			{
				countSwitch2_opk[NrSW]++;
				returnM = IDLE;
			}
		}
		else if(countSwitch_opk[NrSW]>Rep)
		{
			if (countSwitch2_opk[NrSW]>Rep2)
			{
				countSwitch2_opk[NrSW]=0;
				returnM = SHORT;
			}
			else 
			{
				countSwitch2_opk[NrSW]++;
				returnM = IDLE;
			}
		}//else {
			countSwitch_opk[NrSW]++;
		//}

	}
	else
	{
		countSwitch2_opk[NrSW]=0;
		countSwitch_opk[NrSW]=0;
		returnM=IDLE;
	}
	
	return returnM;////////??????????????????

}

/*funkcja przez ca³y czas trzymania wyrzuca informacje o nacisnieciu przycisku/czujnika w przypadku nacisniecia podaje stan "1" na wejscie uC

przyjmuje nastêpuj¹ce argumenty:
NrSw - numer danego przycisku/czujnika(jednoczeœni jego pozycja w tworzonej przez funkcjê tablicy przycisków/czujnikow)
SW - port na którym jest dany przycisk/czujnik
Pozition - pozycja przycisku/czujnika w wektorze portów wejœciowych mikrokontrolera
Rep - czas tzw. debouncingu
Funkcja zwraca wartoœæ IDLE (0), gdy nie zosta³ przyciœniêty przycisk, natomiast wartosc SHORT (1) , gdy przyciœniêto przycisk d³u¿ej ni¿ czas debouncingu (Rep) ,
przy czym wartosc SHORT jest zwracana tylko raz
Nale¿y ustaliæ wartoœc makra tabSize_opk na tak¹ ile jest obs³ugiwanych przycisków
funkcjê wywo³ujemy w przerwaniu i pobieramy jej wartosc przyrownujac do zmiennej globalnej
*/
int obslugaCzujnikaInd (int NrSW, int SW, int Pozition, int Rep){
		int returnM=0;

		//	static int tab_opk[tabSize_opk];
		static int countSwitch_opk[tabSize_sen];

		if (SW & Pozition)
		{
			
			if(countSwitch_opk[NrSW]>Rep)
			{
				//	countSwitch_opk[NrSW]=0;
				returnM = SHORT;
			}else {countSwitch_opk[NrSW]++;}

		}
		else
		{
			countSwitch_opk[NrSW]=0;
			returnM = IDLE;
		}
		
		return returnM;

	}


//diody

void setL(int i){	//zapala poszczegolne diody

	switch (i)
	{
		case TEMP:
			//PORTC |= (1<<PC4);
			PORTC |= (1<<PC0);
		break;
		case TIMER_ON://MENU
			PORTC |= (1<<PC2);
		break;
		case FAN://TIME
			PORTC |= (1<<PC3);
		break;
		case PROG://LAMP
			//PORTC |= (1<<PC0);
			PORTC |= (1<<PC4);
		break;
		case TIMER_OFF://HOT
			PORTC |= (1<<PC1);
		break;
	}


}

void clrL(int i){	//wygasza poszczegolne diody

	switch (i)
	{
		case TEMP:
			//PORTC &= ~(1<<PC4);
			PORTC &= ~(1<<PC0);
		break;
		case TIMER_ON://TIMER_ON
			PORTC &= ~(1<<PC2);
		break;
		case FAN://FAN
			PORTC &= ~(1<<PC3);
		break;
		case PROG://PROG
			//PORTC &= ~(1<<PC0);
			PORTC &= ~(1<<PC4);
		break;
		case TIMER_OFF://TIMER_OFF
			PORTC &= ~(1<<PC1);
		break;
	}
}


//brzeczyk bez generatora, nale¿y tu naprzmiennie za³¹czaæ i wy³¹czaæ buzzer z okreœlon¹ czêstotliwoœci¹ (i=1 lub i=0)
//funkcja umieszczana w obs³udze przerwania, dla zachowania idealnej czestotliwosci 
void buz(int i){  //KPM-G1205B resonant frequency 2200-2500Hz

	switch (i)
	{
		case OFF_BUZ:
			PORTB &= ~(1<<PB6);
		break;
		case ON_BUZ:
			PORTB |= (1<<PB6);
		break;
	}
}

//Lampa
void onLamp1(){
	PORTA |= (1<<PA4);
	}

void offLamp1(){
	PORTA &= ~(1<<PA4);
	}
	
//Lampa2
void onLamp2(){
	PORTA |= (1<<PA5);
}

void offLamp2(){
	PORTA &= ~(1<<PA5);
}

//Lampa3
void onLamp3(){
	PORTA |= (1<<PA6);
}

void offLamp3(){
	PORTA &= ~(1<<PA6);
}




//odczyt stanu peda³a obs³uguj¹cy silnik i uruchomienie silnika w zaleznosci od stanu

int statePed(){
	static int stateP=0;
	static int muxDC=0;
	if (!(PINB & 0x01)){
	stateP++;
		if(stateP>3000){	
		onS1();
		//setL(CTRL);
		muxDC=1;
		stateP=0;
		}
	}else {
//	stateP++;
//	if(stateP>50){
	stateP=0;//tu przebadac
	offS1();
//	clrL(CTRL);
	muxDC=0;
//	}
	}
	return muxDC;
}

/*
int statePed1(){
	static int stateP=0;
	static int muxDC=0;
	if (!(PINA & (1<<PA1))){
		stateP++;
		if(stateP>5000){
			onS1();
			stateP=0;
			muxDC=1;
		}
		}else {
		stateP=0;//tu przebadac
		offS1();
		muxDC=0;
	}
	return muxDC;
}
*/
int statePed2(){
	static int muxDC=0;
	static int stateP=0;
	if (!(PINA & (1<<PA0))){
		stateP++;
		if(stateP>5000){
			onS2();
			stateP=0;
			muxDC=1;
		}
		}else {
		stateP=0;//tu przebadac
		offS2();
		muxDC=0;
	}
	return muxDC;
}
/*
int statePed1T(int timer){
	static int stateP=0;
	static int muxDC=0;
	if (!(PINA & (1<<PA1))){
		stateP++;
		if(stateP>5000){
			onS1();
			stateP=0;
			muxDC=1;
		}
		}else if ((PINA & (1<<PA1)) && timer>0 ){
		onS1();
		muxDC=2;
		
	}
	else {
		stateP=0;//tu przebadac
		offS1();
		muxDC=0;
	}
	return muxDC;
}

*/


int statePed2T(int timer){
	static int muxDC=0;
	static int stateP=0;
	if (!(PINA & (1<<PA0))){
		stateP++;
		if(stateP>5000){
			onS2();
			stateP=0;
			muxDC=1;
		}
		}else if ((PINA & (1<<PA0)) && timer>0 ){
		onS2();
		muxDC=2;
		
	}
		else {
		stateP=0;//tu przebadac
		offS2();
		muxDC=0;
	}
	return muxDC;
}

int statePed1( uint8_t kindOfWork,volatile uint8_t *softStDisable){
	static int stateP=0;
	static int muxDC=0;
	if (!(PINA & (1<<PA1))){
		stateP++;
		if(stateP>5000){
			Engine1Work(kindOfWork,softStDisable);
			stateP=0;
			muxDC=1;
		}
		}else {
		stateP=0;//tu przebadac
		Engine1Work(0,softStDisable);
		muxDC=0;
	}
	return muxDC;
}


int statePed1T(int timer,uint8_t kindOfWork,volatile uint8_t *softStDisable){
	static int stateP=0;
	static int muxDC=0;
	if (!(PINA & (1<<PA1))){
		stateP++;
		if(stateP>5000){
			Engine1Work(kindOfWork,softStDisable);
			stateP=0;
			muxDC=1;
		}
		}else if ((PINA & (1<<PA1)) && timer>0 ){
		Engine1Work(kindOfWork,softStDisable);
		muxDC=2;
		
	}
	else {
		stateP=0;//tu przebadac
		Engine1Work(0,softStDisable);
		muxDC=0;
	}
	return muxDC;
}

/*
int SoftStart(int zezwolenie, softStDisable){
	if (zezwolenie==1 && coss==0){softStDisable=0;onS1();}
	if (zezwolenie==2){softStDisable=1;onS1();}
	if (zezwolenie==0){softStDisable=1; offS1();}
	return zezwolenie
}*/



int statePed2T2(int timer){
	static int muxDC=0;
	static int stateP=0;
	if (!(PINA & (1<<PA1))){
		stateP++;
		if(stateP>5000){
			onS2();
			stateP=0;
			muxDC=1;
		}
		}else if ((PINA & (1<<PA1)) && timer>0 ){
		onS2();
		muxDC=2;
		
	}
	else {
		stateP=0;//tu przebadac
		offS2();
		muxDC=0;
	}
	return muxDC;
}


/*-----------------------------------funkcja do mrugania diodami z wypa³nieniem zaleznym od ustawien-------------------------------------
jako argumenty przyjmuje:
nrLed - led którym chcemy migaæ;
counterL - zmienna globalna inkrementowana w przerwaniu od timera;
timeCycle - zmienna sluzaca do ustawien wypelnienia, musi byc scisle powiazana(krotsza) z wartoscia zmiennej counterL;
*/
void BlinkLed(int nrLed, volatile int counterL, int timeCycle){
	if (counterL>=timeCycle) clrL(nrLed);
	if (counterL<timeCycle) setL(nrLed);
}

//funkcja do multipleksowania diod
void multiplexFunction(/*int mTimer, */int mDL, int mDC, int mDP, int mDS/*, int mLamp*/){
	static int mTimer=0;
	mTimer++;
	if(mTimer==4){mTimer=0;}

/*	if(mLamp){
		if(mTimer%2){onLamp();}
		else{offLamp();}
	}*/


	switch (mTimer)
	{
		case 0:
			//offLamp();
			clrL(START);
			if (mDL==1){setL(PROG);}
		break;
		case 1:
			clrL(PROG);
			if (mDC==1){setL(CTRL);}
		break;
		case 2:
			clrL(CTRL);
			if (mDP==1){setL(TEMP);}
		break;
		case 3:
			clrL(TEMP);
			if (mDS==1){setL(START);}
		break;
	/*	case 4:
			clrL(START);
			if (mLamp==1){onLamp();}
		break;*/
	
	}


}

void setLedFunction(int mDL, int mDC, int mDP, int mDS){

		 if (mDL==1){setL(PROG);}else {clrL(PROG);}
		 if (mDC==1){setL(CTRL);}else {clrL(CTRL);}
		 if (mDP==1){setL(TEMP);}else {clrL(TEMP);}
		 if (mDS==1){setL(START);}else {clrL(START);}
}


//funkcja za³¹czania buzera z generatorem
//funkcja zalacza buzer na ilosc cykli wskazane przez zmienna timeBuz, jest to zmienna globalna dekrementowana w przerwaniu od timera
//funkcja moze byc wstawiona w petli glownej "while", natomiast w warunkach ustawiamy wartosc zmiennej timerBuz na wartosc niezerowa
//aby zezwoliæ na brzêczenie trzeba ustawic flage permission na "1";w chwili gdy trzeba natychmiast wy³¹czyæ brzêczenie flage permission ustawiamy na "0"
void BuzzerWork(int timerBuzF, int permission){
	switch (permission)
	{
	case 0:
		PORTB &=~(1<<PB0);
	case 1:
		if (timerBuzF>0){PORTB |=(1<<PB0);}
		else{PORTB &=~(1<<PB0);}
	break;
	}
}
	
//-----------------------odbsluga wyswietacz LED----------------------------------------------
//zalaczanie i wylaczanie poszczegolnych cyfr
void onDigL0(){
	PORTD&=~(1<<DigL0);
}
void onDigL1(){
	PORTD&=~(1<<DigL1);
}
void onDigL2(){
	PORTD&=~(1<<DigL2);
}
void onDigL3(){
	PORTD&=~(1<<DigL3);
}
void offDigL0(){
	PORTD|=(1<<DigL0);
}
void offDigL1(){
	PORTD|=(1<<DigL1);
}
void offDigL2(){
	PORTD|=(1<<DigL2);
}
void offDigL3(){
	PORTD|=(1<<DigL3);
}

void DsLedOff(){
	offDigL0();
	offDigL1();
	offDigL2();
	offDigL3();
}

//funkcja wysylajaca znaki na dana pozycje
void DsLedSend(char dig0, char dig1, char dig2, char dig3){
	static int DsLedTimer=0;
	DsLedTimer++;
	if(DsLedTimer==4){DsLedTimer=0;}

	switch (DsLedTimer)
	{
		case 0:
			offDigL3();
			SPI_Send(dig0);
			onDigL0();
		break;
		case 1:
			offDigL0();
			SPI_Send(dig1);
			onDigL1();
		break;
		case 2:
			offDigL1();
			SPI_Send(dig2);
			onDigL2();
		break;
		case 3:
			offDigL2();
			SPI_Send(dig3);
			onDigL3();
		break;
	}


}


void RsDataTabRec(unsigned char byte, volatile unsigned char tab[]){
	if (tab[9]==0){tab[9]=byte;}
	else if (tab[8]==0){tab[8]=byte;}
	else if (tab[7]==0){tab[7]=byte;}
	else if (tab[6]==0){tab[6]=byte;}
	else if (tab[5]==0){tab[5]=byte;}
	else if (tab[4]==0){tab[4]=byte;}
	else if (tab[3]==0){tab[3]=byte;}
	else if (tab[2]==0){tab[2]=byte;}
	else if (tab[1]==0){tab[1]=byte;}
	else if (tab[0]==0){tab[0]=byte;}
}
void RsDataTab(unsigned char byte, volatile unsigned char tab[]){
	if (tab[9]==0){tab[9]=byte;}
	else if (tab[8]==0){tab[8]=byte;}
	else if (tab[7]==0){tab[7]=byte;}
	else if (tab[6]==0){tab[6]=byte;}
	else if (tab[5]==0){tab[5]=byte;}
	else if (tab[4]==0){tab[4]=byte;}
	else if (tab[3]==0){tab[3]=byte;}
	else if (tab[2]==0){tab[2]=byte;}
	else if (tab[1]==0){tab[1]=byte;}
	else if (tab[0]==0){tab[0]=byte;}
}
void RsShiftTab(volatile unsigned char tab[]){

	tab[9]=tab[8];
	tab[8]=tab[7];
	tab[7]=tab[6];
	tab[6]=tab[5];
	tab[5]=tab[4];
	tab[4]=tab[3];
	tab[3]=tab[2];
	tab[2]=tab[1];
	tab[1]=tab[0];
	tab[0]=0;
}