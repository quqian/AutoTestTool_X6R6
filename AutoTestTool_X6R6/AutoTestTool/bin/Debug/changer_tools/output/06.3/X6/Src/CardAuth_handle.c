#include "CardAuth_handle.h"
#include "system_info.h"
#include "sc8042b.h"
#include "main_handle.h"
#include "emu_info.h"
#include "pmu.h"



/*****************************************************************************
** Function name:       CardAuthTask
** Descriptions:        //刷卡任务
** input parameters:    argc
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void CardAuthTask(void *argc)
{
	while(1)
    {
        #if 1
		uint32_t authCardTicks = 0;
        CleanReadCardFlag();    //清除读到卡标志
        CleanReadCardTime();    //清除标志
        
        if (0 == system_info.card_state &&  (osGetTimes() - authCardTicks) >= 200)
        {
            //读卡处理
            ReadCardHandle();
            authCardTicks = osGetTimes();
        }
		#endif
        if(((1 == GetReadCardFlag()) && (1 == isTheSameCard())) || 
            (((1 == GetReadCardFlag())) && (15 < ((uint32_t)(GetRtcCount() - SerialNumInfo.lastSendAuthTime)))) ||
            (1 == GetReadCardTime()))
        {
            Sc8042bSpeech(VOIC_CARD);
            //osDelay(80);
            SendCardAuthReq_24G(CARD_AUTH);
        }

		if(0xae == gChgInfo.gun_id[0])
		{
			//ACK
			StartChargingAckProc_24G(1, 0, MANY_TIMES);
			gChgInfo.gun_id[0] = 0;
			printf("gChgInfo.gun_id[0]\n");
		}
		else if(0xae == gChgInfo.gun_id[1])
		{
			//ACK
			StartChargingAckProc_24G(2, 0, MANY_TIMES);
			gChgInfo.gun_id[1] = 0;
			printf("gChgInfo.gun_id[0]\n");
		}
        
        //是否需要开启充电
        isStartChargingBTRF2_4G();
		
		#if 1
		//掉电检测
		if((HT_PMU->PMUSTA&0x08) == 0)
		{
			printf("[进入Enter_StandBy, CardAuthTask]\n");
			Enter_StandBy();
		}
		#endif
		
		osDelay(100);
	}
}	


