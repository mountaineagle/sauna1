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

volatile uint8_t debugCounter=0;

static uint8_t eepromInitDefaultFlag_u8 EEMEM;					/* flag indicate set default value in EEPROM memory during first start of the system */
static strSaunaSavedParameter_t structEepromSavedValue EEMEM;   /* EEPROM struct with main settings for sauna */

static strLedStatus_t Gv_LedStatus;

volatile uint8_t Gv_SwitchCounter_bo = FALSE;		/* !< variable set in 50ms timer interrupt to indicate 50ms time gap >! */
volatile uint8_t Gv_FlagInterrupt50ms_bo = FALSE;	/* !< variable set in 20ms timer interrupt to indicate 20ms time gap >! */
volatile uint8_t Gv_tabRecDataRS485_au8[10] = {0u};	/* !< table for input RS data >! */

static uint8_t Gv_Timer10s_u8	= 0u;				/* !< timer used to count 10s period >!*/
static uint8_t Gv_Timer8s_u8	= 0u;				/* !< timer used to count 8s period >!*/
static uint8_t Gv_Timer6s_u8	= 0u;				/* !< timer used to count 6s period >!*/
static uint8_t Gv_Timer3s_u8	= 0u;				/* !< timer used to count 3s period >!*/
static uint8_t Gv_Timer2s_u8	= 0u;				/* !< timer used to count 2s period >!*/
static uint8_t Gv_Timer1s_u8	= 0u;				/* !< timer used to count 1s period >!*/
static uint8_t Gv_Timer300ms_u8 = 0u;				/* !< timer used to count 300ms period >!*/
static uint8_t Gv_Timer50ms_u8	 = 0u;				/* !< timer used to count 50ms period >!*/
static uint8_t Gv_Timer2sHideMenu_u8 = 0u;			/* !< timer used to count 3s period in hide manu >!*/
static uint8_t Gv_CounterLed_u8 = 0u;				/* ! < counter used to count 500ms period for led >!*/

static uint8_t Gv_StatusTimerReset_bo;				/* !< flag used to reset minute counter data - send to base >!*/

static stateLamp_t Gv_LampState_e = LAMP_IDLE;		/* !< variable for remember actual state of Lamp state machine >! */
static stateFan_t Gv_FanState_e = FAN_IDLE;			/* !< variable for remember actual state of Fan state machine >! */
static stateFurnance_t Gv_FurState_e = FUR_IDLE;	/* !< variable for remember actual state of Furnance state machine >! */

static stateMachine_t Gv_StateMachine_e = SM_OFF;			/* !< variable for remember actual state of main state machine >! */
static stateMachine_t Gv_StateMachinePrevState_e = SM_OFF;	/* !< variable for remember previous state of main state machine >! */
static uint8_t Gv_InitOnEntry_bo = TRUE;					/* !< flag for initialize first entry in every state from main state machine >! */

volatile uint16_t Gv_BuzCounter_u16 = 0u;					/* !< counter for buzzer >! */


/*--------old variables for RS 485 data handle - do not change------------------*/

volatile uint8_t Gv_RsByteSend_u8 = 0u;
volatile uint8_t Gv_RsRecByte_u8 = 0u;
volatile uint8_t Gv_OutputRSDataPresent_u8 = 0u;
volatile uint8_t Gv_TabSendDataRS485_au8[10];
volatile uint8_t Gv_tabRecDataRS485_au8[10];
/* transmission for RS: */
volatile uint8_t Gv_SendRS485AllowFlag_u8 = 0u;
//volatile int returnDataInfo=0;
volatile uint8_t Gv_ReturnData_u8 = 0u;
volatile uint8_t Gv_EnableSend1_u8 = 1u;
volatile uint8_t Gv_NrByte_u8 = 0u;
volatile uint8_t Gv_EnableSend2_u8 = 0u;
volatile uint8_t Gv_TabDataRxc_au8[2]={0u,0u};
volatile int Gv_SendStepLevel_u8=0u;
/* protection for  rs */
volatile uint8_t Gv_SendStepCounter_u8 = 0u;
volatile uint8_t Gv_PresM1_u8=0;

/* finish block for old variable definition */



