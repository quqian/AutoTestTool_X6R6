#include "uart.h"
#include "delay.h"


UART_INFO_STR gUart0Handle;
UART_INFO_STR gUart4Handle;
UART_INFO_STR gUart3Handle;


static void USART_SendData(UART_INFO_STR* USARTx, uint8_t Data);

void Set_Uart_Bandrate(HT_UART_TypeDef* usart, uint32_t bps)
{
	//bps = Fsys/2*(SREL+1)
	//??SREL=Fsys/2*bps-1, Fsys=44MHz
	usart->SREL=SystemFsysClock/(2*bps)-1;
}


//NB-lot
void UART0_Init(uint32_t rate)
{
    static __IO uint8_t UART0_RxBuff[UART_GPRS_SIZE] = {0};
    
	GPIO_InitAFTypeDef GPIO_InitAFStruct;
	memset(&GPIO_InitAFStruct,0,sizeof(GPIO_InitAFTypeDef));
	
    //PC2 --RX0
	GPIO_InitAFStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitAFStruct.GPIO_AFMode = PC2_RX0;
	GPIO_InitAFStruct.GPIO_InputStruct = GPIO_Input_Up;
	HT_GPIO_AFInit(HT_GPIOC,&GPIO_InitAFStruct);
	
	//PC3--TX0
	GPIO_InitAFStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitAFStruct.GPIO_AFMode = PC3_TX0;
	GPIO_InitAFStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_AFInit(HT_GPIOC,&GPIO_InitAFStruct);
	
	UART_InitTypeDef UART_InitStructure;
    HT_CMU_ClkCtrl1Config(CMU_CLKCTRL1_UART0EN, ENABLE);
    UART_InitStructure.UART_Logic = UartLogicPositive;
    UART_InitStructure.UART_StopBits = OneStopBits;
    UART_InitStructure.UART_WordLength = WordLength_8Bits;
    UART_InitStructure.UART_Parity = UartParity_Disable;
    UART_InitStructure.UART_BaudRate = rate;
    UART_InitStructure.ReceiveEN = ENABLE;
    UART_InitStructure.SendEN = ENABLE;

    HT_UART_Init(HT_UART0, &UART_InitStructure);
	
	HT_UART_ITConfig(HT_UART0,UART_UARTCON_TXIE|UART_UARTCON_RXIE,ENABLE);
	
    NVIC_ClearPendingIRQ(UART0_IRQn);			//清除挂起状态
	NVIC_SetPriority(UART0_IRQn, 0);			//设置优先级
	NVIC_EnableIRQ(UART0_IRQn);					//使能串口中断
        
    //接收缓存区初始化
    gUart0Handle.sysAddr = HT_UART0;
    gUart0Handle.s_mutex = osMutexCreate(NULL);
	gUart0Handle.xSemaphore = xSemaphoreCreateBinary(); 
    FIFO_S_Init(&gUart0Handle.rxFIFO, (void*)UART0_RxBuff, sizeof(UART0_RxBuff));
    FIFO_S_Flush(&gUart0Handle.rxFIFO);
}


