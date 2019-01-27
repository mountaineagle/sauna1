#ifndef _MAIN_PRIV_H_
#define _MAIN_PRIV_H_

/* global variable */

volatile uint8_t Gv_flagInterrupt50ms_bo;	/* !< variable set in 20ms timer interrupt to indicate 20ms time gap >!*/

uint8_t Gv_flag50ms_bo;			/* !< flag set every 20ms used to increment static timer in function >!*/

uint8_t Gv_Timer8s_u8 = 0u;		/* !< timer used to count 8s period >!*/
uint8_t Gv_Timer6s_u8 = 0u;		/* !< timer used to count 6s period >!*/
uint8_t Gv_Timer3s_u8 = 0u;		/* !< timer used to count 3s period >!*/
uint8_t Gv_Timer2s_u8 = 0u;		/* !< timer used to count 2s period >!*/
uint8_t Gv_Timer1s_u8 = 0u;		/* !< timer used to count 1s period >!*/

uint8_t Gv_ExtTimer1min_bo;		/* !< fag used to indicate 1min period >!*/

int8_t Gv_ActualTemperature_u8;	/* !< variable stored temperature readed from sensor >!*/

stateLamp_t Gv_LampState_e;
stateFan_t Gv_FanState_e;
stateFurnance_t Gv_FurState_e;
/* global variable - finish block*/

/*----- macro definition --------- */
#define TRUE 0xFFu
#define FALSE 0x00u


/*RS data code */
#define RS_LAMP_ON		0x06u
#define RS_LAMP_OFF		0x07u
#define RS_FAN_ON		0x06u
#define RS_FAN_OFF		0x07u
#define RS_FUR_ON		0x06u
#define RS_FUR_OFF		0x07u


#define TIM_MENU_CHANGE 2000u
#define CALIBRATION_MAX 10
#define CALIBRATION_MIN -10
#define WARMING_TIME_MIN 10u
#define WARMING_TIME_MAX 360u
#define TEMPERATURE_HOTMAX_MAX 110u
#define TEMPERATURE_HOTMAX_MIN 50u
#define TEMPERATURE_HOTMIN_MAX 50u
#define TEMPERATURE_HOTMIN_MIN 30u
#define HIST_TEMP_MAX 5u
#define HIST_TEMP_MIN 2u
#define TIMER_FAN_MAX 60u
#define TIMER_FAN_MIN 5u
#define DELAY_WARMING_TIME_MIN 0u
#define DELAY_WARMING_TIME_MAX 7200u

/* value for time event */
#define TIMER_EVENT_1S 20u	/* !< indicate 1s period >!*/
#define TIMER_EVENT_2S 40u  /* !< indicate 2s period >!*/
#define TIMER_EVENT_3S 60u	/* !< indicate 3s period >!*/
#define TIMER_EVENT_4S 80u	/* !< indicate 4s period >!*/
#define TIMER_EVENT_5S 100u /* !< indicate 5s period >!*/
#define TIMER_EVENT_6S 120u	/* !< indicate 6s period >!*/
#define TIMER_EVENT_8S 160u	/* !< indicate 8s period >!*/

/*fan states */
#define FAN_IDLE		0u
#define FAN_OFF			1u
#define FAN_ON			2u
#define FAN_ON_EXEC		3u

/* value for led management*/
#define LED_OFF 0
#define LED_ON 1
#define LED_BLINK 2

/* value for in state */
#define TIME_TO_OFF 900u  /* !< sec to off controller from in state */

/* value LCD */
#define POINT_LCD_MARK 0x7Fu

/* value for event_u8 */
#define SW_OFF_ON 1u
#define SW_UP 3u
#define SW_DOWN 4u
#define SW_FAST_UP 5u
#define SW_FAST_DOWN 6u
#define SW_VERY_FAST_UP 7u
#define SW_VERY_FAST_DOWN 8u
#define SW_LAMP 9u
#define SW_FAN 10u
#define SW_TIMER 11u
#define SW_MENU 13u
/*-------------------------------*/

#define DISPLSY_SEGMENT_NR 4u  /* used to calculate actual segment nr */

#define CHANGE_MENU_SW 1u
#define CHANGE_MENU_TIM 2u

/* LED display conversion level */
#define TEMP_LCD_CONV_LEVEL 0u
#define TIME_LCD_CONV_LEVEL 1u
#define CALIB_LCD_CONV_LEVEL 2u

/* hidden menu definition */
#define MAXMEN 45u
#define MINMEN 40u
#define MENU_L0_WARM_TIME 40u
#define MENU_L0_T_MAX 41u
#define MENU_L0_T_MIN 42u
#define MENU_L0_FAN_TIME 43u
#define MENU_L0_T_HIST 44u
#define MENU_L0_CORR 45u
#define MENU_L0_DEFAULT 46u
#define MENU_L0_7 47u
#define MENU_L1_WARM_TIME 400u
#define MENU_L1_T_MAX 410u
#define MENU_L1_T_MIN 420u
#define MENU_L1_FAN_TIME 430u
#define MENU_L1_T_HIST 440u
#define MENU_L1_CALIB 450u
#define MENU_L1_DEFAULT_ON 460u
#define MENU_L1_DEFAULT_OFF 460u
#define MENU_L1_7 470u

/* -------------TYPE DEFINITIONS ---------------*/

typedef struct {
	uint8_t		histTemp_u8;
	uint8_t		timerFanSet_u8;
	uint8_t		temperatureHotMax_u8;
	uint8_t		temperatureHotMin_u8;
	uint16_t	warmingTime_u16;
	uint16_t	delayWarmingTime_u16;
	uint8_t		temperatureHot_u8;
	int8_t		calibration_s8;
	uint16_t	warmingTimeMax_u16;
}strSaunaParam_t;

typedef struct {
	uint8_t ProgLed_u8;
	uint8_t FanLed_u8;
	uint8_t TimerOnLed_u8;
	uint8_t TimerOffLed_u8;
	uint8_t TempLed_u8;
} strLedStatus_t;

typedef enum{
	SM_HIDE_MENU		= 0u,
	SM_OFF				= 1u,
	SM_IN				= 2u,
	SM_FAN_SET			= 3u,
	SM_LAMP_SET			= 4u,
	SM_TEMP_SET			= 5u,
	SM_TIMER_SET		= 6u,
	SM_FURNANCE_ON		= 7u,
	SM_FURNANCE_DELAY	= 8u,
	SM_FAN_ON			= 9u,
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

typedef enum{
	FUR_IDLE			= 0u,
	FUR_OFF				= 1u,
	FUR_AUTO_ON			= 2u,
	FUR_AUTO_ON_EXEC	= 3u,
}stateFurnance_t;


#endif