#include "timer.h"
#include "ht6xxx.h"


void Timer0_Init(void)
{
	HT_CMU->WPREG = 0xA55A;
	HT_CMU->CLKCTRL1 |= 0x0001; //定时器0使能
	HT_TMR0->TMRCON |= 0x07;	//周期定时，计数器使能
	HT_TMR0->TMRDIV = 19660-1;//预分频器
	HT_TMR0->TMRPRD = 1000;		//重装值  1000ms
	
	HT_TMR0->TMRIE = 1;
	NVIC_EnableIRQ(TIMER_0_IRQn);
}
