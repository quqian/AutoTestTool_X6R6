#ifndef __UART_H__
#define __UART_H__

#include "includes.h"
#include "FIFO.h"



#define UART_BLUE_SIZE      356
#define UART_GPRS_SIZE      128
#define UART_DEBUG_SIZE		150


//校验位
#define USART_Parity_NO_1  ((uint16_t)0x0070)  //固定位1
#define USART_Parity_No_0  ((uint16_t)0x0010)	//固定0
#define USART_Parity_Even  ((uint16_t)0x0050)	//偶校验
#define USART_Parity_Odd   ((uint16_t)0x0030)	//奇校验
#define USART_Parity_NONE  ((uint16_t)0x0000)	//无校验

//停止位
#define USART_StopBits_1  ((uint16_t)0x0000)	//1位停止位
#define USART_StopBits_2  ((uint16_t)0x0100)	//2位停止位


#define USART_WordLength_8b	((uint16_t)0x0000)	//
#define USART_WordLength_7b	((uint16_t)0x0080)	//

typedef struct{
    
    FIFO_S_t rxFIFO;
    HT_UART_TypeDef *sysAddr;
    osMutexId s_mutex;				//互斥量
	osMutexId xSemaphore; 			//发送中断信号量
}UART_INFO_STR;


extern UART_INFO_STR gUart0Handle;
extern UART_INFO_STR gUart4Handle;
extern UART_INFO_STR gUart3Handle;

void UART0_Init(uint32_t rate);
void UART4_Init(uint32_t rate);
void UART3_Init(uint32_t rate);
extern void Uart_Send(UART_INFO_STR* USARTx,char *data,uint16_t len);
extern void USART_SendData(UART_INFO_STR* USARTx, uint8_t Data);

#endif
