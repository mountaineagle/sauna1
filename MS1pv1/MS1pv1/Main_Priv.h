#ifndef _MAIN_PRIV_H_
#define _MAIN_PRIV_H_

/*
do zrobienia:
nie dzia³a brzeczyk w menu ukrytym
nie ddzia³¹ wejscie w mwnu ukryte - dziala co drugie nacisniecie on off
*/

/*----- macro definition --------- */

#define TRUE	(uint8_t)0xFFu		/* !< used to indicate for TRUE >!*/
#define FALSE	(uint8_t)0x00u		/* !< used to indicate for FALSE >!*/
#define INVALID (uint8_t)0xFFu		/* !< used to indicate for INVALID value >!*/


/*! \brief RS data code */
#define RS_PANEL_PRES	(uint8_t)0x01u		/* !< value send by RS485 to indicate panel presence >!*/
#define RS_CTRL_ON		(uint8_t)0x02u		/* !< value send by RS485 to switch on controller >!*/
#define RS_CTRL_OFF		(uint8_t)0x03u		/* !< value send by RS485 to switch off controller >!*/
#define RS_LAMP_OFF		(uint8_t)0x06u		/* !< value send by RS485 to switch on lamp >!*/
#define RS_LAMP_ON		(uint8_t)0x07u		/* !< value send by RS485 to switch off lamp >!*/
#define RS_FUR_ON		(uint8_t)0x09u		/* !< value send by RS485 to switch on furnace >!*/
#define RS_FUR_OFF		(uint8_t)0x0Au		/* !< value send by RS485 to switch off furnace >!*/
#define RS_FAN_ON		(uint8_t)0x0Bu		/* !< value send by RS485 to switch on fan >!*/
#define RS_FAN_OFF		(uint8_t)0x0Cu		/* !< value send by RS485 to switch off fan >!*/
#define RS_MIN_RES		(uint8_t)0x0Du		/* !< value send by RS485 to reset minute counter in base module >!*/

/*! \brief maximum, minimum and default values for hidden menu parameters */
#define CALIBRATION_MAX (int8_t)10
#define CALIBRATION_MIN (int8_t)-10
#define CALIBRATION_DEF (int8_t)0
#define WARMING_TIME_MAX_MIN 10
#define WARMING_TIME_MAX_MAX 360
#define WARMING_TIME_MAX_DEF 240
#define TEMPERATURE_HOTMAX_MAX 110u
#define TEMPERATURE_HOTMAX_MIN 50u
#define TEMPERATURE_HOTMAX_DEF 90u
#define TEMPERATURE_HOTMIN_MAX 50u
#define TEMPERATURE_HOTMIN_MIN 30u
#define TEMPERATURE_HOTMIN_DEF 50u
#define HIST_TEMP_MAX 5u
#define HIST_TEMP_MIN 2u
#define HIST_TEMP_DEF 2u
#define TIMER_FAN_MAX 60u
#define TIMER_FAN_MIN 5u
#define TIMER_FAN_DEF 10u

/*! \brief default values for some of sauna parameters defined after each switch on controller */
/*#define WARMING_TIME_DEF 240u this should be set for actual value of WARMING_TIME_MAX parameter */ 
#define TEMPERATURE_HOT_DEF 50u
#define DELAY_WARMING_TIME_MIN (int16_t)0
#define DELAY_WARMING_TIME_MAX (int16_t)1200
#define DELAY_WARMING_TIME_DEF (int16_t)0

/*! \brief value for time event */
#define TIMER_EVENT_1S		20u		/* !< indicate 1s period >!*/
#define TIMER_EVENT_2S		40u		/* !< indicate 2s period >!*/
#define TIMER_EVENT_3S		60u		/* !< indicate 3s period >!*/
#define TIMER_EVENT_4S		80u		/* !< indicate 4s period >!*/
#define TIMER_EVENT_5S		100u	/* !< indicate 5s period >!*/
#define TIMER_EVENT_6S		120u	/* !< indicate 6s period >!*/
#define TIMER_EVENT_7S		140u	/* !< indicate 7s period >!*/
#define TIMER_EVENT_8S		160u	/* !< indicate 8s period >!*/
#define TIMER_EVENT_10S		200u	/* !< indicate 10s period >!*/
#define TIMER_EVENT_300MS	6u		/* !< indicate 300ms period >!*/
#define TIMER_SWITCH_50MS	1u		/* !< indicate 50ms period >!*/

