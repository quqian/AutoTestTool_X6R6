#ifndef __SIMSPI_H__
#define __SIMSPI_H__


#include "includes.h"


//µÁ‘¥
#define FM1752_POWER_ON()     HT_GPIO_BitsSet(HT_GPIOB,GPIO_Pin_2)
#define FM1752_POWER_OFF()    HT_GPIO_BitsReset(HT_GPIOB,GPIO_Pin_2)

//NSS
#define SPI_CS_HIGH()   HT_GPIO_BitsSet(HT_GPIOD,GPIO_Pin_11)
#define SPI_CS_LOW()    HT_GPIO_BitsReset(HT_GPIOD,GPIO_Pin_11)

//CPND
#define NPD_HIGH()      HT_GPIO_BitsSet(HT_GPIOC,GPIO_Pin_3)
#define NPD_LOW()       HT_GPIO_BitsReset(HT_GPIOC,GPIO_Pin_3)

//clock
#define SPI_CLK_HIGH()  HT_GPIO_BitsSet(HT_GPIOD,GPIO_Pin_9)
#define SPI_CLK_LOW()   HT_GPIO_BitsReset(HT_GPIOD,GPIO_Pin_9)

//master?out
#define SPI_MSO_HIGH()  HT_GPIO_BitsSet(HT_GPIOD,GPIO_Pin_10)
#define SPI_MSO_LOW()   HT_GPIO_BitsReset(HT_GPIOD,GPIO_Pin_10)

//??
#define SPI_MSI_READ() HT_GPIO_BitsRead(HT_GPIOD,GPIO_Pin_8)



void SIM_SPI_Init(void);
void SIM_SPI_WriteByte(unsigned char TxData);
unsigned char SIM_SPI_ReadByte(void);


#endif
