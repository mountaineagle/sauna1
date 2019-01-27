/*
 * MS1pv1.c
 *
 * Created: 2018-05-01 10:27:43
 * Author : Karol Orligora
 */ 

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include "SW_Dv2.h"   
#include "RS485Fn.h" 
#include "SPI.h" 
#include "Timers.h" 
#include "RS_Functions.h"
#include "Main_Priv.h"

/*-----------global variable definition -------------*/

static strSaunaParam_t structEEPROMSavedValue EEMEM;
static strSaunaParam_t Gv_WorkingParam;
static strLedStatus_t Gv_LedStatus;


volatile uint16_t timerMenuChange_u16 = 0;

static stateMachine_t Gv_StateMachine_e = SM_OFF;
static stateMachine_t Gv_StateMachinePrevState_e = SM_OFF;
static uint8_t Gv_InitOnEntry_bo = TRUE;









struct strParam{ //wartosci musza sie miescic w wartosciach min Max
	
	uint8_t histTempDef;
	uint8_t timerFanSetDef;
	uint8_t temperatureHotMaxDef;
	uint8_t temperatureHotMinDef;
	uint16_t warmingTimeDef;
	uint16_t delayWarmingTimeDef;
	uint8_t temperatureHotDef;
	int8_t calibrationDef;
	uint16_t warmingTimeMaxDef;	
	};
	struct strParam strParamE1 EEMEM;
	
unsigned char tabMenuParamE[14] EEMEM;
unsigned char eAddr EEMEM;
	

//przyciski
volatile uint8_t sw0;
volatile uint8_t sw1;
volatile uint8_t sw2;
volatile uint8_t sw3;
volatile uint8_t sw4;
volatile uint8_t sw5;
volatile uint8_t sw6;
volatile uint8_t sw7;
volatile uint8_t sw8;
volatile uint8_t sw9;
volatile uint8_t sw10;
volatile uint8_t sw11;
volatile uint8_t sw12;

//rs
volatile unsigned char rsByteSend=0x00;
volatile unsigned char rsRecByte=0x00;
volatile int enableRS=0;
volatile unsigned char tabDataRS[10];
volatile unsigned char tabRec[10];
//transmisja rs nowa:
volatile uint8_t sendRS485M;
volatile int returnDataInfo=0;
volatile unsigned char returnData=0;
volatile int enableSend1=1;
volatile int nrByte=0;
volatile int enableSend2=0;
volatile unsigned char tabDataRxc[2]={0,0};
volatile int sendStep=0;
//zabezp rsa
volatile uint8_t stepS;
volatile uint8_t presM1=0;
//buzer
volatile int timerBuz=0;

volatile int timerStop=0;
volatile int counterL=0;
volatile int mark=0;
volatile int timerL1=1000;
volatile int timerL2=1000;
volatile int timerL3=1000;
volatile int timerP=700;
volatile int timer0=0;
volatile uint16_t timerMenu1;
volatile uint16_t timerFurON;
volatile uint16_t timerMenu2;

volatile int timerServ=1000;//timer do obslugi menu
volatile int timerServ2=0;//timer do obslugi menu
volatile int timerSec=0; //timer od przeliczania sekund
volatile unsigned int sec;

volatile int timerLedStop=0;

//timer do wejscia w ukryte menu
volatile uint16_t timerHideMenu=0;

//flaga countera do przyciskow
volatile int switchCounterM=0;
//do wyswietlania co 10s temperatury nastawionej
uint16_t timerShowSetTemp=0;


//////////////////////////////////////////////////////////////
void setDefaultParameter(strSaunaParam_t*structDefParameter)
{
		structDefParameter->histTemp_u8 = 2;
		structDefParameter->timerFanSet_u8 = 10;
		structDefParameter->temperatureHotMax_u8 = 90;
		structDefParameter->temperatureHotMin_u8 = 50;
		structDefParameter->warmingTime_u16 = 240;
		structDefParameter->delayWarmingTime_u16 = 0;
		structDefParameter->temperatureHot_u8 = 50;
		structDefParameter->calibration_s8 = 0;
		structDefParameter->warmingTimeMax_u16 = 240;
}

void saveDefaultParameter(strSaunaParam_t*structDefParameter,strSaunaParam_t*structEepromParam)
{
	setDefaultParameter(structDefParameter);
	eeprom_write_block(structDefParameter,structEepromParam,sizeof((*structDefParameter)));
}

void initEepromStruct(strSaunaParam_t*structDefParameter,strSaunaParam_t*structEepromParam, uint8_t* eAddr)
{
	if ((eeprom_read_byte(eAddr))==0xFF)
	{
		saveDefaultParameter(structDefParameter,structEepromParam);	
		eeprom_write_byte(eAddr,0);
	}
}

	
	typedef struct
	{
		uint8_t nextState_au8[5];		//przejœcia do nastêpnych stanów
		void (*callback)(uint8_t event_u8, strSaunaParam_t *Gv_WorkingParam, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);	//funkcja zwrotna
		//const char* first_line;		//tekst dla 1. linii LCD
		
	} menuItem_t;
	
	
void voidFunction(uint8_t event,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel) 
{
		
}

	
/*void showHideMenuLevel(uint8_t event_u8,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel) 
{
		timerMenuChange=TIM_MENU_CHANGE;
		fillTabByLcdDataMenu(event_u8, displayOutData_pa);
}*/
	
	
void setHistTempFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue_pstr, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		
		if(event_u8==SW_UP){(structWorkingValue_pstr->histTemp_u8)++;}
		if(event_u8==SW_DOWN){(structWorkingValue_pstr->histTemp_u8)--;}
		if((structWorkingValue_pstr->histTemp_u8) > HIST_TEMP_MAX){structWorkingValue_pstr->histTemp_u8=HIST_TEMP_MIN;}
		if((structWorkingValue_pstr->histTemp_u8) < HIST_TEMP_MIN){structWorkingValue_pstr->histTemp_u8=HIST_TEMP_MAX;}
		
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL,(int16_t)structWorkingValue_pstr->histTemp_u8, displayOutData_pa);
}
	
void setCalibrationFunction(uint8_t event,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel) 
{
		
		if(event==SW_UP){(structWorkingValue->calibration_s8)++;}
		if(event==SW_DOWN){(structWorkingValue->calibration_s8)--;}
		if((structWorkingValue->calibration_s8) > CALIBRATION_MAX){structWorkingValue->calibration_s8=CALIBRATION_MIN;}
		if((structWorkingValue->calibration_s8) < CALIBRATION_MIN){structWorkingValue->calibration_s8=CALIBRATION_MAX;}
		
		fillLcdDataTab(CALIB_LCD_CONV_LEVEL,(int16_t)structWorkingValue->calibration_s8, displayOutData_pa);
}
	
void setMaxWorkingTimerFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		
		if(event_u8==SW_UP){(structWorkingValue->warmingTimeMax_u16)++;}
		if(event_u8==SW_DOWN){(structWorkingValue->warmingTimeMax_u16)--;}
		if(event_u8==SW_FAST_UP){(structWorkingValue->warmingTimeMax_u16)+=10;}
		if(event_u8==SW_FAST_DOWN){(structWorkingValue->warmingTimeMax_u16)-=10;}
		if(event_u8==SW_VERY_FAST_UP){(structWorkingValue->warmingTimeMax_u16)+=60;}
		if(event_u8==SW_VERY_FAST_DOWN){(structWorkingValue->warmingTimeMax_u16)-=60;}
		if((structWorkingValue->warmingTimeMax_u16) > WARMING_TIME_MAX){structWorkingValue->warmingTimeMax_u16 = WARMING_TIME_MIN;}
		if((structWorkingValue->warmingTimeMax_u16) < WARMING_TIME_MIN){structWorkingValue->warmingTimeMax_u16 = WARMING_TIME_MAX;}	
		
		fillLcdDataTab(TIME_LCD_CONV_LEVEL,(int16_t)structWorkingValue->warmingTimeMax_u16, displayOutData_pa);
}

void setTempMaxFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel) 
{	
		if(event_u8==SW_UP){(structWorkingValue->temperatureHotMax_u8)++;}
		if(event_u8==SW_DOWN){(structWorkingValue->temperatureHotMax_u8)--;}
		if(event_u8==SW_FAST_UP){(structWorkingValue->temperatureHotMax_u8)++;}
		if(event_u8==SW_FAST_DOWN){(structWorkingValue->temperatureHotMax_u8)--;}
		if(event_u8==SW_VERY_FAST_UP){(structWorkingValue->temperatureHotMax_u8)+=10;}
		if(event_u8==SW_VERY_FAST_DOWN){(structWorkingValue->temperatureHotMax_u8)-=10;}
		if((structWorkingValue->temperatureHotMax_u8) > TEMPERATURE_HOTMAX_MAX){structWorkingValue->temperatureHotMax_u8 = TEMPERATURE_HOTMAX_MIN;}
		if((structWorkingValue->temperatureHotMax_u8) < TEMPERATURE_HOTMAX_MIN){structWorkingValue->temperatureHotMax_u8 = TEMPERATURE_HOTMAX_MAX;}	
		
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL,(int16_t)structWorkingValue->warmingTimeMax_u16, displayOutData_pa);
}
	
void setTempMinFunction(uint8_t event,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel) 
{	
	if(event==SW_UP)
	{
		(structWorkingValue->temperatureHotMin_u8)++;
	}
	if(event==SW_DOWN)
	{
		(structWorkingValue->temperatureHotMin_u8)--;
	}
	if(event==SW_FAST_UP)
	{
		(structWorkingValue->temperatureHotMin_u8)++;
	}
	if(event==SW_FAST_DOWN)
	{
		(structWorkingValue->temperatureHotMin_u8)--;
	}
	if(event==SW_VERY_FAST_UP)
	{
		(structWorkingValue->temperatureHotMin_u8)+=10;
	}
	if(event==SW_VERY_FAST_DOWN)
	{
		(structWorkingValue->temperatureHotMin_u8)-=10;
	}
	if((structWorkingValue->temperatureHotMin_u8) > TEMPERATURE_HOTMIN_MAX )
	{
		structWorkingValue->temperatureHotMin_u8=TEMPERATURE_HOTMIN_MIN;
	}
	if((structWorkingValue->temperatureHotMin_u8) < TEMPERATURE_HOTMIN_MIN )
	{
		structWorkingValue->temperatureHotMin_u8=TEMPERATURE_HOTMIN_MAX;
	}	
		
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL,(int16_t)structWorkingValue->warmingTimeMax_u16, displayOutData_pa);
}