/*! \brief value for led management*/
#define LED_OFF		(uint8_t)0u		/* !< value corresponding to off state for LED diode >!*/
#define LED_ON		(uint8_t)1u	    /* !< value corresponding to on state for LED diode >!*/
#define LED_BLINK	(uint8_t)2u		/* !< value corresponding to blink state for LED diode >!*/

/*! \brief value for in state */
#define TIME_TO_OFF 900u  /* !< sec to off controller from in state */

#define TIME_DELAY_10H 600u  /* !< indicate minutes for 10h */

/*! \brief value LCD */
#define POINT_LCD_MARK 0x7Fu	/* !< value corresponding to point mark on LED display >!*/

/*! \brief value for event_u8 */
#define SW_IDLE 			(uint8_t)0u		/* !< indicate that none of the switch was press >!*/
#define SW_OFF_ON			(uint8_t)1u		/* !< indicate that on/off switch was press >!*/
#define SW_UP				(uint8_t)2u		/* !< indicate that + switch was press >!*/
#define SW_DOWN				(uint8_t)3u		/* !< indicate that - switch was press >!*/
#define SW_FAST_UP			(uint8_t)4u		/* !< indicate that + switch was press for medium time >!*/
#define SW_FAST_DOWN		(uint8_t)5u		/* !< indicate that - switch was press for medium time >!*/
#define SW_VERY_FAST_UP		(uint8_t)6u		/* !< indicate that + switch was press for long time >!*/
#define SW_VERY_FAST_DOWN	(uint8_t)7u		/* !< indicate that - switch was press for long time >!*/
#define SW_LAMP				(uint8_t)8u		/* !< indicate that Lamp switch was press >!*/
#define SW_FAN				(uint8_t)9u		/* !< indicate that Fan switch was press >!*/
#define SW_TIMER			(uint8_t)10u	/* !< indicate that Time switch was press >!*/
#define SW_MENU				(uint8_t)11u	/* !< indicate that Time switch was press for long time and enter in hiddenMenu state >!*/

#define SW_FAST_CHANGE_10			10		/* !< indicate value change by switch execute >!*/
#define SW_FAST_CHANGE_60			60		/* !< indicate value change by switch execute >!*/
/*-------------------------------*/

#define DISPLSY_SEGMENT_NR		(uint8_t)3u  /* used to calculate actual segment nr */

#define CHANGE_MENU_SW			(uint8_t)1u
#define CHANGE_MENU_TIM			(uint8_t)2u

/*! \brief LED display conversion level */
#define CALIB_LCD_CONV_LEVEL	(uint8_t)0u
#define TEMP_LCD_CONV_LEVEL		(uint8_t)1u
#define TIME_LCD_CONV_LEVEL		(uint8_t)2u
#define ERROR_LCD_LEVEL			(uint8_t)3u

/*! \brief Temperature sensors errors */
#define SENS_ERROR1		(uint8_t)1u
#define SENS_ERROR2		(uint8_t)2u

/* -------------TYPE DEFINITIONS ---------------*/

/*! \struct to save the processed data from RS485 */
typedef struct {
	uint8_t MinuteEvent_bo;
	uint8_t Error1Sen_bo;
	uint8_t Error2Sen_bo;
	uint8_t Temp100_u8;
	uint8_t Temp10_u8;
	uint8_t Temp1_u8;
}inputDataRS_t;

/*! \struct to save the processed input data from RS485 and calculated temperature */
typedef struct {
	inputDataRS_t dataInRS;
	uint16_t countedTemperature_s16;
}processedDataInRS_t;

/*! \Structure to save sauna parameter which are written to EEPROM memory */
typedef struct {
	/* parameter saved in EEPROM memory */
	int16_t		WarmingTimeMax_s16;
	uint8_t		TemperatureHotMax_u8;
	uint8_t		TemperatureHotMin_u8;
	uint8_t		TimerFanSet_u8;
	uint8_t		HistTemp_u8;
	int8_t		Calibration_s8;
	uint8_t		TemperatureHot_u8;
}strSaunaSavedParameter_t;

