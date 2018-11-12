#include "pmu.h"
#include "cmu.h"
#include "emu.h"
#include "gun.h"
#include "includes.h"
#include "delay.h"

void PMU_Init(void)
{
	HT_CMU->WPREG = 0xA55A;
	HT_PMU->PMUCON = 0x0008;
	//HT_PMU->VDETPCFG = 0x0007;             //300us, 2144ms
	
	HT_GPIOE->IOCFG |= 0x0200;             //PE9 as LVDIN1
    HT_PMU->VDETCFG	= 0x0089;								//VCC检测阈值=4.6V;BOR检测阈值=2.4V
	HT_PMU->VDETPCFG= 0x0020;								//VCC_DET,BOR_DET检测时间=300us,周期=67ms	
//	HT_PMU->VDETCFG = 0x0089;  
	
	HT_PMU->PMUIF = 0x0000;
	HT_PMU_ITConfig(PMU_PMUIE_VCCIE|PMU_PMUIE_LVD1IE,ENABLE);
	NVIC_EnableIRQ(PMU_IRQn);
}


/*******************************************************************************
功能描述：	掉电切换MCU
*******************************************************************************/
void PowerOff_MCU(void)
{
	HT_CMU->WPREG = 0xA55A;

	HT_GPIOF->IOCFG =0x0000;
	HT_GPIOF->PTDIR|=0x0007;
	HT_GPIOF->PTDAT=0x0000;	
	Write_ECADR(0x98,      0x0);
	Write_ECADR(VAR_EMUCTRL, 0x0382);
	Write_ECADR(VAR_ADCCFG, 0x0003);
	Write_ECADR(VAR_ANA_control, 0x27A6);	
	//HT_TBS->TBSCON &=~0X01;
	

	HT_GPIOA->PTDIR|=0x1128;  //SDA AUDIO_EN  AU_DATA RELAY1
			 //pa.3   .5        .8     .12  
	HT_GPIOA->PTOD&=~0X1128;  
	HT_GPIOA->PTDAT=0x1128;		

	HT_GPIOB->PTDIR|=0x403F;  //BT_RET BT_EN NB_EN RELAY2 LED1 LED2 LED3
			 //pb.0     .1    .2   .3    .4    .5   .14
	HT_GPIOB->PTOD&=~0X403F;  
	HT_GPIOB->PTDAT=0x403f;

	HT_GPIOC->PTDIR|=0x000C;  //RX0  TX0
		   //pC.2  .3 
	HT_GPIOC->PTOD&=~0X000C;  		
	HT_GPIOC->PTDAT=0x000C;	

	// HT_GPIOD->PTDAT =0x0000; 
	HT_GPIOD->PTDIR|=0xDF3C;  //AU_RST,AU_BUSY,EEWP, SCL, POWEKEY NB_RESET WAKE_M  WAKE_AP  DEG_TX BT_C/D BT_IRQ
		   // pD.2   .3      .4   .5    .8       .9       .10    .11     .12     .14     .15 
	HT_GPIOD->PTOD&=~0XDF3C;  
	HT_GPIOD->PTDAT|=0xDF30; 	//为么为低才	

	HT_GPIOE->PTDIR|=0x0036;  //TXD4 RXD4 RXD3 TXD4 LVDIN
		   //pE.1   .2  .4   .5    .9 
	HT_GPIOE->PTOD&=~0X0036;  
	HT_GPIOE->PTDAT=0x0036; 
	HT_GPIOA->PTUP|=(GPIO_Pin_3|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_12);//pa 3.5.6.8.7.12
	HT_GPIOB->PTUP|=(GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_14);
	HT_GPIOC->PTUP|=(GPIO_Pin_2|GPIO_Pin_3);
	HT_GPIOD->PTUP|=(GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_14|GPIO_Pin_15);
	HT_GPIOE->PTUP|=(GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_9);

	//	NVIC_ClearPendingIRQ(PMU_IRQn);											//清除挂起状态
	//	NVIC_SetPriority(PMU_IRQn, 3);											//设置优先级
	//	NVIC_EnableIRQ(PMU_IRQn);												//使能PMU中断

	HT_CMU->WPREG = 0;
}


void enter_hold_init(void)
{
	//run_osc();
	HT_CMU->WPREG = 0xA55A;
	HT_CMU->CLKCTRL0 = 0x2400;             //close HRC...
	HT_CMU->CLKCTRL1 = 0x000;             //close HRC..
	NVIC_DisableIRQ(PMU_IRQn);												//禁止PMU中断
	HT_PMU->PMUCON = 0x0008;
	HT_PMU->VDETCFG	= 0x008d;								//VCC检测阈值=4.6V;BOR检测阈值=2.4V
	HT_PMU->VDETPCFG = 0x0027;								//VCC_DET,BOR_DET检测时间=300us,周期=67ms
	HT_PMU ->PMUIE    |= 0x01;  //??VSYS??

//	HT_RTC->RTCIE &= 0xFFFE;				      //RTC秒中断关闭
//	HT_RTC->RTCCON &= 0xFFDF;             //RTC定时器1关闭
	//HT_RTC->RTCCON |= 0x20;               //RTC定时器1使能
	
	//Delay_FreeDog();
	HT_PMU_ITConfig(PMU_PMUIE_VCCIE|PMU_PMUIE_LVD1IE,ENABLE);
	NVIC_ClearPendingIRQ(PMU_IRQn);											//清除挂起状态
	NVIC_SetPriority(PMU_IRQn, 3);											//设置优先级
	NVIC_EnableIRQ(PMU_IRQn);												//使能PMU中断
	HT_CMU->WPREG = 0;
}

/*******************************************************************************
功能描述：	进入掉电休眠循环
输入参数：
返回参数：
函数说明：
*******************************************************************************/
void Enter_StandBy(void)
{
	uint8_t i = 0,j = 0;
	
	while(1)
	{
		//CL_LOG("进入Enter_StandBy！进入Enter_StandBy！进入Enter_StandBy！\n");
		Delay_mSec(1);
		if(SET != HT_PMUState_FlagStatusGet(PMU_PMUSTA_VCCFLG))
		{//  掉电
			i++;
			j = 0;
		}
		else
		{//上电
			j++;
			i = 0; 
		}

		if(j>=5)
		{
			return;
		}
		if(i>=5)
		{
			break ;
		}
	}
	
	//进入低功耗
	while(1)
	{
		//CL_LOG("进入低功耗！进入低功耗！进入低功耗！\n");
		Disable_Int();
		
		RTCTMR_InitTypeDef RTCTMR_InitStruct;
		RTCTMR_InitStruct.NewState = ENABLE;
		RTCTMR_InitStruct.Count = 8-1;
		HT_RTC_TMRConfig(RTCTMR1,&RTCTMR_InitStruct);
		HT_RTC_ITConfig(RTC_RTCIE_RTC1IE, ENABLE);
		NVIC_EnableIRQ(RTC_IRQn);
		
		SwitchTo_Flrc();		
		PowerOff_MCU();
		Feed_WDT16S();
		enter_hold_init();
		//Goto_Sleep();
		Goto_Hold();
		HT_RTC->RTCIF=0; 
		NVIC_ClearPendingIRQ(RTC_IRQn);
		if(SET == HT_PMUState_FlagStatusGet(PMU_PMUSTA_VCCFLG))
		{
			//CL_LOG("进入低功耗重启系统！\n");
			Reset_CPU();
		}
	}
}