void setTimeFanFunction(uint8_t event,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel) 
{
		if(event==SW_UP){(structWorkingValue->timerFanSet_u8)++;}
		if(event==SW_DOWN){(structWorkingValue->timerFanSet_u8)--;}
		if(event==SW_FAST_UP){(structWorkingValue->timerFanSet_u8)++;}
		if(event==SW_FAST_DOWN){(structWorkingValue->timerFanSet_u8)--;}
		if(event==SW_VERY_FAST_UP){(structWorkingValue->timerFanSet_u8)+=10;}
		if(event==SW_VERY_FAST_DOWN){(structWorkingValue->timerFanSet_u8)-=10;}
		if(structWorkingValue->timerFanSet_u8>TIMER_FAN_MAX){structWorkingValue->timerFanSet_u8=TIMER_FAN_MIN;}
		if(structWorkingValue->timerFanSet_u8<TIMER_FAN_MIN){structWorkingValue->timerFanSet_u8=TIMER_FAN_MAX;}
		
		fillLcdDataTab(TIME_LCD_CONV_LEVEL,(int16_t)structWorkingValue->warmingTimeMax_u16, displayOutData_pa);
}	
	
	void showHideMenuLevel(uint8_t event_u8, strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) {
		uint8_t displayMenuLevel_u8=0;
		switch (menuLevel_u8)
		{
			case 1:
				displayMenuLevel_u8=1;
			break;
			case 3:
				displayMenuLevel_u8=2;
			break;
			case 5:
				displayMenuLevel_u8=3;
			break;
			case 7:
				displayMenuLevel_u8=4;
			break;
			case 9:
				displayMenuLevel_u8=5;
			break;
			case 11:
				displayMenuLevel_u8=6;
			break;
		}
		fillTabByLcdDataMenu(displayMenuLevel_u8, displayOutData_pa);
	}	
	
	
	
void setFurnanceWorkTime(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		if(SW_UP == event_u8)
		{
			(structWorkingValue->warmingTime_u16)++;
		}
		else if(SW_DOWN == event_u8)
		{
			(structWorkingValue->warmingTime_u16)--;
		}
		else if(SW_FAST_UP == event_u8)
		{
			(structWorkingValue->warmingTime_u16) += 10;
		}
		else if(SW_FAST_DOWN == event_u8)
		{
			(structWorkingValue->warmingTime_u16) -= 10;
		}
		else if(SW_VERY_FAST_UP == event_u8)
		{
			(structWorkingValue->warmingTime_u16) += 60;
		}
		else if(SW_VERY_FAST_DOWN == event_u8 )
		{
			(structWorkingValue->warmingTime_u16) -= 60;
		}
		
		if(structWorkingValue->warmingTime_u16 > structWorkingValue->warmingTimeMax_u16)
		{
			structWorkingValue->warmingTime_u16 = WARMING_TIME_MIN;
		}
		if(structWorkingValue->warmingTime_u16 < WARMING_TIME_MIN)
		{
			structWorkingValue->warmingTime_u16 = structWorkingValue->warmingTimeMax_u16;
		}	
		
		fillLcdDataTab(TIME_LCD_CONV_LEVEL,(int16_t)structWorkingValue->warmingTime_u16, displayOutData_pa);
}
	
void setFurnanceDelay(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel) {
		
		if(SW_UP == event_u8)
		{
			(structWorkingValue->delayWarmingTime_u16)++;
		}
		else if(SW_DOWN == event_u8)
		{
			(structWorkingValue->delayWarmingTime_u16)--;
		}
		else if(SW_FAST_UP == event_u8)
		{
			(structWorkingValue->delayWarmingTime_u16)+=10;
		}
		else if(SW_FAST_DOWN == event_u8)
		{
			(structWorkingValue->delayWarmingTime_u16)-=10;
		}
		else if(SW_VERY_FAST_UP == event_u8)
		{
			(structWorkingValue->delayWarmingTime_u16)+=60;
		}
		else if(SW_VERY_FAST_DOWN == event_u8)
		{
			(structWorkingValue->delayWarmingTime_u16)-=60;
		}
		
		if(structWorkingValue->delayWarmingTime_u16 > DELAY_WARMING_TIME_MAX)
		{
			structWorkingValue->delayWarmingTime_u16 = DELAY_WARMING_TIME_MIN;
		}
		if(structWorkingValue->delayWarmingTime_u16 < DELAY_WARMING_TIME_MIN)
		{
			structWorkingValue->delayWarmingTime_u16 = DELAY_WARMING_TIME_MAX;
		}
		
		fillLcdDataTab(TIME_LCD_CONV_LEVEL,(int16_t)structWorkingValue->delayWarmingTime_u16, displayOutData_pa);
}
	
static void TempSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel) 
{
	/* temperature changing depends on event*/
	if(SW_UP == event_u8)
	{
		(structWorkingValue->temperatureHot_u8)++;
	}
	else if(SW_DOWN == event_u8)
	{	
		(structWorkingValue->temperatureHot_u8)--;
	}
	else if(SW_FAST_UP == event_u8)
	{
		(structWorkingValue->temperatureHot_u8)++;
	}
	else if(SW_FAST_DOWN == event_u8)
	{
		(structWorkingValue->temperatureHot_u8)--;
	}
	else if(SW_VERY_FAST_UP == event_u8)
	{
		(structWorkingValue->temperatureHot_u8) += 10;
	}
	else if(SW_VERY_FAST_DOWN == event_u8)
	{
		(structWorkingValue->temperatureHot_u8) -= 10;
	}	
	else if(structWorkingValue->temperatureHot_u8 > structWorkingValue->temperatureHotMax_u8)
	{
		structWorkingValue->temperatureHot_u8 = structWorkingValue->temperatureHotMin_u8;
	}
	else if(structWorkingValue->temperatureHot_u8 < structWorkingValue->temperatureHotMin_u8)
	{
		structWorkingValue->temperatureHot_u8 = structWorkingValue->temperatureHotMax_u8;
	}	
	/* return to init state*/
	else if(TIMER_EVENT_6S == event_u8)
	{
		Gv_StateMachine_e = Gv_StateMachinePrevState_e;
	}
		
	/* fill Lcd tab with temperature data*/
	fillLcdDataTab(TEMP_LCD_CONV_LEVEL,(int16_t)structWorkingValue->temperatureHot_u8, displayOutData_pa);
		
	return ;
}