/*! \ Structure to save sauna parameter */
typedef struct {
	/* parameter saved in EEPROM memory */
	strSaunaSavedParameter_t HiddenMenuParam;
	/* parameter not saved at this moment in EEPROM memory */
	int16_t	WarmingTime_s16;
	int16_t	DelayWarmingTime_s16;
	uint8_t	TemperatureHot_u8;
	int8_t	ActualTemperature_s8;		/* !< variable stored temperature readed from sensor and modified by calibration factor>!*/
	uint8_t TimerCurrentFanSet_u8;		/* !< variable used to store current fan work time for each power cycle >!*/
}strSaunaParam_t;

/*! \struct to save execute hidden menu */
typedef struct{
	uint8_t nextState_au8[5];		/* !< tab for store next states which will be execute in case of switch event or time event >!*/
	void (*callback)(uint8_t event_u8, strSaunaParam_t *Gv_WorkingParam, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);	/* !< pointer for function which will be called in case of call of this struct element >!*/
} menuItem_t;

/*! \brief Stores information about LED status */
typedef struct {
	uint8_t ProgLed_u8;
	uint8_t FanLed_u8;
	uint8_t TimerOnLed_u8;
	uint8_t TimerOffLed_u8;
	uint8_t TempLed_u8;
} strLedStatus_t;

/*! \brief Enumeration values used to convert digit and character to correspondent LED display value */
typedef enum{
	D0  = 0xC0u,
	D1	= 0xF9u,
	D2	= 0xA4u,
	D3	= 0xB0u,
	D4	= 0x99u,
	D5	= 0x92u,
	D6	= 0x82u,
	D7	= 0xF8u,
	D8	= 0x80u,
	D9	 = 0x90u,
	LEF	 = 0x8Eu,
	SPCJ = 0xFFu,
	Lo	 = 0xA3u,
	Ln   = 0xABu,
	Lf	 = 0x7Eu,
	LE	 = 0x86u,
	DSL1 = 0xF7u, 
	DSL2 = 0xB7u, 
	DSL3 = 0xB6u,
	MMIN = 0xBFu,
	MPOINT = 0x7Fu
}ledDisplayChar_t;

/*! \brief Enumeration values for main state machine */
typedef enum{
	SM_HIDE_MENU		= 0u,
	SM_OFF				= 1u,
	SM_IN				= 2u,
	SM_TEMP_SET			= 3u,
	SM_TIMER_SET		= 4u,
	SM_FURNANCE_ON		= 5u,
	SM_FURNANCE_DELAY	= 6u,
	SM_FAN_ON			= 7u,
	SM_ERROR			= 8u,
}stateMachine_t;

/*! \brief Enumeration values for lamp state machine */
typedef enum{
	LAMP_IDLE			= 0u,
	LAMP_OFF			= 1u,
	LAMP_ON1			= 2u,
	LAMP_ON1_EXEC		= 3u,
	LAMP_ON2			= 4u,
	LAMP_ON2_EXEC		= 5u,
	LAMP_AUTO_ON		= 6u,
	LAMP_AUTO_ON_EXEC	= 7u,
}stateLamp_t;

/*! \brief Enumeration values for fan state machine */
typedef enum{
	FAN_IDLE			= 0u,
	FAN_OFF				= 1u,
	FAN_ON1				= 2u,
	FAN_ON1_EXEC		= 3u,
	FAN_ON2				= 4u,
	FAN_ON2_EXEC		= 5u,
	FAN_AUTO_ON			= 6u,
	FAN_AUTO_ON_EXEC	= 7u,
}stateFan_t;

/*! \brief Enumeration values for furnace state machine */
typedef enum{
	FUR_IDLE			= 0u,
	FUR_OFF				= 1u,
	FUR_ON				= 2u,
	FUR_AUTO_ON			= 3u,
	FUR_AUTO_ON_EXEC	= 4u,
	FUR_BREAK			= 5u,
	FUR_BREAK_EXEC		= 6u,	
}stateFurnance_t;

