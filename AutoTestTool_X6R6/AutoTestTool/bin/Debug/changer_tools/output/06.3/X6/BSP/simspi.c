#include "simspi.h"


void SIM_SPI_Delay(void);


void SIM_SPI_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//PD9--SPI_CLK  PD10--SPI_MOSI  PD11---SPI_CS  
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOOUT;
	GPIO_InitStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_Init(HT_GPIOD,&GPIO_InitStruct);
	
	//PC3--NDP
    memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOOUT;
	GPIO_InitStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_Init(HT_GPIOC,&GPIO_InitStruct);
    
    //PD8--SPI_MISO
    memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOIN;
	GPIO_InitStruct.GPIO_InputStruct = GPIO_Input_Floating;
	HT_GPIO_Init(HT_GPIOD,&GPIO_InitStruct);
    
    //PC2--IRQ
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOIN;
	GPIO_InitStruct.GPIO_InputStruct = GPIO_Input_Up;
	HT_GPIO_Init(HT_GPIOC,&GPIO_InitStruct);
    
    //PB2--POWER_EN
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOOUT;
	GPIO_InitStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_Init(HT_GPIOB,&GPIO_InitStruct);
    
    //打开电源
    FM1752_POWER_ON();
    
    //CPDN  高电平
    NPD_HIGH();
    
    //CS 高电平
    SPI_CS_HIGH();
    
    //数据输出
    SPI_MSO_HIGH();
    
    //时钟
    SPI_CLK_LOW();
}

void SIM_SPI_Delay(void)
{
    int i = 0;
	
    for(i = 0; i < 8; i++)
	{
        __NOP();
    }
}


void SIM_SPI_WriteByte(unsigned char TxData)
{
    unsigned char i = 0;

    for(i = 0; i < 8; i++)
    {
        if(TxData & 0x80)
        {
            SPI_MSO_HIGH();
        }
        else
        {
            SPI_MSO_LOW();
        }
        TxData <<= 1;
        SPI_CLK_HIGH();//
        SIM_SPI_Delay();
        SPI_CLK_LOW();
        SIM_SPI_Delay();
    }
}


unsigned char SIM_SPI_ReadByte(void)
{
    unsigned char read_value = 0;
    unsigned char i = 0;
	
    for(i = 0; i < 8; i++)
	{
        SPI_CLK_HIGH();
        SIM_SPI_Delay();
         read_value <<= 1;
        if(SPI_MSI_READ())
		{
          read_value |= 0x01;
        }

        SPI_CLK_LOW();//
 
        SIM_SPI_Delay();
    }
 
    return (read_value);
}


