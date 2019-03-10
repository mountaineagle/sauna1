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

static uint8_t eepromInitDefaultFlag_u8 EEMEM; 
static strSaunaSavedParameter_t structEepromSavedValue EEMEM;

static strLedStatus_t Gv_LedStatus;

volatile uint8_t Gv_switchCounter_bo = FALSE;		/* !< variable set in 50ms timer interrupt to indicate 50ms time gap >! */
volatile uint8_t Gv_flagInterrupt50ms_bo = FALSE;	/* !< variable set in 20ms timer interrupt to indicate 20ms time gap >! */
volatile uint8_t Gv_tabRecDataRS485_au8[10] = {0u};			/* !< table for input RS data >! */

uint8_t Gv_flag50ms_bo;						/* !< flag set every 50ms used to increment static timer in function >!*/
uint8_t Gv_Timer10s_u8	= 0u;				/* !< timer used to count 10s period >!*/
uint8_t Gv_Timer8s_u8	= 0u;				/* !< timer used to count 8s period >!*/
uint8_t Gv_Timer6s_u8	= 0u;				/* !< timer used to count 6s period >!*/
uint8_t Gv_Timer3s_u8	= 0u;				/* !< timer used to count 3s period >!*/
uint8_t Gv_Timer2s_u8	= 0u;				/* !< timer used to count 2s period >!*/
uint8_t Gv_Timer1s_u8	= 0u;				/* !< timer used to count 1s period >!*/
uint8_t Gv_Timer300ms_u8 = 0u;				/* !< timer used to count 300ms period >!*/
uint8_t Gv_Timer50ms_u8	 = 0u;				/* !< timer used to count 50ms period >!*/
uint8_t Gv_Timer3sHideMenu_u8 = 0u;			/* !< timer used to count 3s period in hide manu >!*/
uint8_t Gv_CounterLed_u8 = 0u;				/* ! < counter used to count 500ms period for led >!*/

uint8_t Gv_ExtTimer1min_bo;				/* !< flag used to indicate 1min period >!*/
uint8_t Gv_StatusTimerReset_bo;			/* !< flag used to reset minute counter data - send to base >!*/

stateLamp_t Gv_LampState_e = LAMP_IDLE;		/* !< variable for remember actual state of Lamp state machine >! */
stateFan_t Gv_FanState_e = FAN_IDLE;		/* !< variable for remember actual state of Fan state machine >! */
stateFurnance_t Gv_FurState_e = FUR_IDLE;	/* !< variable for remember actual state of Furnance state machine >! */

//volatile uint16_t timerMenuChange_u16 = 0; /* !< variable for remember actual state of Fan state machine >! */

static stateMachine_t Gv_StateMachine_e = SM_OFF;			/* !< variable for remember actual state of main state machine >! */
static stateMachine_t Gv_StateMachinePrevState_e = SM_OFF;	/* !< variable for remember previous state of main state machine >! */
static uint8_t Gv_InitOnEntry_bo = TRUE;					/* !< flag for initialize first entry in every state from main state machine >! */

volatile uint16_t Gv_BuzCounter_u16 = 0u;	/* !< counter for buzzer >! */


/*--------old variables for RS 485 data handle - do not change------------------*/

volatile unsigned char rsByteSend=0x00;
volatile unsigned char rsRecByte=0x00;
volatile int Gv_OutputRSDataPresent_u8=0;
volatile unsigned char Gv_TabSendDataRS485_au8[10];
volatile unsigned char Gv_tabRecDataRS485_au8[10];
/* transmission for RS: */
volatile uint8_t Gv_SendRS485AllowFlag_u8;
volatile int returnDataInfo=0;
volatile unsigned char returnData=0;
volatile int Gv_EnableSend1_u8=1;
volatile int nrByte=0;
volatile int Gv_EnableSend2_u8=0;
volatile unsigned char tabDataRxc[2]={0,0};
volatile int Gv_SendStepLevel_u8=0;
/* protection for  rs */
volatile uint8_t Gv_SendStepCounter_u8;
volatile uint8_t presM1=0;

/* finish block for old variable definition */




static void saveDefaultParameterToEeprom(strSaunaSavedParameter_t *structEepromParam)
{
	const strSaunaSavedParameter_t structDefParameter = {
        (uint16_t)WARMING_TIME_MAX_DEF,
        (uint8_t)TEMPERATURE_HOTMAX_DEF,
        (uint8_t)TEMPERATURE_HOTMIN_DEF,
        (uint8_t)TIMER_FAN_DEF,
        (uint8_t)HIST_TEMP_DEF,
        (int8_t)CALIBRATION_DEF
    };
	
	eeprom_write_block(&structDefParameter,structEepromParam,sizeof(structDefParameter));
}

static void initEepromStruct(strSaunaParam_t *structWorkingValue, strSaunaSavedParameter_t* structEepromParam, uint8_t* eepromAddr_u8)
{
	if ((eeprom_read_byte(eepromAddr_u8))==0xFFu)
	{
		saveDefaultParameterToEeprom(structEepromParam);
		eeprom_write_byte(eepromAddr_u8,0u);
	}
	fillWorkingStructDuringStart(structWorkingValue, structEepromParam);
}


static void fillWorkingStructDuringStart(strSaunaParam_t *structWorkingValue, strSaunaSavedParameter_t *structEepromParam)
{
	eeprom_read_block(&(structWorkingValue->HiddenMenuParam),structEepromParam,sizeof(*structEepromParam));
	fillWorkingStructDuringSwitchingOn(structWorkingValue);
	
}

static void SaveToEEPROM(strSaunaParam_t* savedValue)
{
	eeprom_write_block(&(savedValue->HiddenMenuParam),&structEepromSavedValue,sizeof(structEepromSavedValue));
}


static void fillWorkingStructDuringSwitchingOn(strSaunaParam_t *structWorkingValue)
{
	structWorkingValue->TemperatureHot_u8 = TEMPERATURE_HOT_DEF;
	structWorkingValue->DelayWarmingTime_u16 = DELAY_WARMING_TIME_DEF;
	structWorkingValue->WarmingTime_u16 = structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16;
}
	
static void voidFunction(uint8_t event_u8,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		
}

	
/*void showHideMenuLevel(uint8_t event_u8,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel) 
{
		timerMenuChange=TIM_MENU_CHANGE;
		fillTabByLcdDataMenu(event_u8, displayOutData_pa);
}*/
	
	
static void setHistTempFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue_pstr, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		
		if(event_u8==SW_UP){(structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8)++;}
		if(event_u8==SW_DOWN){(structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8)--;}
		if((structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8) > HIST_TEMP_MAX){structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8=HIST_TEMP_MIN;}
		if((structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8) < HIST_TEMP_MIN){structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8=HIST_TEMP_MAX;}
		
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL,(int16_t)structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8, displayOutData_pa);
}
	
static void setCalibrationFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		
		if(event_u8==SW_UP){(structWorkingValue->HiddenMenuParam.Calibration_s8)++;}
		if(event_u8==SW_DOWN){(structWorkingValue->HiddenMenuParam.Calibration_s8)--;}
		if((structWorkingValue->HiddenMenuParam.Calibration_s8) > CALIBRATION_MAX){structWorkingValue->HiddenMenuParam.Calibration_s8=CALIBRATION_MIN;}
		if((structWorkingValue->HiddenMenuParam.Calibration_s8) < CALIBRATION_MIN){structWorkingValue->HiddenMenuParam.Calibration_s8=CALIBRATION_MAX;}
		
		fillLcdDataTab(CALIB_LCD_CONV_LEVEL,(int16_t)structWorkingValue->HiddenMenuParam.Calibration_s8, displayOutData_pa);
}
	
static void setMaxWorkingTimerFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		
		if(event_u8==SW_UP){(structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16)++;}
		if(event_u8==SW_DOWN){(structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16)--;}
		if(event_u8==SW_FAST_UP){(structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16)+=10;}
		if(event_u8==SW_FAST_DOWN){(structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16)-=10;}
		if(event_u8==SW_VERY_FAST_UP){(structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16)+=60;}
		if(event_u8==SW_VERY_FAST_DOWN){(structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16)-=60;}
		if((structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16) > WARMING_TIME_MAX_MAX){structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16 = WARMING_TIME_MAX_MIN;}
		if((structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16) < WARMING_TIME_MAX_MIN){structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16 = WARMING_TIME_MAX_MAX;}	
		
		fillLcdDataTab(TIME_LCD_CONV_LEVEL,(int16_t)structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16, displayOutData_pa);
}

