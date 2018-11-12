#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "includes.h"
#include "FIFO.h"
#include "24g_proto.h"


#define COST_TEMP_CNT                   6


enum {
    CARD_AUTH = 0,
    CARD_CHECK_MONEY,
	CARD_MONTH,
};


enum {
    EVENT_PROMPT = 1,
    EVENT_ALARM,
    EVENT_FAULT,
};

typedef struct {
    uint8_t gSc8042bMux;
    uint8_t gBuff1024Byte;
    uint8_t gOrderReportMux;
    uint8_t gUpgradeReportMux;
    uint8_t gBlueSendMux;
    uint8_t preserve[1];    //保留
}PROTO_MUX_TYPE;

typedef struct {
    uint32_t isReadCardFlag:1;
    uint32_t ReadCardTimeLenth:1;
    uint32_t isNeedSendUpgradeFlag:1;      //
    uint32_t isBT_UpgradeFlag:1;      //
    uint32_t isUART_UpgradeFlag:1;      //
    uint32_t isBTSet24gAddrFlag:2;      //
    uint32_t fuseState:2;
}RF2_4G_CARD_FLAG_TYPE;

#define  POWER_CHECK_CNT                        60

typedef struct {
    uint32_t chargingTime;
    uint32_t beginTime;                         //计时开始时间
    //uint16_t timeLimit;                         //时间限制 秒
    uint16_t currentPower;                      //实时充电功率 0.1w
    //uint8_t  isTesting;                         //0:没有测试状态；1:测试打开；2:应急码打开；3:后台远程控制打开
    //uint8_t  isPullOutGun;                      //是否疑似拔枪 0不是；1是并发送事件，未收到应答； 0xff:发送事件并收到应答,或者发送超时没有应答
    uint8_t  checkPowerCnt;                     //检测功率次数
    //uint8_t  inCnt;
    uint8_t  isFull;
    uint8_t  powerIndex;
    uint16_t power[POWER_CHECK_CNT];
    //uint8_t  startChargerTimers;                //开始充电通知发送次数
    //uint8_t  resetEmuChipFlag;
    uint8_t  stopFlag;                          //诊断临时关闭继电器标志
    uint32_t stopTime;                          //关闭继电器时间
    uint8_t isRebootMCU;                  
	uint8_t isTimesFlag;
}GUN_CHARGING_STR;

typedef struct
{
    uint8_t  isOverTimeFlag;
    uint8_t  heartReceiveFlag;
	uint16_t chargingTotalPower;                //充电总功率 w
	
	uint8_t  netStatus;
    uint32_t  money;                       //刷卡或扫码选择的充电金额  元(单位：0.1元)
	uint8_t  current_usr_gun_id;          //1~12
    uint8_t  mode;                        //充电模式 0：智能充满 1：按金额 2：按时长 3：按电量
    
	uint8_t  current_usr_card_id[16];       //刷卡获取的卡号
	uint32_t user_card_balance;           //用户卡余额 从刷卡鉴权应答得到 手机启动 分
	
	RF2_4G_CARD_FLAG_TYPE  RF24GCardFlag;
	PROTO_MUX_TYPE gProtoMuxType;
    uint32_t gUpgradeWaitTimerTick;
    uint8_t  reserve[3];
	uint8_t isBlueToothInit;
	uint32_t gStartChargeTimeTick[GUN_NUM_MAX];
	uint32_t gCardChargeTimeTick[GUN_NUM_MAX];
	uint32_t gStartChargeNoticeTick[GUN_NUM_MAX];
	uint32_t gStopChargeNoticeTick[GUN_NUM_MAX];
	uint32_t gCardAuthAckNoticeTick[GUN_NUM_MAX];
	uint32_t gStartChargingNoticeTick[GUN_NUM_MAX];
	//uint32_t gScanCodeChargerNoticeTick[GUN_NUM_MAX];
	GUN_CHARGING_STR gunCharging[GUN_NUM_MAX];
	uint32_t ReceiveR6Ticks;
	uint8_t lastFuseholderStatus;				//上一次保险丝状态 0：正常 1-故障
	uint8_t sendFuseholderStatus;				//发送次数
	uint8_t blue_connectstate;					//蓝牙链接状态  0 :表示蓝牙已断开连接              1:表示蓝牙已连接
	uint8_t gun_id[GUN_NUM_MAX];
	uint8_t TestCardFlag;
	uint8_t ReadCardFlag;
	uint8_t UpperMonitorTestFlag;
	uint32_t UpperMonitorTicks;
	uint8_t ReceiveR624gDataFlag;
	uint8_t X6OldTestFlag;
	char  blue_name[12];						//蓝牙名称
}CHG_INFO_STR;