static void OffStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t* displayOutData_pa)
{
	/*used to count position of sec mark on disply */
	static uint8_t tickDislplayMark_u8 = 0u; 
		/* initialize variable and set states on entry to this state */
	if (TRUE == Gv_InitOnEntry_bo)
	{
		/* remember value in case of return from other states */
		Gv_StateMachinePrevState_e = SM_OFF;
			
		/*clear led after return from delay state*/
		clrL(TEMP);
		clrL(TIMER_OFF);
		clrL(TIMER_ON);
		fanState_pu8 = FAN_OFF;
		lampState_pu8 = LAMP_OFF;
		furnanceState_pu8 = FUR_OFF;
			
		displayOutData_pa[0] = 0;
		displayOutData_pa[1] = 0;
		displayOutData_pa[2] = 0;
		displayOutData_pa[3] = 0;
			
		/* clear initOnEntry flag - initialize was done*/
		Gv_InitOnEntry_bo = FALSE;
	}
		
	if (SW_MENU == event_u8)//menu ukryte
	{
		Gv_StateMachine_e = SM_HIDE_MENU;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if (SW_OFF_ON == event_u8)
	{
		
		Gv_StateMachine_e = SM_IN;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(SW_FAN == event_u8)
	{
		if (FAN_ON_EXEC == fanState_pu8)
		{
			fanState_pu8 = FAN_OFF;
		}
		if (FAN_IDLE == fanState_pu8)
		{
			fanState_pu8 = FAN_ON;
		}
	}
	else if(SW_LAMP == event_u8)
	{
		if (LAMP_ON2_EXEC == lampState_pu8)
		{
			lampState_pu8 = LAMP_OFF;
		}
		if (LAMP_IDLE == lampState_pu8)
		{
			lampState_pu8 = LAMP_ON2;
		}
	}
	else 
	{
		; /* do nothing */
	}
	
	
	/* count & save sec mark */
	if(TIMER_EVENT_1S == event_u8)
	{
		if (tickDislplayMark_u8 < DISPLSY_SEGMENT_NR)
		{
			tickDislplayMark_u8 ++ ;
		}
		else
		{
			tickDislplayMark_u8 = 0;
		}
	}	
	
	/* add point mark on LCD display every second*/
	displayOutData_pa[tickDislplayMark_u8] = displayOutData_pa[tickDislplayMark_u8] & POINT_LCD_MARK;
	
	return;
}



static void InStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t* displayOutData_pa)
{
	/*used to count position of sec mark on display */
	static uint8_t tickDislplayMark_u8 = 0u; 
	
	/*initialize variable and set states on entry to this state */
	if (TRUE == Gv_InitOnEntry_bo)
	{
		/* remember value in case of return from other states */
		Gv_StateMachinePrevState_e = SM_IN;	
		
		/*clear led after return from delay state*/
		clrL(TEMP);
		clrL(TIMER_OFF);
		clrL(TIMER_ON);
		Gv_FanState_e = FAN_OFF;
		Gv_LampState_e = LAMP_OFF;
		Gv_FurState_e = FUR_OFF;
		/* clear initOnEntry flag - initialize was done*/
		Gv_InitOnEntry_bo = FALSE;
	}

	
	/* switch between states depending on event*/
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(SW_UP == event_u8)
	{
		Gv_StateMachine_e = SM_TEMP_SET;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(SW_DOWN == event_u8)
	{
		Gv_StateMachine_e = SM_TEMP_SET;
		Gv_InitOnEntry_bo = TRUE; 
	}
	else if(SW_FAN == event_u8)
	{
		if (FAN_ON_EXEC == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON;
		}
	}
	else if(SW_LAMP == event_u8)
	{
		if (LAMP_ON2_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_ON2;
		}
	}
	else if(SW_TIMER == event_u8)
	{
		Gv_StateMachine_e = SM_TIMER_SET;
		Gv_InitOnEntry_bo = TRUE; 
	}
	else if(TIMER_EVENT_8S == event_u8 && 0u == structWorkingValue->delayWarmingTime_u16)
	{
		Gv_StateMachine_e = SM_FURNANCE_ON;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(TIMER_EVENT_8S == event_u8 )
	{
		Gv_StateMachine_e = SM_FURNANCE_DELAY;
		Gv_InitOnEntry_bo = TRUE;
	}
	else 
	{
		;
	}	
	
	/* fill LCD data tab by temperature hot*/
	fillLcdDataTab(TEMP_LCD_CONV_LEVEL, (int16_t)(structWorkingValue->temperatureHot_u8) , displayOutData_pa);
	
	/* count & save sec mark */
	if(TIMER_EVENT_1S == event_u8)
	{
		if (tickDislplayMark_u8 < DISPLSY_SEGMENT_NR)
		{
			tickDislplayMark_u8++;
		}
		else
		{
			tickDislplayMark_u8 = 0;
		}
	}	
	
	/* add point mark on LCD display every second*/
	displayOutData_pa[tickDislplayMark_u8] = displayOutData_pa[tickDislplayMark_u8] & POINT_LCD_MARK;
	
	return ;
	
}


static void HideMenuStateExecute()
{
	return;
}



static void FurnanceOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa)
{
	static uint8_t tickDislplayMark_u8 = 0u;
	/* initialize variable on entry to state */
	if (TRUE == Gv_InitOnEntry_bo)
	{
		/* remember value in case of return from other states */
		Gv_StateMachinePrevState_e = SM_FURNANCE_ON
			
		/* clear led after return from delay state */
		Gv_LedStatus.TempLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOffLed_u8 = LED_ON;
		
		/* switching on the lamp */
		Gv_LampState_e = LAMP_AUTO_ON;
		/* init display position for point */
		tickDislplayMark_u8 = 0u;
			
		/* clear initOnEntry flag - initialize was done */
		Gv_InitOnEntry_bo = FALSE;
	}
	
	/* switch executing */
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(SW_UP == event_u8)
	{
		Gv_StateMachine_e = SM_TEMP_SET;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(SW_DOWN == event_u8)
	{
		Gv_StateMachine_e = SM_TEMP_SET;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(SW_FAN == event_u8)
	{
		if (FAN_ON_EXEC == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON;
		}
	}
	else if(SW_LAMP == event_u8)
	{
		if (LAMP_ON2_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_ON2;
		}
	    if (LAMP_ON1_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
	}
	else if(SW_TIMER == event_u8)
	{
		Gv_StateMachine_e = SM_TIMER_SET;
		Gv_InitOnEntry_bo = TRUE; 
	}
	else 
	{
		; /* do nothing */
	}
	
	/* count timer */
	if(TRUE == Gv_ExtTimer1min_bo)
	{
		structWorkingValue->warmingTime_u16 -- ; 
	}
	
	if (0u == structWorkingValue->warmingTime_u16)
	{
		Gv_StateMachine_e = SM_FAN_ON;
		Gv_InitOnEntry_bo = TRUE; 
	}
	
	/* fill LCD data tab by temperature hot*/
	if (TIMER_EVENT_6S > Gv_Timer8s_u8)
	{
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL, (int16_t)(structWorkingValue->temperatureHot_u8) , displayOutData_pa);
	}
	else 
	{
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL, (int16_t)(Gv_ActualTemperature_u8) , displayOutData_pa);
	}
	
	return ;
	
}

static void TimeFlagSet()
{
	/* clear global flag after one cycle of main function*/
	if (TRUE == Gv_flag50ms_bo)
	{
		Gv_flag50ms_bo = FALSE;
	}
	
	/*clear interrupt flag, set global flag */
	if (TRUE == Gv_flagInterrupt50ms_bo)
	{
		Gv_flagInterrupt50ms_bo = FALSE;
		Gv_flag50ms_bo = TRUE;
	}
	
	return ;
}

static void ClearAllTimers()
{
	Gv_Timer8s_u8 = 0u;
	Gv_Timer6s_u8 = 0u;
	Gv_Timer2s_u8 = 0u;
	Gv_Timer1s_u8 = 0u;
}


static void TimeCounter()
{
	if (TIMER_EVENT_8S == Gv_Timer8s_u8)
	{
		Gv_Timer8s_u8 = 0u;
	}
	else
	{
		/* do nothing */
	}
	
	if (TIMER_EVENT_6S == Gv_Timer6s_u8)
	{
		Gv_Timer6s_u8 = 0u;
	}
	else
	{
		/* do nothing */
	}
	
	if (TIMER_EVENT_3S == Gv_Timer3s_u8)
	{
		Gv_Timer3s_u8 == 0u;
	}
	else
	{
		/* do nothing */
	}
	
	if (TIMER_EVENT_2S == Gv_Timer2s_u8)
	{
		Gv_Timer2s_u8 == 0u;
	}
	else
	{
		/* do nothing */
	}
	
	if (TIMER_EVENT_1S == Gv_Timer1s_u8)
	{
		Gv_Timer1s_u8 == 0u;
	}
	else
	{
		/* do nothing */
	}
	
	if (TRUE == Gv_flagInterrupt50ms_bo)
	{
		Gv_flagInterrupt50ms_bo = FALSE;
		Gv_Timer8s_u8 ++;
		Gv_Timer6s_u8 ++;
		Gv_Timer2s_u8 ++;
		Gv_Timer1s_u8 ++;
	}
}




static void FurnanceDelayStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa)
{
	if (TRUE == Gv_InitOnEntry_bo)
	{
		/* remember value in case of return from other states */
		Gv_StateMachinePrevState_e = SM_FURNANCE_DELAY;
		
		/* clear led after return from delay state */
		Gv_LedStatus.TempLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_ON;
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;
		
		/* clear initOnEntry flag - initialize was done */
		Gv_InitOnEntry_bo = FALSE;
	}
	
	/* switch executing */
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(SW_UP == event_u8)
	{
		Gv_StateMachine_e = SM_TEMP_SET;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(SW_DOWN == event_u8)
	{
		Gv_StateMachine_e = SM_TEMP_SET;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(SW_FAN == event_u8)
	{
		if (FAN_ON_EXEC == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON;
		}
	}
	else if(SW_LAMP == event_u8)
	{
		if (LAMP_ON2_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_ON2;
		}
		if (LAMP_ON1_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
	}
	else if(SW_TIMER == event_u8)
	{
		Gv_StateMachine_e = SM_TIMER_SET;
		Gv_InitOnEntry_bo = TRUE; 
	}
	else 
	{
		; /* do nothing */
	}	
	
	/* count timer */
	if(TIMER_EVENT_1S == event_u8)
	{
		structWorkingValue->delayWarmingTime_u16 -- ; 
	}
	
	/* switch to next state */
	if (0u == structWorkingValue->delayWarmingTime_u16 )
	{
		Gv_StateMachine_e = SM_FURNANCE_ON;
		Gv_InitOnEntry_bo = TRUE; 
	}
	
	/* fill LCD data tab by delay timer */
	fillLcdDataTab(TIME_LCD_CONV_LEVEL, (int16_t)(structWorkingValue->delayWarmingTime_u16) , displayOutData_pa);
	
	return ;
}


void TimerSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel2_u8)
{
	static uint8_t menuLevel_u8 = 0u;
	
	/* initialize variable on entry to state */
	if (TRUE == Gv_InitOnEntry_bo)
	{
		/* clear led after return from delay state */
		Gv_LedStatus.TempLed_u8 = LED_ON;
		Gv_LedStatus.TimerOnLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;
		
		/* clear initOnEntry flag - initialize was done */
		Gv_InitOnEntry_bo = FALSE;
	}
	
	/* switch executing */
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if((TIMER_EVENT_6S == event_u8 || (SW_TIMER == event_u8 && 0xFF == menuLevel_u8)))
	{
		Gv_StateMachine_e = Gv_StateMachinePrevState_e;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if (SW_TIMER == event_u8)
	{
		menuLevel_u8 = ~menuLevel_u8;
	}
	
	if (0u == menuLevel_u8)
	{
		setFurnanceWorkTime(event_u8, structWorkingValue, displayOutData_pa, menuLevel_u8);
		Gv_LedStatus.TimerOffLed_u8 = LED_BLINK;
	}
	else
	{
		setFurnanceDelay(event_u8, structWorkingValue, displayOutData_pa, menuLevel_u8);
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_BLINK;
	}
	
	return ;
}


static void FanOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel2_u8)
{
	if (TRUE == Gv_InitOnEntry_bo)
	{
		/* remember value in case of return from other states */
		Gv_StateMachinePrevState_e = SM_FAN_ON;
		
		/* clear led after return from delay state */
		Gv_LedStatus.TempLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;
		Gv_LedStatus.FanLed_u8 = LED_ON;
		
		/* switch on the fan */
		fanState_pu8 = FAN_ON;
		
		/* clear initOnEntry flag - initialize was done */
		Gv_InitOnEntry_bo = FALSE;
	}
	
	/* switch executing */
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(SW_FAN == event_u8)
	{
		if (FAN_ON_EXEC == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON;
		}
	}
	else if(SW_LAMP == event_u8)
	{
		if (LAMP_ON2_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_ON2;
		}
		if (LAMP_ON1_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
	}
	else
	{
		; /* do nothing */
	}
	
	/* count timer */
	if(TIMER_EVENT_1S == event_u8)
	{
		structWorkingValue->timerFanSet_u8 -- ;
	}
	/* switching to next state */
	if (0u == structWorkingValue->timerFanSet_u8 )
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	
	/* LED display */
	/* fill LCD data tab by delay timer */
	fillLcdDataTab(TEMP_LCD_CONV_LEVEL, (int16_t)(Gv_ActualTemperature_u8) , displayOutData_pa);
	
	return ;
}


		//-------------------------obliczenie aktualnej temperatury po otrzymaniu wszystkich cyfr...
		if (getAllDigit==1){
			Gv_ActualTemperature_u8=(uint8_t)TabToTempConv(temp100,temp10,temp1);
			Gv_ActualTemperature_u8=Gv_ActualTemperature_u8+calibration;
			getAllDigit=0;
			makeCount=1;
		}
		
static uint8_t SwEventChoose (uint8_t switchCounter_bo)	
 {     
	uint8_t swEvent_u8 = 0u;
	if (TRUE == switchCounter_bo)
	{
		switchCounter_bo = FALSE;
	    if ( SHORT == obslugaPrzyciskuKrotkiego2(0u,PINA,0x01u,15u) )// ON/OFF
		{
			swEvent_u8 = SW_OFF_ON; 
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego2(1u,PINA,0x02u,15u) )// LAMP<-TEMP
		{
			swEvent_u8 = SW_LAMP; 
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego2(2u,PINA,0x04u,15u) )// STRZ GORA
		{
			swEvent_u8 = SW_UP; 
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego2(3u,PINA,0x08u,15u) )// WIATRAK<-TIMER
		{
			swEvent_u8 = SW_FAN;
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego2(4u,PINA,0x10u,15u) )// MENU<-LAMP2
		{
			swEvent_u8 = SW_TIMER;
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego2(5u,PINA,0x20u,15u) )// STRZ DOL
		{
			swEvent_u8 = SW_DOWN; 
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego(6u,PINA,0x10u,15u) )// MENU UKRYTE
		{
			swEvent_u8 = SW_MENU;
		}
	    if ( SHORT2 == obslugaPrzyciskuKrotkiego4(7u,PINA,0x04u,150u,100u) )// Menu bardzo szybkie pzewijanie UP
		{
			swEvent_u8 = SW_VERY_FAST_UP;
		}
	    if ( SHORT1 == obslugaPrzyciskuKrotkiego4(8u,PINA,0x04u,150u,100u) )// Menu szybkie pzewijanie UP
	    {
		    swEvent_u8 = SW_FAST_UP;
	    }
	    if ( SHORT2 == obslugaPrzyciskuKrotkiego4(9u,PINA,0x20u,150u,100u) )// Menu bardzo szybkie pzewijanie DOWN
		{
			swEvent_u8 = SW_VERY_FAST_DOWN;
		}
	    if ( SHORT1 == obslugaPrzyciskuKrotkiego4(10u,PINA,0x20u,150u,100u) )// Menu szybkie pzewijanie DOWN
	    {
		    swEvent_u8 = SW_FAST_DOWN;
	    }
	}
	return swEvent_u8;
 }

static void StateMachine(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
{
	switch(Gv_StateMachine_e)
	{
		case SM_HIDE_MENU:
			HideMenuStateExecute();
			break;
		case SM_OFF:
			OffStateExecute(event_u8, structWorkingValue, displayOutData_pa);
			break;
		case SM_IN:
			InStateExecute(event_u8, structWorkingValue, displayOutData_pa);
			break;
		case SM_TEMP_SET:
			TempSetStateExecute(event_u8, structWorkingValue, displayOutData_pa);
			break;
		case SM_TIMER_SET:
			TimerSetStateExecute(event_u8, structWorkingValue, displayOutData_pa);
			break;
		case SM_FURNANCE_DELAY:
			FurnanceDelayStateExecute(event_u8, structWorkingValue, displayOutData_pa);
			break;
		case SM_FURNANCE_ON:
			FurnanceOnStateExecute(event_u8, structWorkingValue, displayOutData_pa);
			break;
		case SM_FAN_ON:
			FanOnStateExecute(event_u8, structWorkingValue, displayOutData_pa);
			break;

	}
}

void FurnanceStateMachine(uint8_t* furnanceState_pu8, uint8_t* minMarkFurnance_pu8)
{
	static uint16_t timerFur_u16 = 0u;
	
	if (timerFur_u16 > 0 && *minMarkFurnance_pu8 == 1)
	{
		timerFur_u16 --;
	}
		
	switch (*furnanceState_pu8)
	{
		case FUR_IDLE: /* IDLE state */
		
		break;
		case FUR_AUTO_ON: /* switch on the furnance */
			RsDataTab(RS_FUR_ON, tabDataRS);
			timerFur_u16 = Gv_WorkingParam.warmingTime_u16;
			setL(TEMP);
			*furnanceState_pu8 = FUR_AUTO_ON_EXEC;
		break;
		case FUR_AUTO_ON_EXEC: /* automatic working of the furnance */
			if(0u == timerFur_u16) /* if timer reach 0 then furnance state change for OFF */
			{
				*furnanceState_pu8 = FUR_OFF;
			}
		break;
		case FUR_OFF: /* switching off the furnance */
			RsDataTab(RS_FUR_OFF, tabDataRS);
			clrL(TEMP);
			*furnanceState_pu8 = FUR_IDLE;
		break;

	}
	
}

void FanStateMachine(uint8_t* fanState_pu8, uint8_t* minMarkFan_pu8)
{
	static uint16_t timerFan_u16 = 0u;
		
	if (timerFan_u16 > 0u && *minMarkFan_pu8 == 1)
	{
		timerFan_u16 --;		
	}

	switch (*fanState_pu8)
	{
		case FAN_IDLE: /* IDLE state */
		
		break;
		case FAN_ON1: /* switching on fan in state off */
			RsDataTab(RS_FAN_ON, tabDataRS);
			timerFan_u16 = Gv_WorkingParam.delayWarmingTime_u16;
			*fanState_pu8 = FAN_ON1_EXEC;
		break;
		case FAN_ON2: /* switching on fan in state differ than off */
			RsDataTab(RS_FAN_ON, tabDataRS);
			timerFan_u16 = Gv_WorkingParam.timerFanSet_u8;
			*fanState_pu8 = FAN_ON1_EXEC;
		break;
		case FAN_ON1_EXEC:/* executing of working lamp after switch on */
			BlinkLed(FAN, counterL,500u);
			
			if(timerFan_u16==0) /* if counter == 0u then fan is switching off */
			{
				*fanState_pu8 = FAN_OFF;
			}
		break;
		case FAN_AUTO_ON: /* automatic work start */
			RsDataTab(RS_FAN_ON, tabDataRS);
			timerFan_u16 = Gv_WorkingParam.timerFanSet_u8;
			setL(FAN);
			*fanState_pu8 = FAN_AUTO_ON_EXEC;
		break;
		case FAN_AUTO_ON_EXEC: /* automatic work execute */
			if(0u == timerFan_u16) /* if counter == 0u then fan is switching off */
			{
				*fanState_pu8 = FAN_OFF;
			}
		break;
		case FAN_OFF: /* switch off the fan */
			RsDataTab(RS_FAN_OFF, tabDataRS);
			clrL(FAN);
			*fanState_pu8 = FAN_IDLE;
		break;

	}

}

void LampStateMachine(uint8_t* lampState_pu8, uint8_t* minMarkLamp_pu8)
{
	static uint16_t timerLamp_u16 = 0u;
	
	if (timerLamp_u16 > 0 && 1u == *minMarkLamp_pu8)
	{
		timerLamp_u16--;
	}
		
	switch (*lampState_pu8)
	{
		case LAMP_IDLE: /* IDLE state */
		
		break;
		case LAMP_ON1: /* switching on in state off */
			RsDataTab(RS_LAMP_ON, tabDataRS);
			timerLamp_u16 = Gv_WorkingParam.delayWarmingTime_u16;
			*lampState_pu8 = LAMP_ON1_EXEC;
		break;
		case LAMP_ON1_EXEC:/* executing of working lamp in state off */
			BlinkLed(PROG, counterL,500u);
		
			if(timerLamp_u16 == 0u) /* if counter == 0u then lamp is switching off */
			{
				*lampState_pu8 = LAMP_OFF;
			}
		break;
		case LAMP_ON2: /* switching on lamp in state differ than off */
			RsDataTab(RS_LAMP_ON, tabDataRS);
			*lampState_pu8 = LAMP_ON2_EXEC;
		break;
		case LAMP_ON2_EXEC: /* working lamp in state differ than off */
			BlinkLed(PROG, counterL,500u);
		break;
		case LAMP_AUTO_ON: /* automatic work start */
			RsDataTab(RS_LAMP_ON, tabDataRS);
			//timerLamp=structWorkingValue.timerFanSet;
			setL(PROG);
			*lampState_pu8 = LAMP_AUTO_ON_EXEC;
		break;
		case LAMP_AUTO_ON_EXEC: /* automatic work execute */
/*
		if(timerFan==0) //jesli timer dojdzie do zera to wiatrak sie wylacza
		{
			*fanState=5;
		}*/
		break;
		case LAMP_OFF: /* switching off the lamp */
			RsDataTab(RS_LAMP_OFF, tabDataRS);
			clrL(PROG);
			*lampState_pu8 = LAMP_IDLE;
		break;

	}
}

static void LedWorkStatus()
{
	
	if (LED_BLINK == Gv_LedStatus.ProgLed_u8)
	{
		BlinkLed(PROG, counterL,500u);
	}
	else if(LED_ON == Gv_LedStatus.ProgLed_u8)
	{
		setL(PROG);				
	}
	else
	{
		clrL(PROG);
	}

	if (LED_BLINK == Gv_LedStatus.FanLed_u8)
	{
		BlinkLed(FAN, counterL,500u);
	}
	else if(LED_ON == Gv_LedStatus.FanLed_u8)
	{
		setL(FAN);
	}
	else
	{
		clrL(FAN);
	}
 
	if (LED_BLINK == Gv_LedStatus.TimerOnLed_u8)
	{
		BlinkLed(TIMER_ON, counterL,500);
	}
	else if(LED_ON == Gv_LedStatus.TimerOnLed_u8)
	{
		setL(TIMER_ON);
	}
	else
	{
		clrL(TIMER_ON);
	}
	
	if (LED_BLINK == Gv_LedStatus.TimerOffLed_u8)
	{
		BlinkLed(TIMER_OFF, counterL,500);
	}
	else if(LED_ON == Gv_LedStatus.TimerOffLed_u8)
	{
		setL(TIMER_OFF);
	}
	else
	{
		clrL(TIMER_OFF);
	}

	if (LED_BLINK == Gv_LedStatus.TempLed_u8)
	{
		BlinkLed(TEMP, counterL,500);
	}
	else if(LED_ON == Gv_LedStatus.TempLed_u8)
	{
		setL(TEMP);
	}
	else
	{
		clrL(TEMP);
	}
	
}

	menuItem_t tabHideMenu[13]={
		//{{SW_UP, SW_DOWN, SW_CHANGE, TIME_CHANGE, IDLE}, pointerToFunction}
		{{0,0,1,0,0}, voidFunction(event,&Gv_WorkingParam, displayOutData_pa, 0)},
		{{1,1,3,2,1}, showHideMenuLevel(event,&Gv_WorkingParam, displayOutData_pa, 1)},
		{{2,2,3,2,2}, setMaxWorkingTimerFunction(event,&Gv_WorkingParam, displayOutData_pa, 2)},
		{{3,3,5,4,3}, showHideMenuLevel(event,&Gv_WorkingParam, displayOutData_pa, 3)},
		{{4,4,5,4,4}, setTempMaxFunction(event,&Gv_WorkingParam, displayOutData_pa, 4)},
		{{5,5,7,6,5}, showHideMenuLevel(event,&Gv_WorkingParam, displayOutData_pa, 5)},
		{{6,6,7,6,6}, setTempMinFunction(event,&Gv_WorkingParam, displayOutData_pa, 6)},
		{{7,7,9,8,7}, showHideMenuLevel(event,&Gv_WorkingParam, displayOutData_pa, 7)},
		{{8,8,9,8,8}, setTimeFanFunction(event,&Gv_WorkingParam, displayOutData_pa, 8)},
		{{9,9,11,10,9}, showHideMenuLevel(event,&Gv_WorkingParam, displayOutData_pa, 9)},
		{{10,10,11,10,10}, setHistTempFunction(event,&Gv_WorkingParam, displayOutData_pa, 10)},
		{{11,11,0,12,11}, showHideMenuLevel(event,&Gv_WorkingParam, displayOutData_pa, 11)},
		{{12,12,0,12,12}, setCalibrationFunction(event,&Gv_WorkingParam, displayOutData_pa, 12)},
	};
	
	
	menuItem_t tabMenu[5]={
		//{{SW_UP, SW_DOWN, SW_CHANGE, TIME_CHANGE, IDLE}, pointerToFunction}
		{{0,0,1,0,0}, voidFunction(event,&Gv_WorkingParam, displayOutData_pa, 0)},
		{{1,1,3,2,1}, showHideMenuLevel(event,&Gv_WorkingParam, displayOutData_pa, 1)},
		{{2,2,3,2,2}, setFurnanceWorkTime(event,&Gv_WorkingParam, displayOutData_pa, 2)},
		{{3,3,0,4,3}, showHideMenuLevel(event,&Gv_WorkingParam, displayOutData_pa, 3},
		{{4,4,0,4,4}, setFurnanceDelay(event,&Gv_WorkingParam, displayOutData_pa, 4)},
	};
	
	uint8_t timerFanSet_u8;
	uint8_t temperatureHotMax;
	uint8_t temperatureHotMin;
	uint16_t warmingTime;
	uint16_t delayWarmingTime;
	uint8_t temperatureHot;
	int8_t calibration;
	uint16_t warmingTimeMax;

	
void fillTabByLcdDataMenu(uint8_t ledDisplayLevel_u8, uint8_t *displayOutData_pa){
	
	static uint8_t previousLevel_u8 = 0u;
	
	if (previousLevel_u8 != ledDisplayLevel_u8)
	{
		DsLedOff();
		previousLevel_u8 = ledDisplayLevel_u8;
	}
		
	displayOutData_pa[0] = F;
	displayOutData_pa[2] = sp;
	displayOutData_pa[3] = sp;
	
	switch (ledDisplayLevel_u8)
	{
		case 0://menu 1
			displayOutData_pa[1] = d1;
		break;
		case 1: //menu 2
			displayOutData_pa[1] = d2;
		break;
		case 2://menu 3
			displayOutData_pa[1] = d3;
		break;
		case 3://menu 4
			displayOutData_pa[1] = d4;
		break;
		case 4://menu 5
			displayOutData_pa[1] = d5;
		break;
		case 5://menu 6
			displayOutData_pa[1] = d6;
		break;
	}
	
	return;
}

		
void fillLcdDataTab(uint8_t conversionLevel_u8, int16_t valueToConvert_u16, uint8_t *displayOutData_pa){
		
		static uint8_t previousConversionLevel_u8=0;
		uint8_t digData_a[4]={0};
			
		if (previousConversionLevel_u8!=conversionLevel_u8){
			DsLedOff();
			previousConversionLevel_u8=conversionLevel_u8;
		}

		switch (conversionLevel_u8)
		{
			/* calibration calculating */
			case 0:
				if ((valueToConvert_u16)>=0)
				{
					TempConvToDigit((uint16_t)(valueToConvert_u16),digData_a);
					DigTempConvToDsp(digData_a,displayOutData_pa);
				}
				else 
				{
					TempConvToDigit((uint16_t)(abs(valueToConvert_u16)),digData_a);
					DigTempConvToDsp(digData_a,displayOutData_pa);
					displayOutData_pa[1]=0xBF;
				}
			break;
			/* temperature calculating */
			case 1: 
				TempConvToDigit((uint16_t)valueToConvert_u16,digData_a);
				DigTempConvToDsp(digData_a,displayOutData_pa);
			break;
			/* time calculating */
			case 2:
				TimeConv((uint16_t)valueToConvert_u16,digData_a);
				TimeConvToDsp(digData_a,displayOutData_pa);
			break;	
		}
		
		return;
	}
	
	
	
	volatile uint8_t	currentMenu = 0;
	volatile uint8_t	event = E_IDLE;

	
	void saveToEEPROM(uint8_t* saveFlag, strSaunaParam_t* savedValue)
	{
		if (*saveFlag==1)
		{
			eeprom_write_block(savedValue,&strParamE1,sizeof(savedValue));	
			*saveFlag=0;
		}
			
	}
	
	void changeMenu(uint8_t *event_pu8, strSaunaParam_t *structure_pstr , uint8_t * tab_pu8 )
	{
		//przejdz do nastepnego stanu
		if(*event_pu8==H_MENU_NEXT_ITEM || *event_pu8==H_MENU_NEXT_ITEM_TIM)
		{	
			currentMenu = tabHideMenu[currentMenu].next_state[event_pu8];	
		}
		//wywolaj funkcje zwrotna
		tabHideMenu[currentMenu].callback(*event_pu8,structure_pstr, tab_pu8);
		//wyswietl komunikat skojarzony z pozycja (opcja)
		
		//skasuj zdarzenie
		*event_pu8 = E_IDLE;
	}
	
	changeMenuLevelByTimer(uint8_t *event_pu8, uint8_t *changeMenuTimer_pu16)
	{
		if (H_MENU_NEXT_ITEM == *event_pu8)
		{
			*changeMenuTimer_pu16 = TIME_3S;
		}
		if (0 == *chaneMenuTimer)
		{
			*event_pu8 = H_MENU_NEXT_ITEM_TIM;
		}
	}

	
	
	
	
	
void TimeConv(uint16_t time_u16, uint8_t digTime[4]){
	digTime[0]=(uint8_t)(time_u16/600);
	digTime[1]=(uint8_t)((time_u16/60)%10);
	digTime[2]=(uint8_t)((time_u16%60)/10);
	digTime[3]=(uint8_t)((time_u16%60)%10);
}

void DigConvToDsp(uint8_t tabIn_au8[4], uint8_t tabOutDsp_au8[4]){
	for (int i=0; i<4; i++){
		if (tabIn_au8[i]==0){tabOutDsp_au8[i]=0xC0;	}
		else if (tabIn_au8[i]==1){tabOutDsp_au8[i]=0xF9;}
		else if (tabIn_au8[i]==2){tabOutDsp_au8[i]=0xA4;}
		else if (tabIn_au8[i]==3){tabOutDsp_au8[i]=0xB0;}
		else if (tabIn_au8[i]==4){tabOutDsp_au8[i]=0x99;}
		else if (tabIn_au8[i]==5){tabOutDsp_au8[i]=0x92;}
		else if (tabIn_au8[i]==6){tabOutDsp_au8[i]=0x82;}
		else if (tabIn_au8[i]==7){tabOutDsp_au8[i]=0xF8;}
		else if (tabIn_au8[i]==8){tabOutDsp_au8[i]=0x80;}
		else if (tabIn_au8[i]==9){tabOutDsp_au8[i]=0x90;}
	}
	if (tabIn_au8[0]==0){tabOutDsp_au8[0]=0xFF;}
	//if (tabIn[0]==0 && tabIn[1]==0){tabOutDsp[1]=0xFF; - problem przy przedstawianiu czasu
	//if (tabIn[0]==0 && tabIn[1]==0 && tabIn[2]==0){tabOutDsp[2]=0xFF; - problem przy przedstawianiu czasu
}


void TimeConvToDsp(uint8_t tabIn_au8[4], uint8_t tabOutDsp_au8[4]){
	for (int i=0; i<4; i++){
		if (tabIn_au8[i]==0){tabOutDsp_au8[i]=0xC0;	}
		else if (tabIn_au8[i]==1){tabOutDsp_au8[i]=0xF9;}
		else if (tabIn_au8[i]==2){tabOutDsp_au8[i]=0xA4;}
		else if (tabIn_au8[i]==3){tabOutDsp_au8[i]=0xB0;}
		else if (tabIn_au8[i]==4){tabOutDsp_au8[i]=0x99;}
		else if (tabIn_au8[i]==5){tabOutDsp_au8[i]=0x92;}
		else if (tabIn_au8[i]==6){tabOutDsp_au8[i]=0x82;}
		else if (tabIn_au8[i]==7){tabOutDsp_au8[i]=0xF8;}
		else if (tabIn_au8[i]==8){tabOutDsp_au8[i]=0x80;}
		else if (tabIn_au8[i]==9){tabOutDsp_au8[i]=0x90;}
	}
	if (tabIn_au8[0]==0){
		tabOutDsp_au8[0]=tabOutDsp_au8[1];
		tabOutDsp_au8[1]=0x89;
	}
	else{
		tabOutDsp_au8[3]=tabOutDsp_au8[2];
		tabOutDsp_au8[2]=0x89;
	}
	
	//if (tabIn[0]==0){tabOutDsp[0]=0xFF;}
	//if (tabIn[0]==0 && tabIn[1]==0){tabOutDsp[1]=0xFF; - problem przy przedstawianiu czasu
	//if (tabIn[0]==0 && tabIn[1]==0 && tabIn[2]==0){tabOutDsp[2]=0xFF; - problem przy przedstawianiu czasu
}



void DigTempConvToDsp(uint8_t tabInDigit_au8[4], uint8_t tabOutLcdRepresentation_au8[4]){ //funkcja nie uwzglednia ujemnych temperatur poniewaz jest wykozystywana tylko do manipulacji na dodatnich temperaturach
	for (int i=0; i<4; i++){
		if (tabInDigit_au8[i]==0){tabOutLcdRepresentation_au8[i]=0xC0;	}
		else if (tabInDigit_au8[i]==1){tabOutLcdRepresentation_au8[i]=0xF9;}
		else if (tabInDigit_au8[i]==2){tabOutLcdRepresentation_au8[i]=0xA4;}
		else if (tabInDigit_au8[i]==3){tabOutLcdRepresentation_au8[i]=0xB0;}
		else if (tabInDigit_au8[i]==4){tabOutLcdRepresentation_au8[i]=0x99;}
		else if (tabInDigit_au8[i]==5){tabOutLcdRepresentation_au8[i]=0x92;}
		else if (tabInDigit_au8[i]==6){tabOutLcdRepresentation_au8[i]=0x82;}
		else if (tabInDigit_au8[i]==7){tabOutLcdRepresentation_au8[i]=0xF8;}
		else if (tabInDigit_au8[i]==8){tabOutLcdRepresentation_au8[i]=0x80;}
		else if (tabInDigit_au8[i]==9){tabOutLcdRepresentation_au8[i]=0x90;}
	}
	if (tabInDigit_au8[0]==0){tabOutLcdRepresentation_au8[0]=0xFF;}
	if (tabInDigit_au8[0]==0 && tabInDigit_au8[1]==0){tabOutLcdRepresentation_au8[1]=0xFF; }
	if (tabInDigit_au8[0]==0 && tabInDigit_au8[1]==0 && tabInDigit_au8[2]==0){tabOutLcdRepresentation_au8[2]=0xFF;}
}

void TempConvToDigit (uint16_t temperature_u16, uint8_t digTemp_au8[4])
{ 
	/*funkcja nie uwzglednia ujemnych temperatur poniewaz jest 
	  wykozystywana tylko do manipulacji na dodatnich temperaturach*/
	digTemp_au8[0] = 0x00;
	digTemp_au8[1] = (uint8_t)temperature_u16/100;
	digTemp_au8[2] = (uint8_t)(temperature_u16%100)/10;
	digTemp_au8[3] = (uint8_t)(temperature_u16%100)%10;
}

int16_t TabToTempConv(uint8_t firstDig100_u8, uint8_t secDigit10_u8, uint8_t  thrDigit1_u8)
{
	int16_t temperature_s16;
	
	if (firstDig100_u8==2){
		temperature_s16 = -(int16_t)secDigit10_u8-(int16_t)thrDigit1_u8;
	}
	else 
	{
		temperature_s16 = (int16_t)(firstDig100_u8 + secDigit10_u8 + thrDigit1_u8);
		
	}
	
	return temperature_s16;
}


//////////////////////////////////////////////////////////////


/*

void SwitchExecute(){
	//przycisk od lampy
	if(0==sw0){
		if (OFF_STATE==mainStateMachine || ON_STATE==mainStateMachine || WARM==mainStateMachine
		|| DELAY_WARM==mainStateMachine || FAN_WORK==mainStateMachine)
		
		if (lampState==0)
		{
			lampState=1;
		}
		else
		{
			lampState=5;
		}
		
		
		sw0=1;
	}
	
//przycisk od wiatraka
	if(0==sw1){
		if (OFF_STATE==mainStateMachine || ON_STATE==mainStateMachine || WARM==mainStateMachine
		|| DELAY_WARM==mainStateMachine || FAN_WORK==mainStateMachine)
		
		if (fanState==0)
		{
			fanState=1;
		}
		else
		{
			fanState=5;
		}
		
		sw1=1;
	}
//przycisk od wlaczenia menu
	if(0 == sw3_bo){
		if (OFF_STATE==mainStateMachine && timer???==3000 )//menu ukryte
		{
			mainStateMachine = HIDE_MENU_INIT_STATE;
		}
		if ( ON_STATE == mainStateMachine || WARM_STATE == mainStateMachine //menu normalne
		|| DELAY_WARM_STATE == mainStateMachine || FAN_WORK_STATE == mainStateMachine )
		{
			//setMenu_bo==TRUE;
			mainStateMachine = MENU_INIT_STATE;
		}
		
		sw1=1;
	}
	//obsluga przycisku on/off
	if(TRUE==sw1_bo)
	{
		if( mainStateMachine != OFF_STATE && mainStateMachine != OFF_INIT_STATE )
		{
			mainStateMachine = OFF_INIT_STATE;
		}
		else if( OFF_STATE == mainStateMachine )
		{
			mainStateMachine = ON_INIT_STATE;
		}
		
		sw1_bo=FALSE;
	}
	if(TRUE==sw2_bo)//przycisk wlaczania menu 
	{
		if( ON_STATE == mainStateMachine )
		{	
			mainStateMachine = MENU_INIT_STATE;
		}
		else if( MENU_STATE == mainStateMachine )
		{
			menuSw_bo=TRUE;
		}
		else if( HIDE_MENU_STATE == mainStateMachine )
		{
			*event_u8=H_MENU_NEXT_ITEM;
		}
		sw2_bo=FALSE;
	}
	if(TRUE==sw3_bo)//plus switch
	{
		if( ON_STATE == mainStateMachine )
		{
			incrementSwitch_bo=TRUE;
		}
		if( HIDE_MENU_STATE == mainStateMachine )
		{
			*event_u8=MENU_UP;//#define as 0
		}
		sw3_bo=FALSE;
	}
	if(TRUE==sw4_bo)//minus switch
	{
		if( ON_STATE == mainStateMachine )
		{
			decrementSwitch_bo=TRUE;
		}
		if( HIDE_MENU_STATE == mainStateMachine )
		{
			*event_u8=MENU_DOWN;//define as 1
		}
		sw4_bo=FALSE;
	}
	if(TRUE==sw5_bo)//zalaczanie lampy
		{
			if( ON_STATE == mainStateMachine )
			{
				decrementSwitch_bo=TRUE;
			}
			sw5_bo=FALSE;
		}
	if(TRUE==sw6_bo)//zalaczanie wiatraka
		{
			if( ON_STATE == mainStateMachine )
			{
				decrementSwitch_bo=TRUE;
			}
			sw6_bo=FALSE;
		}
	if(TRUE==sw7_bo && TRUE==sw8_bo)//jednoczesne nacisniecie przycisku on i menu (przycisk on caly czas trzymania wystawia 1 a przycisk menu po 3 s wyatwia 1 i wtedy mamy przejsicie do menu ukrytego   )
		{
			mainStateMachine = HIDE_MENU_STATE;
			*event_u8=MENU_UP;
		}

}

*/



int main(void)
{
	//zmienne lokalne:
	uint16_t modeManagerState=0;
	uint16_t prevWorkPanelStateF=0;
	
	//wartosci domyslne podczas startu urzadzebnia:
/*
	uint8_t temperatureHotMaxDef=110;
	uint8_t temperatureHotMinDef=30;
	uint8_t timerFanSetDef=10;
	uint8_t histTempDef=2;
	//domyslne wartosci czasow grzania i opoznenia
	uint16_t warmingTimeDef=0;
	uint16_t delayWarmingTimeDef=0;*/


	//nowe zmienne lokalne 
	
	
	
	//nowe zmiene lokalne koniec
	struct paramValue{ //wartosci domyslne musza sie miescic w wartosciach min Max
			uint8_t histTemp;
			uint8_t timerFanSet;
			uint8_t temperatureHotMax;
			uint8_t temperatureHotMin;
			uint16_t warmingTime;
			uint16_t delayWarmingTime;
			uint8_t temperatureHot;
			int8_t calibration;
			uint16_t warmingTimeMax;
			
		}defaultVal, savedValue;
	
	defaultVal.histTemp=2;
	defaultVal.timerFanSet=10;
	defaultVal.temperatureHotMax=90;
	defaultVal.temperatureHotMin=50;
	defaultVal.warmingTime=150;
	defaultVal.delayWarmingTime=0;
	defaultVal.temperatureHot=50;
	defaultVal.calibration=0;
	defaultVal.warmingTimeMax=150;
	
	

	//znaczniki mowiace o pierwszym odczycie temperatury i czasu
	uint8_t firstReadTemp=1;
	uint8_t firstReadTime2=1;
	uint8_t firstReadTime1=1;
	//tablice do konwersji liczba na znaki wyswietlacza
	uint8_t digTime[4]={0,0,0,0};
	uint8_t tabOutDsp[4]={0,0,0,0};
		
	//tablica ze znakami od aktualnej temperatury
	uint8_t tabTemp[4]={0,0,0,0};

	//do menu
	uint8_t returnMenuM=1; //info o miejscu z ktorego wlaczylismy opcje ustawienia temperatury
	uint16_t minMen=MINMEN;
	uint16_t maxMen=MAXMEN;
	uint16_t workPanelState40Set=(uint16_t)MENU_L1_WARM_TIME;
	uint16_t workPanelState41Set=(uint16_t)MENU_L1_T_MAX;
	uint16_t workPanelState42Set=(uint16_t)MENU_L1_T_MIN;
	uint16_t workPanelState43Set=(uint16_t)MENU_L1_FAN_TIME;
	uint16_t workPanelState44Set=(uint16_t)MENU_L1_T_HIST;
	uint16_t workPanelState45Set=(uint16_t)MENU_L1_CALIB;
	uint16_t workPanelState46Set=(uint16_t)MENU_L1_DEFAULT_OFF;
	uint16_t workPanelState47Set=(uint16_t)MENU_L1_7;
	//informacja o przeprowadzeniu przeliczenia na wyswietlacz
	uint8_t makeCount=1;
	

	//przy konwersji znakow odebranych przez rsa485 na temperature
	uint8_t temp1=0;
	uint8_t temp10=0;
	uint8_t temp100=0;
	
	//lampa
	uint8_t lampF=0;
	uint8_t auomaticLampTurnOn=0;
	
	//obecnosc panela
	uint8_t sendPanelInfo=0;
	
	//obsluga grzania
	uint8_t usingTimersSets=0;
	//uint8_t hot=0;
	uint8_t sendMWorkFurnance=0;
	uint8_t histTemp=0;//=histTempDef;
	
	//parametryzacja:
	
	//do czasow grzania pieca i 
	
	uint16_t warmingTimeMin=10;
	uint16_t warmingTimeMax=150;
	uint16_t warmingTime=warmingTimeMax;	
	//opoznienia zalaczenia pieca
	uint16_t delayWarmingTime=0;
	uint16_t delayWarmingTimeMin=0;
	uint16_t delayWarmingTimeMax=720;
	//parametryzacja histerezy
	uint8_t histTempMax=5;
	uint8_t histTempMin=1;
	//parametryzacja czasu dzialania wiatraka:
	uint8_t timerFanSet=0;//=timerFanSetDef;
	uint8_t timerFanSetMax=60;
	uint8_t timerFanSetMin =3;
	//parametryzacja temperatury minimalnej:
	uint8_t temperatureHotMinMax=50;
	uint8_t temperatureHotMinMin=20;
	//parametryzacja temperatury maksymalnej:
	uint8_t temperatureHotMaxMax=110;
	uint8_t temperatureHotMaxMin=60;
	//ustawienie zmiennej od kalibracji 
	int8_t calibration=0;

		
		
	uint8_t temperatureHot=0;
	uint8_t temperatureHotMax=0;
	uint8_t temperatureHotMin=0;
	//temperatura aktualna odczytana z bazy
	uint8_t actualTemperature=0;
	
	//obsluga wentylatora
	uint8_t sendMWorkFan=0;
	uint8_t timerFanWork=0;
	
	//wylaczenie nieuzywanego sterownika
	uint8_t timerOffController=0;
	
	//dodatkowa obsluga wiatraka
	uint8_t fanOn=0;
	
	//znacznik minut z bazy
	uint8_t getTimeMark=0;
	//do menu
	uint8_t tabMenu[15]={0xFF,0x8E, 0xC0, 0xF9,0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90,0xA3,0xAB,0x7E };//{puste pole, F, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, o, n, f}
	
	enum charDsp {sp=0xFF, F=0x8E, d0=0xC0, d1=0xF9,d2=0xA4,d3=0xB0,d4=0x99,d5=0x92,d6=0x82,d7=0xF8,d8=0x80,d9=0x90,o=0xA3,n=0xAB,f=0x7E, E=0x86 };
	enum charDsp charDsp1;
	//brak czujnika
	uint8_t error1SenM=0;
	uint8_t error2SenM=0;
	
	//flaga do wejscia w ukryte menu 
	uint8_t hideMenuMark=0;
	
	//zmienna mowiaca o przeslaniu wszystkich trzech cyfr temperatury-ustawiana przy otrzymaniu cyfry jednosci, kasowana w chwili przeliczenia
	uint8_t getAllDigit=0;
	
	  //--------------------inicjacja portow uC na poczatku programu-----------------------
	  DDRB = 0x00;
	  DDRC = 0x00;
	  DDRD = 0x00;
	  PORTB = 0x00;
	  PORTC = 0x00;
	  PORTD = 0x00;
	  //----------------------------ustawienie nieu¿ywanych pinów-----------------------------------------------
	  DDRA |= (1<<PA6);
	  PORTA |= (1<<PA6);
	  DDRA |= (1<<PA7);
	  PORTA |= (1<<PA7);
	  DDRB |= (1<<PB2);
	  PORTB |= (1<<PB2);
	  DDRB |= (1<<PB3);
	  PORTB |= (1<<PB3);
	  DDRC |= (1<<PC5);
	  PORTC |= (1<<PC5);
	  DDRC |= (1<<PC6);
	  PORTC |= (1<<PC6);
	  DDRC |= (1<<PC7);
	  PORTC |= (1<<PC7);

	  //-------------------ustawienia portów dla obslugi wyswietlacza LED-------------------------

	  DDRD |= (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7);
	  PORTD |= (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7);
	  
	  //-------------------ustawienia portów dla obslugi przyciskow-------------------------
	  DDRA &= ~(1<<PA0);
	  DDRA &= ~(1<<PA1);
	  DDRA &= ~(1<<PA2);
	  DDRA &= ~(1<<PA3);
	  DDRA &= ~(1<<PA4);
	  DDRA &= ~(1<<PA5);
	  DDRA &= ~(1<<PA6);
	  DDRA &= ~(1<<PA7);
	  PORTA |= (1<<PA0) | (1<<PA1) | (1<<PA2) | (1<<PA3) | (1<<PA4) | (1<<PA5);// | (1<<PA6) | (1<<PA7);

	  //-----------------------------ustawienia portow dla lacznosci 485 Master--------------------------------------
	  DDRD |= (1<<PD2);
	  PORTD &= ~(1<<PD2);
	  DDRD &= ~(1<<PD3);
	  PORTD &= ~(1<<PD3);
	  
	  DDRB |= (1<<PB1);
	  PORTB &= ~(1<<PB1);

	  //-----------------------------ustawienia portów dla diod-----------------------------------------------
	  DDRC |= (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3) | (1<<PC4) ;//| (1<<PC5) | (1<<PC6); // piny od diod ustawione jako wyjœcia
	  PORTC &= ~(1<<PC0); PORTC &= ~(1<<PC1); PORTC &= ~(1<<PC2); PORTC &= ~(1<<PC3); PORTC &= ~(1<<PC4); /*PORTC &= ~(1<<PC5); PORTC &= ~(1<<PC6); *///ustawienie pinow w stan niski

	  //-----------------------------ustawienia portów dla buzzera---------------------------------------------

	  DDRB |= (1<<PB0); //nozka 0 - ustawiona jako wyjscie
	  PORTB &= ~(1<<PB0); //ustawienie w stan niski na poczatku
	  
	  //----------------------------inicjalizacja lacznosci SPI------------------------------------------
	  SPI_Init();
	  //----------------------------inicjalizacja lacznosci i przerwan od USART--------------------------
	  
	  USART_Init(38400, 16000000);
	  USART_InitInterrupt();
	  //----------------------------inicjalizacja przerwañ od timera0------------------------------------
	  Timer0Init(194);
	  //----------------------------inicjalizacja przerwañ od timera2------------------------------------
	  Timer2Init(194);

	  _delay_ms(300);
//savedValue=defaultVal;
	  if ((eeprom_read_byte(&eAddr))==0xFF){
		  eeprom_write_block(&defaultVal,&strParamE1,sizeof(defaultVal));
		  eeprom_write_byte(&eAddr, 0);}
	  if ((eeprom_read_byte(&eAddr))==0){
		  eeprom_read_block(&savedValue,&strParamE1,sizeof(savedValue));
		  histTemp=savedValue.histTemp;
		  timerFanSet=savedValue.timerFanSet;
		  temperatureHotMax=savedValue.temperatureHotMax;
		  temperatureHotMin=savedValue.temperatureHotMin;
		  warmingTime=savedValue.warmingTime;
		  delayWarmingTime=savedValue.delayWarmingTime;
		  temperatureHot=savedValue.temperatureHot;
		  calibration=savedValue.calibration;
		  warmingTimeMax=savedValue.warmingTimeMax;
		  
	  }
	  
	  _delay_ms(300);
	  //globalne uaktywnienie przerwañ
	  sei();
	  wdt_enable(WDTO_2S);
	 // int writeEe=0;
	
	
    /* Replace with your application code */
    while (1) 
    {
		wdt_reset();
		if (tabRec[9]+tabRec[8]+tabRec[7]+tabRec[6]+tabRec[5]+tabRec[4]+tabRec[3]+tabRec[2]+tabRec[1]+tabRec[0]>0)
		{
			switch (tabRec[9]){
				
				case 0x20:
					getTimeMark=1;
				break;
				case 0x21:
					error1SenM=1;
				break;
				case 0x22:
					//error1SenM=0;
				break;
				case 0x23:
					error2SenM=1;
				break;
				case 0x24:
					//error2SenM=0;
				break;
				case 0x32:
					//tabTemp[1]=0xFF;//pusty ekran
					temp100=0;
				break;
				case 0x33:
					//tabTemp[1]=0xF9; //1
					temp100=100;
				break;
				case 0x34:
					//tabTemp[1]=0xBF; //-
					temp100=2;
				break;
				case 0x35:
					//tabTemp[2]=0xC0; // 0
					temp10=0;
				break;
				case 0x36:
					//tabTemp[2]=0xF9; //1
					temp10=10;
				break;
				case 0x37:
					//tabTemp[2]=0xA4;//2
					temp10=20;
				break;
				case 0x38:
					//tabTemp[2]=0xB0;//3
					temp10=30;
				break;
				case 0x39:
					//tabTemp[2]=0x99;//4
					temp10=40;
				break;
				case 0x3A:
					//tabTemp[2]=0x92; //5
					temp10=50;
				break;
				case 0x3B:
					//tabTemp[2]=0x82;//6
					temp10=60;
				break;
				case 0x3C:
					//tabTemp[2]=0xF8;//7
					temp10=70;
				break;
				case 0x3D:
					//tabTemp[2]=0x80;//8
					temp10=80;
				break;
				case 0x3E:
					//tabTemp[2]=0x90;//9
					temp10=90;
				break;
				case 0x3F:
					//tabTemp[3]=0xC0; // 0
					temp1=0;
					getAllDigit=1;
				break;
				case 0x40:
					//tabTemp[3]=0xF9; //1
					temp1=1;
					getAllDigit=1;
				break;
				case 0x41:
					//tabTemp[3]=0xA4;//2
					temp1=2;
					getAllDigit=1;
				break;
				case 0x42:
					tabTemp[3]=0xB0;//3
					temp1=3;
					getAllDigit=1;
				break;
				case 0x43:
				//	tabTemp[3]=0x99;//4
					temp1=4;
					getAllDigit=1;
				break;
				case 0x44:
				//	tabTemp[3]=0x92; //5
					temp1=5;
					getAllDigit=1;
				break;
				case 0x45:
					//tabTemp[3]=0x82;//6
					temp1=6;
					getAllDigit=1;
				break;
				case 0x46:
				//	tabTemp[3]=0xF8;//7
					temp1=7;
					getAllDigit=1;
				break;
				case 0x47:
					//tabTemp[3]=0x80;//8
					temp1=8;
					getAllDigit=1;
				break;
				case 0x48:
					//tabTemp[3]=0x90;//9
					temp1=9;
					getAllDigit=1;
				break;
				
			}
			RsShiftTab(tabRec);
		}
	     //------------------wysylka przez RSa----------------------------------
		 if(sendRS485M==1){
			 //zabezpieczenie dla RS485 konieczne w kilku miejscach programu
			 if (!(PIND & (1<<PD3))){nrByte=0;enableSend2=0;}
			 if (!(PIND & (1<<PD2))){enableSend1=0;sendStep=0;stepS=0;}
			 //zabezpieczenie dla RS485 zabezpieczajace przed ustawieniem 1 z obu stron
			 if (!(sendStep==0)){presM1++;}
			 else {presM1=0;}
			 if (presM1==15){sendStep=0; presM1=0;stepS=0;PORTD &= ~(1<<PD2);}
			 ///////////////////////////////////////RS 10 5 16//////////////////////////////////////

			 
			 if (tabDataRS[9]+tabDataRS[8]+tabDataRS[7]+tabDataRS[6]+tabDataRS[5]+tabDataRS[4]+tabDataRS[3]+tabDataRS[2]+tabDataRS[1]+tabDataRS[0]>0) {enableRS=1;}
			 else {enableRS=0;}
			 if ((!(PIND & (1<<PD3))) && enableRS==1){ //jesli wykryto zero z przeciwnej strony i mamy cos do wyslania

				 if (sendStep==0 ){
					 PORTD |= (1<<PD2); //ustawienie nozki na 1 swiadczace o wysylaniu sekwencji danych przez mastera
					 /*_delay_us(5);*/
					 USART_SendByteM(tabDataRS[9]);
				 sendStep=1;}//wysylka danych
				 else if (sendStep==1){
					 USART_SendByteM(tabDataRS[9]);
				 sendStep=2;}
				 else if (sendStep==2){
					 stepS++;
					 //if (returnData==0xFF && enableSend1==1){enableSend1=0;RsShiftTab(tabDataRS); sendStep=0;stepS=0;
					 if (returnData==tabDataRS[9] && enableSend1==1){enableSend1=0;RsShiftTab(tabDataRS); sendStep=0;stepS=0;presM1=0;
						 _delay_us(10);
					 PORTD &= ~(1<<PD2);}
					 else if (/*returnData==0x00 &&*/ enableSend1==1){enableSend1=0;sendStep=0;stepS=0;presM1=0;
						 _delay_us(10);
					 PORTD &= ~(1<<PD2);}

					 if (stepS==5){
						 sendStep=0;
						 //nrByte=0;
						 PORTD &= ~(1<<PD2);
						 stepS=0;
						 presM1=0;
					 }
				 }
			 }

			 
			 // if((PIND & (1<<PD3)) && enableSend2==1){_delay_us(100);USART_SendByteM(0xFF);enableSend2=0;nrByte=0;}
			 if((PIND & (1<<PD3)) && enableSend2==1){_delay_us(10);USART_SendByteM(tabDataRxc[0]);enableSend2=0;nrByte=0;}
			 if((PIND & (1<<PD3)) && enableSend2==2){_delay_us(10);USART_SendByteM(0x00);enableSend2=0;nrByte=0;}
			 
			 sendRS485M=0;
		 }
		 
        //------------------obsluga przyciskow----------------------------------
        //-----------------odczyt stanow w takt przerwania od timera1-----------
        if (switchCounterM==1){
	        switchCounterM=0;
	        if (sw0==0) sw0 = obslugaPrzyciskuKrotkiego2(0,PINA,0x01,15);// ON/OFF
	        if (sw1==0) sw1 = obslugaPrzyciskuKrotkiego2(1,PINA,0x02,15);// LAMP<-TEMP
	        if (sw2==0) sw2 = obslugaPrzyciskuKrotkiego2(2,PINA,0x04,15);// STRZ GORA
	        if (sw3==0) sw3 = obslugaPrzyciskuKrotkiego2(3,PINA,0x08,150);// WIATRAK<-TIMER
	        if (sw4==0) sw4 = obslugaPrzyciskuKrotkiego2(4,PINA,0x10,15);// MENU<-LAMP2
	        if (sw5==0) sw5 = obslugaPrzyciskuKrotkiego2(5,PINA,0x20,15);// STRZ DOL
	        sw6 = obslugaPrzyciskuKrotkiego(6,PINA,0x10,15);// MENU UKRYTE
	       // if (sw7==0) sw7 = obslugaPrzyciskuKrotkiego2(7,PINA,0x10,300);// SERV_OFF
	       // if (sw8==0) sw8 = obslugaPrzyciskuKrotkiego2(8,PINA,0x40,15);// LAMP1
	       // if (sw9==0) sw9 = obslugaPrzyciskuKrotkiego2(9,PINA,0x80,15);// LAMP3
	        if (sw10==0) sw10 = obslugaPrzyciskuKrotkiego4(10,PINA,0x04,150,100);// Menu szybkie pzewijanie UP
	        if (sw11==0) sw11 = obslugaPrzyciskuKrotkiego4(11,PINA,0x20,150,100);// Menu szybkie pzewijanie DOWN
	       // sw12=obslugaPrzyciskuKrotkiego(12, PINA ,0x04,25);
	        
        }
		
		//-------------------------obliczenie aktualnej temperatury po otrzymaniu wszystkich cyfr...
		if (getAllDigit==1){
			actualTemperature=(uint8_t)TabToTempConv(temp100,temp10,temp1);
			actualTemperature=actualTemperature+calibration;
			getAllDigit=0;
			makeCount=1;
		}
		
		
		
	    //-------------wysylanie informacji o obecnosci panela--------------
        if (timerP<=0){if (sendPanelInfo==1){RsDataTab(0x01, tabDataRS);sendPanelInfo=0;}}
		else {sendPanelInfo=1;}
	

    }
}



ISR(TIMER0_OVF_vect) //wywolywana co 0.001s
 {   
;
        
		if (timerHideMenu<3000) {
			timerHideMenu++;
		}
		 if (timerSec==1000){
	        timerSec=0;  
	        if (sec==3600){sec=0;}
			else {sec++;}
         }
         else {timerSec++;}
		if (timerMenu1<6000){
			timerMenu1++;
		}
		if (timerFurON<10000) {
			timerFurON++;
		}
		if(timerMenu2<2000){
			timerMenu2++;
		}
		
		if(timerShowSetTemp<10000)
		{timerShowSetTemp++;}
		else {timerShowSetTemp=0;}
		
	if (timerLedStop==2000){timerLedStop=0;}
		else{timerLedStop++;}
	if (timerLedStop==2000){timerLedStop=0;}
		else{timerLedStop++;}
    if (timerL1==0) {timerL1=1000;} //od lampy1
    else{timerL1--;}
    if (timerL2==0) {timerL2=1000;} //od lampy2
    else{timerL2--;}
    if (timerL3==0) {timerL3=1000;} //od lampy3
    else{timerL3--;}
   
    if (timerP==0) {timerP=301;} //od diody lampy
    else{timerP--;}
         
     if (timerServ>0) {timerServ--;} //timer od menu serwisowego
     if (timerServ2>0){timerServ2--;}else{timerServ2=2000;}

     
     if(timer0==600) {timer0=0; mark=1;}
     else {timer0++;}
     if (counterL==1000){counterL=0;}
     else {counterL++;}
     if  (timerBuz>0) {timerBuz--;}
     else {timerBuz=0;}
     
     if  (timerStop>0) {timerStop--;}
     else {timerStop=0;}
		      
	sendRS485M=1;

   
    TCNT0=194;
 }

ISR(TIMER2_OVF_vect) //wywolywana co 0.001s
{
	switchCounterM=1;
    //TCNT2=226;
	TCNT2=50;
}


/////////////////////////////RS 10 5 16/////////////////////////////////////////////////////////
	

		ISR (USART_RXC_vect){	
			static unsigned char emptyRxc;	
					if(!(PIND & (1<<PD3)) && sendStep==2){//jesli zero z przeciwnej strony i master jest w trybie oczekiwania na odpowiedz -  master wysyla bajt
						returnData = UDR;// USART_Receive();
						enableSend1=1;
					}
					else if ((PIND & (1<<PD3)) && sendStep==0){//jesli wykrylo 1 z przeciwnej strony a funkcja od wysy³ania ma krok ustawiony na 0(na pewno se zakonczyla)
						if (nrByte==0){
							tabDataRxc[0] = UDR;//USART_Receive();
							nrByte=1;}
						else if (nrByte==1){
							tabDataRxc[1] = UDR;//USART_Receive();
							nrByte=0;
							if (tabDataRxc[0]==tabDataRxc[1] )
							{RsDataTabRec(tabDataRxc[0],tabRec);enableSend2=1; }
							if (!(tabDataRxc[0]==tabDataRxc[1]))
							{enableSend2=2; }
						}
					}
					else {emptyRxc=UDR;/*USART_Receive();*/}
				}
				
