
#ifndef __CKB24_UM_H__
#define __CKB24_UM_H__

#include "includes.h"


#define CKB24_POWER_EN()    HT_GPIO_BitsSet(HT_GPIOB,GPIO_Pin_1) 
#define CKB24_POWER_DIS()   HT_GPIO_BitsReset(HT_GPIOB,GPIO_Pin_1) 

#define CKB24_RST_HIGH()    HT_GPIO_BitsSet(HT_GPIOB,GPIO_Pin_0) 
#define CKB24_RST_LOW()     HT_GPIO_BitsReset(HT_GPIOB,GPIO_Pin_0) 

#define CKB24_CD_HIGH()     HT_GPIO_BitsSet(HT_GPIOD,GPIO_Pin_14)
#define CKB24_CD_LOW()      HT_GPIO_BitsReset(HT_GPIOD,GPIO_Pin_14)


#define GET_BLUE_IRQ()      HT_GPIO_BitsRead(HT_GPIOD,GPIO_Pin_15)

#define BLUE_DEBUG      0

#define RECEIVE_PKT_LEN              64

typedef struct{
    char *cmd;
    char *res;
    int wait;
    int nwait;
    int (*process)(char ok, uint8_t retry);
}BlUE_INITTAB;



enum{
    USER_CMD_ADDR = 0,
	USER_CMD_GWID,
};

enum{
    BLUE_RESET = 0,
	BLUE_MODE_ECHO,
	BLUE_MODE_VER,
	BLUE_MODE_GETNAME,
	BLUE_MODE_SETNAME,
	BLUE_MODE_DEVT = 5,
	BLUE_MODE_SETDEVT,
	BLUE_MODE_SETGWID,
	BLUE_MODE_GETGWID,
	BLUE_MODE_PAIR,
    BLUE_STATE_NUM = 10
};



enum{
    NODE_BLUE = 0,
    NODE_24G
};

enum {
    BLUE_AB,
    BLUE_CD,
    BLUE_TARGET,
    BLUE_ADDR,
    BLUE_LEN,
    BLUE_RX_DATA,
    BLUE_CHK,
    BLUE_END,
};


#pragma pack(1)

typedef struct {
    uint8_t  ab;                                //0.
    uint8_t  cd;                                //1.
    uint8_t  target;                            //2.
    uint8_t  addr[6];                           //3.
    uint16_t len;                               //4.
}BLUE_RF2_4G_HEAD_STR;

typedef struct {
    BLUE_RF2_4G_HEAD_STR head;
    uint8_t  data[100];
}BLUE_RF2_4G_PKT_STR;


#pragma pack()


int BuleReconnect(void);
void CKB24_UM_Init(void);


int SetBluePair(int value);
int SetBlueName(char *name);
//获取2.4G gwmac地址
extern int GetGateWay24GMacAddr(uint8_t cmd);
//设置2.4G gwmac地址
extern int SetGateWay24GMacAddr(char *mac, uint8_t cmd);
//获取蓝牙名称
extern int GetBlueName(void);


#endif

