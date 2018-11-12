
#ifndef __BLUE_H__
#define __BLUE_H__
#include "stdint.h"
#include "includes.h"

#define OUT_NET_PKT_LEN             128

enum {
    BT_SHAKE_REQ = 1,               	//握手请求
    BT_OPPO_SITE_AUTH,              	//用户反向鉴权
    BT_START_CHARGING,              	//开启充电
    BT_STOP_CHARGING,               	//结束充电
    BT_HEART_BEAT = 5,                  //遥信及心跳
    BT_COST_TEMPLATE_UPLOAD,        	//计费模版上传
    BT_REQ_COST_TEMPLATE,           	//请求计费模板
    BT_FW_UPGRADE_NOTICE,           	//固件升级开始通知//手机请求升级->X6 , X6应答->手机
    BT_FW_DOWN_LOAD,                	//固件下发
    BT_REQ_BREAK = 0x0A,                //请求断开蓝牙链接
    BT_DEV_REGISTER	= 0x0B,				//设备注册
	BT_DEV_LOGIN	= 0x0C,				//设备登陆
	BT_HISTORY_ORDER_UPLOAD=0x0D,		//历史订单上传
	BT_COST_TEMPLATE_DOWNLOAD=0x0E,		//计费模板下发
    BT_CMD_SET_CHARGER_SN = 0x0F,   	//设置充电桩编号
    BT_CMD_SET_DEVICE_ID = 0x10,           	//设置充电桩识别码
    BT_CMD_SET_GW_ADDR = 0x11,             	//设置 2.4G 网关地址
    BT_CMD_SET_SERVER_ADDR = 0x12,         	//设置服务器IP
    BT_CMD_SET_SERVER_PORT = 0x13,         	//设置服务器端口
    BT_CMD_SET_TERMINAL_INFO = 0x14,    //设置终端编号信息
    BT_CMD_REMO_CTRL,               	//远程控制
};


#pragma pack(1)

typedef struct {
    uint8_t  ab;                                //0.
    uint8_t  cd;                                //1.
    uint8_t  target;                            //2.
    uint16_t len;                               //4.
}BLUE_HEAD_RECEIVE_STR;

typedef struct {
    BLUE_HEAD_RECEIVE_STR head;
    uint8_t  data[OUT_NET_PKT_LEN];             // x6报文头 + x6静荷
}BLUE_TOOTH_RECEIVE_PKT_STR;

typedef struct {
    uint8_t  ab;                                //0.
    uint8_t  cd;                                //1.
    uint8_t  target;                            //2.
    uint8_t  addr[6];  
    uint16_t len;                               //4.
}BLUE_HEAD_SEND_STR;

typedef struct {
    BLUE_HEAD_SEND_STR head;
    uint8_t  data[OUT_NET_PKT_LEN];             // x6报文头 + x6静荷
}BLUE_TOOTH_SEND_PKT_STR;


typedef struct {
    uint8_t  start_ee;
    uint8_t  cmd;
    uint8_t  len;
}BLUE_PROTO_HEAD_STR;

typedef struct {
    BLUE_PROTO_HEAD_STR head;
    uint8_t  data[64];
    //uint8_t  checkSum;
}BLUE_PROTO_STR;

/***********手机握手请求**********/
typedef struct {
    uint32_t time;
	char phonesSn[12];
}BLUE_SHAKE_REQ_STR;

typedef struct {
    char     name[4];                               //0. 设备型号
    uint8_t  chargerSn[5];                          //1. 桩编号
    uint8_t  fwVer;                                 //2. 固件版本号
    uint8_t  portCnt;                               //3. 插座数目
    uint8_t  startNo;                               //4. 插座起始编号
}BLUE_SHAKE_ACK_STR;

typedef struct {
    uint8_t Local24gAddr[5];					    //本地2.4mac地址
}BLUE_2_4G_ADDR_STR;

typedef struct {
    uint8_t Result;
}BLUE_2_4G_ADDR_ACK_STR;

typedef struct {
    uint8_t DeviceID[8];					    //设备号
}BLUE_DEV_ID_STR;

typedef struct {
    uint8_t Result;
}BLUE_DEV_ID_ACK_STR;

typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  order[ORDER_SECTION_LEN];              //1. bcd 订单号
    uint8_t  mode;                                  //2. 0：智能充满；1：按金额，具体参数见 充电参数；2：按时长；3：按电量
    uint16_t chargingPara;                                  //3. 智能充满，为0；按金额，分辨率1分；按时长，分辨率1分钟；按电量，分辨率0.01kwh
	uint8_t orderSource;							//4. 订单来源
	uint8_t subsidyType;							//5. 补贴类型
	uint16_t subsidyPararm;							//6. 补贴参数
}BLUE_START_CHARGING_STR;

typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  result;                                //1. 0:启动成功；1: 启动失败
    uint8_t  reason;                                //2. 1：端口故障；2：没有计费模版；3：已经在充电中；4：设备没有校准；5：参数错误
	uint32_t startTime;								//3. 启动时间
	uint16_t startElec;								//4. 启动电量
}BLUE_START_CHARGING_ACK_STR;

/**********请求计费模板********/ 
typedef struct{
	uint8_t gun_id;
	uint32_t template_id;
}BLUE_COST_TEMPLATE_REQ;

/**********计费模板下发ACK*********/ 
typedef struct {
	uint8_t  result;
}BLUE_COST_TEMPLATE_ACK_STR;

typedef struct {
    uint8_t  gunId;
	uint32_t template_id;
    uint8_t  mode;                              //1：按功率段计费 2按统一收费
	uint8_t Data[256];
}COST_TEMPLATE_HEAD_STR;

typedef struct {
    uint8_t  gunId;
	uint8_t  result;
}COST_TEMPLATE_ACK_STR;

/**********固件升级********/ 
typedef struct{
	uint32_t     fw_size;
	uint32_t     package_num;                       
	uint16_t     checkSum;
    uint8_t      fw_version;
}BLUE_DOWN_FW_REQ_STR;
typedef struct {
	uint8_t result;                             //0: 升级成功  1: 接收失败 2: 校验失败 3: 写入失败
}BLUE_DOWN_FW_ACK_STR;

/**********固件下发********/ 
typedef struct {
	uint8_t data[64];
}BLUE_FW_DOWNLOAD_REQ_STR;
typedef struct {
	uint8_t result;
	uint8_t index;
}BLUE_FW_DOWNLOAD_ACK_STR;

/**********请求断开链接********/ 
typedef struct {
	uint32_t     timestamp;	
}BLUE_DISCONNECT_DEV_REQ_STR;
typedef struct {
	uint8_t     status;	
}BLUE_DISCONNECT_DEV_ACK_STR;

/**********遥信及心跳*********/ 
typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始 
    uint8_t  status;                                //1. 充电口状态：0 空闲；1 占用；2 故障；3 离线
    uint8_t  errCode;                               //2. 1~255  当充电口状态为2故障时有效；1：状态异常；2：计量芯片通信故障
}BLUE_GUN_HEART_BEAT_STR;

typedef struct {
    uint8_t  simSignal;                             //0. Sim卡信号
    uint8_t  temp;                                  //1. 环境温度 度 -50度偏移  -50~200
    uint8_t  portCnt;                               //2. 本次报文所包含的充电接口数目
    BLUE_GUN_HEART_BEAT_STR gunStatus[GUN_NUM_MAX];
}HEART_BEAT_STR;

typedef struct {
    uint32_t time;                                  //0. 系统时间
    uint8_t  result;                                //1. 0接收成功；1接收失败
}BLUE_HEART_BEAT_ACK_STR;

/***********请求结束充电*********/
typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  order[ORDER_SECTION_LEN];              //1. bcd 订单号
}BLUE_STOP_CHARGING_STR;

typedef struct {
    uint8_t  port;                                  //0. 枪口 从1开始
    uint8_t  result;                                //1. 0:成功；1: 失败
	uint32_t stopTime;								//2. 结束时间
	uint16_t stopElec;								//3. 结束电量
	uint8_t stopReason;								//4. 结束原因
	uint32_t start_time;							//开始充电时间
	uint16_t startElec;              			//0.01kwh 开始充电电量
	uint16_t money;                     			//已充电金额 分
	uint16_t chargerPower;             				//用于功率段计费的功率 w
}BLUE_STOP_CHARGING_ACK_STR;

#pragma pack()



//手机蓝牙数据接收处理
extern void RecvBtData(void);
extern uint16_t GetHistoryOrderNum(void);
extern void BlueSendHeartBeat(void);
#endif