/*!----------------------------------------------------------------------------
*
* \fn static void saveDefaultParameterToEeprom(strSaunaSavedParameter_t *structEepromParam)
* \brief Load default value to eeprom memory during first start of system.
* \pre None
* \post None
* \param [in, out] structEepromParam - EEPROM struct for sauna parameter
* \return None
*
*---------------------------------------------------------------------------*/
static void saveDefaultParameterToEeprom(strSaunaSavedParameter_t *structEepromParam);

/*!----------------------------------------------------------------------------
*
* \fn static void initEepromStruct(strSaunaParam_t *structWorkingValue, strSaunaSavedParameter_t* structEepromParam, uint8_t* eepromAddr_u8)
* \brief Load default value to eeprom memory during first start of system, and write it to working ram struct.
* \pre None
* \post None
* \param [in, out] structWorkingValue - struct with menu settings for sauna
* \param [in, out] structEepromParam - EEPROM struct for sauna parameter
* \param [in, out] eepromAddr_u8 - EEPROM flag indicating for load default value to eeprom memory in case first start of system
* \return None
*
*---------------------------------------------------------------------------*/
static void initEepromStruct(strSaunaParam_t *structWorkingValue, strSaunaSavedParameter_t* structEepromParam, uint8_t* eepromAddr_u8);

/*!----------------------------------------------------------------------------
*
* \fn static void fillWorkingStructDuringStart(strSaunaParam_t *structWorkingValue, strSaunaSavedParameter_t *structEepromParam)
* \brief Load default and stored value for sauna parameter in case of start system.
* \pre None
* \post None
* \param [in, out]  structWorkingValue - struct with menu settings for sauna
* \param [in, out]  structEepromParam - EEPROM struct for sauna parameter
* \return None
*
*---------------------------------------------------------------------------*/
static void fillWorkingStructDuringStart(strSaunaParam_t *structWorkingValue, strSaunaSavedParameter_t *structEepromParam);

/*!----------------------------------------------------------------------------
*
* \fn static void SaveToEEPROM(strSaunaParam_t* savedValue)
* \brief Load sauna parameter to eeprom memory.
* \pre None
* \post None
* \param [in, out] savedValue - struct with menu parameters for sauna
* \return None
*
*---------------------------------------------------------------------------*/
static void SaveToEEPROM(strSaunaParam_t* savedValue);

/*!----------------------------------------------------------------------------
*
* \fn static void fillWorkingStructDuringSwitchingOn(strSaunaParam_t *structWorkingValue)
* \brief Load default value for some of the sauna parameter in case of start system.
* \pre None
* \post None
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \return None
*
*---------------------------------------------------------------------------*/
static void fillWorkingStructDuringSwitchingOn(strSaunaParam_t *structWorkingValue);

/*!----------------------------------------------------------------------------
*
* \fn static void BackToOffState(uint8_t event,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
* \brief First function in array of structures for hidden menu, used to back from hidden menu to off state.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in, out] displayOutData_pa - array with data for Led display
* \param [in]      menuLevel_u8 - hide menu array of structures level
* \return None
*
*---------------------------------------------------------------------------*/
static void BackToOffState(uint8_t event,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);

/*!----------------------------------------------------------------------------
*
* \fn static void setHistTempFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue_pstr, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
* \brief Function used to set histerese for furnace heat.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in, out] displayOutData_pa - array with data for Led display
* \param [in]      menuLevel_u8 - hide menu array of structures level
* \return None
*
*---------------------------------------------------------------------------*/	
static void setHistTempFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue_pstr, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);

/*!----------------------------------------------------------------------------
*
* \fn static void setCalibrationFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
* \brief Function used to set calibration parameter for temperature sensor.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in, out] displayOutData_pa - array with data for Led display
* \param [in]      menuLevel_u8 - hide menu array of structures level
* \return None
*
*---------------------------------------------------------------------------*/	
static void setCalibrationFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);

/*!----------------------------------------------------------------------------
*
* \fn static void setMaxWorkingTimerFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
* \brief Function used to set max temperature bound for furnace.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in, out] displayOutData_pa - array with data for Led display
* \param [in]      menuLevel_u8 - hide menu array of structures level
* \return None
*
*---------------------------------------------------------------------------*/
static void setMaxWorkingTimerFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);

/*!----------------------------------------------------------------------------
*
* \fn static void setTempMaxFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
* \brief Function used to set min temperature bound for furnace.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in, out] displayOutData_pa - array with data for Led display
* \param [in]      menuLevel_u8 - hide menu array of structures level
* \return None
*
*---------------------------------------------------------------------------*/
static void setTempMaxFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);

