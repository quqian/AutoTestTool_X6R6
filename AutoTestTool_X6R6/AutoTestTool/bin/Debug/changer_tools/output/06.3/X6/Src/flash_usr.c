#include "flash_usr.h"
#include "upgrade.h"
#include "system_info.h"

FLASH_PART flash_partition[PART_NUM]=
{
	{AppUpBkpAddr,      		100,    0, 0},			//固件升级缓存区
	{SysInfoFlashAddr,  		1,      0, 0},			//系统配置文件
	{SysInfoFlashAddr1, 		1,      0, 0},			//系统配置文件1
	{GunInfoFlashAddr,  		1,      0, 0},			//枪头信息
	{GunInfoFlashAddr3, 		1,      0, 0},			//枪头信息3
	{GunInfoFlashAddr1, 		1,      0, 0},			//枪头信息1
	{GunInfoFlashAddr2, 		1,      0, 0},			//枪头信息2
	{EnergyFlashAddr,   		1,      0, 0},			//电量
	{OrderFlashAddr,    		40,     0, 0},			//订单
	{CostTemplateFlashAddr,  	1,      0, 0},          //计费模板
	{UpgradeFlashAddr,  		1,      0, 0},          //升级信息
	{OtherFlashAddr,  			9,      0, 0},          //其他部分
};

static osMutexId flash_mutex;

void Flash_Init(void)
{
	flash_mutex = osMutexCreate(NULL);
}

int Flash_Write(PARTITION n,void* buff, int len)
{
	uint32_t base = flash_partition[n].s_base + flash_partition[n].pos_w;
	uint32_t Address = base;
	uint8_t *p = buff;
	uint8_t bytes;
    uint32_t i = 0;
	uint8_t timesFlag = 0;
//    uint8_t qqqqqqqqqqqqqq = 0;
	osMutexWait (flash_mutex,1000);
	for(i = 0;i < len; i++)
    {
		while(1)
		{
			HT_Flash_ByteWrite(p,Address,1);
			HT_Flash_ByteRead(&bytes,Address,1);
			//if(1 == qqqqqqqqqqqqqq)
			//{
			//	qqqqqqqqqqqqqq = 0;
			//	printf("[%d][%d]! \n", bytes, *p);
			//}
			if(bytes == *p)
            {
				break;
			}
			//else
			//{
			//	CL_LOG("flash写数据出错[%d][%d], [%d][%d]! \n", Address, n, bytes, *p);
				//qqqqqqqqqqqqqq = 1;
			//}
			if(50 < timesFlag++)
			{
				CL_LOG("flash写数据出错[%d][%d]! \n", Address, n);
                osMutexRelease (flash_mutex);
				return 1;
			}
			Feed_WDT();
		}
		p += 1;
		Address += 1;
	}
	flash_partition[n].pos_w += len;
	osMutexRelease (flash_mutex);

	return 0;
}

int FlashWriteData(PARTITION n,void* buff, int len)
{
	while(1)
	{
		Flash_EraseALL(n);
		if(0 == Flash_Write(n, buff, len))
		{
			break;
		}
		else
		{
			Feed_WDT();
			CL_LOG("flash[%d]写数据! \n", n);
			//osDelay(100);
		}
	}
    return 0;
}

void FlashWriteGunInfoData(void)
{
	FlashWriteData(GUNINFO_PART, gun_info, sizeof(gun_info));
}

void FlashWriteGunInfo_1_Data(void)
{
	FlashWriteData(GUNINFO_PART1, (void*)&gGunInfo1, sizeof(gGunInfo1));
}

void FlashWriteGunInfo_2_Data(void)
{
	FlashWriteData(GUNINFO_PART2, (void*)&gGunInfo2, sizeof(gGunInfo2));
}

void FlashWriteGunInfo_3_Data(void)
{
	FlashWriteData(GUNINFO_PART3, (void*)&gGunInfo3, sizeof(gGunInfo3));
}

void FlashWriteCostTemplateData(void)
{
	FlashWriteData(COST_TEMPLATE_PART, (void*)&gCostTemplateInfo, sizeof(gCostTemplateInfo));
}

