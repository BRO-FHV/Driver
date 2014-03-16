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

enum Timer { TIMER0 = 0, TIMER1MS, TIMER2, TIMER3, TIMER4, TIMER5, TIMER6, TIMER7 };

int TimerEnable(Timer timer);
int TimerDisable(Timer timer);
int TimerReset(Timer timer);
int TimerConfigure(Timer timer, uint8_t compareModeOn, uint8_t oneShot);


#endif /* DR_TIMER_H_ */