#pragma pack(1)

typedef struct {
    uint16_t startPower;                        //起始功率 单位w
    uint16_t endPower;                          //结束功率 单位w
    uint16_t price;                             //指定时长充电费用，分辨率分
    uint16_t duration;                          //计费时长,分钟  费率=100,时长=120表示1元充2小时
}segment_str;

typedef struct {
    uint8_t  segmentCnt;                        //功率段数目，1~4
    segment_str segmet[COST_TEMP_CNT];
}multiPower_t;

typedef struct {
    uint16_t  price;                            //指定时长充电费用，分辨率:分
    uint16_t  duration;                         //计费时长,分钟  费率=100,时长=120表示1元充2小时
}unify_t;

// cost template   计费模版下载
typedef struct {
    uint8_t  gunId;
	uint32_t template_id;                       //计费模版id
    uint8_t  mode;                              //1：按功率段计费 2按统一收费
    union {
        multiPower_t powerInfo;
        unify_t unifyInfo;
    }Data;/////
}COST_TEMPLATE_REQ_STR;



typedef struct{
    uint8_t AuthSerialNum;
    uint8_t StartChargeSN;
    uint8_t StopChargingAckSN;
    uint8_t StartChargingAckSN;
    uint8_t StartChargingNoticeSN;
    uint8_t UpgradeReqSN;
    uint8_t FWDownLoadSN;
    uint8_t FWUpgradeAckSN;
    uint8_t CostTemplateAckSN;
    uint8_t RemoteCtrlAckSN;
    uint8_t HeartBatAckSN;
    uint8_t CardAuthResultSN;
    uint8_t EventNoticeSN;
    uint8_t ReqCostTemplateSN;
    uint8_t StopChargingNoticeSN;
    uint8_t OrderReportNoticeSN;
    START_OR_STOP_CHARGING_BTRF2_4G_TYPE isSendChargeInform[GUN_NUM_MAX];   //是否已经发送充电通知 0: 已经发送 1: 没有发送
    uint8_t AuthFlag[GUN_NUM_MAX];
	uint8_t ScanCodeChargerFlag[GUN_NUM_MAX];
    uint32_t lastSendAuthTime;
	START_CHARGING_STR StartChargingNotice;
}PACK_SERIAL_NUM_TYPE;

//充电结束通知
typedef struct {
	uint8_t  gun_id;                            //1.
	uint8_t  user_type;                         //2.
    uint8_t  stop_reason;                       //3.结束原因
    uint8_t  reasonDetail;                      //4. 结束原因细节
    uint32_t errStatus;                         //5. 充电过程中曾经出现过的异常状态
    uint8_t  fwVer;                             //4.固件版本
	uint8_t  user_account[20];                  //5.用户账号
	uint8_t  order[ORDER_SECTION_LEN];          //6.订单号
	uint32_t startTime;                         //
	uint32_t stop_time;                         //7.结束充电时间
	uint32_t startElec;
	uint32_t stop_power;                        //8.结束充电电量
	uint32_t charger_cost;                      //9.订单费用  分
	uint32_t template_id;                       //10.费率id
	uint16_t chargingPower;                     //11.按功率段计费的功率，单位：w
}STOP_CHARGING_NOTICE_REQ_STR;

typedef struct {
    uint32_t wIndex;
	uint32_t rIndex;
}ORDER_STR;

typedef struct{
    uint8_t  station_id[8];                     // bcd, SN=10, 10 byte  0001123456789
}sys_info1;//1k

typedef struct{
	uint32_t magic_number;
	uint8_t  pcb_sn[8];
}gun_info_str3;	//1k

typedef struct{
    uint32_t fw_version_preserve;                        	// 版本号
	uint8_t  baseGunSn;							//枪头起始编号				
    uint8_t  printSwitch;                       //打印开关 0关闭  1打开
    uint8_t card_state;                         //读卡器状态  0 :表示初始化ok 1: 表示初始化出错
	char  reserve_blue_name[12];				//保留		//蓝牙名称
	char  btVersion[20];					    //蓝牙版本
    IS_START_CHARGING_BTRF2_4G_TYPE isStartCharge[GUN_NUM_MAX];   //是否需要开启充电
    EVENT_NOTICE_STR_BTRF2_4G NoticeEvent[GUN_NUM_MAX][6];   //0：读卡器通信故障； 1：保险丝熔断； 2：负载超限； 3：环境高温预警； 4：器件故障 5.拔枪
	uint16_t chargingFullTime;                  //充满续冲时间阈值 秒时
}system_info_t;	//1k

