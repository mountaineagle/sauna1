#ifndef _MAIN_PRIV_H_
#define _MAIN_PRIV_H_

/*----- macro definition --------- */
#define TRUE 0xFFu
#define FALSE 0x00u

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


/* value for led management*/
#define LED_OFF 0
#define LED_ON 1
#define LED_BLINK 2

/* value for in state */
#define TIME_TO_OFF 900u  /* !< sec to off controller from in state */

/* value LCD */
#define POINT_LCD_MARK 0x7Fu

/* value for event_u8 */
#define SW_OFF_ON 0u
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
#define TIMER_EVENT_1S 14u
#define TIMER_EVENT_6S 15u
#define TIMER_EVENT_8S 16u

/*-------------------------------*/

#define DISPLSY_SEGMENT_NR 4u  /* used to calculate actual segment nr */

#define CHANGE_MENU_SW 1u
#define CHANGE_MENU_TIM 2u

// sposoby przeliczania danych do wyswietlacza
#define TEMP_LCD_CONV_LEVEL 0u
#define TIME_LCD_CONV_LEVEL 1u
#define CALIB_LCD_CONV_LEVEL 2u


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
	SM_HIDE_MENU	= 0u,
	SM_OFF			= 1u,
	SM_IN			= 2u,
	SM_FAN_SET		= 3u,
	SM_LAMP_SET		= 4u,
	SM_TEMP_SET		= 5u,
	SM_TIMER_SET	= 6u,
	SM_FURNANCE_ON  = 7u,
	SM_FAN_ON       = 8u,
}stateMachine_t;



#endif