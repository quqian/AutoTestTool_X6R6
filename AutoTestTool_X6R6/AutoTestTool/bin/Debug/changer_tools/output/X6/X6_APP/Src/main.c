#include "includes.h"
#include "cmu.h"
#include "pmu.h"
#include "delay.h"

extern void vMainTask(void* argc);


uint32_t RstsrTmp;


void Init_TaskSystem(void)
{
	PMU_Init();
	Enter_StandBy();
	Delay_mSec(500);
	SystemClock_Config();
}

int main()
{
	RstsrTmp = HT_PMU->RSTSTA; 
	HT_PMU->RSTSTA = 0x0000;
	Feed_WDT();
	HT_CMU->WPREG = 0xA55A;
	HT_CMU->CLKCTRL0 &= ~0x0380;//¹Ø±ÕÍ£Õð¼ì²â
	HT_CMU->WPREG = 0x0;

	Init_TaskSystem();
	Enable_Int();

	xTaskCreate((TaskFunction_t)vMainTask,"vMainTask",1024,NULL,1,NULL);
	vTaskStartScheduler();
	while(1); 
}


