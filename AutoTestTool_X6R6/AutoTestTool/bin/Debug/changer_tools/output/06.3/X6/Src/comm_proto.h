#ifndef __COMM_PROTO_H__
#define __COMM_PROTO_H__

#include "includes.h"
#include "24g_proto.h"


#define COM_OUT_NET_PKT_LEN                 150
#define MAX_TERMINAL_NUM                8

typedef enum {
    FIND_AA,
    FIND_55,
    FIND_CHARGER_TYPE,
    FIND_CHAGER_SN,
    FIND_SRC,
    FIND_DEST,
    FIND_LEN,
    FIND_VER,
    FIND_SERNUM,
    FIND_CMD,
    RX_DATA,
    FIND_CHK,
    FIND_END,
} PROTO_MSG_STR;


enum{
	DEBUG_CMD_CARD_TEST = 0x02,              //刷卡测试   
	DEBUG_CMD_TRUMPTE = 0x05,		//
	DEBUG_CMD_RELAY = 0x06,
	DEBUG_CMD_WRITEPCB = 0x07,	
    DEBUG_CMD_SN = 0x08,
    DEBUG_CMD_BLUE = 0x09,
    DEBUG_CMD_GETSVER = 0x0A,
    DEBUG_CMD_READPCB = 0x0B,
    DEBUG_CMD_DEV_TYPE  = 0x0d,                     //0x0d
    DEBUG_CMD_GW_ADDR= 0x0E,                              //0x0e
    DEBUG_CMD_TER_MINAL= 0x0F,                            //0x0f
    DEBUG_CMD_SERVER_ADDR= 0x10,                          //0x10
    DEBUG_CMD_SERVER_PORT= 0x11,                          //0x11
    DEBUG_CMD_PRINTF = 0x12,                               //0x12
    DEBUG_CMD_REBOOT = 0x13,                               //0x13
    DEBUG_CMD_DEVICE_ID = 0x14,                            //0x14

	DEBUG_CMD_SETRTC = 0x15,
	DEBUG_CMD_READRTC = 0x16,
	DEBUG_CMD_FLASHTEST = 0x17,
	DEBUG_CMD_LED = 0x1A,
	DEBUG_CMD_SERVER_COM	= 0x1C,
	DEBUG_CMD_GETSN			= 0x1E,
	DEBUG_CMD_GETICODE		= 0x1F,
	DEBUG_CMD_AGE_TEST		= 0x20,				//启动老化测试
	DEBUG_CMD_GET_AGE_RESULT = 0x21,			//获取老化测试结果
	
    DEBUG_CMD_UPGRADE_CMD = 0xF1,                          //固件升级
    DEBUG_CMD_UPGRADE_DOWN_LOAD = 0xF2,                    //固件下发
    DEBUG_CMD_24G_COMMUNICATION = 0xF3,                    //24G通信测试
    DEBUG_CMD_TEST = 0x99,
};

#pragma pack(1)

typedef struct {
    uint8_t  aa;                                    //0.
    uint8_t  five;                                  //1.
    uint8_t  src[5];                                //2. 桩号   Bcd
    uint8_t  dest[5];                               //3.
    uint16_t len;                                   //4.
    uint8_t  ver;                                   //5.
    uint8_t  sn;                                    //6.
    uint8_t  cmd;                                   //7.
}OUT_NET_HEAD_STR;

typedef struct{
	uint32_t     fw_size;
	uint32_t     package_num;                       
	uint16_t     checkSum;
    uint8_t      fw_version;
}DEBUG_DOWN_FW_STR;

typedef struct {
    OUT_NET_HEAD_STR head;
    uint8_t  data[COM_OUT_NET_PKT_LEN];
}OUT_PKT_STR;


typedef struct {
    uint8_t  chargerSn[MAC_LEN];
}CHARGER_STR;

typedef struct {
    uint8_t  cnt;                               //x6数目
    CHARGER_STR chargerInfo[MAX_TERMINAL_NUM];  //X6编号
}TERMINAL_STR;


#pragma pack()


extern void DebugPktProc(OUT_PKT_STR *pBuff, uint16_t len);
extern void SendTestPktAck(uint8_t cmd, uint8_t *pkt, uint8_t len);

#endif