void FlashWriteSystemInfoData(void)
{
	FlashWriteData(SYSCONF_PART, (void*)&system_info, sizeof(system_info));
}

void FlashWriteDataToSystemInfo1(void)
{
	FlashWriteData(SYSCONF_PART1, (void*)&gSystemInfo1, sizeof(gSystemInfo1));
}

#if 0
/*****************************************************************************
** Function name:       FlashWritePage
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void FlashWritePage(PARTITION n,void* buff, int len)
{
    uint32_t base = flash_partition[n].s_base + flash_partition[n].pos_w;
    uint32_t Address = base;
    uint8_t *p = buff;
    uint8_t bytes;
    osMutexWait (flash_mutex,1000);
    for(int i = 0;i < len;i++)
    {
        while(1)
        {
            HT_Flash_ByteWrite(p,Address,1);
            HT_Flash_ByteRead(&bytes,Address,1);
            if(bytes == *p)
            {
                break;
            }
        }
        p += 1;
        Address += 1;
    }
    flash_partition[n].pos_w += len;
    osMutexRelease (flash_mutex);
}

/*****************************************************************************
** Function name:       WritePageDataToFlash
** Descriptions:        page：写到第几页
                        buffer：要写的数据首地址
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void WritePageDataToFlash(PARTITION n, uint32_t page, uint8_t *buffer)
{
    uint32_t Address = flash_partition[n].s_base;
	uint8_t *pbuffer = buffer;
	uint32_t i = 0;
	uint8_t bytes;

    osMutexWait (flash_mutex,1000);
	//擦除App存储页面并写入数据//擦除一页
	HT_Flash_PageErase(Address + page * FLASH_PAGE_SIZE);

	//写入一页的数据
	for(i = 0; i < FLASH_PAGE_SIZE; i++)
    {
		//每次写入1个byte的数据
		while(1)
        {
			HT_Flash_ByteWrite(pbuffer, Address + page * FLASH_PAGE_SIZE + i, 1);
			HT_Flash_ByteRead(&bytes,Address + page * FLASH_PAGE_SIZE + i,1);
			if(bytes == *pbuffer)
            {
				break;
			}
		}
		pbuffer += 1;
	}
    osMutexRelease (flash_mutex);
}

#endif

int Flash_Read(PARTITION n,void* buff, int len)
{
	uint32_t Address = flash_partition[n].s_base + flash_partition[n].pos_r;
	osMutexWait (flash_mutex,1000);
	HT_Flash_ByteRead(buff,Address,len);
	flash_partition[n].pos_r += len;
	osMutexRelease (flash_mutex);
	return len;
}

void Flash_EraseALL(PARTITION n)
{
	uint32_t Address = flash_partition[n].s_base;
	uint32_t pages = flash_partition[n].s_count;
	osMutexWait (flash_mutex,1000);

	for(int i = 0;i < pages;i++)
    {
		HT_Flash_PageErase(Address+i*FLASH_PAGE_SIZE);
	}
	flash_partition[n].pos_r = 0;
	flash_partition[n].pos_w = 0;
	osMutexRelease (flash_mutex);
}


void Flash_ErasePage(PARTITION n,uint16_t page)
{
	uint32_t Address = flash_partition[n].s_base;
	osMutexWait (flash_mutex,1000);
	if(page > flash_partition[n].s_count)
    {
        CL_LOG("页的长度大于区域的最大长度! \n");
        return;
    }
	HT_Flash_PageErase(Address+page*FLASH_PAGE_SIZE);
	osMutexRelease (flash_mutex);
}


void Flash_SetPos_r(PARTITION n, uint32_t pos)
{
	osMutexWait (flash_mutex,1000);
	flash_partition[n].pos_r = pos;
	osMutexRelease (flash_mutex);
}

void Flash_SetPos_w(PARTITION n,uint32_t pos)
{
	osMutexWait (flash_mutex,1000);
	flash_partition[n].pos_w = pos;
	osMutexRelease (flash_mutex);
}