/*----------------------------------------------------------------------------
*
* FUNCTION NAME: saveDefaultParameterToEeprom(strSaunaSavedParameter_t *structEepromParam)
*
* FUNCTION ARGUMENTS:
*    structEepromParam - EEPROM struct for sauna parameter
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Load default value to eeprom memory during first start of system.
*
*---------------------------------------------------------------------------*/
static void saveDefaultParameterToEeprom(strSaunaSavedParameter_t *structEepromParam)
{
	const strSaunaSavedParameter_t structDefParameter = {
        (uint16_t)WARMING_TIME_MAX_DEF,
        (uint8_t)TEMPERATURE_HOTMAX_DEF,
        (uint8_t)TEMPERATURE_HOTMIN_DEF,
        (uint8_t)TIMER_FAN_DEF,
        (uint8_t)HIST_TEMP_DEF,
        (int8_t)CALIBRATION_DEF,
		(uint8_t)TEMPERATURE_HOT_DEF
    };

	eeprom_write_block(&structDefParameter,structEepromParam,sizeof(structDefParameter));

	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: initEepromStruct(strSaunaParam_t *structWorkingValue, strSaunaSavedParameter_t* structEepromParam, uint8_t* eepromAddr_u8)
*
* FUNCTION ARGUMENTS:
*    structWorkingValue - struct with menu settings for sauna
*    structEepromParam - EEPROM struct for sauna parameter
*    eepromAddr_u8 - EEPROM flag indicating for load default value to eeprom memory in case first start of system
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Load default value to eeprom memory during first start of system, and write it to working ram struct.
*
*---------------------------------------------------------------------------*/
static void initEepromStruct(strSaunaParam_t *structWorkingValue, strSaunaSavedParameter_t *structEepromParam, uint8_t *eepromAddr_u8)
{
	if ((0xFFu == eeprom_read_byte(eepromAddr_u8)))
	{
		saveDefaultParameterToEeprom(structEepromParam);
		eeprom_write_byte(eepromAddr_u8,0u);
	}
	fillWorkingStructDuringStart(structWorkingValue, structEepromParam);
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: fillWorkingStructDuringStart(strSaunaParam_t *structWorkingValue, strSaunaSavedParameter_t *structEepromParam)
*
* FUNCTION ARGUMENTS:
*    structWorkingValue - struct with menu settings for sauna
*    structEepromParam - EEPROM struct for sauna parameter
*	
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Load default and stored value for sauna parameter in case of start system.
*
*---------------------------------------------------------------------------*/
static void fillWorkingStructDuringStart(strSaunaParam_t *structWorkingValue, strSaunaSavedParameter_t *structEepromParam)
{
	eeprom_read_block(&(structWorkingValue->HiddenMenuParam),structEepromParam,sizeof(*structEepromParam));
	fillWorkingStructDuringSwitchingOn(structWorkingValue);
	return;
	
}


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: SaveToEEPROM(strSaunaParam_t* savedValue)
*
* FUNCTION ARGUMENTS:
*    savedValue - struct with menu settings for sauna
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Load sauna parameter to eeprom memory.
*
*---------------------------------------------------------------------------*/
static void SaveToEEPROM(strSaunaParam_t* savedValue)
{
	cli(); /* turn off interrupt before eeprom write */
	eeprom_write_block(&(savedValue->HiddenMenuParam),&structEepromSavedValue,sizeof(structEepromSavedValue));
	sei(); /* turn on interrupt after eeprom write */
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: fillWorkingStructDuringSwitchingOn(strSaunaParam_t *structWorkingValue)
*
* FUNCTION ARGUMENTS:
*    structWorkingValue - struct with menu settings for sauna
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Load default value for some of the sauna parameter in case of start system.
*
*---------------------------------------------------------------------------*/
static void fillWorkingStructDuringSwitchingOn(strSaunaParam_t *structWorkingValue)
{
	structWorkingValue->DelayWarmingTime_s16 = DELAY_WARMING_TIME_DEF;
	structWorkingValue->WarmingTime_s16 = structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16;
	structWorkingValue->TimerCurrentFanSet_u8 = structWorkingValue->HiddenMenuParam.TimerFanSet_u8;
	structWorkingValue->TemperatureHot_u8 = structWorkingValue->HiddenMenuParam.TemperatureHot_u8;
	
	return;
}
	
/*----------------------------------------------------------------------------
*
* FUNCTION NAME: BackToOffState(uint8_t event_u8,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*    menuLevel_u8 - hide menu array of structures level
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    First function in array of structures for hidden menu, used to back from hidden menu to off state.
*
*---------------------------------------------------------------------------*/
static void BackToOffState(uint8_t event_u8,strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
	Gv_StateMachine_e = SM_OFF;
	Gv_InitOnEntry_bo = TRUE;
	return ;
}

	
/*void showHideMenuLevel(uint8_t event_u8,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel) 
{
		timerMenuChange=TIM_MENU_CHANGE;
		fillTabByLcdDataMenu(event_u8, displayOutData_pa);
}*/
	

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: setHistTempFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue_pstr, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*    menuLevel_u8 - hide menu array of structures level
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to set histerese for furnace heat.
*
*---------------------------------------------------------------------------*/
static void setHistTempFunction(uint8_t event_u8,strSaunaParam_t *structWorkingValue_pstr, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		
		if(	   SW_UP == event_u8
		|| SW_FAST_UP == event_u8
		|| SW_VERY_FAST_UP == event_u8)
		{
			(structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8)++;
		}
		if(	   SW_DOWN == event_u8
		|| SW_FAST_DOWN == event_u8
		|| SW_VERY_FAST_DOWN == event_u8)
		{
			(structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8)--;
		}
		if((structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8) > HIST_TEMP_MAX)
		{
			structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8 = HIST_TEMP_MIN;
		}
		if((structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8) < HIST_TEMP_MIN)
		{
			structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8 = HIST_TEMP_MAX;
		}
		
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL,(int16_t)structWorkingValue_pstr->HiddenMenuParam.HistTemp_u8, displayOutData_pa);
		
		return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: setCalibrationFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*    menuLevel_u8 - hide menu array of structures level
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to set calibration parameter for temperature sensor.
*
*---------------------------------------------------------------------------*/
static void setCalibrationFunction(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		
		if(	   SW_UP == event_u8 
			|| SW_FAST_UP == event_u8
			|| SW_VERY_FAST_UP == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.Calibration_s8)++;
		}
		if(	   SW_DOWN == event_u8
			|| SW_FAST_DOWN == event_u8
		    || SW_VERY_FAST_DOWN == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.Calibration_s8)--;
		}
		if((structWorkingValue->HiddenMenuParam.Calibration_s8) > (int8_t)CALIBRATION_MAX)
		{
			structWorkingValue->HiddenMenuParam.Calibration_s8 = (int8_t)CALIBRATION_MIN;
		}
		if((structWorkingValue->HiddenMenuParam.Calibration_s8) < (int8_t)CALIBRATION_MIN)
		{
			structWorkingValue->HiddenMenuParam.Calibration_s8 = (int8_t)CALIBRATION_MAX;
		}
		
		fillLcdDataTab(CALIB_LCD_CONV_LEVEL,(int16_t)structWorkingValue->HiddenMenuParam.Calibration_s8, displayOutData_pa);
		
		return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: setMaxWorkingTimerFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*    menuLevel_u8 - hide menu array of structures level
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to set max timer for furnace heat.
*
*---------------------------------------------------------------------------*/
static void setMaxWorkingTimerFunction(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		if(SW_UP == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16)++;
		}
		if(SW_DOWN == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16)--;
		}
		if(SW_FAST_UP == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16) += (int16_t)SW_FAST_CHANGE_10;
		}
		if(SW_FAST_DOWN == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16) -= (int16_t)SW_FAST_CHANGE_10;
		}
		if(SW_VERY_FAST_UP == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16) += (int16_t)SW_FAST_CHANGE_60;
		}
		if(SW_VERY_FAST_DOWN == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16) -= (int16_t)SW_FAST_CHANGE_60;
		}
		if((structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16) > (int16_t)WARMING_TIME_MAX_MAX)
		{
			structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16 = (int16_t)WARMING_TIME_MAX_MIN;
		}
		if((structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16) < (int16_t)WARMING_TIME_MAX_MIN)
		{
			structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16 = (int16_t)WARMING_TIME_MAX_MAX;
		}	
		
		fillLcdDataTab(TIME_LCD_CONV_LEVEL,(int16_t)structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16, displayOutData_pa);
		
		return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: setTempMaxFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*    menuLevel_u8 - hide menu array of structures level
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to set max temperature bound for furnace.
*
*---------------------------------------------------------------------------*/
static void setTempMaxFunction(uint8_t event_u8,strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{	
		if(SW_UP == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8)++;
		}
		if(SW_DOWN == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8)--;
		}
		if(SW_FAST_UP == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8)++;
		}
		if(SW_FAST_DOWN == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8)--;
		}
		if(SW_VERY_FAST_UP == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8) += SW_FAST_CHANGE_10;
		}
		if(SW_VERY_FAST_DOWN == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8) -= SW_FAST_CHANGE_10;
		}
		if((structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8) > TEMPERATURE_HOTMAX_MAX)
		{
			structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8 = TEMPERATURE_HOTMAX_MIN;
		}
		if((structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8) < TEMPERATURE_HOTMAX_MIN)
		{
			structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8 = TEMPERATURE_HOTMAX_MAX;
		}	
		
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL,(int16_t)structWorkingValue->HiddenMenuParam.TemperatureHotMax_u8, displayOutData_pa);
		
		return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: setTempMinFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*    menuLevel_u8 - hide menu array of structures level
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to set min temperature bound for furnace.
*
*---------------------------------------------------------------------------*/
static void setTempMinFunction(uint8_t event_u8,strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{	
	if(SW_UP == event_u8)
	{
		(structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8)++;
	}
	if(SW_DOWN == event_u8)
	{
		(structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8)--;
	}
	if(SW_FAST_UP == event_u8)
	{
		(structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8)++;
	}
	if(SW_FAST_DOWN == event_u8)
	{
		(structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8)--;
	}
	if(SW_VERY_FAST_UP == event_u8)
	{
		(structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8) += SW_FAST_CHANGE_10;
	}
	if(SW_VERY_FAST_DOWN == event_u8)
	{
		(structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8) -= SW_FAST_CHANGE_10;
	}
	if((structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8) > TEMPERATURE_HOTMIN_MAX )
	{
		structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8 = TEMPERATURE_HOTMIN_MIN;
	}
	if((structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8) < TEMPERATURE_HOTMIN_MIN )
	{
		structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8 = TEMPERATURE_HOTMIN_MAX;
	}	
		
	fillLcdDataTab(TEMP_LCD_CONV_LEVEL,(int16_t)structWorkingValue->HiddenMenuParam.TemperatureHotMin_u8, displayOutData_pa);
	
	return;
}


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: setTimeFanFunction(uint8_t event_u8,strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*    menuLevel_u8 - hide menu array of structures level
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to set max fan working time in hidden menu.
*
*---------------------------------------------------------------------------*/
static void setTimeFanFunction(uint8_t event_u8,strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		if(SW_UP == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.TimerFanSet_u8)++;
		}
		if(SW_DOWN == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.TimerFanSet_u8)--;
		}
		if(SW_FAST_UP == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.TimerFanSet_u8)++;
		}
		if(SW_FAST_DOWN == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.TimerFanSet_u8)--;
		}
		if(SW_VERY_FAST_UP == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.TimerFanSet_u8) += SW_FAST_CHANGE_10;
		}
		if(SW_VERY_FAST_DOWN == event_u8)
		{
			(structWorkingValue->HiddenMenuParam.TimerFanSet_u8) -= SW_FAST_CHANGE_10;
		}
		if(structWorkingValue->HiddenMenuParam.TimerFanSet_u8 > TIMER_FAN_MAX)
		{
			structWorkingValue->HiddenMenuParam.TimerFanSet_u8 = TIMER_FAN_MIN;
		}
		if(structWorkingValue->HiddenMenuParam.TimerFanSet_u8 < TIMER_FAN_MIN)
		{
			structWorkingValue->HiddenMenuParam.TimerFanSet_u8 = TIMER_FAN_MAX;
		}
		
		fillLcdDataTab(TIME_LCD_CONV_LEVEL,(int16_t)structWorkingValue->HiddenMenuParam.TimerFanSet_u8, displayOutData_pa);
		
		return;
}	

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: showHideMenuLevel(uint8_t event_u8, strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*    menuLevel_u8 - hide menu array of structures level
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to set display hidden menu level based on array of structures level.
*
*---------------------------------------------------------------------------*/
static void showHideMenuLevel(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8) 
{
		uint8_t displayMenuLevel_u8 = 0u;
		switch (menuLevel_u8)
		{
			case 1:
				displayMenuLevel_u8=1u;
			break;
			case 3:
				displayMenuLevel_u8=2u;
			break;
			case 5:
				displayMenuLevel_u8=3u;
			break;
			case 7:
				displayMenuLevel_u8=4u;
			break;
			case 9:
				displayMenuLevel_u8=5u;
			break;
			case 11:
				displayMenuLevel_u8=6u;
			break;
		}
		fillTabByLcdDataMenu(displayMenuLevel_u8, displayOutData_pa);
		
		return;
	}	
	
	
/*----------------------------------------------------------------------------
*
* FUNCTION NAME: setFurnanceWorkTime(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa) 
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to set working time for furnace on state.
*
*---------------------------------------------------------------------------*/	
static void setFurnanceWorkTime(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa) 
{
		if (SW_OFF_ON == event_u8)
		{
			Gv_StateMachine_e = SM_OFF;
			Gv_InitOnEntry_bo = TRUE;
		}
		else if(SW_UP == event_u8)
		{
			(structWorkingValue->WarmingTime_s16)++;
		}
		else if(SW_DOWN == event_u8)
		{
			(structWorkingValue->WarmingTime_s16)--;
		}
		else if(SW_FAST_UP == event_u8)
		{
			(structWorkingValue->WarmingTime_s16) += (int16_t)SW_FAST_CHANGE_10;
		}
		else if(SW_FAST_DOWN == event_u8)
		{
			(structWorkingValue->WarmingTime_s16) -= (int16_t)SW_FAST_CHANGE_10;
		}
		else if(SW_VERY_FAST_UP == event_u8)
		{
			(structWorkingValue->WarmingTime_s16) += (int16_t)SW_FAST_CHANGE_60;
		}
		else if(SW_VERY_FAST_DOWN == event_u8 )
		{
			(structWorkingValue->WarmingTime_s16) -= (int16_t)SW_FAST_CHANGE_60;
		}
		
		if(structWorkingValue->WarmingTime_s16 > (int16_t)(structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16))
		{
			structWorkingValue->WarmingTime_s16 = (int16_t)WARMING_TIME_MAX_MIN;
		}
		/* (SW_IDLE != event_u8) - add also protection by set max time in case of check status of time in heat state */
		else if((structWorkingValue->WarmingTime_s16 < (int16_t)WARMING_TIME_MAX_MIN) && (SW_IDLE != event_u8) ) 
		{
			structWorkingValue->WarmingTime_s16 = (structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16);
			
		}	
		
		fillLcdDataTab(TIME_LCD_CONV_LEVEL,(int16_t)structWorkingValue->WarmingTime_s16, displayOutData_pa);
}


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: setFurnanceDelay(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, stateMachine_t enableForSetState_e)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*    enableForSetState_e - value corresponding with state when changing of delay is possible
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to set delay in furnace switching on.
*
*---------------------------------------------------------------------------*/
static void setFurnanceDelay(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa, stateMachine_t enableForSetState_e) 
{
		
		if (SW_OFF_ON == event_u8)
		{
			Gv_StateMachine_e = SM_OFF;
			Gv_InitOnEntry_bo = TRUE;
		}
		else if ( SM_FURNANCE_ON == enableForSetState_e)
		{
			; /* do not set delay timer if we are in heat state and go to delay settings */
		}
		else if((SW_UP == event_u8) && (TIME_DELAY_10H >structWorkingValue->DelayWarmingTime_s16))
		{
			(structWorkingValue->DelayWarmingTime_s16)++;
		}
		else if((SW_DOWN == event_u8) && (TIME_DELAY_10H >= structWorkingValue->DelayWarmingTime_s16))
		{
			(structWorkingValue->DelayWarmingTime_s16)--;
		}
		else if((SW_UP == event_u8) && (TIME_DELAY_10H <= structWorkingValue->DelayWarmingTime_s16) )
		{
			(structWorkingValue->DelayWarmingTime_s16) += (int16_t)SW_FAST_CHANGE_10;
		}
		else if((SW_DOWN == event_u8) && (TIME_DELAY_10H < structWorkingValue->DelayWarmingTime_s16))
		{
			(structWorkingValue->DelayWarmingTime_s16) -= (int16_t)SW_FAST_CHANGE_10;
		}
		else if(SW_FAST_UP == event_u8)
		{
			(structWorkingValue->DelayWarmingTime_s16) += (int16_t)SW_FAST_CHANGE_10;
		}
		else if(SW_FAST_DOWN == event_u8)
		{
			(structWorkingValue->DelayWarmingTime_s16) -= (int16_t)SW_FAST_CHANGE_10;
		}
		else if(SW_VERY_FAST_UP == event_u8)
		{
			(structWorkingValue->DelayWarmingTime_s16) += (int16_t)SW_FAST_CHANGE_60;
		}
		else if(SW_VERY_FAST_DOWN == event_u8)
		{
			(structWorkingValue->DelayWarmingTime_s16) -= (int16_t)SW_FAST_CHANGE_60;
		}
		
		if(structWorkingValue->DelayWarmingTime_s16 > DELAY_WARMING_TIME_MAX)
		{
			structWorkingValue->DelayWarmingTime_s16 = DELAY_WARMING_TIME_MIN;
		}
		else if(structWorkingValue->DelayWarmingTime_s16 < DELAY_WARMING_TIME_MIN)
		{
			structWorkingValue->DelayWarmingTime_s16 = DELAY_WARMING_TIME_MAX;
		}
		
		fillLcdDataTab(TIME_LCD_CONV_LEVEL,(int16_t)structWorkingValue->DelayWarmingTime_s16, displayOutData_pa);
}


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: TempSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa) 
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to set hot temperature of furnace.
*
*---------------------------------------------------------------------------*/	
static void TempSetStateExecute(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa) 
{
	
		/* initialize variable on entry to state */
	if (TRUE == Gv_InitOnEntry_bo)
	{	
		ClearAllTimers();
		Gv_LedStatus.TempLed_u8 = LED_BLINK;
		
		/* clear initOnEntry flag - initialize was done */
		Gv_InitOnEntry_bo = FALSE;
	}
	/* temperature changing depends on event*/
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(SW_UP == event_u8)
	{
		(structWorkingValue->TemperatureHot_u8)++;
		Gv_Timer10s_u8 = 0u;
	}
	else if(SW_DOWN == event_u8)
	{	
		(structWorkingValue->TemperatureHot_u8)--;
		Gv_Timer10s_u8 = 0u;
	}
	else if(SW_FAST_UP == event_u8)
	{
		(structWorkingValue->TemperatureHot_u8)++;
		Gv_Timer10s_u8 = 0u;
	}
	else if(SW_FAST_DOWN == event_u8)
	{
		(structWorkingValue->TemperatureHot_u8)--;
		Gv_Timer10s_u8 = 0u;
	}
	else if(SW_VERY_FAST_UP == event_u8)
	{
		(structWorkingValue->TemperatureHot_u8) += SW_FAST_CHANGE_10;
		Gv_Timer10s_u8 = 0u;
	}
	else if(SW_VERY_FAST_DOWN == event_u8)
	{
		(structWorkingValue->TemperatureHot_u8) -= SW_FAST_CHANGE_10;
		Gv_Timer10s_u8 = 0u;
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
	else if(TIMER_EVENT_6S == Gv_Timer10s_u8)
	{
		Gv_StateMachine_e = Gv_StateMachinePrevState_e;		
		
		/* remember value in case of return from other states */
		Gv_StateMachinePrevState_e = SM_TEMP_SET;
		
		structWorkingValue->HiddenMenuParam.TemperatureHot_u8 = structWorkingValue->TemperatureHot_u8;
		DsLedOff();
		SaveToEEPROM(structWorkingValue);
		
		//Gv_InitOnEntry_bo = TRUE;
	}
		
	/* fill Lcd tab with temperature data*/
	fillLcdDataTab(TEMP_LCD_CONV_LEVEL,(int16_t)structWorkingValue->TemperatureHot_u8, displayOutData_pa);
		
	return ;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: OffStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t* displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function to execute behavior of sauna panel in off state.
*
*---------------------------------------------------------------------------*/
static void OffStateExecute(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa)
{
	/*used to count position of sec mark on display */
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
		Gv_LedStatus.ProgLed_u8 = LED_OFF;
		Gv_LedStatus.FanLed_u8= LED_OFF;
		
		Gv_FanState_e = FAN_OFF;
		Gv_LampState_e = LAMP_OFF;
		Gv_FurState_e = FUR_OFF;
			
		ClearDispalyData(displayOutData_pa);
		tickDislplayMark_u8 = 0;
		ClearAllTimers();
		
		/*sent Off information by RS485*/
		OnOffRSinfoSend(FALSE);
			
		/* clear initOnEntry flag - initialize was done*/
		Gv_InitOnEntry_bo = FALSE;
	}
		
	if (SW_MENU == event_u8) /* Hidden menu */
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
/*
		if (FAN_ON1_EXEC == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON1;
		}*/
		if (FAN_IDLE != Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		else if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON1;
		}
	}
	else if(SW_LAMP == event_u8)
	{
/*
		if (LAMP_ON1_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_ON1;
		}*/
		if (LAMP_IDLE != Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		else if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_ON1;
		}
	}
	else 
	{
		; /* do nothing */
	}
	
	
	/* count & save sec mark */
	if(TIMER_EVENT_1S == Gv_Timer1s_u8)
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
	
	ClearDispalyData(displayOutData_pa);
	
	/* add point mark on LCD display every second*/
	displayOutData_pa[tickDislplayMark_u8] = displayOutData_pa[tickDislplayMark_u8] & POINT_LCD_MARK;
	
	return;
}


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: ClearDispalyData(uint8_t* displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function to execute Led display clear.
*
*---------------------------------------------------------------------------*/
static void ClearDispalyData(uint8_t* displayOutData_pa)
{			
	displayOutData_pa[0] = SPCJ;
	displayOutData_pa[1] = SPCJ;
	displayOutData_pa[2] = SPCJ;
	displayOutData_pa[3] = SPCJ;
	return ;
}


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: InStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t* displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    processedRSInData - struct with data read by RS485
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function to execute behavior of sauna after switching on.
*
*---------------------------------------------------------------------------*/
static void InStateExecute(uint8_t event_u8, strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
{
	/*used to count position of sec mark on display */
	static uint8_t tickDislplayMark_u8 = 0u; 
	
	/*initialize variable and set states on entry to this state */
	if (TRUE == Gv_InitOnEntry_bo)
	{
		
		/* set default parameter for some working data after start */
		if ( SM_OFF == Gv_StateMachinePrevState_e)
		{
			fillWorkingStructDuringSwitchingOn(structWorkingValue);
		}
		
		/* remember value in case of return from other states */
		Gv_StateMachinePrevState_e = SM_IN;	
		

		
		ClearDispalyData(displayOutData_pa);
		tickDislplayMark_u8 = 0u;
		
		ClearAllTimers();
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
	
	/* initialize variable after back from temperature set state */
	if (SM_TEMP_SET == Gv_StateMachinePrevState_e)
	{
		/* Clear internal timers */
		ClearAllTimers();
		tickDislplayMark_u8 = 0u;
		Gv_LedStatus.TempLed_u8 = LED_OFF;
		Gv_StateMachinePrevState_e = SM_IN;
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
/*
		if (FAN_ON1_EXEC == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON2;
		}*/
		if (FAN_IDLE != Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		else if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON2;
		}
	}
	else if(SW_LAMP == event_u8)
	{
/*
		if (LAMP_ON2_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_ON2;
		}*/
		if (LAMP_IDLE != Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		else if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_ON2;
		}
	}
	else if(SW_TIMER == event_u8)
	{
		Gv_StateMachine_e = SM_TIMER_SET;
		Gv_InitOnEntry_bo = TRUE; 
	}
	else if((TIMER_EVENT_8S == Gv_Timer8s_u8) && (0u == structWorkingValue->DelayWarmingTime_s16))
	{
		Gv_StateMachine_e = SM_FURNANCE_ON;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if(TIMER_EVENT_8S == Gv_Timer8s_u8 )
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
	if(TIMER_EVENT_1S == Gv_Timer1s_u8)
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

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: HideMenuStateExecute(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t* displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function for execute hidden menu state.
*
*---------------------------------------------------------------------------*/
static void HideMenuStateExecute(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t* displayOutData_pa)
{	
	static uint8_t currentMenuState_u8 = 0u;
	menuItem_t tabHideMenu[13]={
		//{{SW_UP, SW_DOWN, SW_CHANGE, TIME_CHANGE, IDLE}, pointerToFunction}
		{{0,0,1,0,0}, BackToOffState},
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
		
		
		Gv_FanState_e = FAN_OFF;
		Gv_LampState_e = LAMP_OFF;
		Gv_FurState_e = FUR_OFF;
		
		/*clear led after return from delay state*/
		Gv_LedStatus.TempLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;
		Gv_LedStatus.ProgLed_u8 = LED_BLINK;


		/* clear initOnEntry flag - initialize was done*/
		Gv_InitOnEntry_bo = FALSE;
	}
	
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
		DsLedOff();
		SaveToEEPROM(structWorkingValue);
	}
	
	Gv_LedStatus.ProgLed_u8 = LED_BLINK;
	ChangeMenu(event_u8, &currentMenuState_u8, tabHideMenu, structWorkingValue, displayOutData_pa);
	
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: FurnanceOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    processedRSInData - struct with data read by RS485
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function to execute behavior of sauna in case of furnace normal working phase.
*
*---------------------------------------------------------------------------*/

static void FurnanceOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
{
	/* initialize variable on entry to state */
	if (TRUE == Gv_InitOnEntry_bo)
	{		
		/* clear led after return from delay state */
		Gv_LedStatus.TempLed_u8 = LED_ON;
		Gv_LedStatus.TimerOnLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOffLed_u8 = LED_ON;
		if (SM_TIMER_SET != Gv_StateMachinePrevState_e)
		{	
			/* switching on the lamp */
			Gv_LampState_e = LAMP_AUTO_ON;
			/* switching on the furnace on entry */
			Gv_FurState_e = FUR_ON;
		}
		/* reset min timer */
		Gv_StatusTimerReset_bo = TRUE;
		
		Gv_BuzCounter_u16 = 50u;
		
		/* Clear internal timers */
		ClearAllTimers();
		
		/* remember value in case of return from other states */
		Gv_StateMachinePrevState_e = SM_FURNANCE_ON;
		/* clear initOnEntry flag - initialize was done */
		Gv_InitOnEntry_bo = FALSE;
	}
	/* initialize variable after back from temperature set state */
	if (SM_TEMP_SET == Gv_StateMachinePrevState_e)
	{
		/* Clear internal timers */
		ClearAllTimers();
		Gv_LedStatus.TempLed_u8 = LED_ON;
		Gv_StateMachinePrevState_e = SM_FURNANCE_ON;
	}

	/* switch executing */
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if (	TRUE == processedRSInData->dataInRS.Error1Sen_bo
			 || TRUE == processedRSInData->dataInRS.Error2Sen_bo)
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
		if (FAN_IDLE != Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		else if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON2;
		}
	}
	else if(SW_LAMP == event_u8)
	{
		if (LAMP_IDLE != Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		else if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_AUTO_ON;
		}
	  /*  if (LAMP_ON1_EXEC == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}*/
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
	
	/* histerese for switching on and off furnace */
/*
	if (   (processedRSInData->countedTemperature_s16 <= (structWorkingValue->TemperatureHot_u8 - structWorkingValue->HiddenMenuParam.HistTemp_u8))
		&& (FUR_AUTO_ON_EXEC != Gv_FurState_e)
		&& (FUR_ON != Gv_FurState_e))*/
	if (   (processedRSInData->countedTemperature_s16 <= (structWorkingValue->TemperatureHot_u8 - structWorkingValue->HiddenMenuParam.HistTemp_u8))
		&& (FUR_IDLE == Gv_FurState_e))
	{
		//Gv_FurState_e = FUR_AUTO_ON;
		Gv_FurState_e = FUR_ON;
	}					
/*
	else if(   (processedRSInData->countedTemperature_s16 >= (structWorkingValue->TemperatureHot_u8/ *+histTemp* /))
			&& (FUR_OFF != Gv_FurState_e) 
			&& (FUR_IDLE != Gv_FurState_e))*/
	else if(   (processedRSInData->countedTemperature_s16 >= (structWorkingValue->TemperatureHot_u8/*+histTemp*/))
	&& (FUR_AUTO_ON_EXEC != Gv_FurState_e))
	{
		//Gv_FurState_e = FUR_BREAK;
		Gv_FurState_e = FUR_OFF;
	}
	else
	{
		; /* do nothng */
	} 
	
	/* count timer */
	if(TRUE == processedRSInData->dataInRS.MinuteEvent_bo)
	{
		structWorkingValue->WarmingTime_s16 -- ; 
	}
	
	if (0u == structWorkingValue->WarmingTime_s16)
	{
		Gv_StateMachine_e = SM_FAN_ON;
		Gv_InitOnEntry_bo = TRUE; 
		Gv_FurState_e = FUR_OFF;
	}
	
	/* fill LCD data tab by temperature hot*/
	if (TIMER_EVENT_8S > Gv_Timer10s_u8)
	{
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL, (int16_t)(structWorkingValue->ActualTemperature_s8), displayOutData_pa);
		addAnimationToLedDisplay(displayOutData_pa);
	}
	else 
	{	
		fillLcdDataTab(TEMP_LCD_CONV_LEVEL, (int16_t)(structWorkingValue->TemperatureHot_u8), displayOutData_pa);
	}
	
	return ;
	
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: addAnimationToLedDisplay(uint8_t *displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to add animation to Led display during furnance switch on state.
*
*---------------------------------------------------------------------------*/
static void addAnimationToLedDisplay(uint8_t *displayOutData_pa)
{
	if (Gv_Timer10s_u8>TIMER_EVENT_7S)
	{ 
		displayOutData_pa[0]= (uint8_t)DSL3;
	}
	else if (Gv_Timer10s_u8>TIMER_EVENT_6S)
	{ 
		displayOutData_pa[0]= (uint8_t)DSL2;
	}
	else if (Gv_Timer10s_u8>TIMER_EVENT_5S)
	{ 
		displayOutData_pa[0] = (uint8_t)DSL1;
	}
	else if (Gv_Timer10s_u8>TIMER_EVENT_4S)
	{ 
		displayOutData_pa[0] = (uint8_t)SPCJ;
	}
	else if (Gv_Timer10s_u8>TIMER_EVENT_3S)
	{ 
		displayOutData_pa[0]= (uint8_t)DSL3;
	}
	else if (Gv_Timer10s_u8>TIMER_EVENT_2S)
	{ 
		displayOutData_pa[0]= (uint8_t)DSL2;
	}
	else if (Gv_Timer10s_u8>TIMER_EVENT_1S)
	{ 
		displayOutData_pa[0]= (uint8_t)DSL1;
	}
	else
	{ 
		displayOutData_pa[0]= (uint8_t)SPCJ;
	}	
		
	return ;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: ClearAllTimers()
*
* FUNCTION ARGUMENTS:
*    None
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to clear some of the global timers used in file.
*
*---------------------------------------------------------------------------*/

static void ClearAllTimers()
{
	Gv_Timer10s_u8 = 0u;
	Gv_Timer8s_u8 = 0u;
	Gv_Timer6s_u8 = 0u;
	Gv_Timer2s_u8 = 0u;
	Gv_Timer1s_u8 = 0u;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: TimeCounter()
*
* FUNCTION ARGUMENTS:
*    None
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to manage all global timers used in file.
*
*---------------------------------------------------------------------------*/
static void TimeCounter()
{
	if (TIMER_EVENT_10S == Gv_Timer10s_u8)
	{
		Gv_Timer10s_u8 = 0u;
	}
	else
	{
		; /* do nothing */
	}
	
	if (TIMER_EVENT_8S == Gv_Timer8s_u8)
	{
		Gv_Timer8s_u8 = 0u;
	}
	else
	{
		; /* do nothing */
	}
	
	if (TIMER_EVENT_6S == Gv_Timer6s_u8)
	{
		Gv_Timer6s_u8 = 0u;
	}
	else
	{
		; /* do nothing */
	}
	
	if (TIMER_EVENT_3S == Gv_Timer3s_u8)
	{
		Gv_Timer3s_u8 = 0u;
	}
	else
	{
		; /* do nothing */
	}
	
	if (TIMER_EVENT_2S == Gv_Timer2s_u8)
	{
		Gv_Timer2s_u8 = 0u;
	}
	else
	{
		; /* do nothing */
	}
	
	if (TIMER_EVENT_1S == Gv_Timer1s_u8)
	{
		Gv_Timer1s_u8 = 0u;
	}
	else
	{
		; /* do nothing */
	}
	
	if (TIMER_EVENT_300MS == Gv_Timer300ms_u8)
	{
		Gv_Timer300ms_u8 = 0u;
	}
	else
	{
		; /* do nothing */
	}
	
	if (TIMER_SWITCH_50MS == Gv_Timer50ms_u8)
	{
		Gv_Timer50ms_u8 = 0u;
	}
	else
	{
		; /* do nothing */
	}
	

	
	if (TIMER_EVENT_1S == Gv_CounterLed_u8)
	{
		Gv_CounterLed_u8 = 0u;
	}
	else
	{
		; /* do nothing */
	}
	
	
	if (TRUE == Gv_FlagInterrupt50ms_bo)
	{
		Gv_FlagInterrupt50ms_bo = FALSE;
		Gv_Timer10s_u8 ++;
		Gv_Timer8s_u8 ++;
		Gv_Timer6s_u8 ++;
		Gv_Timer2s_u8 ++;
		Gv_Timer1s_u8 ++;
		Gv_Timer300ms_u8 ++;
		Gv_Timer50ms_u8 ++;
		Gv_CounterLed_u8 ++;
		
		if (TIMER_EVENT_2S > Gv_Timer2sHideMenu_u8)
		{
			Gv_Timer2sHideMenu_u8 ++ ;
		}
	}
}


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: FurnanceDelayStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    processedRSInData - struct with data read by RS485
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function to execute behavior of sauna in case of delay of furnace switching on phase.
*
*---------------------------------------------------------------------------*/

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
		
		Gv_BuzCounter_u16 = 50u;
		
		/* reset min timer */
		Gv_StatusTimerReset_bo = TRUE;
		
		/* clear initOnEntry flag - initialize was done */
		Gv_InitOnEntry_bo = FALSE;
	}
	/* initialize variable after back from temperature set state */
	if (SM_TEMP_SET == Gv_StateMachinePrevState_e)
	{
		/* Clear internal timers */
		ClearAllTimers();
		Gv_StateMachinePrevState_e = SM_FURNANCE_DELAY;
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
		if (FAN_IDLE != Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		else if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_ON2;
		}
	}
	else if(SW_LAMP == event_u8)
	{
		/*if (LAMP_ON2_EXEC == Gv_LampState_e)
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
		}*/
		if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_ON2;
		}
		else if (LAMP_IDLE != Gv_LampState_e)
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
	if(TRUE == processedRSInData->dataInRS.MinuteEvent_bo)
	{
		structWorkingValue->DelayWarmingTime_s16 -- ; 
	}
	
	/* switch to next state */
	if (0u == structWorkingValue->DelayWarmingTime_s16 )
	{
		Gv_StateMachine_e = SM_FURNANCE_ON;
		Gv_InitOnEntry_bo = TRUE; 
	}
	
	/* fill LCD data tab by delay timer */
	fillLcdDataTab(TIME_LCD_CONV_LEVEL, (int16_t)(structWorkingValue->DelayWarmingTime_s16) , displayOutData_pa);
	
	return ;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: TimerSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function to execute time settings for current power cycle of sauna.
*
*---------------------------------------------------------------------------*/
static void TimerSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa)
{
	static uint8_t menuLevel_u8 = 0u;
	
	/* initialize variable on entry to state */
	if (TRUE == Gv_InitOnEntry_bo)
	{
		/* clear led after return from delay state */
		Gv_LedStatus.TempLed_u8 = LED_BLINK;
		Gv_LedStatus.TimerOnLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;
		
		/* set menu level for 0 */
		menuLevel_u8 = 0u;
		
		ClearAllTimers();
		
		/* clear initOnEntry flag - initialize was done */
		Gv_InitOnEntry_bo = FALSE;
	}
	
	/* switch executing */
	if (SW_OFF_ON == event_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	else if((TIMER_EVENT_6S == Gv_Timer6s_u8 || (SW_TIMER == event_u8 && 0xFF == menuLevel_u8)))
	{
		Gv_StateMachine_e = Gv_StateMachinePrevState_e;
		Gv_InitOnEntry_bo = TRUE;
		Gv_StateMachinePrevState_e = SM_TIMER_SET;
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
		setFurnanceDelay(event_u8, structWorkingValue, displayOutData_pa, Gv_StateMachinePrevState_e);
		Gv_LedStatus.TimerOffLed_u8 = LED_OFF;
		Gv_LedStatus.TimerOnLed_u8 = LED_BLINK;
	}
	
	/* reset timer in case of make settings */
	if (   SW_TIMER == event_u8 
		|| SW_UP == event_u8 ||  SW_DOWN == event_u8
		|| SW_FAST_UP == event_u8 || SW_FAST_DOWN == event_u8
		|| SW_VERY_FAST_UP == event_u8 || SW_VERY_FAST_DOWN == event_u8)
	{
		Gv_Timer6s_u8 = 0u;
	}
	
	return ;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: FanOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    processedRSInData - struct with data read by RS485
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function to execute behavior of sauna in case of fan phase.
*
*---------------------------------------------------------------------------*/
static void FanOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
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
		Gv_FanState_e = FAN_AUTO_ON;
		
		Gv_BuzCounter_u16 = 50u;
		
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
/*
		if (FAN_AUTO_ON_EXEC == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_AUTO_ON;
		}*/
		if (FAN_IDLE != Gv_FanState_e)
		{
			Gv_FanState_e = FAN_OFF;
		}
		else if (FAN_IDLE == Gv_FanState_e)
		{
			Gv_FanState_e = FAN_AUTO_ON;
		}
	}
	else if(SW_LAMP == event_u8)
	{
/*
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
		}*/
		if (LAMP_IDLE != Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_OFF;
		}
		else if (LAMP_IDLE == Gv_LampState_e)
		{
			Gv_LampState_e = LAMP_AUTO_ON;
		}
	}
	else
	{
		; /* do nothing */
	}
	
	/* count timer */
	if(TRUE == processedRSInData->dataInRS.MinuteEvent_bo) 
	{
		structWorkingValue->TimerCurrentFanSet_u8 -- ;
	}
	/* switching to next state */
	if (0u == structWorkingValue->TimerCurrentFanSet_u8)
	{
		Gv_StateMachine_e = SM_OFF;
		Gv_InitOnEntry_bo = TRUE;
	}
	
	/* LED display */
	/* fill LCD data tab by delay timer */
	fillLcdDataTab(TEMP_LCD_CONV_LEVEL, (int16_t)(structWorkingValue->ActualTemperature_s8) , displayOutData_pa);
	
	return ;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: ErrorStateExecute(uint8_t event_u8, processedDataInRS_t *processedRSInData, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with menu settings for sauna
*    processedRSInData - struct with data read by RS485
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function to execute behavior of sauna in case temperature sensor error detecting.
*
*---------------------------------------------------------------------------*/
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
	}
		
		
	/* switch executing */
	if (SW_OFF_ON == event_u8)
	{
		processedRSInData->dataInRS.Error1Sen_bo = FALSE;
		processedRSInData->dataInRS.Error2Sen_bo = FALSE;
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

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: SwEventChoose (volatile uint8_t *switchCounter_bo, uint8_t *swEvent_u8 )
*
* FUNCTION ARGUMENTS:
*    switchCounter_bo - counter used to measure debounce time for switch
*    swEvent_u8 - flag indicate switching on particular switch
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function to check event related with switch manipulate.
*
*---------------------------------------------------------------------------*/
static void SwEventChoose (volatile uint8_t *switchCounter_bo, uint8_t *swEvent_u8 )	
 {     
	*swEvent_u8 = 0u;
	if (TRUE == *switchCounter_bo)
	{
		*switchCounter_bo = FALSE;
		
	    if (   (IDLE == obslugaPrzyciskuKrotkiego(12u,PINA,0x10u,1u)) 
			&& (SHORT == obslugaPrzyciskuKrotkiego2(0u,PINA,0x01u,15u))) // ON/OFF
		    
		{
			*swEvent_u8 = SW_OFF_ON;
			Gv_BuzCounter_u16 = 50u; 
		}
	    else if ( (SHORT == obslugaPrzyciskuKrotkiego2(1u,PINA,0x02u,15u))
				&& (SM_ERROR != Gv_StateMachine_e)
				&& (SM_HIDE_MENU != Gv_StateMachine_e)
				&& (SM_TEMP_SET != Gv_StateMachine_e)
				&& (SM_TIMER_SET != Gv_StateMachine_e)) // LAMP
		{
			*swEvent_u8 = SW_LAMP; 
			Gv_BuzCounter_u16 = 50u;
		}
	    else if (  (SHORT == obslugaPrzyciskuKrotkiego2(2u,PINA,0x04u,15u)) 
				&& (SM_OFF != Gv_StateMachine_e)
				&& (SM_ERROR != Gv_StateMachine_e)) // UP
		{
			*swEvent_u8 = SW_UP;
			Gv_BuzCounter_u16 = 50u; 
		}
	    else if ( SHORT == obslugaPrzyciskuKrotkiego2(3u,PINA,0x08u,15u) 
				&& (SM_ERROR != Gv_StateMachine_e)
				&& (SM_HIDE_MENU != Gv_StateMachine_e)
				&& (SM_TEMP_SET != Gv_StateMachine_e)
			    && (SM_TIMER_SET != Gv_StateMachine_e)) // FAN
		{
			*swEvent_u8 = SW_FAN;
			Gv_BuzCounter_u16 = 50u;
		}
	    else if (  (SHORT == obslugaPrzyciskuKrotkiego2(4u,PINA,0x10u,15u)) 
				&& (SM_OFF != Gv_StateMachine_e) 
				&& (SM_ERROR != Gv_StateMachine_e) 
				&& (SM_TEMP_SET != Gv_StateMachine_e)) // MENU/TIMER
		{
			*swEvent_u8 = SW_TIMER;
			Gv_BuzCounter_u16 = 50u;
		}
	    else if (  (SHORT == obslugaPrzyciskuKrotkiego2(5u,PINA,0x20u,15u)) 
				&& (SM_OFF != Gv_StateMachine_e) 
				&& (SM_ERROR != Gv_StateMachine_e) ) // DOWN
		{
			*swEvent_u8 = SW_DOWN; 
			Gv_BuzCounter_u16 = 50u;
		}
	    else if (  (SHORT == obslugaPrzyciskuKrotkiego(6u,PINA,0x10u,1500u)) //MENU
				&& (SHORT == obslugaPrzyciskuKrotkiego(11u,PINA,0x01u,1500u)) //ON-OFF
				&& (SM_OFF == Gv_StateMachine_e)) // HIDDEN MENU
		{
			*swEvent_u8 = SW_MENU;
			Gv_BuzCounter_u16 = 100u;
		}
	    if (   (SHORT2 == obslugaPrzyciskuKrotkiego4(7u,PINA,0x04u,150u,100u)) 
			&& (SM_OFF != Gv_StateMachine_e)
			&& (SM_ERROR != Gv_StateMachine_e))// very fast UP
		{
			*swEvent_u8 = SW_VERY_FAST_UP;
			Gv_BuzCounter_u16 = 20u;
		}
	    if (   (SHORT1 == obslugaPrzyciskuKrotkiego4(8u,PINA,0x04u,150u,100u)) 
			&& (SM_OFF != Gv_StateMachine_e)
			&& (SM_ERROR != Gv_StateMachine_e)) // fast UP
	    {
		    *swEvent_u8 = SW_FAST_UP;
			Gv_BuzCounter_u16 = 20u;
	    }
	    if (   (SHORT2 == obslugaPrzyciskuKrotkiego4(9u,PINA,0x20u,150u,100u)) 
			&& (SM_OFF != Gv_StateMachine_e)
			&& (SM_ERROR != Gv_StateMachine_e)) // very fast DOWN
		{
			*swEvent_u8 = SW_VERY_FAST_DOWN;
			Gv_BuzCounter_u16 = 20u;
		}
	    if (   (SHORT1 == obslugaPrzyciskuKrotkiego4(10u,PINA,0x20u,150u,100u)) 
			&& (SM_OFF != Gv_StateMachine_e)
			&& (SM_ERROR != Gv_StateMachine_e)) // fast DOWN
	    {
		    *swEvent_u8 = SW_FAST_DOWN;
			Gv_BuzCounter_u16 = 20u;
	    }
	}
	return;
 }

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: StateMachine(uint8_t event_u8, strSaunaParam_t * structWorkingValue,  processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    event_u8 - value indicate for switch event
*    structWorkingValue - struct with main menu settings for sauna
*    processedRSInData - struct with main data read by RS485
*    displayOutData_pa - array with data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Main state machine for sauna.
*
*---------------------------------------------------------------------------*/

static void StateMachine(uint8_t event_u8, strSaunaParam_t * structWorkingValue,  processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
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
			FanOnStateExecute(event_u8, structWorkingValue, processedRSInData, displayOutData_pa);
			break;
		case SM_ERROR:
			ErrorStateExecute(event_u8, processedRSInData, structWorkingValue, displayOutData_pa);
		break;

	}
	return ;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: FurnanceStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData)
*
* FUNCTION ARGUMENTS:
*    structWorkingValue - struct with main menu settings for sauna
*    processedRSInData - struct with main data read by RS485
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    State machine for manage switching on and off the Furnance, depending on sauna state.
*
*---------------------------------------------------------------------------*/
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
			; /* do nothing */
		break;
		case FUR_ON: /* switch on the furnace for the first time */
			RsDataTab(RS_FUR_ON, Gv_TabSendDataRS485_au8);
			//timerFur_u16 = structWorkingValue->WarmingTime_s16;
			Gv_LedStatus.TempLed_u8 = LED_ON;
			Gv_FurState_e = FUR_AUTO_ON_EXEC;
		break;
		case FUR_AUTO_ON: /* switch on the furnace for the first time */
			RsDataTab(RS_FUR_ON, Gv_TabSendDataRS485_au8);
			Gv_FurState_e = FUR_AUTO_ON_EXEC;
		break;
		case FUR_AUTO_ON_EXEC: /* automatic working of the furnace */
			; /* do nothing */
		break;
		case FUR_BREAK: /* switch off the furnace for some period because of reach max temperature */
			 RsDataTab(RS_FUR_OFF, Gv_TabSendDataRS485_au8);
			 Gv_FurState_e = FUR_BREAK_EXEC;
		break;
		case FUR_BREAK_EXEC: /* switch off the furnace for some period because of reach max temperature */
			; /* do nothing */
		break;
		case FUR_OFF: /* switching off the furnace */
			RsDataTab(RS_FUR_OFF, Gv_TabSendDataRS485_au8);
			Gv_LedStatus.TempLed_u8 = LED_OFF;
			Gv_FurState_e = FUR_IDLE;
		break;

	}
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: FanStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData)
*
* FUNCTION ARGUMENTS:
*    structWorkingValue - struct with main menu settings for sauna
*    processedRSInData - struct with main data read by RS485
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    State machine for manage switching on and off the Fan, depending on sauna state.
*
*---------------------------------------------------------------------------*/

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
			timerFan_u16 = structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16;
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
			//timerFan_u16 = structWorkingValue->HiddenMenuParam.TimerFanSet_u8;
			Gv_LedStatus.FanLed_u8 = LED_ON;
			Gv_FanState_e = FAN_AUTO_ON_EXEC;
		break;
		case FAN_AUTO_ON_EXEC: /* automatic work execute */
			; /* do nothing */
		break;
		case FAN_OFF: /* switch off the fan */
			RsDataTab(RS_FAN_OFF, Gv_TabSendDataRS485_au8);
			Gv_LedStatus.FanLed_u8 = LED_OFF;
			Gv_FanState_e = FAN_IDLE;
		break;

	}
	return;
}


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: LampStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData)
*
* FUNCTION ARGUMENTS:
*    structWorkingValue - struct with main menu settings for sauna
*    processedRSInData - struct with main data read by RS485
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    State machine for manage switching on and off the Lamp, depending on sauna state.
*
*---------------------------------------------------------------------------*/

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
			timerLamp_u16 = structWorkingValue->HiddenMenuParam.WarmingTimeMax_s16;
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
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: LedWorkStatus()
*
* FUNCTION ARGUMENTS:
*    None
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to manage the LED status (switch on, switch off , blink LED diode)
*
*---------------------------------------------------------------------------*/

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
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: fillTabByLcdDataMenu(uint8_t ledDisplayLevel_u8, uint8_t *displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    ledDisplayLevel_u8 - value indicate menu level
*    displayOutData_pa - array with data for display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used t set hidden menu level on Led display.
*
*---------------------------------------------------------------------------*/	
	
static void fillTabByLcdDataMenu(uint8_t ledDisplayLevel_u8, uint8_t *displayOutData_pa)
{
	static uint8_t previousLevel_u8 = 0u;
	
	if (previousLevel_u8 != ledDisplayLevel_u8)
	{
		DsLedOff();
		previousLevel_u8 = ledDisplayLevel_u8;
	}
		
	displayOutData_pa[0] = LEF;
	displayOutData_pa[2] = SPCJ;
	displayOutData_pa[3] = SPCJ;
	
	switch (ledDisplayLevel_u8)
	{
		case 1://menu 1
			displayOutData_pa[1] = D1;
		break;
		case 2: //menu 2
			displayOutData_pa[1] = D2;
		break;
		case 3://menu 3
			displayOutData_pa[1] = D3;
		break;
		case 4://menu 4
			displayOutData_pa[1] = D4;
		break;
		case 5://menu 5
			displayOutData_pa[1] = D5;
		break;
		case 6://menu 6
			displayOutData_pa[1] = D6;
		break;
	}
	
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: fillLcdDataTab(uint8_t conversionLevel_u8, int16_t valueToConvert_u16, uint8_t *displayOutData_pa)
*
* FUNCTION ARGUMENTS:
*    conversionLevel_u8 - value indicate conversion type (time, temperature, calibration)
*    valueToConvert_u16 - value to convert
*    displayOutData_pa - array with data for display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to convert data like temperature, time to corresponding display data.
*
*---------------------------------------------------------------------------*/                                                                                                             
		
static void fillLcdDataTab(uint8_t conversionLevel_u8, int16_t valueToConvert_u16, uint8_t *displayOutData_pa)
{	
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
			case 3:/* error info */
				displayOutData_pa[3] = SPCJ;
				displayOutData_pa[2] = SPCJ;
				displayOutData_pa[0] = LE;
				if (SENS_ERROR1 == valueToConvert_u16)
				{
					displayOutData_pa[1] = D1;
				}
				else if (SENS_ERROR2 == valueToConvert_u16)
				{
					displayOutData_pa[1] = D2;
				}
				else 
				{
					displayOutData_pa[1] = D0;
				}
			break;
		}
		
		return;
	}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: ChangeMenu(uint8_t event_u8, uint8_t *currentMenuState_u8, menuItem_t *tabHideMenu, strSaunaParam_t *structure_pstr , uint8_t * tab_pu8 )
*
* FUNCTION ARGUMENTS:
*    event_u8 - event related with switch manipulation
*    currentMenuState_u8 - Current hide men state 
*    tabHideMenu - tab with menu levels and function 
*    structure_pstr - structure with sauna menu parameter
*    tab_pu8 - data for Led display
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to change Hide menu level.
*
*---------------------------------------------------------------------------*/

static void ChangeMenu(uint8_t event_u8, uint8_t *currentMenuState_u8, menuItem_t *tabHideMenu, strSaunaParam_t *structure_pstr , uint8_t * tab_pu8 )
{
	const uint8_t switchEvent_u8 = 2u; /* to choose proper position in tab related witch sw event */
	const uint8_t timeExpired_u8 = 3u; /* to choose proper position in tab related witch time expired */
	/* choose  menu state by switch and after 3s info go to settings */
	if ( SW_TIMER == event_u8 || SW_MENU == event_u8 )
	{	
		*currentMenuState_u8 = tabHideMenu[*currentMenuState_u8].nextState_au8[switchEvent_u8];	
		Gv_Timer2sHideMenu_u8 = 0u;	
		DsLedOff();
		SaveToEEPROM(structure_pstr);
	}
	else if ( TIMER_EVENT_2S == Gv_Timer2sHideMenu_u8)		
	{	
		*currentMenuState_u8 = tabHideMenu[*currentMenuState_u8].nextState_au8[timeExpired_u8];
		Gv_Timer2sHideMenu_u8 ++;
		Gv_BuzCounter_u16 = 50u; 
		
	}
		
	/* call proper function */
	tabHideMenu[*currentMenuState_u8].callback(event_u8,structure_pstr, tab_pu8,*currentMenuState_u8);
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: TimeConv(uint16_t time_u16, uint8_t digTime[4])
*
* FUNCTION ARGUMENTS:
*    time_u16 - value for time to convert
*    digTime[4] - array with digit value corresponding to time value
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to change input time data for its digit representation.
*
*---------------------------------------------------------------------------*/

static void TimeConv(uint16_t time_u16, uint8_t digTime_au8[4])
{
	digTime_au8[0]=(uint8_t)(time_u16/600);
	digTime_au8[1]=(uint8_t)((time_u16/60)%10);
	digTime_au8[2]=(uint8_t)((time_u16%60)/10);
	digTime_au8[3]=(uint8_t)((time_u16%60)%10);
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: DigConvToDsp(uint8_t tabIn_au8[4], uint8_t tabOutDsp_au8[4])
*
* FUNCTION ARGUMENTS:
*    tabIn_au8[4] - array with digit corresponding to represented data
*    tabOutDsp_au8[4] - value corresponding LED representation of input digit
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to change input data for their Led display representation.
*
*---------------------------------------------------------------------------*/

static void DigConvToDsp(uint8_t tabIn_au8[4], uint8_t tabOutDsp_au8[4])
{
	for (int i=0; i<4u; i++)
	{
		if (0u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D0;	}
		else if (1u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D1;}
		else if (2u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D2;}
		else if (3u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D3;}
		else if (4u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D4;}
		else if (5u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D5;}
		else if (6u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D6;}
		else if (7u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D7;}
		else if (8u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D8;}
		else if (9u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D9;}
	}
	
	if (0u == tabIn_au8[0])
	{
		tabOutDsp_au8[0] = SPCJ;
	}
	//if (tabIn[0]==0 && tabIn[1]==0){tabOutDsp[1]=0xFF; - problem przy przedstawianiu czasu
	//if (tabIn[0]==0 && tabIn[1]==0 && tabIn[2]==0){tabOutDsp[2]=0xFF; - problem przy przedstawianiu czasu
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: TimeConvToDsp(uint8_t tabIn_au8[4], uint8_t tabOutDsp_au8[4])
*
* FUNCTION ARGUMENTS:
*    tabIn_au8[4] - array with digit corresponding to time data
*    tabOutDsp_au8[4] - value corresponding LED representation of input digit
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to change input data for their Led display representation.
*
*---------------------------------------------------------------------------*/

static void TimeConvToDsp(uint8_t tabIn_au8[4], uint8_t tabOutDsp_au8[4])
{
	for (int i=0; i<4u; i++)
	{
		if (0u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D0;	}
		else if (1u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D1;}
		else if (2u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D2;}
		else if (3u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D3;}
		else if (4u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D4;}
		else if (5u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D5;}
		else if (6u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D6;}
		else if (7u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D7;}
		else if (8u == tabIn_au8[i])	
			{tabOutDsp_au8[i] = D8;}
		else if (9u == tabIn_au8[i])
			{tabOutDsp_au8[i] = D9;}
	}
	if (0u == tabIn_au8[0]){
		tabOutDsp_au8[0]=tabOutDsp_au8[1];
		tabOutDsp_au8[1]=0x89;
	}
	else{
		tabOutDsp_au8[3]=tabOutDsp_au8[2];
		tabOutDsp_au8[2]=0x89;
	}
	return;
	//if (tabIn[0]==0){tabOutDsp[0]=0xFF;}
	//if (tabIn[0]==0 && tabIn[1]==0){tabOutDsp[1]=0xFF; - problem przy przedstawianiu czasu
	//if (tabIn[0]==0 && tabIn[1]==0 && tabIn[2]==0){tabOutDsp[2]=0xFF; - problem przy przedstawianiu czasu
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: DigTempConvToDsp(uint8_t tabInDigit_au8[4], uint8_t tabOutLcdRepresentation_au8[4])
*
* FUNCTION ARGUMENTS:
*    tabInDigit_au8[4] - array with data corresponding to hundred, decimal, and unit parts of temperature
*    tabOutLcdRepresentation_au8[4] - value corresponding LED representation of input digit
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to change input data for their Led display representation.
*
*---------------------------------------------------------------------------*/

static void DigTempConvToDsp(uint8_t tabInDigit_au8[4], uint8_t tabOutLcdRepresentation_au8[4])
{ 
	/* function doesn't consider negative temperature, because it is used 
	   only to calculate the positive temperature */
	for (int i=0; i<4u; i++)
	{
		if (tabInDigit_au8[i]==0)
			{tabOutLcdRepresentation_au8[i] = D0;}
		else if (tabInDigit_au8[i]==1)
			{tabOutLcdRepresentation_au8[i] = D1;}
		else if (tabInDigit_au8[i]==2)
			{tabOutLcdRepresentation_au8[i] = D2;}
		else if (tabInDigit_au8[i]==3)
			{tabOutLcdRepresentation_au8[i] = D3;}
		else if (tabInDigit_au8[i]==4)
			{tabOutLcdRepresentation_au8[i] = D4 ;}
		else if (tabInDigit_au8[i]==5)
			{tabOutLcdRepresentation_au8[i] = D5;}
		else if (tabInDigit_au8[i]==6)
			{tabOutLcdRepresentation_au8[i] = D6;}
		else if (tabInDigit_au8[i]==7)
			{tabOutLcdRepresentation_au8[i] = D7;}
		else if (tabInDigit_au8[i]==8)
			{tabOutLcdRepresentation_au8[i] = D8;}
		else if (tabInDigit_au8[i]==9)
			{tabOutLcdRepresentation_au8[i] = D9;}
	}
	if (0u == tabInDigit_au8[0])
		{tabOutLcdRepresentation_au8[0] = SPCJ;}
	if ((0u == tabInDigit_au8[0]) && (0u == tabInDigit_au8[1]))
		{tabOutLcdRepresentation_au8[1] = SPCJ; }
	if ((0u == tabInDigit_au8[0]) && (0u == tabInDigit_au8[1]) && (0u == tabInDigit_au8[2]))
		{tabOutLcdRepresentation_au8[2] = SPCJ;}
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: TempConvToDigit(uint16_t temperature_u16, uint8_t digTemp_au8[4])
*
* FUNCTION ARGUMENTS:
*    temperature_u16 - value of temperature to decomposite
*    digTemp_au8[4] - array for corresponding parts of decomposed temperature
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to decomposite temperature value for specified part corresponding to hundred, decimal and unit parts of this value.
*
*---------------------------------------------------------------------------*/

static void TempConvToDigit (uint16_t temperature_u16, uint8_t digTemp_au8[4])
{ 
	/* function doesn't consider negative temperature, because it is used 
	   only to calculate the positive temperature */
	digTemp_au8[0] = 0x00u;
	digTemp_au8[1] = (uint8_t)temperature_u16/100u;
	digTemp_au8[2] = (uint8_t)(temperature_u16%100u)/10u;
	digTemp_au8[3] = (uint8_t)(temperature_u16%100u)%10u;
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: TabToTempConv(uint8_t firstDig100_u8, uint8_t secDigit10_u8, uint8_t  thrDigit1_u8)
*
* FUNCTION ARGUMENTS:
*    firstDig100_u8 - value corresponding to hundred parts
*    secDigit10_u8 - value corresponding to decimal parts
*    thrDigit1_u8 - value corresponding to unit parts
*
* RETURN VALUE:
*    temperature_s16 - calculated temperature value
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to recalculate temperature from specified part corresponding to hundred, decimal and unit parts to one value.
*
*---------------------------------------------------------------------------*/

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
		return ;
}


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: CheckData485Presence()
*
* FUNCTION ARGUMENTS:
*    None
*
* RETURN VALUE:
*    retVal_bo - TRUE if data are present in buffer, FALSE if no data in buffer
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
	return;
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
	return;
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: OnOffRSinfoSend(uint8_t statusSend_bo)
*
* FUNCTION ARGUMENTS:
*    statusSend_bo - status of operation which we want to set 
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to send information by RS485 about switch on and off the controller
*
*---------------------------------------------------------------------------*/

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
	
	return;
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
	return;        
}

/*----------------------------------------------------------------------------
*
* FUNCTION NAME: OutputExecute(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSData, uint8_t tabOutDsp_au8[])
*
* FUNCTION ARGUMENTS:
*    tabOutDsp_au8 - data for LED display
*	 processedRSData - struct with data read from base module by RS485
*	 structWorkingValue - struct with main menu param for sauna
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
	//fillLcdDataTab(2, (int16_t) debugCounter, tabOutDsp_au8);
	DsLedSend(tabOutDsp_au8[0], tabOutDsp_au8[1], tabOutDsp_au8[2], tabOutDsp_au8[3]);
	LedWorkStatus();
	BuzzerWork(Gv_BuzCounter_u16, 1u);
	SendDataByRs();
	return;
}


/*----------------------------------------------------------------------------
*
* FUNCTION NAME: SendDataByRs()
*
* FUNCTION ARGUMENTS:
*    None
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    Function used to send array with data by RS485 depend on additional line, function also check reply after sending data, and in case of error send data again.
*
*---------------------------------------------------------------------------*/

				
static void SendDataByRs()
{
/*--------old concept for sending data by RS 485- do not change------------------*/
	if(Gv_SendRS485AllowFlag_u8==1)
	{
		/* protection for RS485 - it is needed in a few places in code ??? */
		if (!(PIND & (1<<PD3)))
		{
			Gv_NrByte_u8=0;Gv_EnableSend2_u8=0;
		}
		if (!(PIND & (1<<PD2)))
		{
			Gv_EnableSend1_u8=0; Gv_SendStepLevel_u8=0; Gv_SendStepCounter_u8=0;
		}
		/* protection for RS485 - it preserve by set 1 from both sides */
		if (!(Gv_SendStepLevel_u8==0))
		{
			Gv_PresM1_u8++;
		}
		else 
		{
			Gv_PresM1_u8=0;
		}
		if (Gv_PresM1_u8==15)
		{
			Gv_SendStepLevel_u8=0; Gv_PresM1_u8=0; Gv_SendStepCounter_u8=0; PORTD &= ~(1<<PD2);
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
		if ((!(PIND & (1<<PD3))) && Gv_OutputRSDataPresent_u8==1) /* if 0 detect from opposite site and there is data to send in buffer */
		{ 
			if (Gv_SendStepLevel_u8==0 )
			{
				debugCounter++;
				if(debugCounter==255){debugCounter=0;}
				PORTD |= (1<<PD2); /* set pin for 1 which tell us about sending data by Master device */
				/*_delay_us(5);*/
				USART_SendByteM(Gv_TabSendDataRS485_au8[9]);
				Gv_SendStepLevel_u8=1;}/* data sending */
			else if (Gv_SendStepLevel_u8==1)
			{
				USART_SendByteM(Gv_TabSendDataRS485_au8[9]);
				Gv_SendStepLevel_u8=2;
			}
			else if (Gv_SendStepLevel_u8==2)
			{
				Gv_SendStepCounter_u8++;
				//if (returnData==0xFF && enableSend1==1){enableSend1=0;RsShiftTab(tabDataRS); sendStep=0;stepS=0;
				if (Gv_ReturnData_u8==Gv_TabSendDataRS485_au8[9] && Gv_EnableSend1_u8==1)
				{
					Gv_EnableSend1_u8=0;RsShiftTab(Gv_TabSendDataRS485_au8); Gv_SendStepLevel_u8=0;Gv_SendStepCounter_u8=0;Gv_PresM1_u8=0;
					_delay_us(10);
					PORTD &= ~(1<<PD2);
				}
				else if (/*returnData==0x00 &&*/ Gv_EnableSend1_u8==1)
				{
					Gv_EnableSend1_u8=0;Gv_SendStepLevel_u8=0;Gv_SendStepCounter_u8=0;Gv_PresM1_u8=0;
					_delay_us(10);
					PORTD &= ~(1<<PD2);
				}
				
				if (Gv_SendStepCounter_u8==5)
				{
					Gv_SendStepLevel_u8=0;
					//nrByte=0;
					PORTD &= ~(1<<PD2);
					Gv_SendStepCounter_u8=0;
					Gv_PresM1_u8=0;
				}
			}
		}

			 
		// if((PIND & (1<<PD3)) && enableSend2==1){_delay_us(100);USART_SendByteM(0xFF);enableSend2=0;nrByte=0;}
		if((PIND & (1<<PD3)) && Gv_EnableSend2_u8==1)
		{
			_delay_us(10);USART_SendByteM(Gv_TabDataRxc_au8[0]);Gv_EnableSend2_u8=0;Gv_NrByte_u8=0;
		}
		if((PIND & (1<<PD3)) && Gv_EnableSend2_u8==2)
		{
			_delay_us(10);USART_SendByteM(0x00);Gv_EnableSend2_u8=0;Gv_NrByte_u8=0;
		}
			 
		Gv_SendRS485AllowFlag_u8=0;
	}
	/* end of old concept for sending data */
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
	//wdt_enable(WDTO_2S);
	
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
	static uint8_t displayOutData_au8[4] = {0xFFu,0xFFu,0xFFu,0xFFu};
	//static uint8_t menuLevel_u8 = 0u;
	static processedDataInRS_t processedRSInputData = {{0u}, 0u};
	
	SetRegisterUC(); /* set register status */
	InitDataOnEntry(&structWorkingValue);
	
	while(1)
	{
		wdt_reset();
		TimeCounter(); /* function to increment timers */
		ProcessInputRSData(&processedRSInputData); /* switch execute, input 485 data execute */
		SwEventChoose(&Gv_SwitchCounter_bo, &event_u8);
		SensorTemperatureCalibration(&structWorkingValue, processedRSInputData.countedTemperature_s16);

		StateMachine(event_u8, &structWorkingValue, &processedRSInputData, displayOutData_au8);
		OutputExecute(&structWorkingValue, &processedRSInputData ,displayOutData_au8); /* led execute, display led execute, rsData set*/
	}

}

ISR(TIMER0_OVF_vect) //wywolywana co 0.001s
{ 
	if  (Gv_BuzCounter_u16 > 0u) 
	{
		 Gv_BuzCounter_u16 --;
	}
	
	
	Gv_SwitchCounter_bo = TRUE;
	Gv_SendRS485AllowFlag_u8 = 1u;
   
    TCNT0=194;
 }

ISR(TIMER2_OVF_vect) //wywolywana co 0.001s
{
	static uint8_t internalCounter_u8 = 0u;
	internalCounter_u8 ++;
	if (25u == internalCounter_u8)
	{
		Gv_FlagInterrupt50ms_bo = TRUE; //every 50ms
		internalCounter_u8 = 0u;
	}
	//switchCounterM=1;
    //TCNT2=226;
	//TCNT2=50;
	TCNT2=130;
}


/////////////////////////////RS 10 5 16/////////////////////////////////////////////////////////
	

ISR (USART_RXC_vect){	
	/*--------old concept for receiving data by RS 485 - do not change------------------*/
	static uint8_t emptyRxc_u8;	/* for get data from  uSART buffer */
	if(!(PIND & (1<<PD3)) && Gv_SendStepLevel_u8==2)/* if from base side detect 0 and  Master wait for response for reply - master send byte */
	{
		Gv_ReturnData_u8 = UDR;// USART_Receive();
		Gv_EnableSend1_u8=1;
	}
	else if ((PIND & (1<<PD3)) && Gv_SendStepLevel_u8==0) /*if from base side detect 0 and  function responsible for sending has flag set for 0 (sending for sure has finished) */
	{
		if (Gv_NrByte_u8==0)
		{
			Gv_TabDataRxc_au8[0] = UDR;//USART_Receive();
			Gv_NrByte_u8=1;
		}
		else if (Gv_NrByte_u8==1)
		{
			Gv_TabDataRxc_au8[1] = UDR;//USART_Receive();
			Gv_NrByte_u8=0;
			if (Gv_TabDataRxc_au8[0]==Gv_TabDataRxc_au8[1] )
			{
				RsDataTabRec(Gv_TabDataRxc_au8[0],Gv_tabRecDataRS485_au8);Gv_EnableSend2_u8=1; 
			}
			if (!(Gv_TabDataRxc_au8[0]==Gv_TabDataRxc_au8[1]))
			{
				Gv_EnableSend2_u8=2; 
			}
		}
	}
	else 
	{
		emptyRxc_u8 = UDR;/*USART_Receive();*/
	}
	/*--------end of old concept for receiving data ------------------*/
}
				