static void setTempMaxFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{	
		if(event_u8==SW_UP){(structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8)++;}
		if(event_u8==SW_DOWN){(structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8)--;}
		if(event_u8==SW_FAST_UP){(structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8)++;}
		if(event_u8==SW_FAST_DOWN){(structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8)--;}
		if(event_u8==SW_VERY_FAST_UP){(structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8)+=10;}
		if(event_u8==SW_VERY_FAST_DOWN){(structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8)-=10;}
		if((structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8) > TEMPERATURE_HOTMAX_MAX){structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8 = TEMPERATURE_HOTMAX_MIN;}
		if((structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8) < TEMPERATURE_HOTMAX_MIN){structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8 = TEMPERATURE_HOTMAX_MAX;}	
		
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL,(int16_t)structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16, displayOutData_pa);
}
	
static void setTempMinFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{	
	if(event_u8==SW_UP)
	{
		(structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8)++;
	}
	if(event_u8==SW_DOWN)
	{
		(structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8)--;
	}
	if(event_u8==SW_FAST_UP)
	{
		(structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8)++;
	}
	if(event_u8==SW_FAST_DOWN)
	{
		(structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8)--;
	}
	if(event_u8==SW_VERY_FAST_UP)
	{
		(structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8)+=10;
	}
	if(event_u8==SW_VERY_FAST_DOWN)
	{
		(structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8)-=10;
	}
	if((structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8) > TEMPERATURE_HOTMIN_MAX )
	{
		structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8=TEMPERATURE_HOTMIN_MIN;
	}
	if((structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8) < TEMPERATURE_HOTMIN_MIN )
	{
		structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8=TEMPERATURE_HOTMIN_MAX;
	}	
		
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL,(int16_t)structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16, displayOutData_pa);
}

static void setTimeFanFunction(uint8_t event_u8,strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		if(event_u8==SW_UP){(structWorkingValue->HiddenMenuParam.TimerFanSet_u8)++;}
		if(event_u8==SW_DOWN){(structWorkingValue->HiddenMenuParam.TimerFanSet_u8)--;}
		if(event_u8==SW_FAST_UP){(structWorkingValue->HiddenMenuParam.TimerFanSet_u8)++;}
		if(event_u8==SW_FAST_DOWN){(structWorkingValue->HiddenMenuParam.TimerFanSet_u8)--;}
		if(event_u8==SW_VERY_FAST_UP){(structWorkingValue->HiddenMenuParam.TimerFanSet_u8)+=10;}
		if(event_u8==SW_VERY_FAST_DOWN){(structWorkingValue->HiddenMenuParam.TimerFanSet_u8)-=10;}
		if(structWorkingValue->HiddenMenuParam.TimerFanSet_u8>TIMER_FAN_MAX){structWorkingValue->HiddenMenuParam.TimerFanSet_u8=TIMER_FAN_MIN;}
		if(structWorkingValue->HiddenMenuParam.TimerFanSet_u8<TIMER_FAN_MIN){structWorkingValue->HiddenMenuParam.TimerFanSet_u8=TIMER_FAN_MAX;}
		
		fillLcdDataTab(TIME_LCD_CONV_LEVEL,(int16_t)structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16, displayOutData_pa);
}	
	
static void showHideMenuLevel(uint8_t event_u8, strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) {
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
		
		return;
	}	
	
	
	
static void setFurnanceWorkTime(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa) 
{
		if(SW_UP == event_u8)
		{
			(structWorkingValue->WarmingTime_u16)++;
		}
		else if(SW_DOWN == event_u8)
		{
			(structWorkingValue->WarmingTime_u16)--;
		}
		else if(SW_FAST_UP == event_u8)
		{
			(structWorkingValue->WarmingTime_u16) += 10;
		}
		else if(SW_FAST_DOWN == event_u8)
		{
			(structWorkingValue->WarmingTime_u16) -= 10u;
		}
		else if(SW_VERY_FAST_UP == event_u8)
		{
			(structWorkingValue->WarmingTime_u16) += 60u;
		}
		else if(SW_VERY_FAST_DOWN == event_u8 )
		{
			(structWorkingValue->WarmingTime_u16) -= 60u;
		}
		
		if(structWorkingValue->WarmingTime_u16 > structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16)
		{
			structWorkingValue->WarmingTime_u16 = WARMING_TIME_MAX_MIN;
		}
		if(structWorkingValue->WarmingTime_u16 < WARMING_TIME_MAX_MIN)
		{
			structWorkingValue->WarmingTime_u16 = structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16;
		}	
		
		fillLcdDataTab(TIME_LCD_CONV_LEVEL,(int16_t)structWorkingValue->WarmingTime_u16, displayOutData_pa);
}
	
static void setFurnanceDelay(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa) {
		
		if(SW_UP == event_u8)
		{
			(structWorkingValue->DelayWarmingTime_u16)++;
		}
		else if(SW_DOWN == event_u8)
		{
			(structWorkingValue->DelayWarmingTime_u16)--;
		}
		else if(SW_FAST_UP == event_u8)
		{
			(structWorkingValue->DelayWarmingTime_u16)+=10u;
		}
		else if(SW_FAST_DOWN == event_u8)
		{
			(structWorkingValue->DelayWarmingTime_u16)-=10u;
		}
		else if(SW_VERY_FAST_UP == event_u8)
		{
			(structWorkingValue->DelayWarmingTime_u16)+=60u;
		}
		else if(SW_VERY_FAST_DOWN == event_u8)
		{
			(structWorkingValue->DelayWarmingTime_u16)-=60u;
		}
		
		if(structWorkingValue->DelayWarmingTime_u16 > DELAY_WARMING_TIME_MAX)
		{
			structWorkingValue->DelayWarmingTime_u16 = DELAY_WARMING_TIME_MIN;
		}
		if(structWorkingValue->DelayWarmingTime_u16 < DELAY_WARMING_TIME_MIN)
		{
			structWorkingValue->DelayWarmingTime_u16 = DELAY_WARMING_TIME_MAX;
		}
		
		fillLcdDataTab(TIME_LCD_CONV_LEVEL,(int16_t)structWorkingValue->DelayWarmingTime_u16, displayOutData_pa);
}
	
