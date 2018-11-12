#include "includes.h"
#include "simuart.h"
#include "delay.h"


SIM_UART_STR gSimUartCtrl[SIM_UART_CNT];


void SimUartSendByte(uint8_t sendStat, uint8_t sendData, uint8_t uartIndex);
void IO_USART_TX_PutVal(unsigned char val, uint8_t uartIndex);
void TIM_Cmd(int timer, int en);

void TIMER_1_IRQHandler(void)
{
    SIM_UART_STR *pSimUart = &gSimUartCtrl[0];
	
	if(	HT_TMR1->TMRIF == 1)
	{
		HT_TMR1->TMRIF = 0;

        SimUartSendByte(pSimUart->gSendStat, pSimUart->gSendData, 0);
        pSimUart->gSendStat++;
        
        if (11 == pSimUart->gSendStat) 
		{
            
            if (++pSimUart->sendCnt < pSimUart->allCnt) 
			{
                pSimUart->gSendData = pSimUart->pSendBuf[pSimUart->sendCnt];
                pSimUart->gSendStat = 0;
            }
			else
			{
				pSimUart->VirtualSendFlag = 0xa5;
                TIM_Cmd(pSimUart->timerIndex, DISABLE);
            }
        }
	}
}



void Time1_Init()
{
    EnWr_WPREG();
    
    HT_TMR1->TMRCON |= 0x06;	//周期定时
    HT_TMR1->TMRDIV = 19-1;     //预分频器  1MHZ
    HT_TMR1->TMRPRD = gSimUartCtrl[0].baudRate;	    //重装值
   
    HT_CMU->CLKCTRL1 |= 0x0002; //定时器1使能
    HT_TMR1->TMRIE = 1;
	
	NVIC_EnableIRQ(TIMER_1_IRQn);
}

int OpenTimer(uint8_t timer, uint16_t us)
{
	if (1 == timer) 
	{
		HT_TMR1->TMRCON = (0x3<<1) | (0<<0); //向上计数   //MODE周期计数
		HT_TMR1->TMRDIV = 19 - 1;  //1us
		HT_TMR1->TMRPRD = us;		//重装值52us
		HT_TMR1->TMRIE = 1;
		return CL_OK;
    }
	
	return CL_FAIL;
}


void TIM_Cmd(int timer, int en)
{
	if(timer == 1) 
	{
        if(en)
		{
            //Time1_Init();
            setbit(HT_TMR1->TMRCON,0);
        }
        else
		{
            //Time1_Init();
            clrbit(HT_TMR1->TMRCON,0);
        }
	}
}


void IO_USART_TX_PutVal(unsigned char val, uint8_t uartIndex)
{
    if (0 == uartIndex) 
	{
    	if (val)
		{
            HT_GPIO_BitsSet(HT_GPIOD, GPIO_Pin_12);
    	}
		else
		{
            HT_GPIO_BitsReset(HT_GPIOD, GPIO_Pin_12);
    	}
    }
}

void SimUartSendByte(uint8_t sendStat, uint8_t sendData, uint8_t uartIndex)
{
	if (sendStat == COM_START_BIT) 
	{//发送起始位
		IO_USART_TX_PutVal(0, uartIndex);
	}
    else if(sendStat == COM_STOP_BIT) 
	{//stop  high
		IO_USART_TX_PutVal(1, uartIndex);
	}
	else if(sendStat == 10) 
	{//stop  delay
	}
	else
	{
		IO_USART_TX_PutVal(sendData & (1<<(sendStat-1)), uartIndex);
	}
}




void SimuartGpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//DBG_TX  PD12
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOOUT;
	GPIO_InitStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_Init(HT_GPIOD,&GPIO_InitStruct);

    //DBG_TX
    HT_GPIO_BitsSet(HT_GPIOD, GPIO_Pin_12);
}


int SimuUartSendData(SIM_UART_STR *pSimUart, const uint8_t *pData, uint16_t len)
{
    int waitCnt = 4*len;
    int cnt = 0;

    pSimUart->allCnt = len;
    pSimUart->sendCnt = 0;
    pSimUart->pSendBuf = (void*)pData;
    pSimUart->gSendData = pData[0];
    pSimUart->gSendStat = 0;
    pSimUart->VirtualSendFlag = 0;
	OpenTimer(pSimUart->timerIndex, pSimUart->baudRate);
    TIM_Cmd(pSimUart->timerIndex, ENABLE);
	#if 1
    while (pSimUart->sendCnt < pSimUart->allCnt)
	{
        //Delay_mSec(2);
		osDelay(30);
        Feed_WDT();
        if ((waitCnt < ++cnt) || (0xa5 == pSimUart->VirtualSendFlag))
		{
            TIM_Cmd(pSimUart->timerIndex, DISABLE);
            return CL_FAIL;
        }
    }
	#endif
	
    return CL_OK;
}



int SimuartInit(void)
{
    int i = 0;
    
    memset(gSimUartCtrl, 0, sizeof(gSimUartCtrl));
    for (i=0; i<SIM_UART_CNT; i++) 
	{
        gSimUartCtrl[i].recvStat = COM_STOP_BIT;
    }
    gSimUartCtrl[0].timerIndex = 1;
    gSimUartCtrl[0].baudRate = 52; //波特率 19200
    
	EnWr_WPREG();
	setbit(HT_CMU->CLKCTRL1 ,1);//打开定时器1
    NVIC_EnableIRQ(TIMER_1_IRQn);
    	
    SimuartGpioInit();
    
    return CL_OK;
}




