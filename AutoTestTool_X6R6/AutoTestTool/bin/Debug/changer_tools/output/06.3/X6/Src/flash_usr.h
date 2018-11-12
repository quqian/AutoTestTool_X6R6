#ifndef __FLASH_USR_H__
#define __FLASH_USR_H__

#include "includes.h"



#define FLASH_PAGE_SIZE						1024					//页大小

#define BootFlashAdde						(uint32_t)0x00000		                //Boot运行入口地址(8K)
#define AppFlashAddr						(uint32_t)(BootFlashAdde + 0x02000)		//App运行入口地址(100K)
#define AppUpBkpAddr						(uint32_t)(AppFlashAddr + 0x19000)		//app缓存数据存储基地址(100K  头+固件)
#define SysConfigBaseAddr					(uint32_t)(AppUpBkpAddr + 0x19000)		//其他配置信息(48K)

#define SysInfoFlashAddr					(SysConfigBaseAddr)					//系统配置文件(1K)
#define SysInfoFlashAddr1					(SysInfoFlashAddr + 0x400)			//系统配置1文件(1K)
#define GunInfoFlashAddr					(SysInfoFlashAddr1 + 0x400)    		//枪头信息(1K)
#define GunInfoFlashAddr3					(GunInfoFlashAddr + 0x400)    		//枪头信息(1K)
#define GunInfoFlashAddr1					(GunInfoFlashAddr3 + 0x400)    		//枪头信息1(1K)
#define GunInfoFlashAddr2					(GunInfoFlashAddr1 + 0x400)    		//枪头信息2(1K)
#define EnergyFlashAddr	    				(GunInfoFlashAddr2 + 0x400)			//电量(1K)
#define OrderFlashAddr						(EnergyFlashAddr + 0x400)			//历史订单(30K)
#define CostTemplateFlashAddr				(OrderFlashAddr + 0x7800)			//计费模板(1K)
#define UpgradeFlashAddr					(CostTemplateFlashAddr + 0x400)   	//升级信息区(1K)
#define OtherFlashAddr						(UpgradeFlashAddr + 0x400)			//其他(9K)


typedef enum{
	FIRMWARE_PART = 0,
	SYSCONF_PART,
	SYSCONF_PART1,
	GUNINFO_PART,
	GUNINFO_PART3,
	GUNINFO_PART1,
	GUNINFO_PART2,
	ENERGY_PART,
	ORDER_PART,
	COST_TEMPLATE_PART,
	UPGRADE_PART,
	OTHER_PART,
	PART_NUM,
}PARTITION;


typedef struct {
    uint32_t s_base;  		//start addr
    uint32_t s_count; 		//section count
	uint32_t pos_w;			//写偏移量
	uint32_t pos_r;			//读偏移量
}FLASH_PART;

extern FLASH_PART flash_partition[PART_NUM];

void Flash_Init(void);
void Flash_Erase(PARTITION n);
void Flash_EraseALL(PARTITION n);
int Flash_Read(PARTITION n,void* buff, int len);
int Flash_Write(PARTITION n,void* buff, int len);
void Flash_ErasePage(PARTITION n,uint16_t page);
void Flash_SetPos_r(PARTITION n,uint32_t pos);
void Flash_SetPos_w(PARTITION n,uint32_t pos);
extern int FlashWriteData(PARTITION n,void* buff, int len);
extern void FlashWriteGunInfoData(void);
extern void FlashWriteSystemInfoData(void);
extern void FlashWriteDataToSystemInfo1(void);
extern void FlashWriteGunInfo_1_Data(void);
extern void FlashWriteGunInfo_2_Data(void);
extern void FlashWriteGunInfo_3_Data(void);
extern void FlashWriteCostTemplateData(void);

/*****************************************************************************
** Function name:       FlashWritePage
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern void FlashWritePage(PARTITION n,void* buff, int len);
/*****************************************************************************
** Function name:       WritePageDataToFlash
** Descriptions:        page：写到第几页
                        buffer：要写的数据首地址
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
extern void WritePageDataToFlash(PARTITION n, uint32_t page, uint8_t *buffer);

#endif