static void TempSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa) 
{
	/* temperature changing depends on event*/
	if(SW_UP == event_u8)
	{
		(structWorkingValue->TemperatureHot_u8)++;
	}
	else if(SW_DOWN == event_u8)
	{	
		(structWorkingValue->TemperatureHot_u8)--;
	}
	else if(SW_FAST_UP == event_u8)
	{
		(structWorkingValue->TemperatureHot_u8)++;
	}
	else if(SW_FAST_DOWN == event_u8)
	{
		(structWorkingValue->TemperatureHot_u8)--;
	}
	else if(SW_VERY_FAST_UP == event_u8)
	{
		(structWorkingValue->TemperatureHot_u8) += 10;
	}
	else if(SW_VERY_FAST_DOWN == event_u8)
	{
		(structWorkingValue->TemperatureHot_u8) -= 10;
	}	
	else if(structWorkingValue->TemperatureHot_u8 > structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8)
	{
		structWorkingValue->TemperatureHot_u8 = structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8;
	}
	else if(structWorkingValue->TemperatureHot_u8 < structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8)
	{
		structWorkingValue->TemperatureHot_u8 = structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8;
	}	
	/* return to init state*/
	else if(TIMER_EVENT_6S == event_u8)
	{
		Gv_StateMachine_e = Gv_StateMachinePrevState_e;
	}
		
	/* fill Lcd tab with temperature data*/
	fillLcdDataTab(TEMP_LCD_CONV_LEVEL,(int16_t)structWorkingValue->TemperatureHot_u8, displayOutData_pa);
		
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
		Gv_LedStatus.TempLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;
		
		Gv_FanState_e = FAN_OFF;
		Gv_LampState_e = LAMP_OFF;
		Gv_FurState_e = FUR_OFF;
			
		displayOutData_pa[0] = 0;
		displayOutData_pa[1] = 0;
		displayOutData_pa[2] = 0;
		displayOutData_pa[3] = 0;
		
		/*sent Off information by RS485*/
		OnOffRSinfoSend(FALSE);
			
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
		/*sent Off information by RS485*/
		OnOffRSinfoSend(TRUE);
		Gv_StateMachine_e = SM_IN;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(SW_FAN == event_u8)
	{
		if (FAN_ON1_EXEC == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON1;
		}
	}
	else if(SW_LAMP == event_u8)
	{
		if (LAMP_ON1_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_ON1;
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



static void InStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t* displayOutData_pa)
{
	/*used to count position of sec mark on display */
	static uint8_t tickDislplayMark_u8 = 0u; 
	
	/*initialize variable and set states on entry to this state */
	if (TRUE == Gv_InitOnEntry_bo)
	{
		/* remember value in case of return from other states */
		Gv_StateMachinePrevState_e = SM_IN;	
		
		/* set default parameter for some working data */
		fillWorkingStructDuringSwitchingOn(structWorkingValue);
		
		/*clear led after return from delay state*/
		Gv_LedStatus.TempLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;

		Gv_FanState_e = FAN_OFF;
		Gv_LampState_e = LAMP_OFF;
		Gv_FurState_e = FUR_OFF;
		/* clear initOnEntry flag - initialize was done*/
		Gv_InitOnEntry_bo = FALSE;
	}

	
	if (	TRUE == processedRSInData->dataInRS.Error1Sen_bo
		||	TRUE == processedRSInData->dataInRS.Error2Sen_bo)
	{
		Gv_StateMachine_e = SM_ERROR;
		Gv_InitOnEntry_bo = TRUE;
	}
	/* switch between states depending on event*/
	else if (SW_OFF_ON == event_u8)
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
		if (FAN_ON1_EXEC == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON2;
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
	else if(TIMER_EVENT_8S == event_u8 && 0u == structWorkingValue->DelayWarmingTime_u16)
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
	fillLcdDataTab(TEMP_LCD_CONV_LEVEL, (int16_t)(structWorkingValue->TemperatureHot_u8) , displayOutData_pa);
	
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
	displayOutData_pa[tickDislplayMark_u8] = displayOutData_pa[tickDislplayMark_u8] & (uint8_t)MPOINT;
	
	return ;
	
}


static void HideMenuStateExecute(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t* displayOutData_pa)
{	
	static uint8_t currentMenuState_u8 = 0u;
	/*	menuItem_t tabHideMenu[13]={
			//{{SW_UP, SW_DOWN, SW_CHANGE, TIME_CHANGE, IDLE}, pointerToFunction}
			{{0,0,1,0,0}, voidFunction(event_u8,structWorkingValue, displayOutData_pa, 0)},
			{{1,1,3,2,1}, showHideMenuLevel(event_u8,structWorkingValue, displayOutData_pa, 1)},
			{{2,2,3,2,2}, setMaxWorkingTimerFunction(event_u8,structWorkingValue, displayOutData_pa, 2)},
			{{3,3,5,4,3}, showHideMenuLevel(event_u8,structWorkingValue, displayOutData_pa, 3)},
			{{4,4,5,4,4}, setTempMaxFunction(event_u8,structWorkingValue, displayOutData_pa, 4)},
			{{5,5,7,6,5}, showHideMenuLevel(event_u8,structWorkingValue, displayOutData_pa, 5)},
			{{6,6,7,6,6}, setTempMinFunction(event_u8,structWorkingValue, displayOutData_pa, 6)},
			{{7,7,9,8,7}, showHideMenuLevel(event_u8,structWorkingValue, displayOutData_pa, 7)},
			{{8,8,9,8,8}, setTimeFanFunction(event_u8,structWorkingValue, displayOutData_pa, 8)},
			{{9,9,11,10,9}, showHideMenuLevel(event_u8,structWorkingValue, displayOutData_pa, 9)},
			{{10,10,11,10,10}, setHistTempFunction(event_u8,structWorkingValue, displayOutData_pa, 10)},
			{{11,11,0,12,11}, showHideMenuLevel(event_u8,structWorkingValue, displayOutData_pa, 11)},
			{{12,12,0,12,12}, setCalibrationFunction(event_u8,structWorkingValue, displayOutData_pa, 12)},
		};*/
	
	menuItem_t tabHideMenu[13]={
		//{{SW_UP, SW_DOWN, SW_CHANGE, TIME_CHANGE, IDLE}, pointerToFunction}
		{{0,0,1,0,0}, voidFunction},
		{{1,1,3,2,1}, showHideMenuLevel},
		{{2,2,3,2,2}, setMaxWorkingTimerFunction},
		{{3,3,5,4,3}, showHideMenuLevel},
		{{4,4,5,4,4}, setTempMaxFunction},
		{{5,5,7,6,5}, showHideMenuLevel},
		{{6,6,7,6,6}, setTempMinFunction},
		{{7,7,9,8,7}, showHideMenuLevel},
		{{8,8,9,8,8}, setTimeFanFunction},
		{{9,9,11,10,9}, showHideMenuLevel},
		{{10,10,11,10,10}, setHistTempFunction},
		{{11,11,0,12,11}, showHideMenuLevel},
		{{12,12,0,12,12}, setCalibrationFunction},
	};
	
	/*
	menuItem_t tabMenu[5]={
		//{{SW_UP, SW_DOWN, SW_CHANGE, TIME_CHANGE, IDLE}, pointerToFunction}
		{{0,0,1,0,0}, voidFunction(event,&Gv_WorkingParam, displayOutData_pa, 0)},
		{{1,1,3,2,1}, showHideMenuLevel(event,&Gv_WorkingParam, displayOutData_pa, 1)},
		{{2,2,3,2,2}, setFurnanceWorkTime(event,&Gv_WorkingParam, displayOutData_pa, 2)},
		{{3,3,0,4,3}, showHideMenuLevel(event,&Gv_WorkingParam, displayOutData_pa, 3},
		{{4,4,0,4,4}, setFurnanceDelay(event,&Gv_WorkingParam, displayOutData_pa, 4)},
	};*/
	
	/*initialize variable and set states on entry to this state */
	if (TRUE == Gv_InitOnEntry_bo)
	{
		/* remember value in case of return from other states 
		Gv_StateMachinePrevState_e = SM_IN;	*/
		
		/* set current status of hide menu */
		currentMenuState_u8 = 0u;
		event_u8 = SW_TIMER;
		
		/*clear led after return from delay state*/
		Gv_LedStatus.TempLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;

		Gv_FanState_e = FAN_OFF;
		Gv_LampState_e = LAMP_OFF;
		Gv_FurState_e = FUR_OFF;
		/* clear initOnEntry flag - initialize was done*/
		Gv_InitOnEntry_bo = FALSE;
	}
	
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	
	changeMenu(event_u8, &currentMenuState_u8, tabHideMenu, structWorkingValue, displayOutData_pa);
	
	return;
}



static void FurnanceOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
{
	/* initialize variable on entry to state */
	if (TRUE == Gv_InitOnEntry_bo)
	{
		/* remember value in case of return from other states */
		Gv_StateMachinePrevState_e = SM_FURNANCE_ON;
			
		/* clear led after return from delay state */
		Gv_LedStatus.TempLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOffLed_u8 = LED_ON;
		
		/* switching on the lamp */
		Gv_LampState_e = LAMP_AUTO_ON;
		
		/* switching on the furnance on entry */
		Gv_FurState_e = FUR_ON;
		
		/* reset min timer */
		Gv_StatusTimerReset_bo = TRUE;
			
		/* clear initOnEntry flag - initialize was done */
		Gv_InitOnEntry_bo = FALSE;
	}
	

	/* switch executing */
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if (		TRUE == processedRSInData->dataInRS.Error1Sen_bo
				||	TRUE == processedRSInData->dataInRS.Error2Sen_bo)
	{
		Gv_StateMachine_e = SM_ERROR;
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
		if (FAN_ON1_EXEC == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON2;
		}
	}
	else if(SW_LAMP == event_u8)
	{
		if (LAMP_AUTO_ON_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_AUTO_ON;
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
	
	//----------histereza zalaczania i wylaczania pieca
	if (processedRSInData->countedTemperature_s16 <= (structWorkingValue->TemperatureHot_u8 - structWorkingValue->HiddenMenuParam.HistTemp_u8))
	{
		Gv_FurState_e = FUR_AUTO_ON;
	}					
	else if(processedRSInData->countedTemperature_s16 >= (structWorkingValue->TemperatureHot_u8/*+histTemp*/))
	{
		Gv_FurState_e = FUR_BREAK;
	}
	else
	{
		; /* do nothng */
	} 
	
	/* count timer */
	if(TRUE == Gv_ExtTimer1min_bo)
	{
		structWorkingValue->WarmingTime_u16 -- ; 
	}
	
	if (0u == structWorkingValue->WarmingTime_u16)
	{
		Gv_StateMachine_e = SM_FAN_ON;
		Gv_InitOnEntry_bo = TRUE; 
		Gv_FurState_e = FUR_OFF;
	}
	
	/* fill LCD data tab by temperature hot*/
	if (TIMER_EVENT_8S > Gv_Timer10s_u8)
	{
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL, (int16_t)(structWorkingValue->TemperatureHot_u8) , displayOutData_pa);
	}
	else 
	{	
		addAnimationToLedDisplay(displayOutData_pa);
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL, (int16_t)(structWorkingValue->ActualTemperature_s8) , displayOutData_pa);
	}
	
	return ;
	
}

static void addAnimationToLedDisplay(uint8_t *displayOutData_pa)
{
	if (Gv_Timer10s_u8>TIMER_EVENT_7S)
	{ 
		displayOutData_pa[0]= DSL3;
	}
	else if (Gv_Timer10s_u8>TIMER_EVENT_6S)
	{ 
		displayOutData_pa[0]= DSL2;
	}
	else if (Gv_Timer10s_u8>TIMER_EVENT_5S)
	{ 
		displayOutData_pa[0] = DSL1;
	}
	else if (Gv_Timer10s_u8>TIMER_EVENT_4S)
	{ 
		displayOutData_pa[0] = SPCJ;
	}
	else if (Gv_Timer10s_u8>TIMER_EVENT_3S)
	{ 
		displayOutData_pa[0]= DSL3;
	}
	else if (Gv_Timer10s_u8>TIMER_EVENT_2S)
	{ 
		displayOutData_pa[0]= DSL2;
	}
	else if (Gv_Timer10s_u8>TIMER_EVENT_1S)
	{ 
		displayOutData_pa[0]= DSL1;
	}
	else
	{ 
		displayOutData_pa[0]= SPCJ;
	}	
		
	return ;
}


static void ClearAllTimers()
{
	Gv_Timer10s_u8 = 0u;
	Gv_Timer8s_u8 = 0u;
	Gv_Timer6s_u8 = 0u;
	Gv_Timer2s_u8 = 0u;
	Gv_Timer1s_u8 = 0u;
}

/*umieœciæ funkcjê na koñcu programu programu */
static void TimeCounter()
{
	if (TIMER_EVENT_10S == Gv_Timer10s_u8)
	{
		Gv_Timer10s_u8 = 0u;
	}
	else
	{
		/* do nothing */
	}
	
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
	
	if (TIMER_EVENT_300MS == Gv_Timer300ms_u8)
	{
		Gv_Timer300ms_u8 = 0u;
	}
	else
	{
		/* do nothing */
	}
	
	if (TIMER_SWITCH_50MS == Gv_Timer50ms_u8)
	{
		Gv_Timer50ms_u8 = 0u;
	}
	else
	{
		/* do nothing */
	}
	
	if (TIMER_EVENT_3S > Gv_Timer3sHideMenu_u8)
	{
			Gv_Timer3sHideMenu_u8 ++ ;
	}
	else
	{
		/* do nothing */
	}
	
	if (TIMER_EVENT_1S > Gv_CounterLed_u8)
	{
		Gv_CounterLed_u8 ++ ;
	}
	else
	{
		Gv_CounterLed_u8 = 0u;
	}
	
	
	if (TRUE == Gv_flagInterrupt50ms_bo)
	{
		Gv_flagInterrupt50ms_bo = FALSE;
		Gv_Timer10s_u8 ++;
		Gv_Timer8s_u8 ++;
		Gv_Timer6s_u8 ++;
		Gv_Timer2s_u8 ++;
		Gv_Timer1s_u8 ++;
		Gv_Timer300ms_u8++;
		Gv_Timer50ms_u8++;
	}
}




static void FurnanceDelayStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
{
	if (TRUE == Gv_InitOnEntry_bo)
	{
		/* remember value in case of return from other states */
		Gv_StateMachinePrevState_e = SM_FURNANCE_DELAY;
		
		/* clear led after return from delay state */
		Gv_LedStatus.TempLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_ON;
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;
		
		/* reset min timer */
		Gv_StatusTimerReset_bo = TRUE;
		
		/* clear initOnEntry flag - initialize was done */
		Gv_InitOnEntry_bo = FALSE;
	}
	

	/* switch executing */
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if (	TRUE == processedRSInData->dataInRS.Error1Sen_bo
			||	TRUE == processedRSInData->dataInRS.Error2Sen_bo)
	{
		Gv_StateMachine_e = SM_ERROR;
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
		if (FAN_ON1_EXEC == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON2;
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
		structWorkingValue->DelayWarmingTime_u16 -- ; 
	}
	
	/* switch to next state */
	if (0u == structWorkingValue->DelayWarmingTime_u16 )
	{
		Gv_StateMachine_e = SM_FURNANCE_ON;
		Gv_InitOnEntry_bo = TRUE; 
	}
	
	/* fill LCD data tab by delay timer */
	fillLcdDataTab(TIME_LCD_CONV_LEVEL, (int16_t)(structWorkingValue->DelayWarmingTime_u16) , displayOutData_pa);
	
	return ;
}


static void TimerSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa)
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
		setFurnanceWorkTime(event_u8, structWorkingValue, displayOutData_pa);
		Gv_LedStatus.TimerOffLed_u8 = LED_BLINK;
	}
	else
	{
		setFurnanceDelay(event_u8, structWorkingValue, displayOutData_pa);
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_BLINK;
	}
	
	return ;
}


static void FanOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa)
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
		Gv_FanState_e = FAN_ON1;
		
		/* reset min timer */
		Gv_StatusTimerReset_bo = TRUE;
		
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
		if (FAN_AUTO_ON_EXEC == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_AUTO_ON;
		}
	}
	else if(SW_LAMP == event_u8)
	{
		if (LAMP_AUTO_ON_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_AUTO_ON;
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
		structWorkingValue->HiddenMenuParam.TimerFanSet_u8 -- ;
	}
	/* switching to next state */
	if (0u == structWorkingValue->HiddenMenuParam.TimerFanSet_u8 )
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	
	/* LED display */
	/* fill LCD data tab by delay timer */
	fillLcdDataTab(TEMP_LCD_CONV_LEVEL, (int16_t)(structWorkingValue->ActualTemperature_s8) , displayOutData_pa);
	
	return ;
}


