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
typedef void (*InterruptRoutine)(void);

/**
 * \brief enable a timer
 *
 * \param timer
 *
 * \return 0 on success, -1 on failure
 */
int32_t TimerEnable(Timer timer);

/**
 * \brief disable a timer
 *
 * \param timer
 *
 * \return 0 on success, -1 on failure
 */
int32_t TimerDisable(Timer timer);

/**
 * \brief reset a timer (counting state)
 *
 * \param timer
 *
 * \return 0 on success, -1 on failure
 */
int32_t TimerReset(Timer timer);

/**
 * \brief configure the compare mode and the auto reaload mode. Timermust be disabled
 *
 * \param timer
 * \param compareModeOn
 * \param autoReload
 *
 * \return 0 on success, -1 on failure
 */
int32_t TimerConfiguration(Timer timer, uint32_t milliseconds, InterruptRoutine routine) ;

/**
 * \brief clears the interrupt flag of the passed timer
 *
 * \param timer
 *
 * \return 0 on success, -1 on failure
 */
int32_t TimerInterruptFlagClear(Timer timer);

#endif /* DR_TIMER_H_ */
