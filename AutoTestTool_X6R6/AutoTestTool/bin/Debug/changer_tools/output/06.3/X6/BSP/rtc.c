#include <time.h>
#include "rtc.h"

__IO uint32_t gSysSecond = 0;

void RTC_Init(void)
{
    RTC_InitTypeDef  RTC_InitStructure;

    RTC_InitStructure.RTC_TMR1.NewState = ENABLE;               /*!< 使能RTC定时器1              */
    RTC_InitStructure.RTC_TMR1.Count = 5;                       /*!< 定时周期6s                  */
    RTC_InitStructure.RTC_TMR2.NewState = DISABLE;              /*!< 关闭RTC定时器2              */
    RTC_InitStructure.RTC_TMR1.Count = 0;
    RTC_InitStructure.RTC_Alarm.Muster.hour = 0x99;             /*!< 不产生闹钟                  */
    RTC_InitStructure.RTC_Alarm.Muster.minute = 0x99;
    RTC_InitStructure.RTC_TOUT = Tout1Hz;                       /*!< Tout输出1Hz                 */
    HT_RTC_Init(&RTC_InitStructure);
	
	//RTC补偿参数
	EnWr_WPREG();
	HT_TBS->TBSCON = 0x0301;
	HT_TBS->TBSIE = 0x00;
	HT_TBS->TBSPRD = 0x00;
	if(HT_RTC_LoadInfoData() == TRUE)
	{
		CL_LOG("HT_RTC_LoadInfoData OK \n");
	}
	else
	{
		CL_LOG("HT_RTC_LoadInfoData Failed \n");
	}
	HT_RTC_ITConfig(RTC_RTCIE_SECIE, ENABLE);
    NVIC_EnableIRQ(RTC_IRQn);
}

static char gTimeStr[16];

//void RTC_IRQHandler(void)
//{
//    if (HT_RTC->RTCIF & 0x01) {
//        HT_RTC->RTCIF &= (~0x01);
//        gSysSecond++;
//    }
//}

char* GetCurrentTime(void)
{
    uint8_t day[8];

    HT_RTC_Read(day);
    sprintf(gTimeStr,"%02u-%02u %02u:%02u:%02u", day[2], day[3], day[4], day[5], day[6]);
    return gTimeStr;
}

int LinuxTickToDay(uint32_t tick, uint8_t *pDay)
{
    struct tm *time_now;
    time_t time_unix;

	time_unix = tick;
	time_now = localtime(&time_unix);
    pDay[0] = time_now->tm_wday;
    pDay[1] = time_now->tm_year - 100;
    pDay[2] = time_now->tm_mon+1;
    pDay[3] = time_now->tm_mday;
    pDay[4] = time_now->tm_hour;
    pDay[5] = time_now->tm_min;
    pDay[6] = time_now->tm_sec;
    return CL_OK;
}


//	
void SetRtcCount(time_t timestamp)
{
    uint8_t day[8];

    LinuxTickToDay(timestamp+8*60*60, day);
	
    HT_RTC_Write(day);
}


time_t GetTimeStamp(void)
{
	struct tm time_now;
	memset(&time_now,0,sizeof(struct tm));
	uint8_t day[8];
    HT_RTC_Read(day);
	time_now.tm_wday = day[0];
	time_now.tm_year = day[1]+100;
	time_now.tm_mon =  day[2]-1;
	time_now.tm_mday = day[3];
	time_now.tm_hour = day[4]-8;
	time_now.tm_min =  day[5];
	time_now.tm_sec =  day[6];
	
	return mktime(&time_now);
}

/*****************************************************************************
** Function name:       GetRtcCount
** Descriptions:        //获得时间
** input parameters:    None
** output parameters:   None
** Returned value:	  	None
** Author:              quqian
*****************************************************************************/
inline uint32_t GetRtcCount(void)
{
	#if 1
	return GetTimeStamp();
	#else
    return gSysSecond;
	#endif
}