static void ErrorStateExecute(uint8_t event_u8, processedDataInRS_t *processedRSInData, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa)
{
	uint8_t errorId_u8 = 0u;
		
	/*initialize variable and set states on entry to this state */
	if (TRUE == Gv_InitOnEntry_bo)
	{
		/* remember value in case of return from other states */
		//Gv_StateMachinePrevState_e = SM_IN;
			
		/*clear led after return from delay state*/
		Gv_LedStatus.TempLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;
			
		Gv_FanState_e = FAN_OFF;
		//Gv_LampState_e = LAMP_OFF;
		Gv_FurState_e = FUR_OFF;
		
		/* clear initOnEntry flag - initialize was done*/
		Gv_InitOnEntry_bo = FALSE;
			
		//wyslanie komendy do wylaczenia pieca 
	}
		
		
	/* switch executing */
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if (TRUE == processedRSInData->dataInRS.Error1Sen_bo)
	{
		errorId_u8 = SENS_ERROR1;
	}
	else if (TRUE == processedRSInData->dataInRS.Error2Sen_bo)
	{
		errorId_u8 = SENS_ERROR2;
	}
	else
	{
		; /* do nothing */
	}
		
	/* fill LCD data tab by temperature hot*/
	fillLcdDataTab(ERROR_LCD_LEVEL, (int16_t)errorId_u8 , displayOutData_pa);		
	
	return ;
}