/*!----------------------------------------------------------------------------
*
* \fn static void setTempMinFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
* \brief Function used to set max fan working time in hidden menu.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in, out] displayOutData_pa - array with data for Led display
* \param [in]      menuLevel_u8 - hide menu array of structures level
* \return None
*
*---------------------------------------------------------------------------*/	
static void setTempMinFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);

/*!----------------------------------------------------------------------------
*
* \fn static void setTimeFanFunction(uint8_t event_u8,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8)
* \brief Function used to set display hidden menu level based on array of structures level.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in, out] displayOutData_pa - array with data for Led display
* \param [in]      menuLevel_u8 - hide menu array of structures level
* \return None
*
*---------------------------------------------------------------------------*/
static void setTimeFanFunction(uint8_t event_u8,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);


/*!----------------------------------------------------------------------------
*
* \fn static void setFurnanceWorkTime(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa)
* \brief Function used to set working time for furnace on state.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in, out] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void setFurnanceWorkTime(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa);


/*!----------------------------------------------------------------------------
*
* \fn static void setFurnanceDelay(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, stateMachine_t enableForSetState_e)
* \brief Function used to set delay for furnace switching on.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in]      processedRSInData - struct with main data read by RS485
* \param [in, out] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/	
static void setFurnanceDelay(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, stateMachine_t enableForSetState_e);


/*!----------------------------------------------------------------------------
*
* \fn static void TempSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa)
* \brief Function used to set hot temperature of furnace.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in, out] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/	
static void TempSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void OffStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t* displayOutData_pa)
* \brief Function to execute behavior of sauna panel in off state.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in, out] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void OffStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t* displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void ClearDispalyData(uint8_t* displayOutData_pa)
* \brief Function to execute Led display clear.
* \pre None
* \post None
* \param [in, out]      displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void ClearDispalyData(uint8_t* displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void InStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t* displayOutData_pa)
* \brief Function to execute behavior of sauna after switching on.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in]      processedRSInData - struct with main data read by RS485
* \param [in, out] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void InStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t* displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void HideMenuStateExecute(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t* displayOutData_pa)
* \brief Function for execute hidden menu state.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in]      structWorkingValue - struct with menu settings for sauna
* \param [in, out] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void HideMenuStateExecute(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t* displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void FurnanceOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
* \brief Function to execute behavior of sauna in case of furnace normal working phase.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in]      processedRSInData - struct with main data read by RS485
* \param [in, out] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void FurnanceOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void addAnimationToLedDisplay(uint8_t *displayOutData_pa)
* \brief Function used to add animation to Led display during furnace switch on state.
* \pre None
* \post None
* \param [in] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void addAnimationToLedDisplay(uint8_t *displayOutData_pa);


/*!----------------------------------------------------------------------------
*
* \fn static void ClearAllTimers()
* \brief Function used to clear some of the global timers used in file.
* \pre None
* \post None
* \param None
* \return None
*
*---------------------------------------------------------------------------*/
static void ClearAllTimers();

/*!----------------------------------------------------------------------------
*
* \fn static void TimeCounter()
* \brief Function used to manage all global timers used in file.
* \pre None
* \post None
* \param None
* \return None
*
*---------------------------------------------------------------------------*/
static void TimeCounter();

/*!----------------------------------------------------------------------------
*
* \fn static void FurnanceDelayStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
* \brief Function to execute behavior of sauna in case of delay of furnace switching on phase
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in]      processedRSInData - struct with main data read by RS485
* \param [in, out] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void FurnanceDelayStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void TimerSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa)
* \brief Function to execute time settings for current power cycle of sauna.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in, out] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void TimerSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void FanOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue,processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
* \brief Function to execute behavior of sauna in case of fan phase.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in]      processedRSInData - struct with main data read by RS485
* \param [in, out] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void FanOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue,processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void ErrorStateExecute(uint8_t event_u8, processedDataInRS_t *processedRSInData, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa)
* \brief Function to execute behavior of sauna in case temperature sensor error detecting.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in]      processedRSInData - struct with main data read by RS485
* \param [in, out] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void ErrorStateExecute(uint8_t event_u8, processedDataInRS_t *processedRSInData, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void SwEventChoose (volatile uint8_t *switchCounter_bo, uint8_t *swEvent_u8 )
* \brief Function to check event related with switch manipulate.
* \pre None
* \post None
* \param [in, out]  switchCounter_bo - counter used to measure debounce time for switch
* \param [in]       swEvent_u8 - flag indicate switching on particular switch
* \return None
*
*---------------------------------------------------------------------------*/
static void SwEventChoose (volatile uint8_t *switchCounter_bo, uint8_t *swEvent_u8 );

