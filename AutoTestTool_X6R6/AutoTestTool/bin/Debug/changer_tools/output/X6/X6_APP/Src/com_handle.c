/*
*********************************************************************************************************
*                                              上位机通信任务
*                                         
* File         : 
* By           : 
* Version      : 
* Description  : 通过上位机进行系统配置
*********************************************************************************************************
*/


#include "com_handle.h"
#include "uart.h"
#include "comm_proto.h"
#include "system_info.h"
#include "charger.h"
#include "pmu.h"
#include "emu_info.h"
#include "CardAuth_handle.h"



void ComTask(void *argc)
{
	static uint8_t  step = FIND_AA;
    static uint8_t  buff[COM_OUT_NET_PKT_LEN];
    static uint16_t pktLen;
    static uint16_t len;
    static uint8_t  sum;
    uint8_t data;
    uint8_t *pBuff = buff;
	
	UART3_Init(19200);
	
	while(1)
    {
		#if 1
		//掉电检测
        if((HT_PMU->PMUSTA&0x08) == 0)
		{
			printf("[进入Enter_StandBy, ComTask]\n");
			Enter_StandBy();
		}
		#endif
		Feed_WDT();

		GreenLedToggle();
		Sc8042bSpeechStartChargerFail();
		//处理电量
        ProcessElec();
		RestoreGunWithChargingCtrlProc();
		
		//Feed_WDT();
		while (CL_OK == FIFO_S_Get(&gUart3Handle.rxFIFO, &data))
        {
			switch (step)
            {
				case FIND_AA:
					if (0xaa == data) 
					{
						pktLen = 0;
						pBuff[pktLen++] = data;
						step = FIND_55;
					}
					break;

				case FIND_55:
					if (0x55 == data) 
					{
						step = FIND_SRC;
						pBuff[pktLen++] = data;
						len = 0;
					}
					else 
					{
						step = FIND_55;
					}
					break;

				case FIND_SRC:
					pBuff[pktLen++] = data;
					if (5 <= ++len) 
					{
						step = FIND_DEST;
						len = 0;
					}
					break;

				case FIND_DEST:
					pBuff[pktLen++] = data;
					if (5 <= ++len) 
					{
						step = FIND_LEN;
						len = 0;
					}
					break;

				case FIND_LEN:
					pBuff[pktLen++] = data;
					if (2 <= ++len) 
					{
						len = (pBuff[pktLen-1]<<8) | pBuff[pktLen-2];
						if (COM_OUT_NET_PKT_LEN <= len)
						{
							CL_LOG("len=%d,error.\n",len);
							step = FIND_AA;
						}
						else
						{
							//CL_LOG("len=%d.\n",len);
							step = FIND_VER;
							sum = 0;
						}
					}
					break;

				case FIND_VER:
					pBuff[pktLen++] = data;
					sum += data;
					step = FIND_SERNUM;
					break;

				case FIND_SERNUM:
					pBuff[pktLen++] = data;
					sum += data;
					step = FIND_CMD;
					break;

				case FIND_CMD:
					pBuff[pktLen++] = data;
					sum += data;
					len = (len >= 4) ? (len - 4) : 0;
					step = len ? RX_DATA : FIND_CHK;
					break;

				case RX_DATA:
					pBuff[pktLen++] = data;
					sum += data;
					if (0 == --len)
					{
						step = FIND_CHK;
					}
					break;

			   case FIND_CHK:
					if (sum == data) 
					{
						//PrintfData("DebugRecvProc", pBuff, pktLen);
						DebugPktProc((OUT_PKT_STR*)pBuff, pktLen);
					}
					else
					{
						CL_LOG("sum=%02x,pkt sum=%02x,error,drop pkt.\n",sum,data);
					}
					step = FIND_AA;
					break;
			}		 
		}
		
		osDelay(100);
	}
}	



