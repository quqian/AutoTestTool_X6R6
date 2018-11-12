#ifndef __EMU_INFO_H__
#define __EMU_INFO_H__


#include "includes.h"
#include "emu.h"


#define FLAG_ELEC 0xAA345678

typedef struct{

    uint32_t flag;
    uint32_t elec[GUN_NUM_MAX];
}EE_ENERGY_ST;




typedef struct{
    uint8_t  status;            //bit0:通信故障 bit1:是否过流 bit2:是否过压
    uint8_t  temp;              //温度 枪头温度 度 -50度偏移  -50~200
    uint16_t voltage;           //电压 0.1v
	uint16_t current;           //电流 1mA
	uint16_t power;             //功率 0.1w
	uint32_t elec;              //电量 0.01kwh
}GUN_STATUS_STR;

typedef struct{
	uint8_t  fuseCnt;
    uint8_t  lowvolCnt;
    uint8_t  overvolcount;
    uint8_t  overcurcount;
}EMU_CTRL_STR;


extern void CalcuElec(void);
extern void SaveElec(void);
extern void ReadElec(void);
extern void ProcessElec(void);
extern int GetGunStatus(int gunId, GUN_STATUS_STR *pGunStatus);


extern GUN_STATUS_STR gun[GUN_NUM_MAX];
extern EMU_CTRL_STR  gEmuCtrl[GUN_NUM_MAX];

#endif
