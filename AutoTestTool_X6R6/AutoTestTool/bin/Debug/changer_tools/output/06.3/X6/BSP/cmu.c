#include "cmu.h"
#include "delay.h"

void SystemClock_Config(void)
{
    CMU_InitTypeDef CMU_InitStruct;
    CMU_InitStruct.SysClkSel = SysPLL;
    CMU_InitStruct.CPUDiv = CPUDiv1;
    HT_CMU_Init(&CMU_InitStruct);
    while(HT_CMU_SysClkGet() != SystemFsysClock)
    {
        HT_CMU_Init(&CMU_InitStruct);
        Delay_mSec(1000);
        Feed_WDT();
    }
   // HT_CMU->CLKCTRL0 &= ~0x0380;//关闭停震检测
}

/*******************************************************************************
功能描述：	切换到内部高速时钟
输入参数：
返回参数：
函数说明：	8MHz
*******************************************************************************/
void SwitchTo_Fhrc(void)
{
	if ((HT_CMU->SYSCLKCFG != 0x0002)
	|| (HT_CMU->HRCADJ    != 0x003D)
	|| (HT_CMU->HRCDIV    != 0x0000)
	|| !(HT_CMU->CLKCTRL0 & 0x0020)
	|| (HT_CMU->SYSCLKDIV != 0x0000))
	{
		EnWr_WPREG();
		HT_CMU->HRCADJ    = 0x003D;				//输出8MHz
		HT_CMU->HRCDIV    = 0x0000;				//Fhrc' = Fhrc = 8MHz
		HT_CMU->CLKCTRL0 |= 0x0020;				//使能HRC
		HT_CMU->SYSCLKDIV = 0x0000;				//Fcpu  = Fsys = 8MHz
		while (HT_CMU->CLKSTA & 0x0008)
		{
			;
		}
		HT_CMU->SYSCLKCFG = 0x0082;				//Fsys = Fhrc'
		__NOP();
		HT_CMU->SYSCLKCFG = 0x0002;				//Fsys = Fhrc'
		DisWr_WPREG();
	}
}


/*******************************************************************************
功能描述：	切换到内部低速时钟
输入参数：
返回参数：
函数说明：	32768Hz
*******************************************************************************/
void SwitchTo_Flrc(void)
{
	if ((HT_CMU->SYSCLKCFG != 0x0000)
	|| (HT_CMU->LRCADJ    != 0x0009)
	|| (HT_CMU->SYSCLKDIV != 0x0000))
	{
		EnWr_WPREG();
		HT_CMU->LRCADJ    = 0x0009;				//输出32768Hz
		HT_CMU->SYSCLKDIV = 0x0000;				//Fcpu = Fsys
		HT_CMU->SYSCLKCFG = 0x0080;				//Fsys = Flrc
		__NOP();
		HT_CMU->SYSCLKCFG = 0x0000;				//Fsys = Flrc
	
		HT_PMU->VDETCFG	= 0x0085;
		DisWr_WPREG();
	}
}


void close_hrc(void)
{
	HT_CMU->WPREG = 0xA55A;
	HT_CMU->CLKCTRL0 = 0x2400;             //close HRC...
	HT_CMU->CLKCTRL1 = 0x000;             //close HRC..
	HT_PMU->PMUCON = 0x0002;
	
	HT_PMU->VDETCFG	= 0x0085;
	//HT_PMU->VDETCFG	= 0x0001;								//VCC检测阈值=2.4V;BOR检测阈值=2.2V
	//HT_PMU->VDETPCFG= 0x0022;								//VCC_DET,BOR_DET检测时间=300us,周期=67ms
	
}



void run_osc(void)
{
	HT_CMU->WPREG = 0xA55A;
	HT_CMU->SYSCLKCFG = 0x81;              //OSC as Fsys 
	HT_CMU->SYSCLKDIV = 0x00;              //PLL:19.66MHz, Fsys/1 as Fcpu		
}


