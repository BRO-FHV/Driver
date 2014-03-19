/*
 * Driver: dr_timer.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 16.03.2014
 * Description: 
 * TODO
 */

#ifndef DR_TIMER_H_
#define DR_TIMER_H_

typedef enum { Timer_TIMER0 = 0, Timer_TIMER1MS, Timer_TIMER2, Timer_TIMER3, Timer_TIMER4, Timer_TIMER5, Timer_TIMER6, Timer_TIMER7 } Timer;
typedef enum { IrqMode_MATCH = 0, IrqMode_OVERFLOW, IrqMode_CAPTURE, IrqMode_ALL, IrqMode_NONE } IrqMode;
typedef enum { IrqWakeen_MAT_WUP_ENA = 0, IrqWakeen_OVF_WUP_ENA, IrqWakeen_TCAR_WUP_ENA, IrqWakeen_ALL, IrqWakeen_NONE } IrqWakeen;


typedef void (*InterruptRoutine)(void);

/**
 * \brief enable a timer
 *
 * \param timer
 *
 * \return 0 on success, -1 on failure
 */
int TimerEnable(Timer timer);

/**
 * \brief disable a timer
 *
 * \param timer
 *
 * \return 0 on success, -1 on failure
 */
int TimerDisable(Timer timer);

/**
 * \brief reset a timer (counting state)
 *
 * \param timer
 *
 * \return 0 on success, -1 on failure
 */
int TimerReset(Timer timer);

/**
 * \brief configure the compare mode and the auto reaload mode. Timermust be disabled
 *
 * \param timer
 * \param compareModeOn
 * \param autoReload
 *
 * \return 0 on success, -1 on failure
 */
int TimerBasicConfiguration(Timer timer, uint8_t enableCompareMode, uint8_t enableAutoReload, uint32_t matchValue, uint32_t loadValue, uint16_t clockSource, uint8_t pre, uint8_t ptv);

/**
 * \brief
 *
 * \param timer
 * \param interrupt routine
 *
 * \return 0 on success, -1 on failure
 */
int TimerInterruptConfiguration(Timer timer, IrqMode irqMode, IrqWakeen irqwakeen, InterruptRoutine routine);

#endif /* DR_TIMER_H_ */
