#ifndef _MAIN_PRIV_H_
#define _MAIN_PRIV_H_

/*
do zrobienia:
globalny timer do obslugi wszystkich pozostalych timerow dodac w przerwaniu od timera
przeniesc gdzieœ funkcje do odliczania kalibracji temperatury

*/

/*----- macro definition --------- */
#define TRUE	(uint8_t)0xFFu
#define FALSE	(uint8_t)0x00u
#define INVALID (uint8_t)0xFFu


/*RS data code */
#define RS_PANEL_PRES	(uint8_t)0x01u
#define RS_CTRL_ON		(uint8_t)0x02u
#define RS_CTRL_OFF		(uint8_t)0x03u
#define RS_LAMP_ON		(uint8_t)0x06u
#define RS_LAMP_OFF		(uint8_t)0x07u
#define RS_FUR_ON		(uint8_t)0x09u
#define RS_FUR_OFF		(uint8_t)0x0Au
#define RS_FAN_ON		(uint8_t)0x0Bu
#define RS_FAN_OFF		(uint8_t)0x0Cu
#define RS_MIN_RES		(uint8_t)0x0Du

/* maximum, minimum and default values for hidden menu parameters */

#define CALIBRATION_MAX (int8_t)10
#define CALIBRATION_MIN (int8_t)-10
#define CALIBRATION_DEF (int8_t)0
#define WARMING_TIME_MAX_MIN 10u
#define WARMING_TIME_MAX_MAX 360u
#define WARMING_TIME_MAX_DEF 240u
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

/* default values for settings */
/*#define WARMING_TIME_DEF 240u this should be set for actual value of WARMING_TIME_MAX parameter */ 
#define TEMPERATURE_HOT_DEF 50u
#define DELAY_WARMING_TIME_MIN (int16_t)0u
#define DELAY_WARMING_TIME_MAX (int16_t)1200u
#define DELAY_WARMING_TIME_DEF (int16_t)0u

/* value for time event */
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

/* value for led management*/
#define LED_OFF		(uint8_t)0u
#define LED_ON		(uint8_t)1u
#define LED_BLINK	(uint8_t)2u

/* value for in state */
#define TIME_TO_OFF 900u  /* !< sec to off controller from in state */

/* value LCD */
#define POINT_LCD_MARK 0x7Fu

/* value for event_u8 */
#define SW_IDLE 			(uint8_t)0u
#define SW_OFF_ON			(uint8_t)1u
#define SW_UP				(uint8_t)2u
#define SW_DOWN				(uint8_t)3u
#define SW_FAST_UP			(uint8_t)4u
#define SW_FAST_DOWN		(uint8_t)5u
#define SW_VERY_FAST_UP		(uint8_t)6u
#define SW_VERY_FAST_DOWN	(uint8_t)7u
#define SW_LAMP				(uint8_t)8u
#define SW_FAN				(uint8_t)9u
#define SW_TIMER			(uint8_t)10u
#define SW_MENU				(uint8_t)11u

#define SW_FAST_CHANGE_10			10u		/* !< indicate value change by switch execute >!*/
#define SW_FAST_CHANGE_60			60u		/* !< indicate value change by switch execute >!*/
/*-------------------------------*/

#define DISPLSY_SEGMENT_NR		(uint8_t)3u  /* used to calculate actual segment nr */

#define CHANGE_MENU_SW			(uint8_t)1u
#define CHANGE_MENU_TIM			(uint8_t)2u

/* LED display conversion level */
#define CALIB_LCD_CONV_LEVEL	(uint8_t)0u
#define TEMP_LCD_CONV_LEVEL		(uint8_t)1u
#define TIME_LCD_CONV_LEVEL		(uint8_t)2u
#define ERROR_LCD_LEVEL			(uint8_t)3u

/* temperature sensors errors */
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
	uint16_t	WarmingTimeMax_u16;
	uint8_t		TemperatureHotMax_u8;
	uint8_t		TemperatureHotMin_u8;
	uint8_t		TimerFanSet_u8;
	uint8_t		HistTemp_u8;
	int8_t		Calibration_s8;
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

/*! \brief Stores  information about LED status */
typedef struct {
	uint8_t ProgLed_u8;
	uint8_t FanLed_u8;
	uint8_t TimerOnLed_u8;
	uint8_t TimerOffLed_u8;
	uint8_t TempLed_u8;
} strLedStatus_t;

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

