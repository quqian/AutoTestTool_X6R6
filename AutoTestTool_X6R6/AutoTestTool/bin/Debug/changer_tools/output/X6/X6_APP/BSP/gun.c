#include "gun.h"
#include "rtc.h"

void LED_Init(void);
void RELAY_Init(void);

void RELAY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//RELAY1
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOOUT;
	GPIO_InitStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_Init(HT_GPIOB,&GPIO_InitStruct);
	
	//RELAY2
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOOUT;
	GPIO_InitStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_Init(HT_GPIOA,&GPIO_InitStruct);
	
	
	//RELAY1 check
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOIN;
	GPIO_InitStruct.GPIO_InputStruct = GPIO_Input_Floating;
	HT_GPIO_Init(HT_GPIOD,&GPIO_InitStruct);
	
	//RELAY2 check
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOIN;
	GPIO_InitStruct.GPIO_InputStruct = GPIO_Input_Up;
	HT_GPIO_Init(HT_GPIOB,&GPIO_InitStruct);
	
}

void LED_Init(void)
{
	 //LED1、LED2、LED14
	GPIO_InitTypeDef GPIO_InitStruct;
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_4|GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOOUT;
	GPIO_InitStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_Init(HT_GPIOB,&GPIO_InitStruct);
	
    LED_OFF(LED1|LED2|LED3);
}



int CheckGunState(uint8_t gunId)
{
	uint16_t ret;
	
	if(gunId < 1 || gunId > GUN_NUM_MAX) return CL_FAIL;
	if(gunId == 1)
	{
		ret =   HT_GPIO_BitsRead(HT_GPIOB,GPIO_Pin_6);

		return (ret == GPIO_Pin_6 ? 1:0);
	}
	else if(gunId == 2)
	{
		ret = HT_GPIO_BitsRead(HT_GPIOD,GPIO_Pin_13);

		return (ret == GPIO_Pin_13 ? 0:1);
	}
	
	return CL_FAIL;
}

int GunTurnOn(uint8_t gunId)
{
	CL_LOG("turn %u gun on\r\n", gunId);
    
    if(gunId < 1 || gunId > GUN_NUM_MAX)
	{
		return CL_FAIL;
	}
    
    switch(gunId)
	{
        case 2:
            HT_GPIO_BitsSet(HT_GPIOA,GPIO_Pin_12);
            LED_ON(LED1);
            break;
        case 1:
            LED_ON(LED3);
            HT_GPIO_BitsSet(HT_GPIOB,GPIO_Pin_3);
            break;
    }
    
    return CL_OK;
}

int GunTurnOff(uint8_t gunId)
{
	CL_LOG("turn %u gun off\r\n", gunId);
    if(gunId < 1 || gunId > GUN_NUM_MAX) return CL_FAIL;
    
    switch(gunId){
        case 2:
            HT_GPIO_BitsReset(HT_GPIOA,GPIO_Pin_12);
            LED_OFF(LED1);
            break;
        case 1:
            LED_OFF(LED3);
            HT_GPIO_BitsReset(HT_GPIOB,GPIO_Pin_3);
            break;
    }
    
    return CL_OK;
}


int TurnOffAllGun(void)
{
    HT_GPIO_BitsReset(HT_GPIOB,GPIO_Pin_3);
    HT_GPIO_BitsReset(HT_GPIOA,GPIO_Pin_12);
    LED_OFF(LED1|LED3);
    return CL_OK;
}

int TurnOnAllGun(void)
{
    HT_GPIO_BitsSet(HT_GPIOA,GPIO_Pin_12);
    HT_GPIO_BitsSet(HT_GPIOB,GPIO_Pin_3);
    LED_ON(LED1|LED3);
  
    return CL_OK;
}

void GunGpioInit(void)
{
    //继电器初始化
    RELAY_Init();
    //LED初始化
    LED_Init();
	
	TurnOffAllGun();
		
    CL_LOG("init ok.\n");	
}