static void SwEventChoose (uint8_t *switchCounter_bo, uint8_t *swEvent_u8 )	
 {     
	*swEvent_u8 = 0u;
	if (TIMER_SWITCH_50MS == *switchCounter_bo)
	{
		*switchCounter_bo = FALSE;
	    if ( SHORT == obslugaPrzyciskuKrotkiego2(0u,PINA,0x01u,15u) )// ON/OFF
		{
			*swEvent_u8 = SW_OFF_ON; 
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego2(1u,PINA,0x02u,15u) )// LAMP<-TEMP
		{
			*swEvent_u8 = SW_LAMP; 
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego2(2u,PINA,0x04u,15u) )// STRZ GORA
		{
			*swEvent_u8 = SW_UP; 
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego2(3u,PINA,0x08u,15u) )// WIATRAK<-TIMER
		{
			*swEvent_u8 = SW_FAN;
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego2(4u,PINA,0x10u,15u) )// MENU<-LAMP2
		{
			*swEvent_u8 = SW_TIMER;
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego2(5u,PINA,0x20u,15u) )// STRZ DOL
		{
			*swEvent_u8 = SW_DOWN; 
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego(6u,PINA,0x10u,15u) )// MENU UKRYTE
		{
			*swEvent_u8 = SW_MENU;
		}
	    if ( SHORT2 == obslugaPrzyciskuKrotkiego4(7u,PINA,0x04u,150u,100u) )// Menu bardzo szybkie pzewijanie UP
		{
			*swEvent_u8 = SW_VERY_FAST_UP;
		}
	    if ( SHORT1 == obslugaPrzyciskuKrotkiego4(8u,PINA,0x04u,150u,100u) )// Menu szybkie pzewijanie UP
	    {
		    *swEvent_u8 = SW_FAST_UP;
	    }
	    if ( SHORT2 == obslugaPrzyciskuKrotkiego4(9u,PINA,0x20u,150u,100u) )// Menu bardzo szybkie pzewijanie DOWN
		{
			*swEvent_u8 = SW_VERY_FAST_DOWN;
		}
	    if ( SHORT1 == obslugaPrzyciskuKrotkiego4(10u,PINA,0x20u,150u,100u) )// Menu szybkie pzewijanie DOWN
	    {
		    *swEvent_u8 = SW_FAST_DOWN;
	    }
	}
	return;
 }

static void StateMachine(uint8_t event_u8, strSaunaParam_t * structWorkingValue,  processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
{
	switch(Gv_StateMachine_e)
	{
		case SM_HIDE_MENU:
			HideMenuStateExecute(event_u8, structWorkingValue, displayOutData_pa);
			break;
		case SM_OFF:
			OffStateExecute(event_u8, structWorkingValue, displayOutData_pa);
			break;
		case SM_IN:
			InStateExecute(event_u8, structWorkingValue, processedRSInData, displayOutData_pa);
			break;
		case SM_TEMP_SET:
			TempSetStateExecute(event_u8, structWorkingValue, displayOutData_pa);
			break;
		case SM_TIMER_SET:
			TimerSetStateExecute(event_u8, structWorkingValue, displayOutData_pa);
			break;
		case SM_FURNANCE_DELAY:
			FurnanceDelayStateExecute(event_u8, structWorkingValue, processedRSInData, displayOutData_pa);
			break;
		case SM_FURNANCE_ON:
			FurnanceOnStateExecute(event_u8, structWorkingValue, processedRSInData, displayOutData_pa);
			break;
		case SM_FAN_ON:
			FanOnStateExecute(event_u8, structWorkingValue, displayOutData_pa);
			break;
		case SM_ERROR:
			ErrorStateExecute(event_u8, processedRSInData , structWorkingValue, displayOutData_pa);
		break;

	}
}

static void FurnanceStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData)
{
	static uint16_t timerFur_u16 = 0u;
	
	if ((timerFur_u16 > 0u) && (TRUE == processedRSInData->dataInRS.MinuteEvent_bo))
	{
		timerFur_u16 --;
	}
		
	switch (Gv_FurState_e)
	{
		case FUR_IDLE: /* IDLE state */
		
		break;
		case FUR_ON: /* switch on the furnance for te first time */
			RsDataTab(RS_FUR_ON, Gv_TabSendDataRS485_au8);
			timerFur_u16 = structWorkingValue->WarmingTime_u16;
			Gv_LedStatus.TempLed_u8 = LED_ON;
			Gv_FurState_e = FUR_AUTO_ON_EXEC;
		break;
		case FUR_AUTO_ON: /* switch on the furnance for te first time */
			RsDataTab(RS_FUR_ON, Gv_TabSendDataRS485_au8);
			Gv_FurState_e = FUR_AUTO_ON_EXEC;
		break;
		case FUR_AUTO_ON_EXEC: /* automatic working of the furnance */
			if(0u == timerFur_u16) /* if timer reach 0 then furnance state change for OFF */
			{
				Gv_FurState_e = FUR_OFF;
			}
		break;
		case FUR_BREAK: /* switch off the furnance for some period because of reach max temperature */
			 RsDataTab(RS_FUR_OFF, Gv_TabSendDataRS485_au8);
			 Gv_FurState_e = FUR_BREAK_EXEC;
		break;
		case FUR_BREAK_EXEC: /* switch off the furnance for some period because of reach max temperature */
			 if(0u == timerFur_u16) /* if timer reach 0 then furnance state change for OFF */
			 {
				Gv_FurState_e = FUR_OFF;
			 }
		break;
		case FUR_OFF: /* switching off the furnance */
			RsDataTab(RS_FUR_OFF, Gv_TabSendDataRS485_au8);
			Gv_LedStatus.TempLed_u8 = LED_OFF;
			Gv_FurState_e = FUR_IDLE;
		break;

	}
	
}

static void FanStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData)
{
	static uint16_t timerFan_u16 = 0u;
		
	if ((timerFan_u16 > 0u) && (TRUE == processedRSInData->dataInRS.MinuteEvent_bo))
	{
		timerFan_u16 --;		
	}

	switch (Gv_FanState_e)
	{
		case FAN_IDLE: /* IDLE state */
		
		break;
		case FAN_ON1: /* switching on fan in state off */
			RsDataTab(RS_FAN_ON, Gv_TabSendDataRS485_au8);
			Gv_LedStatus.FanLed_u8 = LED_BLINK;
			timerFan_u16 = structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16;
			Gv_FanState_e = FAN_ON1_EXEC;
		break;
		case FAN_ON2: /* switching on fan in state differ than off */
			RsDataTab(RS_FAN_ON, Gv_TabSendDataRS485_au8);
			Gv_LedStatus.FanLed_u8 = LED_BLINK;
			timerFan_u16 = structWorkingValue->HiddenMenuParam.TimerFanSet_u8;
			Gv_FanState_e = FAN_ON1_EXEC;
		break;
		case FAN_ON1_EXEC:/* executing of working lamp after switch on */
			
			if(timerFan_u16==0) /* if counter == 0u then fan is switching off */
			{
				Gv_FanState_e = FAN_OFF;
			}
		break;
		case FAN_AUTO_ON: /* automatic work start */
			RsDataTab(RS_FAN_ON, Gv_TabSendDataRS485_au8);
			timerFan_u16 = structWorkingValue->HiddenMenuParam.TimerFanSet_u8;
			Gv_LedStatus.FanLed_u8 = LED_ON;
			Gv_FanState_e = FAN_AUTO_ON_EXEC;
		break;
		case FAN_AUTO_ON_EXEC: /* automatic work execute */
			if(0u == timerFan_u16) /* if counter == 0u then fan is switching off */
			{
				Gv_FanState_e = FAN_OFF;
			}
		break;
		case FAN_OFF: /* switch off the fan */
			RsDataTab(RS_FAN_OFF, Gv_TabSendDataRS485_au8);
			Gv_LedStatus.FanLed_u8 = LED_OFF;
			Gv_FanState_e = FAN_IDLE;
		break;

	}

}

