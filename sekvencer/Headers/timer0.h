/*
 * timer0.h
 *
 * Created: 18. 6. 2015 21:54:43
 *  Author: Martin
 */ 


#ifndef TIMER0_H_
#define TIMER0_H_


void timer0_start(const uint32_t Miliseconds);
void timer0_plan_next(const uint32_t Miliseconds);
void timer0_stop(void);




#endif /* TIMER0_H_ */