//上位机
void UART3_Init(uint32_t rate)
{
	static __IO uint8_t UART3_RxBuff[UART_DEBUG_SIZE] = {0};
	
	GPIO_InitAFTypeDef GPIO_InitAFStruct;
	memset(&GPIO_InitAFStruct,0,sizeof(GPIO_InitAFTypeDef));
	
    //PE4 --RX3
	GPIO_InitAFStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitAFStruct.GPIO_AFMode = PE4_RX3;
	GPIO_InitAFStruct.GPIO_InputStruct = GPIO_Input_Up;
	HT_GPIO_AFInit(HT_GPIOE,&GPIO_InitAFStruct);
	
	//PE5--TX3
	GPIO_InitAFStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitAFStruct.GPIO_AFMode = PE5_TX3;
	GPIO_InitAFStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_AFInit(HT_GPIOE,&GPIO_InitAFStruct);
	
	UART_InitTypeDef UART_InitStructure;
    HT_CMU_ClkCtrl1Config(CMU_CLKCTRL1_UART3_7816_1EN, ENABLE);        
    UART_InitStructure.UART_Logic = UartLogicPositive;          
    UART_InitStructure.UART_StopBits = OneStopBits;             
    UART_InitStructure.UART_WordLength = WordLength_8Bits;      
    UART_InitStructure.UART_Parity = UartParity_Disable;          
    UART_InitStructure.UART_BaudRate = rate;                    
    UART_InitStructure.ReceiveEN = ENABLE;                      
    UART_InitStructure.SendEN = ENABLE;                         

    HT_UART_Init(HT_UART3, &UART_InitStructure);
	
	HT_UART_ITConfig(HT_UART3,UART_UARTCON_TXIE|UART_UARTCON_RXIE,ENABLE);
	
    
    NVIC_ClearPendingIRQ(UART3_IRQn);			//清除挂起状态
	NVIC_SetPriority(UART3_IRQn, 3);			//设置优先级
	NVIC_EnableIRQ(UART3_IRQn);					//使能串口中断
    
    //接收缓存区初始化
    gUart3Handle.sysAddr = HT_UART3;
    gUart3Handle.s_mutex = osMutexCreate(NULL);
	gUart3Handle.xSemaphore = xSemaphoreCreateBinary();
    FIFO_S_Init(&gUart3Handle.rxFIFO, (void*)UART3_RxBuff, sizeof(UART3_RxBuff));
}


//蓝牙
void UART4_Init(uint32_t rate)
{
    static __IO uint8_t UART4_RxBuff[UART_BLUE_SIZE] = {0};
    
	GPIO_InitAFTypeDef GPIO_InitAFStruct;
	memset(&GPIO_InitAFStruct,0,sizeof(GPIO_InitAFTypeDef));
	
    //PE2 --RX4
	GPIO_InitAFStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitAFStruct.GPIO_AFMode = PE2_RX4;
	GPIO_InitAFStruct.GPIO_InputStruct = GPIO_Input_Up;
	HT_GPIO_AFInit(HT_GPIOE,&GPIO_InitAFStruct);
	
	//PE1--TX4
	GPIO_InitAFStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitAFStruct.GPIO_AFMode = PE1_TX4;
	GPIO_InitAFStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_AFInit(HT_GPIOE,&GPIO_InitAFStruct);
	
	UART_InitTypeDef UART_InitStructure;
    HT_CMU_ClkCtrl1Config(CMU_CLKCTRL1_UART4_7816_0EN, ENABLE);        
    UART_InitStructure.UART_Logic = UartLogicPositive;          
    UART_InitStructure.UART_StopBits = OneStopBits;             
    UART_InitStructure.UART_WordLength = WordLength_8Bits;      
    UART_InitStructure.UART_Parity = UartParity_Disable;          
    UART_InitStructure.UART_BaudRate = rate;                    
    UART_InitStructure.ReceiveEN = ENABLE;                      
    UART_InitStructure.SendEN = ENABLE;                         

    HT_UART_Init(HT_UART4, &UART_InitStructure);
	
	HT_UART_ITConfig(HT_UART4,UART_UARTCON_TXIE|UART_UARTCON_RXIE,ENABLE);
	
    NVIC_ClearPendingIRQ(UART4_IRQn);			//清除挂起状态
	NVIC_SetPriority(UART4_IRQn, 3);			//设置优先级
	NVIC_EnableIRQ(UART4_IRQn);					//使能串口中断

    memset(&gUart4Handle, 0, sizeof(gUart4Handle));
    //接收缓存区初始化
    gUart4Handle.sysAddr = HT_UART4;
	gUart4Handle.xSemaphore = xSemaphoreCreateBinary();
    gUart4Handle.s_mutex = osMutexCreate(NULL);
    FIFO_S_Init(&gUart4Handle.rxFIFO, (void*)UART4_RxBuff, sizeof(UART4_RxBuff));
}

void USART_SendData(UART_INFO_STR* USARTx, uint8_t Data)
{
	USARTx->sysAddr->SBUF = Data;
	xSemaphoreTake( USARTx->xSemaphore, 10);
}



void Uart_Send(UART_INFO_STR* USARTx,char *data,uint16_t len)
{
    int i = 0;
    osMutexWait (USARTx->s_mutex,1000);
    for(i=0;i<len;i++)
	{
        USART_SendData(USARTx,data[i]);
    }
    osMutexRelease (USARTx->s_mutex);
}