/*fan states */
/*#define FAN_IDLE	0u
#define FAN_OFF		1u
#define FAN_ON		2u
#define FAN_ON_EXEC	3u
*/

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
* \fn void Acm_SetCodingValuesForMessage_CodingAebPeds_0x5A(Acm_CodingAebPedsType* Params_p)
* \brief This function performs the mapping of BMW-CAF values to ME-Coding values. Also recalculations were done inside.
* \pre None
* \post None
* \param Params_p pointer to send buffer
* \return None
*
*---------------------------------------------------------------------------*/


static void saveDefaultParameterToEeprom(strSaunaSavedParameter_t *structEepromParam);

static void initEepromStruct(strSaunaParam_t *structWorkingValue, strSaunaSavedParameter_t* structEepromParam, uint8_t* eepromAddr_u8);

static void fillWorkingStructDuringStart(strSaunaParam_t *structWorkingValue, strSaunaSavedParameter_t *structEepromParam);

static void SaveToEEPROM(strSaunaParam_t* savedValue);

static void fillWorkingStructDuringSwitchingOn(strSaunaParam_t *structWorkingValue);

static void BackToOffState(uint8_t event,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);
	
static void setHistTempFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue_pstr, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);
	
static void setCalibrationFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);
	
static void setMaxWorkingTimerFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);

static void setTempMaxFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);
	
static void setTempMinFunction(uint8_t event_u8,strSaunaParam_t* structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);

static void setTimeFanFunction(uint8_t event_u8,strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, uint8_t menuLevel_u8);
	
static void setFurnanceWorkTime(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa);
	
static void setFurnanceDelay(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa, stateMachine_t enableForSetState_e);
	
static void TempSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa);

static void OffStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t* displayOutData_pa);

static void ClearDispalyData(uint8_t* displayOutData_pa);

static void InStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t* displayOutData_pa);

static void HideMenuStateExecute(uint8_t event_u8, strSaunaParam_t *structWorkingValue, uint8_t* displayOutData_pa);

static void FurnanceOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa);

static void addAnimationToLedDisplay(uint8_t *displayOutData_pa);

static void ClearAllTimers();
/*umieœciæ funkcjê na koñcu programu programu */
static void TimeCounter();

static void FurnanceDelayStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa);

static void TimerSetStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue, uint8_t *displayOutData_pa);

static void FanOnStateExecute(uint8_t event_u8, strSaunaParam_t * structWorkingValue,processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa);

static void ErrorStateExecute(uint8_t event_u8, processedDataInRS_t *processedRSInData, strSaunaParam_t *structWorkingValue, uint8_t *displayOutData_pa);

static void SwEventChoose (volatile uint8_t *switchCounter_bo, uint8_t *swEvent_u8 );

static void StateMachine(uint8_t event_u8, strSaunaParam_t * structWorkingValue,  processedDataInRS_t *processedRSInData, uint8_t *displayOutData_pa);

static void FurnanceStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData);

static void FanStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData);

static void LampStateMachine(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSInData);

static void LedWorkStatus();

static void fillTabByLcdDataMenu(uint8_t ledDisplayLevel_u8, uint8_t *displayOutData_pa);
		
static void fillLcdDataTab(uint8_t conversionLevel_u8, int16_t valueToConvert_u16, uint8_t *displayOutData_pa);

static void ChangeMenu(uint8_t event_u8, uint8_t *currentMenuState_u8, menuItem_t *tabHideMenu, strSaunaParam_t *structure_pstr , uint8_t * tab_pu8 );

static void TimeConv(uint16_t time_u16, uint8_t digTime[]);

static void DigConvToDsp(uint8_t tabIn_au8[], uint8_t tabOutDsp_au8[]);

static void TimeConvToDsp(uint8_t tabIn_au8[], uint8_t tabOutDsp_au8[]);

static void DigTempConvToDsp(uint8_t tabInDigit_au8[], uint8_t tabOutLcdRepresentation_au8[]);

static void TempConvToDigit (uint16_t temperature_u16, uint8_t digTemp_au8[]);

static int16_t TabToTempConv(uint8_t firstDig100_u8, uint8_t secDigit10_u8, uint8_t  thrDigit1_u8);

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
static void SensorTemperatureCalibration(strSaunaParam_t *structWorkingValue, int16_t dataFromSensor_s16);

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
static uint8_t CheckData485Presence();

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
static void ProcessInputRSData(processedDataInRS_t *processedRSData);

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
static void TimerMinReset();

static void OnOffRSinfoSend(uint8_t statusSend_bo);

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
static void SendDisplayPresence();

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
static void OutputExecute(strSaunaParam_t *structWorkingValue, processedDataInRS_t *processedRSData, uint8_t tabOutDsp_au8[]);

static void SendDataByRs();


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
static void SetRegisterUC(void);

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
static void InitDataOnEntry(strSaunaParam_t *structWorkingValue);

#endif