static void LampStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData)
{
	static uint16_t timerLamp_u16 = 0u;
	
	if ((timerLamp_u16 > 0u) && (TRUE == processedRSInData->dataInRS.MinuteEvent_bo))
	{
		timerLamp_u16--;
	}
		
	switch (Gv_LampState_e)
	{
		case LAMP_IDLE: /* IDLE state */
		
		break;
		case LAMP_ON1: /* switching on in state off */
			RsDataTab(RS_LAMP_ON, Gv_TabSendDataRS485_au8);
			Gv_LedStatus.ProgLed_u8 = LED_BLINK;
			timerLamp_u16 = structWorkingValue->HiddenMenuParam.WarmingTimeMax_u16;
			Gv_LampState_e = LAMP_ON1_EXEC;
		break;
		case LAMP_ON1_EXEC:/* executing of working lamp in state off */		
			if(timerLamp_u16 == 0u) /* if counter == 0u then lamp is switching off */
			{
				Gv_LampState_e = LAMP_OFF;
			}
		break;
		case LAMP_ON2: /* switching on lamp in state differ than off */
			RsDataTab(RS_LAMP_ON, Gv_TabSendDataRS485_au8);
			Gv_LedStatus.ProgLed_u8 = LED_BLINK;
			Gv_LampState_e = LAMP_ON2_EXEC;
		break;
		case LAMP_ON2_EXEC: /* working lamp in state differ than off */
		break;
		case LAMP_AUTO_ON: /* automatic work start */
			RsDataTab(RS_LAMP_ON, Gv_TabSendDataRS485_au8);
			//timerLamp_u16 = structWorkingValue->warmingTime_u16;
			Gv_LedStatus.ProgLed_u8 = LED_ON;
			Gv_LampState_e = LAMP_AUTO_ON_EXEC;
		break;
		case LAMP_AUTO_ON_EXEC: /* automatic work execute */
/*
		if(0u == timerLamp_u16) //jesli timer dojdzie do zera to swiatlo sie wylacza
		{
			*lampState_pu8 = LAMP_OFF;
		}*/
		break;
		case LAMP_OFF: /* switching off the lamp */
			RsDataTab(RS_LAMP_OFF, Gv_TabSendDataRS485_au8);
			Gv_LedStatus.ProgLed_u8 = LED_OFF;
			Gv_LampState_e = LAMP_IDLE;
		break;

	}
}