/*!----------------------------------------------------------------------------
*
* \fn static void StateMachine(uint8_t event_u8, strSaunaParam_t * structWorkingValue,  processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa)
* \brief Main state machine for sauna.
* \pre None
* \post None
* \param [in]      event_u8 - value indicate for switch event
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in]      processedRSInData - struct with main data read by RS485
* \param [in, out] displayOutData_pa - array with data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void StateMachine(uint8_t event_u8, strSaunaParam_t *structWorkingValue,  processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void FurnanceStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData)
* \brief State machine for manage switching on and off the Furnace, depending on sauna state.
* \pre None
* \post None
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in]      processedRSInData - struct with main data read by RS485
* \return None
*
*---------------------------------------------------------------------------*/
static void FurnanceStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData);

/*!----------------------------------------------------------------------------
*
* \fn static void FanStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData)
* \brief State machine for manage switching on and off the Fan, depending on sauna state.
* \pre None
* \post None
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in]      processedRSInData - struct with main data read by RS485
* \return None
*
*---------------------------------------------------------------------------*/
static void FanStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData);

/*!----------------------------------------------------------------------------
*
* \fn static void LampStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData)
* \brief State machine for manage switching on and off the Lamp, depending on sauna state.
* \pre None
* \post None
* \param [in, out] structWorkingValue - struct with main menu settings for sauna
* \param [in]      processedRSInData - struct with main data read by RS485
* \return None
*
*---------------------------------------------------------------------------*/
static void LampStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData);

/*!----------------------------------------------------------------------------
*
* \fn static void LedWorkStatus()
* \brief Function used to manage the LED status (switch on, switch off , blink LED diode)
* \pre None
* \post None
* \param None
* \return None
*
*---------------------------------------------------------------------------*/
static void LedWorkStatus();

