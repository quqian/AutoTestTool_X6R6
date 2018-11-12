
#ifndef __GUN_H__
#define __GUN_H__

#include "includes.h"


#define PLUG_IDLE               0
#define PLUG_USED_NO_OPEN       1
#define PLUG_USED_OPEN          2
#define PLUG_FAULT              3


enum
{
	GUN_CHARGING_IDLE=0,    		    //空闲
    GUN_CHARGING_UNKNOW_POWER=1,        //充电中功率未检测
	GUN_CHARGING_GUN_PULL=2,		    //功率小于1.5w
	GUN_CHARGING_FULL=3,		        //1.5w<功率<40w
	GUN_CHARGING_WORK,                  //40w<功率
};


enum{
    GUN_STATE_IDLE = 0,
    GUN_STATE_WAIT_PLUG_IN,
    GUN_STATE_ON,
    GUN_STATE_OFF,
    GUN_STATE_OFF_WAIT_ACK,
    GUN_STATE_OFF_NO_NET_WAIT_ACK,
    GUN_SATAE_INIT,
};


typedef enum{
    LED1 = GPIO_Pin_4,
    LED2 = GPIO_Pin_5,
    LED3 = GPIO_Pin_14,
}LED_NUM;


typedef enum{

   RELAY_1 = 0,
   RELAY_2 = 1

}RELAY_NUM;

						
#define LED_ON(n)       HT_GPIO_BitsReset(HT_GPIOB,n)
#define LED_OFF(n)      HT_GPIO_BitsSet(HT_GPIOB,n)
#define GREEN_LED_Toggle(n)   HT_GPIO_BitsToggle(HT_GPIOB,n)
#define LED_READ(n)		HT_GPIO_BitsRead(HT_GPIOB,n)



void GunGpioInit(void);
int GunTurnOn(uint8_t gunId);
int GunTurnOff(uint8_t gunId);
int TurnOffAllGun(void);
int TurnOnAllGun(void);
int CheckGunState(uint8_t gunId);

#endif