static void LedWorkStatus()
{
	
	if (LED_BLINK == Gv_LedStatus.ProgLed_u8)
	{
		BlinkLed(PROG, Gv_CounterLed_u8,10u);
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
		BlinkLed(FAN, Gv_CounterLed_u8,10u);
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
		BlinkLed(TIMER_ON, Gv_CounterLed_u8,10u);
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
		BlinkLed(TIMER_OFF, Gv_CounterLed_u8,10u);
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
		BlinkLed(TEMP, Gv_CounterLed_u8,10u);
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

	
	
static void fillTabByLcdDataMenu(uint8_t ledDisplayLevel_u8, uint8_t *displayOutData_pa){
	
	static uint8_t previousLevel_u8 = 0u;
	
	if (previousLevel_u8 != ledDisplayLevel_u8)
	{
		DsLedOff();
		previousLevel_u8 = ledDisplayLevel_u8;
	}
		
	displayOutData_pa[1] = LEF;
	displayOutData_pa[2] = SPCJ;
	displayOutData_pa[3] = SPCJ;
	
	switch (ledDisplayLevel_u8)
	{
		case 0://menu 1
			displayOutData_pa[0] = D1;
		break;
		case 1: //menu 2
			displayOutData_pa[0] = D2;
		break;
		case 2://menu 3
			displayOutData_pa[0] = D3;
		break;
		case 3://menu 4
			displayOutData_pa[0] = D4;
		break;
		case 4://menu 5
			displayOutData_pa[0] = D5;
		break;
		case 5://menu 6
			displayOutData_pa[0] = D6;
		break;
	}
	
	return;
}

		
static void fillLcdDataTab(uint8_t conversionLevel_u8, int16_t valueToConvert_u16, uint8_t *displayOutData_pa){
		
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
			case 3:
				displayOutData_pa[3] = SPCJ;
				displayOutData_pa[2] = SPCJ;
				displayOutData_pa[1] = LE;
				if (SENS_ERROR1 == valueToConvert_u16)
				{
					displayOutData_pa[0] = D1;
				}
				else if (SENS_ERROR2 == valueToConvert_u16)
				{
					displayOutData_pa[0] = D2;
				}
				else 
				{
					displayOutData_pa[0] = D0;
				}
			break;
		}
		
		return;
	}
	
static void changeMenu(uint8_t event_u8, uint8_t *currentMenuState_u8, menuItem_t *tabHideMenu, strSaunaParam_t *structure_pstr , uint8_t * tab_pu8 )
{
	const uint8_t switchEvent_u8 = 2u; /* to choose proper position in tab related witch sw event */
	const uint8_t timeExpired_u8 = 3u; /* to choose proper position in tab related witch time expired */
	/* choose  menu state by switch and after 3s info go to settings */
	if ( SW_TIMER == event_u8 )
	{	
		*currentMenuState_u8 = tabHideMenu[*currentMenuState_u8].nextState_au8[switchEvent_u8];	
		Gv_Timer3sHideMenu_u8 = 0u;	
		SaveToEEPROM(structure_pstr);
	}
	else if ( TIMER_EVENT_3S == Gv_Timer3sHideMenu_u8)		
	{	
		*currentMenuState_u8 = tabHideMenu[*currentMenuState_u8].nextState_au8[timeExpired_u8];
		Gv_Timer3sHideMenu_u8 ++;
	}
		
	/* call proper function */
	tabHideMenu[*currentMenuState_u8].callback(event_u8,structure_pstr, tab_pu8,*currentMenuState_u8);
}
	
static void TimeConv(uint16_t time_u16, uint8_t digTime[4]){
	digTime[0]=(uint8_t)(time_u16/600);
	digTime[1]=(uint8_t)((time_u16/60)%10);
	digTime[2]=(uint8_t)((time_u16%60)/10);
	digTime[3]=(uint8_t)((time_u16%60)%10);
}

static void DigConvToDsp(uint8_t tabIn_au8[4], uint8_t tabOutDsp_au8[4]){
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


static void TimeConvToDsp(uint8_t tabIn_au8[4], uint8_t tabOutDsp_au8[4]){
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



static void DigTempConvToDsp(uint8_t tabInDigit_au8[4], uint8_t tabOutLcdRepresentation_au8[4]){ //funkcja nie uwzglednia ujemnych temperatur poniewaz jest wykozystywana tylko do manipulacji na dodatnich temperaturach
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

static void TempConvToDigit (uint16_t temperature_u16, uint8_t digTemp_au8[4])
{ 
	/*funkcja nie uwzglednia ujemnych temperatur poniewaz jest 
	  wykozystywana tylko do manipulacji na dodatnich temperaturach*/
	digTemp_au8[0] = 0x00u;
	digTemp_au8[1] = (uint8_t)temperature_u16/100u;
	digTemp_au8[2] = (uint8_t)(temperature_u16%100u)/10u;
	digTemp_au8[3] = (uint8_t)(temperature_u16%100u)%10u;
}

static int16_t TabToTempConv(uint8_t firstDig100_u8, uint8_t secDigit10_u8, uint8_t  thrDigit1_u8)
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


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: SensorTemperatureCalibration(int16_t dataFromSensor_s16)
*
* FUNCTION ARGUMENTS:
*    dataFromSensor_s16 - temperature read from sensor
*
* RETURN VALUE:
*    NOne
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to calibrate temperature received from temperature sensor according to calibration factory.
*
*---------------------------------------------------------------------------*/
static void SensorTemperatureCalibration(strSaunaParam_t *structWorkingValue, int16_t dataFromSensor_s16)
{
		structWorkingValue->ActualTemperature_s8 = dataFromSensor_s16 + structWorkingValue->HiddenMenuParam.Calibration_s8;
}


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: CheckData485Presence()
*
* FUNCTION ARGUMENTS:
*    None
*
* RETURN VALUE:
*    retVal_bo - TRUE if data are present in buffer, FALSE if no dta in buffer
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to check presence of data in input buffer, return TRUE if any data are present in buffer
*
*---------------------------------------------------------------------------*/
static uint8_t CheckData485Presence()
{
	uint8_t retVal_bo = FALSE;
	for ( uint8_t i_u8 = 0u; i_u8 <= 9u; i_u8++)
	{
		if (Gv_tabRecDataRS485_au8[i_u8] != 0u)
		{
			retVal_bo = TRUE;
		}
		else 
		{
			; /* do nothing */
		}
	}
	return retVal_bo;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: ProcessInputRSData(processedDataInRS_t *processedRSData)
*
* FUNCTION ARGUMENTS:
*    processedRSData - pointer to struct with RS data processed by program
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to fill input Structure with proper data, depending on data received from RS485 line
*
*---------------------------------------------------------------------------*/
static void ProcessInputRSData(processedDataInRS_t *processedRSData)
{
		/* reset flag for minute mark in next cycle */
		processedRSData->dataInRS.MinuteEvent_bo = FALSE;
		
		if (TRUE == CheckData485Presence())
		{
			switch ( Gv_tabRecDataRS485_au8[9] ){
				case 0x20: /* RS data indicating minute mark */
					processedRSData->dataInRS.MinuteEvent_bo = TRUE;
				break;
				case 0x21: /* RS data indicating error with temperature sensor */
					processedRSData->dataInRS.Error1Sen_bo = TRUE;
				break;
				case 0x22:
				break;
				case 0x23: /* RS data indicating error with temperature sensor */
					processedRSData->dataInRS.Error2Sen_bo = TRUE;
				break;
				case 0x24:
				break;
				/* RS data indicating used to calculate actual temperature read by sensor */
				case 0x32:
					/* information about 0 degree on most significant position */
					processedRSData->dataInRS.Temp100_u8 = 0u;
				break;
				case 0x33:
					/* information about 100 degree on most significant position */
					processedRSData->dataInRS.Temp100_u8 = 100u;
				break;
				case 0x34:
					/* information about temperature below 0,  "-" sign on most significant position */
					processedRSData->dataInRS.Temp100_u8 = 2u; 
				break;
				case 0x35:
					/* information about 0 degree on medium significant position */
					processedRSData->dataInRS.Temp10_u8 = 0u;
				break;
				case 0x36:
					/* information about 10 degree on medium significant position */
					processedRSData->dataInRS.Temp10_u8 = 10u;
				break;
				case 0x37:
					processedRSData->dataInRS.Temp10_u8 = 20u;
				break;
				case 0x38:
					processedRSData->dataInRS.Temp10_u8 = 30u;
				break;
				case 0x39:
					processedRSData->dataInRS.Temp10_u8 = 40u;
				break;
				case 0x3A:
					processedRSData->dataInRS.Temp10_u8 = 50u;
				break;
				case 0x3B:
					processedRSData->dataInRS.Temp10_u8 = 60u;
				break;
				case 0x3C:
					processedRSData->dataInRS.Temp10_u8 = 70u;
				break;
				case 0x3D:
					processedRSData->dataInRS.Temp10_u8 = 80u;
				break;
				case 0x3E:
					processedRSData->dataInRS.Temp10_u8 = 90u;
				break;
				case 0x3F:
					processedRSData->dataInRS.Temp1_u8 = 0u;
				break;
				case 0x40:
					processedRSData->dataInRS.Temp1_u8 = 1u;
				break;
				case 0x41:
					processedRSData->dataInRS.Temp1_u8 = 2u;
				break;
				case 0x42:
					processedRSData->dataInRS.Temp1_u8 = 3u;
				break;
				case 0x43:
					processedRSData->dataInRS.Temp1_u8 = 4u;
				break;
				case 0x44:
					processedRSData->dataInRS.Temp1_u8 = 5u;
				break;
				case 0x45:
					processedRSData->dataInRS.Temp1_u8 = 6u;
				break;
				case 0x46:
					processedRSData->dataInRS.Temp1_u8 = 7u;
				break;
				case 0x47:
					processedRSData->dataInRS.Temp1_u8 = 8u;
				break;
				case 0x48:
					processedRSData->dataInRS.Temp1_u8 = 9u;
				break;
				default:
					;
				break;
				
			}
			/* remove recently read data from array*/
			RsShiftTab(Gv_tabRecDataRS485_au8);
			
			/* convert received temperature to degrees if last digit was received */
			if (processedRSData->dataInRS.Temp1_u8 != INVALID)
			{
				processedRSData->countedTemperature_s16 = TabToTempConv(
												processedRSData->dataInRS.Temp100_u8,
												processedRSData->dataInRS.Temp10_u8,
												processedRSData->dataInRS.Temp1_u8);
				/* set to INVALID to prevent recalculating temperature if new data won't come */
				processedRSData->dataInRS.Temp1_u8 = INVALID;	
			}

		}
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: TimerMinReset()
*
* FUNCTION ARGUMENTS:
*    None
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to reset Minute counter in Base Module; Reset of this counter is neceserry when we go back from 
*    menu options, or temperature options to state where we count furnance time or delay time.
*
*---------------------------------------------------------------------------*/
static void TimerMinReset()
{
	if(TRUE == Gv_StatusTimerReset_bo)
	{
		RsDataTab(RS_MIN_RES, Gv_TabSendDataRS485_au8);
		Gv_StatusTimerReset_bo = FALSE;
	}
	
}

static void OnOffRSinfoSend(uint8_t statusSend_bo)
{
	if (TRUE == statusSend_bo)
	{
		RsDataTab(RS_CTRL_ON, Gv_TabSendDataRS485_au8);
	}
	else if (FALSE == statusSend_bo)
	{
		RsDataTab(RS_CTRL_OFF, Gv_TabSendDataRS485_au8);
	}
	else 
	{
		; /* do nothing */
	}
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: SendDisplayPresence()
*
* FUNCTION ARGUMENTS:
*    None
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Send information to Main Module every 300ms?? to confirm presence of the Panel Module,
*    in case of panel absence Main Module should be switched off.
*
*---------------------------------------------------------------------------*/
static void SendDisplayPresence()
{
	static uint8_t sendPanelInfo_bo = FALSE;
	if (TIMER_EVENT_300MS == Gv_Timer300ms_u8)
	{
		if (sendPanelInfo_bo == TRUE)
		{
			RsDataTab(RS_PANEL_PRES, Gv_TabSendDataRS485_au8);
			sendPanelInfo_bo = FALSE;
		}
	}
	else 
	{
		sendPanelInfo_bo = TRUE;
	}
	        
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: OutputExecute(uint8_t tabOutDsp_au8)
*
* FUNCTION ARGUMENTS:
*    tabOutDsp_au8 - data foe LED display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function execute every cycle in main, used to set output informations.
*
*---------------------------------------------------------------------------*/
static void OutputExecute(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSData, uint8_t tabOutDsp_au8[])
{
	LampStateMachine(structWorkingValue, processedRSData); 
	FanStateMachine(structWorkingValue, processedRSData);
	FurnanceStateMachine(structWorkingValue, processedRSData);
	SendDisplayPresence(); 
	TimerMinReset();
	DsLedSend(tabOutDsp_au8[0], tabOutDsp_au8[1], tabOutDsp_au8[2], tabOutDsp_au8[3]);
	LedWorkStatus();
	SendDataByRs();
}

static void SendDataByRs()
{
//------------------wysylka przez RSa----------------------------------
	if(Gv_SendRS485AllowFlag_u8==1)
	{
		//zabezpieczenie dla RS485 konieczne w kilku miejscach programu
		if (!(PIND & (1<<PD3)))
		{
			nrByte=0;Gv_EnableSend2_u8=0;
		}
		if (!(PIND & (1<<PD2)))
		{
			Gv_EnableSend1_u8=0; Gv_SendStepLevel_u8=0; Gv_SendStepCounter_u8=0;
		}
		//zabezpieczenie dla RS485 zabezpieczajace przed ustawieniem 1 z obu stron
		if (!(Gv_SendStepLevel_u8==0))
		{
			presM1++;
		}
		else 
		{
			presM1=0;
		}
		if (presM1==15)
		{
			Gv_SendStepLevel_u8=0; presM1=0; Gv_SendStepCounter_u8=0; PORTD &= ~(1<<PD2);
		}
		///////////////////////////////////////RS 10 5 16//////////////////////////////////////
			 
		if (	Gv_TabSendDataRS485_au8[9] + Gv_TabSendDataRS485_au8[8] + Gv_TabSendDataRS485_au8[7]
			  + Gv_TabSendDataRS485_au8[6] + Gv_TabSendDataRS485_au8[5] + Gv_TabSendDataRS485_au8[4]
			  + Gv_TabSendDataRS485_au8[3] + Gv_TabSendDataRS485_au8[2] + Gv_TabSendDataRS485_au8[1]
			  + Gv_TabSendDataRS485_au8[0] > 0 ) 
		{
			Gv_OutputRSDataPresent_u8 = 1u;
		}
		else 
		{
			Gv_OutputRSDataPresent_u8 = 0u;
		}
		if ((!(PIND & (1<<PD3))) && Gv_OutputRSDataPresent_u8==1) //jesli wykryto zero z przeciwnej strony i mamy cos do wyslania
		{ 
			if (Gv_SendStepLevel_u8==0 )
			{
				PORTD |= (1<<PD2); //ustawienie nozki na 1 swiadczace o wysylaniu sekwencji danych przez mastera
				/*_delay_us(5);*/
				USART_SendByteM(Gv_TabSendDataRS485_au8[9]);
				Gv_SendStepLevel_u8=1;}//wysylka danych
			else if (Gv_SendStepLevel_u8==1)
			{
				USART_SendByteM(Gv_TabSendDataRS485_au8[9]);
				Gv_SendStepLevel_u8=2;
			}
			else if (Gv_SendStepLevel_u8==2)
			{
				Gv_SendStepCounter_u8++;
				//if (returnData==0xFF && enableSend1==1){enableSend1=0;RsShiftTab(tabDataRS); sendStep=0;stepS=0;
				if (returnData==Gv_TabSendDataRS485_au8[9] && Gv_EnableSend1_u8==1)
				{
					Gv_EnableSend1_u8=0;RsShiftTab(Gv_TabSendDataRS485_au8); Gv_SendStepLevel_u8=0;Gv_SendStepCounter_u8=0;presM1=0;
					_delay_us(10);
					PORTD &= ~(1<<PD2);
				}
				else if (/*returnData==0x00 &&*/ Gv_EnableSend1_u8==1)
				{
					Gv_EnableSend1_u8=0;Gv_SendStepLevel_u8=0;Gv_SendStepCounter_u8=0;presM1=0;
					_delay_us(10);
					PORTD &= ~(1<<PD2);
				}
				
				if (Gv_SendStepCounter_u8==5)
				{
					Gv_SendStepLevel_u8=0;
					//nrByte=0;
					PORTD &= ~(1<<PD2);
					Gv_SendStepCounter_u8=0;
					presM1=0;
				}
			}
		}

			 
		// if((PIND & (1<<PD3)) && enableSend2==1){_delay_us(100);USART_SendByteM(0xFF);enableSend2=0;nrByte=0;}
		if((PIND & (1<<PD3)) && Gv_EnableSend2_u8==1)
		{
			_delay_us(10);USART_SendByteM(tabDataRxc[0]);Gv_EnableSend2_u8=0;nrByte=0;
		}
		if((PIND & (1<<PD3)) && Gv_EnableSend2_u8==2)
		{
			_delay_us(10);USART_SendByteM(0x00);Gv_EnableSend2_u8=0;nrByte=0;
		}
			 
		Gv_SendRS485AllowFlag_u8=0;
	}
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: SetRegisterUC()
*
* FUNCTION ARGUMENTS:
*    None
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to set all register for proper work of microcontroller.
*
*---------------------------------------------------------------------------*/
static void SetRegisterUC(void)
{
	DDRB = 0x00;
	DDRC = 0x00;
	DDRD = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;
	
	/* set unused IO pins */
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

	/* set pins for LED display */
	DDRD |= (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7);
	PORTD |= (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7);
	
	/* set pins for used switch */
	DDRA &= ~(1<<PA0);
	DDRA &= ~(1<<PA1);
	DDRA &= ~(1<<PA2);
	DDRA &= ~(1<<PA3);
	DDRA &= ~(1<<PA4);
	DDRA &= ~(1<<PA5);
	DDRA &= ~(1<<PA6);
	DDRA &= ~(1<<PA7);
	PORTA |= (1<<PA0) | (1<<PA1) | (1<<PA2) | (1<<PA3) | (1<<PA4) | (1<<PA5);// | (1<<PA6) | (1<<PA7);

	/* set pins for RS485 connection - Master configuration */
	DDRD |= (1<<PD2);
	PORTD &= ~(1<<PD2);
	DDRD &= ~(1<<PD3);
	PORTD &= ~(1<<PD3);
	
	DDRB |= (1<<PB1);
	PORTB &= ~(1<<PB1);

	/* set pins for LED diode */
	/* set pins as outputs */
	DDRC |= (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3) | (1<<PC4) ;//| (1<<PC5) | (1<<PC6); 
	/* set pins in High state */
	PORTC &= ~(1<<PC0); PORTC &= ~(1<<PC1); PORTC &= ~(1<<PC2); PORTC &= ~(1<<PC3); PORTC &= ~(1<<PC4); /*PORTC &= ~(1<<PC5); PORTC &= ~(1<<PC6); */

	/* set pins for buzzer */

	DDRB |= (1<<PB0); /* pin 0 - set as output*/
	PORTB &= ~(1<<PB0); /* set pin on LOW level on start */
	
	/* initialize SPI connection */
	SPI_Init();
	
	/* initialize USART connection and USART interruption */
	USART_Init(38400, 16000000);
	USART_InitInterrupt();
	/* initialize timera0 interruption */
	Timer0Init(194);
	/* initialize timera2 interruption */
	Timer2Init(194);	
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: InitDataOnEntry(strSaunaParam_t *structWorkingValue)
*
* FUNCTION ARGUMENTS:
*    structWorkingValue - pointer to structure with sauna parameters
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used fill struct with data from EEPROM also set global interrupt and watchdog.
*
*---------------------------------------------------------------------------*/
static void InitDataOnEntry(strSaunaParam_t *structWorkingValue)
{
	_delay_ms(300);
	/* init EEPROM struct with default value - only once in controller life
	and fill working struct by data from EEPROM every start*/
	initEepromStruct(structWorkingValue, &structEepromSavedValue, &eepromInitDefaultFlag_u8);
	
	/* set global flag for interruption*/
	sei();	
	
	/* set watch dog */
	wdt_enable(WDTO_2S);
	
	_delay_ms(300);
		
}


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: main()
*
* FUNCTION ARGUMENTS:
*    None
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Main function to execute program.
*
*---------------------------------------------------------------------------*/
int main(void)
{
	uint8_t event_u8 = 0u;
	static strSaunaParam_t structWorkingValue = {{0u,0u,0u,0u,0u,0}, 0u,0u,0u,0};
	static uint8_t displayOutData_au8[4] = {0u};
	static uint8_t menuLevel_u8 = 0u;
	static processedDataInRS_t processedRSInData = {{0u}, 0};
	
	SetRegisterUC(); /* set register status */
	InitDataOnEntry(&structWorkingValue);
	
	while(1)
	{
		wdt_reset();
		TimeCounter(); /* function to increment timers */
		ProcessInputRSData(&processedRSInData); /* switch execute, input 485 data execute */
		SwEventChoose(&Gv_Timer50ms_u8, &event_u8);
		SensorTemperatureCalibration(&structWorkingValue, processedRSInData.countedTemperature_s16);

		StateMachine(event_u8, &structWorkingValue, &processedRSInData, displayOutData_au8, menuLevel_u8);
		OutputExecute(&structWorkingValue, &processedRSInData ,displayOutData_au8); /* led execute, display led execute, rsData set*/
	}

}

ISR(TIMER0_OVF_vect) //wywolywana co 0.001s
{ 
    if  (Gv_BuzCounter_u16 > 0u) 
	{
		 Gv_BuzCounter_u16 --;
	}
    else 
	{
		Gv_BuzCounter_u16 = 0u;
	}
	      
	Gv_SendRS485AllowFlag_u8 = 1u;
   
    TCNT0=194;
 }

ISR(TIMER2_OVF_vect) //wywolywana co 0.001s
{
	//switchCounterM=1;
    //TCNT2=226;
	TCNT2=50;
}


/////////////////////////////RS 10 5 16/////////////////////////////////////////////////////////
	

ISR (USART_RXC_vect){	
	static unsigned char emptyRxc;	
	if(!(PIND & (1<<PD3)) && Gv_SendStepLevel_u8==2)//jesli zero z przeciwnej strony i master jest w trybie oczekiwania na odpowiedz -  master wysyla bajt
	{
		returnData = UDR;// USART_Receive();
		Gv_EnableSend1_u8=1;
	}
	else if ((PIND & (1<<PD3)) && Gv_SendStepLevel_u8==0)//jesli wykrylo 1 z przeciwnej strony a funkcja od wysy³ania ma krok ustawiony na 0(na pewno se zakonczyla)
	{
		if (nrByte==0)
		{
			tabDataRxc[0] = UDR;//USART_Receive();
			nrByte=1;
		}
		else if (nrByte==1)
		{
			tabDataRxc[1] = UDR;//USART_Receive();
			nrByte=0;
			if (tabDataRxc[0]==tabDataRxc[1] )
			{
				RsDataTabRec(tabDataRxc[0],Gv_tabRecDataRS485_au8);Gv_EnableSend2_u8=1; 
			}
			if (!(tabDataRxc[0]==tabDataRxc[1]))
			{
				Gv_EnableSend2_u8=2; 
			}
		}
	}
	else 
	{
		emptyRxc=UDR;/*USART_Receive();*/
	}
}
				