typedef struct{
	COST_TEMPLATE_REQ_STR cost_template;
	uint8_t  chargerMethod;						//计费方式 1、固定收费  2、实时收费  3、起步金收费
	uint16_t chargerStartingGold;				//起步金 add
}cost_template_str;	//1k

typedef struct{
	ORDER_STR orderInfo;                            //缓存订单信息
}gun_info_str1;	//1k

typedef struct{
	uint8_t _24gMacAddr[MAC_LEN];				//2.4网关mac地址
	uint8_t Local24gMacAddr[MAC_LEN];			//本地2.4mac地址
}gun_info_str2;	//1k

typedef struct{
	uint8_t  gun_id;
    uint8_t  gun_state;                 //1等待插枪 2充电中
    uint8_t  is_load_on;                //0:没有启动充电，非0:启动充电状态 1:充电中功率未检测  2:功率小于1.5w 3:1.5w<功率<40w 4:40w<功率
	uint8_t  user_type;
	uint8_t  order[ORDER_SECTION_LEN];
    uint16_t charger_time_plan;         //预计充电时长 分钟  至少3分钟
    uint8_t  mode;                      //计费模式 1分功率  2不分功率
    multiPower_t powerInfo;             //分功率段信息
    uint8_t  getPowerFlag;              //检测功率段次数
	uint32_t start_time;
	//uint32_t stop_time;
    uint32_t startElec;                   //电量 
    uint32_t startChargerElec;                   //开始充电时的电量 
    uint16_t money;                     //计费累计消费金额 分
    uint8_t  startMode;                 //启动模式 1刷卡  2扫码
	uint16_t current_usr_money;		    //充电金额  分
    uint8_t  stopReason;
    uint8_t  reasonDetail;              //停止充电原因细节
    uint8_t  chargingMode;              //充电模式 0：智能充满 1：按金额 2：按时长 3：按电量
	uint16_t chargingPower;             //用于功率段计费的功率 0.1w
    uint8_t  powerCheckcnt;             //功率读取次数
    uint8_t  powerSemenIndex;           //功率段指示
    uint16_t chargingElec;              //0.01kwh 累计充电电量
    time_t beginTime;
    uint16_t realChargingTime;          //实际充电时间，不含掉电时间 分钟
    //uint8_t  isSync;                    //订单开启同步状态 0-不需要同步 1-第一次启动 2--网络重连 3--重新上电
	uint8_t  chargerMethod;			    //计费方式 1、固定收费  2、实时收费  3、起步金收费
	uint8_t  maxPower;					//充电过程中最大功率 0.1w 大于10w是10w
	uint16_t startGoldTime;				//起步金时间 分钟
	uint16_t chargerStartingGold;       //起步金额 分 @2018-5-25 add

}gun_info_t;	//2k


#pragma pack()

typedef struct {

	osSemaphoreId rxSem;
	FIFO_S_t rxBtBuff;
    FIFO_S_t rx24GBuff;
	uint8_t  msgIndex; 
	
	uint32_t sendTimers ;
	uint32_t recvTimers ;
	
	uint8_t flag ;
}CKB_CTRL_STR;



extern int GetCpuTemp(void);


extern int CheckSum(uint8_t *pData, uint16_t len);
extern void PrintfData(void *pfunc, uint8_t *pdata, int len);
extern int StringToBCD(unsigned char *BCD, const char *str) ;
extern char *BCDToString(char *dest, unsigned char *BCD, int bytes) ;
extern int isArraryEmpty(uint8_t *array,int len);

extern system_info_t system_info;
extern sys_info1 gSystemInfo1;
extern gun_info_str1 gGunInfo1;
extern gun_info_str2 gGunInfo2;
extern gun_info_str3 gGunInfo3;
extern cost_template_str gCostTemplateInfo;
extern PACK_SERIAL_NUM_TYPE SerialNumInfo;
extern uint32_t ErrorCode;
extern gun_info_t gun_info[GUN_NUM_MAX];
extern CHG_INFO_STR gChgInfo;

extern CKB_CTRL_STR gCKBCtrl;



#endif
