#ifndef __MAIN_HANDLE_H__
#define __MAIN_HANDLE_H__

#include "stdint.h"


#define STR_MAC_LENTH                       11
#define TIME_OF_SECONDS                     2

#define CHARGING_FULL_TIME                  (45*60) //秒



#define RECORD_BLOCK_SIZE                           30720     //订单区域40960字节(30k)
#define SPI_FLASH_PAGE_SIZE					        1024    //flash页大小
#define	RECORD_BLOCK_SIZE_PER_PAGE		            128     //一条订单最大缓存
#define	PER_PAGE_RECORD_COUNT		                (uint32_t)(8)     //每页能存储的订单数
#define RECORD_MAX_BLOCK_NUM                        (uint32_t)(240)     //最多记录订单数量

extern uint32_t isCheckTimeOverFlag;

/*****************************************************************************
** Function name:       SaveOrderReport
** Descriptions:        //保存订单记录
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
extern void SaveOrderReport(uint8_t gun_id);
/*****************************************************************************
** Function name:       SendOrderReport
** Descriptions:        //发送记录订单
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
extern int SendOrderReport(void);
/*****************************************************************************
** Function name:       OpenCheckTimeOver
** Descriptions:        打开检测刷卡开启充电服务器返回是否超时, 超时播报开启失败
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern void OpenCheckTimeOver(void);

/*****************************************************************************
** Function name:       CloseCheckTimeOver
** Descriptions:        关闭检测刷卡开启充电服务器返回是否超时
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern void CloseCheckTimeOver(void);
/*****************************************************************************
** Function name:       isTheSameCard
** Descriptions:        卡不相同返回 1
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern int isTheSameCard(void);

/*****************************************************************************
** Function name:       SetReadCardFlag
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern void SetReadCardFlag(void);

/*****************************************************************************
** Function name:       CleanReadCardFlag
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern void CleanReadCardFlag(void);

/*****************************************************************************
** Function name:       GetReadCardFlag
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern int GetReadCardFlag(void);

/*****************************************************************************
** Function name:       SetReadCardTime
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern void SetReadCardTime(void);

/*****************************************************************************
** Function name:       CleanReadCardTime
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern void CleanReadCardTime(void);

/*****************************************************************************
** Function name:       GetReadCardTime
** Descriptions:        如果为 1 表示 n 秒没有读到卡, 则判断卡已经移开，如果是同一张卡，
                        说明是刷卡第二次，做这个标志的目的就是为了防止同一张卡刷两次的时候要等待 T 秒
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
extern int GetReadCardTime(void);
extern void Sc8042bSpeechStartChargerFail(void);
extern void GreenLedToggle(void);
extern void RestoreGunWithChargingCtrlProc(void);

#endif  /* __MAIN_HANDLE_H__ */

