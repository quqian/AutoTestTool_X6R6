#ifndef __RTC_H__
#define __RTC_H__

#include "includes.h"
#include "time.h"

void RTC_Init(void);
void SetRtcCount(time_t timestamp);
char* GetCurrentTime(void);
time_t GetTimeStamp(void);
/*****************************************************************************
** Function name:       GetRtcCount
** Descriptions:        //获得时间
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern inline uint32_t GetRtcCount(void);
/*****************************************************************************
** Function name:       RtcInitTime
** Descriptions:        //初始化
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern int RtcInitTime(void);

#endif

