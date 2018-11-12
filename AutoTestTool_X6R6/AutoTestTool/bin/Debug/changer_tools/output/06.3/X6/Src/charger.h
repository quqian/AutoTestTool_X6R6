#ifndef __CHARGER_H__
#define __CHARGER_H__

#include "includes.h"
#include "gun.h"
#include "flash_usr.h"


#define  CHARGING_MAX_TIME              (12*60*60)

#define COST_POWER                      1  //分功率
#define COST_UNIFY                      2   //统一计费

#define NO_LOAD_TIME                    (30) //分钟

#define CHARGING_POWER_LIMIT            2200  //整桩最大功率限制 w

#define PUT_OUT_GUN_POWER               15  //0.1w
#define PUT_OUT_GUN_TIME                300  //秒

enum {
    FIRST_START_CHARGING = 1,
    NET_RECOVER,
    POWER_RECOVER,
};

enum{
    CHARGING_FIXED = 1,                         //固定计费
	CHARGING_ACTUAL,                            //实时计费
	CHARGING_START,                             //起步金
};

enum {
    START_FAIL_FAULT = 1,
    START_FAIL_NO_TEMPLATE,
    START_FAIL_CHARGING,
    START_FAIL_NO_ADJUST,
    START_FAIL_PARA_ERR,
};

enum {
    REASON_UNKNOW = 0,
    REASON_COMM_ERR = 1,                //1.记录芯片通信故障
    REASON_RESET_TIME_ENOUGH,           //2.复位时间充够
    REASON_MONEY_ENOUGH,                //3.金额充够
    REASON_TIME_ENOUGH,                 //4.时间充够
    REASON_OVER_23H,                    //5.超过23小时
    REASON_BLUE,                        //6.蓝牙结束
	REASON_TOTAL_POWER_LARGE,           //7.整桩功率过大
	REASON_AVERAGE_POWER_LOW,           //8.平均功率过低
	RELAY_ERR,							//9.继电器故障
};

enum {
    STOP_NORMAL = 0,                    //0.按照服务类型（按电量充电，按时长充电，按金额充电等）正常停止充电
    STOP_SWIPE_CARD,                    //1.用户本地刷卡停止充电
    STOP_INPUT_CODE,                    //2.用户本地输入校验码停止充电
    STOP_PHONE,                         //3.用户远程结束
    STOP_ADMINI,                        //4.管理员远程结束
    STOP_EMERG_STOP,                    //5.急停按下，停止充电
    STOP_PULL_OUT,                      //6.检测到枪头断开，停止充电
    STOP_SYS_REBOOT,                    //7.系统重启停止充电
    STOP_NET_BREAK,                     //8.充电桩断线，停止充电
    STOP_POWER_OFF,                     //9.充电桩掉电，停止充电
    STOP_CHARGER_FAULT,                 //10.充电桩故障，停止充电
    STOP_CAR_FAULT,                     //11.电动车故障，停止充电
    STOP_OTHER,                         //12.其它原因，停止充电 未知原因
    STOP_METER_FAULT,                   //13.电表故障
    STOP_CARD_FAULT,                    //14.刷卡器故障
    STOP_LCD_FAULT,                     //15.LCD故障
    STOP_CHARGING_FULL,                 //16.soc 充满
    STOP_BALANCE_NOT_ENOUG,             //17.余额不足(耗尽)
    STOP_OVER_LOAD,                     //18.过载停止
    STOP_OVER_VOLTAGE,                  //19.过压停止
    STOP_LOW_VOLTAGE,                   //20.欠压停止
    STOP_POWER_TOO_BIG,                 //21.整桩实时功率过大停止充电；
    STOP_BY_BLUETOOTH,					//22 手机蓝牙停止
    STOP_LINKGRAND_FAULT,				//23 设备接地故障
    STOP_TEMPARATURE_HIGH,				//24 设备温度过高
};

#define STOP_UNKNOW STOP_OTHER

enum {
	EVENT_CARD_FAULT = 1,               //1. 读卡器通信故障
	EVENT_FUSE_BREAK = 2,               // 保险丝熔断
	EVENT_OVER_LOAD = 3,                // 负载超限
	EVENT_ENV_TEMP_HIGH = 4,            // 环境高温预警
	EVENT_CHIP_FAULT = 5,               //器件故障
    EVENT_PLUG_PULL_OUT = 6,            //6
    EVENT_SHUT_DOWN,                    //2.
	EVENT_POWER_DROP,                   //3.
	EVENT_NET_BREAK,                    //4.
	EVENT_NET_RECONNECT,                //5.
	EVENT_STOP_CHARGING,                //7
	EVENT_CHARGING_FULL,                //8
	EVENT_START_CHARGING,               //
    EVENT_MULTI_OPEN_FAIL,              //13
    EVENT_CHIP_TEMP_HIGH,               //15
    EVENT_ORDER_RESEND,                 //16
    EVENT_ADJUST_ERR,                   //17
};


extern int GetCostMode(void);
extern int GetChargingGunCnt(void);
extern int CheckStartPara(uint8_t gun_id);
extern void RestoreGunStatus(void);
/*****************************************************************************
** Function name:       BTRF2_4gStartCharging
** Descriptions:        //开启充电
** input parameters:    gunId: 对于本设备有两个插孔，这里取值1或2
                        pOrder: 订单号
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern uint8_t BTRF2_4gStartCharging(uint8_t gunId, uint32_t money, uint8_t *pOrder,uint8_t SerialNum);

/*****************************************************************************
** Function name:       Proto2_4gStopCharging
** Descriptions:        //停止充电
** input parameters:    gunId: 对于本设备有两个插孔，这里取值1或2
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern void Proto2_4gStopCharging(uint8_t gunId);
extern void ChargingCtrlProc(void);
extern void ChargingProc(void);

extern void Charger_Init(void);
/*****************************************************************************
** Function name:       ReadUpgradeInit
** Descriptions:        //升级初始化
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern void ReadUpgradeInit(void);
extern void FuseholderStatusProc(void);
extern void ReadFlashInfoData(PARTITION n, uint8_t *data_info, uint8_t *dataInfo, uint32_t len);

#endif