/*!----------------------------------------------------------------------------
*
* \fn static void fillTabByLcdDataMenu(uint8_t ledDisplayLevel_u8, uint8_t *displayOutData_pa)
* \brief Function used to set hidden menu level on Led display.
* \pre None
* \post None
* \param [in]		 ledDisplayLevel_u8 - value indicate menu level
* \param [in, out]   displayOutData_pa - array with data for display
* \return None
*
*---------------------------------------------------------------------------*/
static void fillTabByLcdDataMenu(uint8_t ledDisplayLevel_u8, uint8_t *displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void fillLcdDataTab(uint8_t conversionLevel_u8, int16_t valueToConvert_u16, uint8_t *displayOutData_pa)
* \brief Function used to convert data like temperature, time to corresponding display data.
* \pre None
* \post None
* \param [in]		 conversionLevel_u8 - value indicate conversion type (time, temperature, calibration)
* \param [in]		 valueToConvert_u16 - value to convert
* \param [in, out]   displayOutData_pa - array with data for display
* \return None
*
*---------------------------------------------------------------------------*/
static void fillLcdDataTab(uint8_t conversionLevel_u8, int16_t valueToConvert_u16, uint8_t *displayOutData_pa);

/*!----------------------------------------------------------------------------
*
* \fn static void ChangeMenu(uint8_t event_u8, uint8_t *currentMenuState_u8, menuItem_t *tabHideMenu, strSaunaParam_t *structure_pstr , uint8_t * tab_pu8 )
* \brief Function used to change Hide menu level.
* \pre None
* \post None
* \param [in]      event_u8 - event related with switch manipulation
* \param [in]      currentMenuState_u8 - Current hide men state
* \param [in]      tabHideMenu - tab with menu levels and function
* \param [in]      structure_pstr - structure with sauna menu parameter
* \param [in, out] tab_pu8 - data for Led display
* \return None
*
*---------------------------------------------------------------------------*/
static void ChangeMenu(uint8_t event_u8, uint8_t *currentMenuState_u8, menuItem_t *tabHideMenu, strSaunaParam_t *structure_pstr , uint8_t *tab_pu8 );

/*!----------------------------------------------------------------------------
*
* \fn static void TimeConv(uint16_t time_u16, uint8_t digTime[4])
* \brief Function used to change input time data for its digit representation.
* \pre None
* \post None
* \param [in]       time_u16 - value for time to convert
* \param [in, out]  digTime[4] - array with digit value corresponding to time value
* \return None
*
*---------------------------------------------------------------------------*/
static void TimeConv(uint16_t time_u16, uint8_t digTime[]);

/*!----------------------------------------------------------------------------
*
* \fn static void DigConvToDsp(uint8_t tabIn_au8[], uint8_t tabOutDsp_au8[])
* \brief Function used to change input data for their Led display representation.
* \pre None
* \post None
* \param [in]       tabIn_au8[4] - array with digit corresponding to represented data
* \param [in, out]  tabOutDsp_au8[4] - value corresponding LED representation of input digit
* \return None
*
*---------------------------------------------------------------------------*/
static void DigConvToDsp(uint8_t tabIn_au8[], uint8_t tabOutDsp_au8[]);

/*!----------------------------------------------------------------------------
*
* \fn static void TimeConvToDsp(uint8_t tabIn_au8[], uint8_t tabOutDsp_au8[])
* \brief Function used to change input data for their Led display representation.
* \pre None
* \post None
* \param [in]      tabIn_au8[4] - array with digit corresponding to time data
* \param [in, out] tabOutDsp_au8[4] - value corresponding LED representation of input digit
* \return None
*
*---------------------------------------------------------------------------*/
static void TimeConvToDsp(uint8_t tabIn_au8[], uint8_t tabOutDsp_au8[]);

/*!----------------------------------------------------------------------------
*
* \fn static void DigTempConvToDsp(uint8_t tabInDigit_au8[], uint8_t tabOutLcdRepresentation_au8[])
* \brief Function used to change input data for their Led display representation.
* \pre None
* \post None
* \param [in]  tabInDigit_au8[4] - array with data corresponding to hundred, decimal, and unit parts of temperature
* \param [in, out]  tabOutLcdRepresentation_au8[4] - value corresponding LED representation of input digit
* \return None
*
*---------------------------------------------------------------------------*/
static void DigTempConvToDsp(uint8_t tabInDigit_au8[], uint8_t tabOutLcdRepresentation_au8[]);

/*!----------------------------------------------------------------------------
*
* \fn static void TempConvToDigit (uint16_t temperature_u16, uint8_t digTemp_au8[])
* \brief Function used to decomposite temperature value for specified part corresponding to hundred, decimal and unit parts of this value.
* \pre None
* \post None
* \param [in]  temperature_u16 - value of temperature to decomposite
* \param [in, out]  digTemp_au8[4] - array for corresponding parts of decomposed temperature
* \return None
*
*---------------------------------------------------------------------------*/
static void TempConvToDigit (uint16_t temperature_u16, uint8_t digTemp_au8[]);

/*!----------------------------------------------------------------------------
*
* \fn static int16_t TabToTempConv(uint8_t firstDig100_u8, uint8_t secDigit10_u8, uint8_t  thrDigit1_u8)
* \brief Function used to recalculate temperature from specified part corresponding to hundred, decimal and unit parts to one value.
* \pre None
* \post None
* \param [in]  firstDig100_u8 - value corresponding to hundred parts
* \param [in]  secDigit10_u8 - value corresponding to decimal parts
* \param [in]  thrDigit1_u8 - value corresponding to unit parts
* \return	   temperature_s16 - calculated temperature value
*
*---------------------------------------------------------------------------*/
static int16_t TabToTempConv(uint8_t firstDig100_u8, uint8_t secDigit10_u8, uint8_t  thrDigit1_u8);

/*!----------------------------------------------------------------------------
*
* \fn static void SensorTemperatureCalibration(strSaunaParam_t *structWorkingValue, int16_t dataFromSensor_s16)
* \brief Function used to calibrate temperature received from temperature sensor according to calibration factory.
* \pre None
* \post None
* \param [in]  dataFromSensor_s16 - temperature read from sensor
* \param [in, out]  structWorkingValue - struct with main menu param for sauna 
* \return None
*
*---------------------------------------------------------------------------*/
static void SensorTemperatureCalibration(strSaunaParam_t *structWorkingValue, int16_t dataFromSensor_s16);

/*!----------------------------------------------------------------------------
*
* \fn static uint8_t CheckData485Presence()
* \brief Function used to check presence of data in input buffer, return TRUE if any data are present in buffer
* \pre None
* \post None
* \param None
* \return retVal_bo - TRUE if data are present in buffer, FALSE if no data in buffer
*
*---------------------------------------------------------------------------*/
static uint8_t CheckData485Presence();

/*!----------------------------------------------------------------------------
*
* \fn static void ProcessInputRSData(processedDataInRS_t *processedRSData)
* \brief Function used to fill input Structure with proper data, depending on data received from RS485 line.
* \pre None
* \post None
* \param [in] processedRSData - pointer to struct with RS data processed by program
* \return None
*
*---------------------------------------------------------------------------*/
static void ProcessInputRSData(processedDataInRS_t *processedRSData);

/*!----------------------------------------------------------------------------
*
* \fn static void TimerMinReset()
* \brief Function used to reset Minute counter in Base Module; Reset of this counter is neceserry when we go back from
*		 menu options, or temperature options to state where we count furnace time or delay time.
* \pre None
* \post None
* \param None
* \return None
*
*---------------------------------------------------------------------------*/
static void TimerMinReset();

/*!----------------------------------------------------------------------------
*
* \fn static void OnOffRSinfoSend(uint8_t statusSend_bo)
* \brief Function used to send information by RS485 about switch on and off the controller
* \pre None
* \post None
* \param [in] statusSend_bo - status of operation which we want to set
* \return None
*
*---------------------------------------------------------------------------*/
static void OnOffRSinfoSend(uint8_t statusSend_bo);


/*!----------------------------------------------------------------------------
*
* \fn static void SendDisplayPresence()
* \brief Send information to Main Module every 300ms to confirm presence of the Panel Module,
* \		 in case of panel absence Main Module should be switched off.
* \pre None
* \post None
* \param [in, out] tabOutDsp_au8 - data for LED display
* \param [in]      processedRSData - struct with data read from Base module by RS485
* \param [in]      structWorkingValue - struct with main menu param for sauna
* \return None
*
*---------------------------------------------------------------------------*/
static void SendDisplayPresence();

/*!----------------------------------------------------------------------------
*
* \fn static void OutputExecute(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSData, uint8_t tabOutDsp_au8[])
* \brief Function execute every cycle in main, used to set output informations.
* \pre None
* \post None
* \param [in, out] tabOutDsp_au8 - data for LED display
* \param [in]      processedRSData - struct with data read from Base module by RS485
* \param [in]      structWorkingValue - struct with main menu param for sauna
* \return None
*
*---------------------------------------------------------------------------*/
static void OutputExecute(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSData, uint8_t tabOutDsp_au8[]);

/*!----------------------------------------------------------------------------
*
* \fn SendDataByRs()
* \brief Function used to send particular data from array by RS485 line.
* \pre None
* \post None
* \param None
* \return None
*
*---------------------------------------------------------------------------*/
static void SendDataByRs();

/*!----------------------------------------------------------------------------
*
* \fn static void SetRegisterUC(void)
* \brief Function used to set all register for proper work of microcontroller.
* \pre None
* \post None
* \param None
* \return None
*
*---------------------------------------------------------------------------*/
static void SetRegisterUC(void);

/*!----------------------------------------------------------------------------
*
* \fn static void InitDataOnEntry(strSaunaParam_t *structWorkingValue)
* \brief Function used fill struct with data from EEPROM also set global interrupt and watchdog.
* \pre None
* \post None
* \param [in, out] structWorkingValue - pointer to structure with sauna parameters
* \return None
*
*---------------------------------------------------------------------------*/
static void InitDataOnEntry(strSaunaParam_t *structWorkingValue);

#endif