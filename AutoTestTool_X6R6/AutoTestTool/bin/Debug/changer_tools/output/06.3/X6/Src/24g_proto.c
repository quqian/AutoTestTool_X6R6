#include "24g_proto.h"
#include "charger.h"
#include "system_info.h"
#include "ckb24_um_handle.h"
#include "sc8042b.h"
#include "emu_info.h"
#include "flash_usr.h"
#include "upgrade.h"
#include "rtc.h"
#include "CardAuth_handle.h"
//#include "main_handle.h"
#include "blue.h"

/*****************************************************************************
** Function name:       MuxSempTake
** Descriptions:        
** input parameters:    
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int MuxSempTake(uint8_t *pSemp)
{
    while (1 == *pSemp)
    {
        Feed_WDT();
        vTaskDelay(30);
    }
    *pSemp = 1;
    return CL_OK;
}

/*****************************************************************************
** Function name:       MuxSempGive
** Descriptions:        
** input parameters:    
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void MuxSempGive(uint8_t *pSemp)
{
    *pSemp = 0;
}

int GetPktSum(uint8_t *pData, uint16_t len)
{
    int i;
    uint8_t sum = 0;

    for (i=0; i<len; i++) {
        sum += pData[i];
    }
    return sum;
}

/*****************************************************************************
** Function name:       CheckUpgradeWaitTimer
** Descriptions:        //升级过程中，大于50s没有接收到数据说明 R6 已经断电或离线了
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void CheckUpgradeWaitTimer(void)
{
    if(1 == GetUpgradeReqestFlag())    //如果2.4G处于升级状态
    {   
        if(osGetTimes() >= gChgInfo.gUpgradeWaitTimerTick)
        {
            if (osGetTimes() - gChgInfo.gUpgradeWaitTimerTick >= 30000)  //大于30s说明 R6 已经断电或离线了
            {
                OpenUpgradeReqestFlag();
                gChgInfo.gUpgradeWaitTimerTick = osGetTimes();
            }
        }
        else    //计数器溢出了
        {   
            if (0xffffffff - gChgInfo.gUpgradeWaitTimerTick + osGetTimes() >= 30000)  //大于30s说明 R6 已经断电或离线了
            {
                OpenUpgradeReqestFlag();
                gChgInfo.gUpgradeWaitTimerTick = osGetTimes();
            }
        }
    }
    
    return;
}

/*****************************************************************************
** Function name:       ClearUpgradeWaitTimer
** Descriptions:        //重新复位等待时间
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void ClearUpgradeWaitTimer(void)
{
    gChgInfo.gUpgradeWaitTimerTick = osGetTimes();   //重新复位等待时间
    
    return;
}

/*****************************************************************************
** Function name:       PackSend24GData
** Descriptions:        打包内层数据包
** input parameters:    proto:外层数据的data部分
                        cmd:相应操作事件的命令
                        len:内层协议数据data部分, 不包含check_sum
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void PackSend24GData(PROTO_STR_BTRF24G_TYPE *proto,uint8_t cmd,uint16_t len, uint8_t SerialNum)
{
    uint8_t *p = (void*)&proto->head.SerialNum;

    //内层协议
    proto->head.ee = 0xee;
    proto->head.len = INNER_VER_CMD_CHECKSUM + len;  //这里+3表示, ver+cmd+check_sum
    proto->head.SerialNum = SerialNum;
    proto->head.cmd = cmd;
    
    proto->data[len] = CheckSum(p, INNER_VER_CMD+len);

    return;
}


/*****************************************************************************
** Function name:       SendProtoPkt_BTRF2_4G
** Descriptions:        打包内层数据包
** input parameters:    cmd:相应操作事件的命令
                        pkt:外层数据包头指针
                        len:内层协议数据data部分, 不包含check_sum
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int SendProtoPkt_BTRF2_4G(uint8_t cmd,BLUE_RF2_4G_PKT_STR *pkt,uint16_t len,uint8_t SerialNum)
{
	PROTO_STR_BTRF24G_TYPE *proto = (void*)pkt->data;

    Feed_WDT();
	//内层协议
	PackSend24GData(proto,cmd,len,SerialNum);
	//外层
	pkt->head.len = sizeof(PROTO_HEAD_BTRF24G_TYPE) + len + INNER_CHECK_SUM;
	
	return SendBlueTooth2_4GPkt(NODE_24G,pkt,pkt->head.len);
}
/*****************************************************************************
** Function name:       SpeechSelectGun
** Descriptions:        语音播报让用户选择枪口
** input parameters:    
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void SpeechSelectGun(uint8_t  gun_id)
{
    uint8_t tmp = 0;
    
    Sc8042bSpeech(VOIC_PLUG_IN_PLUG);
    vTaskDelay(700);
    tmp = gun_id / 10;
    switch(tmp)
    {
        case 0:
            Sc8042bSpeech(gun_id % 10 + VOIC_0);
            vTaskDelay(250);
        break;
        case 1:
            Sc8042bSpeech(VOIC_TEN);
            vTaskDelay(250);
            tmp = gun_id % 10;
            if (tmp != 0) {
                Sc8042bSpeech(tmp + VOIC_0);
                vTaskDelay(250);
            }
        break;
        case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:
            Sc8042bSpeech(tmp + VOIC_0);
            vTaskDelay(250);
            Sc8042bSpeech(VOIC_TEN);
            vTaskDelay(250);
            tmp = gun_id % 10;
            if (tmp != 0) {
                Sc8042bSpeech(tmp + VOIC_0);
                vTaskDelay(250);
            }
        break;
        default:
        break;
    }
    Sc8042bSpeech(VOIC_HAO_CHAZUO);
    vTaskDelay(300);
}

/*****************************************************************************
** Function name:       SendCardAuthReq_24G
** Descriptions:        //卡鉴权
** input parameters:    flag : 0：刷卡鉴权，开始充电；1：仅查询余额；2：手机用户反向鉴权
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
uint8_t gCardReqFlag = 0;
int SendCardAuthReq_24G(int flag)
{
	BLUE_RF2_4G_PKT_STR pkt;
	PROTO_STR_BTRF24G_TYPE *proto = (void*)pkt.data;
	CARD_AUTH_REQ_STR_24G *pMsg = (void*)proto->data;
    uint8_t pCardId[8];
    uint32_t i = 0;
    
    memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
    
    //Sc8042bSpeech(VOIC_READING_CARD);
    //播放语音, 请取走您的卡片
    #if 1
    if((GUN_CHARGING_IDLE < (gun_info[0].is_load_on)) && (GUN_CHARGING_IDLE < (gun_info[1].is_load_on)))
    {
        Sc8042bSpeech(VOIC_SOCKET_OCCUPID);
        vTaskDelay(250);
        return 1;
    }
    #endif
    if(GUN_CHARGING_IDLE < (gun_info[0].is_load_on))
    {
        gChgInfo.current_usr_gun_id = 2;
    }
    else
    {
        gChgInfo.current_usr_gun_id = 1;    //选择1 号枪头
    }
    #if 0//TEST_CARD_AUTH
    BTRF2_4gStartCharging(gChgInfo.current_usr_gun_id, gChgInfo.money, system_info.isStartCharge[gChgInfo.current_usr_gun_id - 1].order, 
                    system_info.isStartCharge[gChgInfo.current_usr_gun_id - 1].isStartChargeSN);
    SpeechChargeing(gChgInfo.current_usr_gun_id + system_info.baseGunSn - 1);
    return 1;
    #endif
    pMsg->gun_id = gChgInfo.current_usr_gun_id + system_info.baseGunSn - 1;
    
    StringToBCD(pCardId, (const char *)gChgInfo.current_usr_card_id);
    memset(pMsg->card_id, 0,sizeof(pMsg->card_id));
    memcpy(&pMsg->card_id[3], pCardId, 5);
    //PrintfData("gChgInfo.current_usr_card_id卡号",gChgInfo.current_usr_card_id,16);
    PrintfData("卡号",&pMsg->card_id[3],5);
	//memcpy((void*)pMsg->card_id, (void*)gChgInfo.current_usr_card_id, sizeof(pMsg->card_id));
	memset(pMsg->card_psw, 0, sizeof(pMsg->card_psw));
    if (CARD_CHECK_MONEY == flag) {
        gCardReqFlag = 0;
		pMsg->optType = flag;
	}
    else
    {
		gCardReqFlag = 1;
		pMsg->optType = 0;
		if (CARD_AUTH == flag)
        {
			pMsg->mode = 1;
			pMsg->chargingPara = 2*100;
	    }
        else if (CARD_MONTH == flag) {//月卡
			pMsg->mode = 0;
            pMsg->chargingPara = 0;
		}
	}
    
    OpenCheckTimeOver();
    SerialNumInfo.AuthFlag[gChgInfo.current_usr_gun_id - 1] = 1;
    #if 1
    for(i = 0; i < (RESENT_24G_MSG_CNT * 4); i++)
    {
    	//if(gChgInfo.isOverTimeFlag == 0)
		//{
		//	CL_LOG("已经开启充电 或 开启充电已经超时 .\n");
		//	break;
		//}
        if (0 == (i % RESENT_24G_MSG_CNT))
        {
            SendProtoPkt_BTRF2_4G(CMD_CARD_AUTH_24G, &pkt, sizeof(CARD_AUTH_REQ_STR_24G), SerialNumInfo.AuthSerialNum);
            PrintfData("发送鉴权请求", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(CARD_AUTH_REQ_STR_24G) + 3);
        }
        
        vTaskDelay(RETRY_24G_DELAY_TIME);
        if(0 == SerialNumInfo.AuthFlag[gChgInfo.current_usr_gun_id - 1])
        {
            break;
        }
    }
    #endif
	SerialNumInfo.AuthSerialNum++;
    SerialNumInfo.lastSendAuthTime = GetRtcCount();
    
    return CL_OK;
}


/*****************************************************************************
** Function name:       SendHeartBat_BTRF2_4G
** Descriptions:        //心跳通知 如果在等待发送开启充电通知那就发送充电通知, 
                        如果在等待发送充电结束通知那就发送充电结束通知，
                        如果充电模板有问题就发送请求更新充电模板
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int SendHeartBat_BTRF2_4G(void)
{
	static uint8_t timesFlag = 0;
	static uint8_t timesFuseFlag = 0;
    uint8_t gun_id = 0;
    int ret = 0;
    uint8_t tempHight = 0;
    
    for(gun_id = 1;gun_id <= GUN_NUM_MAX; gun_id++)
    {
        Feed_WDT();
        //if(0 == SendOrderReport())
        //{
        //    return CL_OK;
        //}
        if(3 < timesFlag++)
		{
			timesFlag = 0;
			if (system_info.NoticeEvent[gun_id - 1][4].status == EVENT_OCCUR)
	        {
	            system_info.NoticeEvent[gun_id - 1][4].status = EVENT_RECOVER;
	            SendEventNotice_BTRF2_4G(gun_id, 
	                system_info.NoticeEvent[gun_id - 1][4].code, 
	                system_info.NoticeEvent[gun_id - 1][4].para1, 
	                system_info.NoticeEvent[gun_id - 1][4].para2, 
	                system_info.NoticeEvent[gun_id - 1][4].status,
	                system_info.NoticeEvent[gun_id - 1][4].level);
	            return CL_OK;
	        }
        	if (system_info.NoticeEvent[gun_id - 1][5].status == EVENT_OCCUR)
	        {
	            system_info.NoticeEvent[gun_id - 1][5].status = EVENT_RECOVER;
	            SendEventNotice_BTRF2_4G(gun_id, 
	                system_info.NoticeEvent[gun_id - 1][5].code, 
	                system_info.NoticeEvent[gun_id - 1][5].para1, 
	                system_info.NoticeEvent[gun_id - 1][5].para2, 
	                system_info.NoticeEvent[gun_id - 1][5].status,
	                system_info.NoticeEvent[gun_id - 1][5].level);
	            return CL_OK;
	        }
		}
    }
    //发送事件通知（0x0F）
    ret = GetCpuTemp();
    if (65 < ret)
    {
        TempNotice(ret);
        if (0 == tempHight)
        {
            tempHight = 1;
            SendEventNotice_BTRF2_4G(0, EVENT_ENV_TEMP_HIGH, ret, 0, EVENT_OCCUR, EVENT_ALARM);
            return CL_OK;
        }
    }
    else if (ret < 55) {
        if (1 == tempHight)
        {
            tempHight = 0;
            SendEventNotice_BTRF2_4G(0, EVENT_ENV_TEMP_HIGH, ret, 0, EVENT_RECOVER, EVENT_PROMPT);
            return CL_OK;
        }
    }
	if ((system_info.NoticeEvent[0][1].status == EVENT_OCCUR) && (2 < timesFuseFlag++))
    {
    	timesFuseFlag = 0;
        system_info.NoticeEvent[0][1].status = EVENT_RECOVER;
        SendEventNotice_BTRF2_4G(0, 
            system_info.NoticeEvent[0][1].code, 
            system_info.NoticeEvent[0][1].para1, 
            system_info.NoticeEvent[0][1].para2, 
            system_info.NoticeEvent[0][1].status,
            system_info.NoticeEvent[0][1].level);
        return CL_OK;
    }
    //发送心跳ACK
    X6ToR6HeartBatACK_BTRF2_4G();
    
	return CL_OK;
}

/*****************************************************************************
** Function name:       SendStartChargingNoticeBTRF_24G
** Descriptions:        //RF2_4G开启充电通知
** input parameters:    gun_id: 枪号
                        startTime: 充电开始时间
                        result: 充电结果
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int SendStartChargingNoticeBTRF_24G(int gun_id, uint32_t startTime, uint8_t result,uint8_t SerialNum)
{
	BLUE_RF2_4G_PKT_STR pkt;
	PROTO_STR_BTRF24G_TYPE *proto = (void*)pkt.data;
	START_CHARGING_NOTICE_STR_24G_TYPE * pMsg = (void*)proto->data;
	gun_info_t *pGunInfo = &gun_info[gun_id-1];
    
    memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
	pMsg->gun_id = gun_id + system_info.baseGunSn - 1;
	memcpy((void*)pMsg->order, (void*)pGunInfo->order, ORDER_SECTION_LEN);
	pMsg->start_time  = startTime;
	pMsg->result  = result;
    PrintfData("RF2_4G开启充电通知订单号",pMsg->order,ORDER_SECTION_LEN);
    printf("gun_id = %d",gun_id);
	SendProtoPkt_BTRF2_4G(CMD_CHARGER_STARTED_24G,&pkt,sizeof(START_CHARGING_NOTICE_STR_24G_TYPE), SerialNum);
    PrintfData("RF2_4G开启充电通知", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(START_CHARGING_NOTICE_STR_24G_TYPE));
	return CL_OK;
}

void SendStartChargingNotice(void)
{
    uint32_t i = 0;
    uint8_t result = 0;
    static uint8_t gun_id = 1;
	static uint8_t timesFlag[GUN_NUM_MAX] = {0,};
	
	for(gun_id = 1; gun_id <= GUN_NUM_MAX; gun_id++)
	{
		if((0xab == SerialNumInfo.StartChargingNotice.StartChargeFlag[gun_id - 1]) && 
			(20 > timesFlag[gun_id - 1]) && (GUN_CHARGING_IDLE < gun_info[gun_id - 1].is_load_on))
		{
			timesFlag[gun_id - 1]++;
			SerialNumInfo.isSendChargeInform[gun_id - 1].isStartChargeFlag = 1;   //保存开启充电通知标志
			for(i = 0; i < (RESENT_24G_MSG_CNT * 2); i++)
			{
				if (0 == (i % RESENT_24G_MSG_CNT))
				{
					CL_LOG("发送开启充电通知 , gun_id = %d \r\n",gun_id);
					SendStartChargingNoticeBTRF_24G(gun_id, gun_info[gun_id - 1].start_time, result, 
													SerialNumInfo.StartChargingNoticeSN);
				}
				
				vTaskDelay(40);
				if(0 == SerialNumInfo.isSendChargeInform[gun_id - 1].isStartChargeFlag)
				{
					SerialNumInfo.StartChargingNotice.StartChargeFlag[gun_id - 1] = 0;
					break;
				}
			}
			SerialNumInfo.StartChargingNoticeSN++;
		}
		else
		{
			timesFlag[gun_id - 1] = 0;
			SerialNumInfo.StartChargingNotice.StartChargeFlag[gun_id - 1] = 0;
		}
	}
}

/*****************************************************************************
** Function name:       CleanStopChargingNotice_BTRF2_4G
** Descriptions:        //RF2_4G清除停止充电通知到flash
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int CleanStopChargingNotice_BTRF2_4G(uint8_t gun_id)
{
    SerialNumInfo.isSendChargeInform[gun_id - 1].isStopChargeFlag = 0;
    
	return CL_OK;
}

/*****************************************************************************
** Function name:       CleanStartChargingNotice_BTRF2_4G
** Descriptions:        //RF2_4G清除开启充电通知到flash
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int CleanStartChargingNotice_BTRF2_4G(uint8_t gun_id)
{
    SerialNumInfo.isSendChargeInform[gun_id - 1].isStartChargeFlag = 0;
    
	return CL_OK;
}

/*****************************************************************************
** Function name:       R6ToX6StopChargingAckBTRF2_4G
** Descriptions:        //停止充电请求应答
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void R6ToX6StopChargingAckBTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
    STOP_CHARGING_ACK_STR_24G *stopChargingACK = (void*)pMsg->data;
    uint8_t  gun_id = stopChargingACK->gun_id;
    ORDER_STR *pOrder = &gGunInfo1.orderInfo;
    static uint8_t SerialNum = 255;
	static uint8_t timesFlag = 0;
    gun_id = gun_id - system_info.baseGunSn + 1;
        
	if(0 == stopChargingACK->result)
    {
        if((SerialNum != pMsg->head.SerialNum) || (((uint32_t)60)*SERIAL_NUM_DELAY_TIME < (GetRtcCount() - gChgInfo.gStopChargeNoticeTick[gun_id - 1])))
        {
        	gChgInfo.gStopChargeNoticeTick[gun_id - 1] = GetRtcCount();
            SerialNum = pMsg->head.SerialNum;
            CleanStopChargingNotice_BTRF2_4G(gun_id);
            
            MuxSempTake(&gChgInfo.gProtoMuxType.gOrderReportMux);
            if(RECORD_MAX_BLOCK_NUM <= ++pOrder->rIndex)    //读指针只能在收到R6发送过来的结束充电应答才能加1, 不然可能会丢失订单
            {
    			pOrder->rIndex = 0;
    		}
			SerialNumInfo.OrderReportNoticeSN++;
			CL_LOG("发送成功, 订单序列号加一.SerialNumInfo.OrderReportNoticeSN = %d\n", SerialNumInfo.OrderReportNoticeSN);

			FlashWriteGunInfo_1_Data();
            MuxSempGive(&gChgInfo.gProtoMuxType.gOrderReportMux);
        }
    }
	else
	{
		if(SerialNumInfo.OrderReportNoticeSN == pMsg->head.SerialNum)
        {
        	CL_LOG("订单序列号与R6相同.SerialNumInfo.OrderReportNoticeSN = %d\n", SerialNumInfo.OrderReportNoticeSN);
        	if(5 < timesFlag++)
			{
				SerialNumInfo.OrderReportNoticeSN += 15;
			}
		}
		CL_LOG("收到R6的订单应答 .\n");
	}
}

/*****************************************************************************
** Function name:       SaveStopChargingNotice_BTRF2_4G
** Descriptions:        //RF2_4G保存停止充电通知到flash
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int SaveStopChargingNotice_BTRF2_4G(uint8_t gun_id)
{
    SerialNumInfo.isSendChargeInform[gun_id - 1].isStopChargeFlag = 1;    //
    
	return CL_OK;
}

/*****************************************************************************
** Function name:       SendOrderReportNotice_BTRF2_4G
** Descriptions:        //发送订单区域保存的订单
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int SendOrderReportNotice_BTRF2_4G(ORDER_REPORT_NOTICE_BTRF2_4G_TYPE * pSaveNoticeMsg, uint32_t SerialNum)
{
	BLUE_RF2_4G_PKT_STR pkt;
	PROTO_STR_BTRF24G_TYPE *proto = (void*)pkt.data;
	ORDER_REPORT_NOTICE_BTRF2_4G_TYPE * pMsg = (void*)proto->data;
    
	memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
    memcpy(pMsg, (void*)pSaveNoticeMsg, sizeof(ORDER_REPORT_NOTICE_BTRF2_4G_TYPE));
    
	SendProtoPkt_BTRF2_4G(CMD_CHARGER_STOPED_24G,&pkt,sizeof(ORDER_REPORT_NOTICE_BTRF2_4G_TYPE), (SerialNum & 0xff));
    PrintfData("发送订单区域保存的订单", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(ORDER_REPORT_NOTICE_BTRF2_4G_TYPE));

	return CL_OK;
}

/*****************************************************************************
** Function name:       SendReqCostTemplate_BTRF2_4G
** Descriptions:        //RF2_4G//请求计费模板
** input parameters:    gun_id: 枪号
                        startTime: 充电开始时间
                        result: 充电结果
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int SendReqCostTemplate_BTRF2_4G(uint8_t gunId)
{
    BLUE_RF2_4G_PKT_STR pkt;
	memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
	PROTO_STR_BTRF24G_TYPE *proto = (void*)pkt.data;
	
    REQ_COST_TEMPLATE_STR_24G *reqCostTemplate = (void*)proto->data;
    reqCostTemplate->gun_id = gunId;

    SerialNumInfo.ReqCostTemplateSN++;
	SendProtoPkt_BTRF2_4G(CMD_COST_REQ_24G,&pkt,sizeof(REQ_COST_TEMPLATE_STR_24G), SerialNumInfo.ReqCostTemplateSN);
    PrintfData("请求计费模板", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(REQ_COST_TEMPLATE_STR_24G));
    
    return CL_OK;
}

/*****************************************************************************
** Function name:       SendEventNotice_BTRF2_4G
** Descriptions:        //事件通知
** input parameters:    
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
uint8_t SendEventNotice_BTRF2_4G(uint8_t gunId,uint8_t event,uint8_t para1,uint32_t para2,uint8_t status,uint8_t  level)
{
	BLUE_RF2_4G_PKT_STR pkt;
	memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
	
	PROTO_STR_BTRF24G_TYPE *proto = (void*)pkt.data;
	EVENT_NOTICE_STR_BTRF2_4G * pMsg = (void*)proto->data;
	pMsg->gun_id = gunId + system_info.baseGunSn - 1;
    pMsg->code = event;
    pMsg->para1 = para1;
    pMsg->para2 = para2;
    pMsg->status = status;
    pMsg->level = level;

    SerialNumInfo.EventNoticeSN++;
	SendProtoPkt_BTRF2_4G(CMD_EVENT_NOTICE_24G,&pkt,sizeof(EVENT_NOTICE_STR_BTRF2_4G), SerialNumInfo.EventNoticeSN);
    PrintfData("//事件通知", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(EVENT_NOTICE_STR_BTRF2_4G));
    
	return CL_OK;
}


/*****************************************************************************
** Function name:       CardAuthAckResult_BTRF2_4G
** Descriptions:        //鉴权结果返回应答
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void CardAuthAckResult_BTRF2_4G(uint8_t result, uint8_t gun_id, uint8_t SerialNum)
{
    //ACK
    BLUE_RF2_4G_PKT_STR pkt;
	memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
	PROTO_STR_BTRF24G_TYPE *proto = (void *)pkt.data;
	CARD_AUTH_ACK_RETURN_STR_24G *stopChargingACK = (void*)proto->data;
    
	stopChargingACK->result = result;
	stopChargingACK->gun_id = gun_id;
	SendProtoPkt_BTRF2_4G(CMD_CARD_AUTH_RESULT_RETURN_24G,&pkt,sizeof(CARD_AUTH_ACK_RETURN_STR_24G), SerialNum);
    PrintfData("收到R6后台的鉴权后, 返回鉴权结果应答", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + 
                sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(CARD_AUTH_ACK_RETURN_STR_24G));
}

void R6ToX6ScanCodeStartChargerAck(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
    ACK_X6_RECEIVED_SCAN_CODE_STR *pScanCodeChargerAck = (void*)pMsg->data;
    //static uint8_t SerialNum = 255;
    
    //if((SerialNum != pMsg->head.SerialNum) || (((uint32_t)60)*SERIAL_NUM_DELAY_TIME < (GetRtcCount() - gChgInfo.gScanCodeChargerNoticeTick[pScanCodeChargerAck->gun_id - system_info.baseGunSn])))
    {
    	//gChgInfo.gScanCodeChargerNoticeTick[pScanCodeChargerAck->gun_id - system_info.baseGunSn] = GetRtcCount();
        //SerialNum = pMsg->head.SerialNum;
        if(0 == pScanCodeChargerAck->result)
        {
			SerialNumInfo.ScanCodeChargerFlag[pScanCodeChargerAck->gun_id - system_info.baseGunSn] = 0xab;
        }
        CL_LOG("R6向X6表示已经收到扫码开启充电应答!\n");
    }
}

/*****************************************************************************
** Function name:       CardAuthAck_BTRF2_4G
** Descriptions:        //收到卡鉴权响应的应答
** input parameters:    pMsg: 内层数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void CardAuthAck_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
    CARD_AUTH_ACK_RETURN_STR_24G *pCardAuthAck = (void*)pMsg->data;
    static uint8_t SerialNum = 255;
    
    if((SerialNum != pMsg->head.SerialNum) || (((uint32_t)60)*SERIAL_NUM_DELAY_TIME < (GetRtcCount() - gChgInfo.gCardAuthAckNoticeTick[pCardAuthAck->gun_id - system_info.baseGunSn])))
    {
    	gChgInfo.gCardAuthAckNoticeTick[pCardAuthAck->gun_id - system_info.baseGunSn] = GetRtcCount();
        SerialNum = pMsg->head.SerialNum;
		
        //if(0 == pCardAuthAck->result)
        {
            SerialNumInfo.AuthFlag[pCardAuthAck->gun_id - system_info.baseGunSn] = 0;
			OpenCheckTimeOver();
        }
		if(0xae == pCardAuthAck->result)
		{
			OptFailNotice(107);
		}
        CL_LOG("没有后台鉴权的, R6立即发送的鉴权响应的应答, gun_id = %d.\n", pCardAuthAck->gun_id);
    }
}

/*****************************************************************************
** Function name:       CardAuthResult_BTRF2_4G
** Descriptions:        //卡鉴权响应
** input parameters:    pMsg: 内层数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void CardAuthResult_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
	CARD_AUTH_ACK_STR_24G *pCardAuth = (void*)pMsg->data;
    uint8_t  gun_id;
    static uint8_t SerialNum = 255;
    
    PrintfData("收到R6后台的卡鉴权响应", (void*)pMsg, sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(CARD_AUTH_ACK_STR_24G) + 3);
    //printf("gun_id = %d",pCardAuth->gun_id - system_info.baseGunSn + 1);
    gun_id = pCardAuth->gun_id - system_info.baseGunSn + 1;
    SerialNumInfo.AuthFlag[gun_id - 1] = 0;
    //CloseCheckTimeOver();
    if((SerialNum != pMsg->head.SerialNum) || (((uint32_t)60)*SERIAL_NUM_DELAY_TIME < (GetRtcCount() - gChgInfo.gCardChargeTimeTick[gun_id - 1])))
    {
    	gChgInfo.gCardChargeTimeTick[gun_id - 1] = GetRtcCount();
        SerialNum = pMsg->head.SerialNum;
        switch(pCardAuth->result)
        {
            case 0: // 0:鉴权成功
            	CL_LOG("卡片余额[%d]\n", pCardAuth->user_momey);
                gChgInfo.user_card_balance = pCardAuth->user_momey;     //卡里面的余额
    			//CL_LOG("card balance = %d\n", gChgInfo.user_card_balance);
    			if (gChgInfo.user_card_balance < 100)
                {
                    MuxSempTake(&gChgInfo.gProtoMuxType.gSc8042bMux);
                    //osDelay(800);   //上条语音没有播报完就被这条抢报了
    				Sc8042bSpeech(VOIC_CARD_RECHARGER);
                    MuxSempGive(&gChgInfo.gProtoMuxType.gSc8042bMux);
    				break;
    			}
                if(GetHistoryOrderNum() >= RECORD_MAX_BLOCK_NUM)
                {
            		CL_LOG("历史订单太多不允许充电HistoryOrderNum:%d \n",GetHistoryOrderNum());
            		//ACK
                    CardAuthAckResult_BTRF2_4G(1, pCardAuth->gun_id - system_info.baseGunSn + 1, pMsg->head.SerialNum);
                    return;
                }
                gChgInfo.mode = 1;      //卡鉴权填写的充电模式
                gChgInfo.current_usr_gun_id = pCardAuth->gun_id - system_info.baseGunSn + 1;    //卡鉴权填写的枪头号
                //ACK
                CardAuthAckResult_BTRF2_4G(0, pCardAuth->gun_id - system_info.baseGunSn + 1, pMsg->head.SerialNum);
                MuxSempTake(&gChgInfo.gProtoMuxType.gSc8042bMux);
                //CloseCheckTimeOver();
                SpeechCardBalance(gChgInfo.user_card_balance);
                MuxSempGive(&gChgInfo.gProtoMuxType.gSc8042bMux);
    			if (gCardReqFlag == 1)
                {
                    if(gChgInfo.user_card_balance > 2 * 100)
                    {
                        gChgInfo.money = 200;   //2元
                    }
                    else //if(gChgInfo.user_card_balance > 2)
                    {
                        gChgInfo.money = gChgInfo.user_card_balance;
                    }
					if ((GUN_STATE_IDLE != gun_info[gChgInfo.current_usr_gun_id-1].gun_state)
						//&& (GUN_STATE_OFF != &gun_info[gChgInfo.current_usr_gun_id-1]->gun_state)
						)
					{
						CL_LOG("插座被占用gun_state[%d] \n", gun_info[gChgInfo.current_usr_gun_id-1].gun_state);
				        return;
				    }
    				if (CL_OK == CheckStartPara(gChgInfo.current_usr_gun_id))
                    {
                        MuxSempTake(&gChgInfo.gProtoMuxType.gSc8042bMux);
                        SpeechSelectGun(pCardAuth->gun_id);
                        //vTaskDelay(100);    //这条语音没有播报完就被其他语音抢报了
                        MuxSempGive(&gChgInfo.gProtoMuxType.gSc8042bMux);
                        
                        system_info.isStartCharge[gChgInfo.current_usr_gun_id - 1].gun_id = gChgInfo.current_usr_gun_id;
                        system_info.isStartCharge[gChgInfo.current_usr_gun_id - 1].mode = gChgInfo.mode;
                        system_info.isStartCharge[gChgInfo.current_usr_gun_id - 1].chargingPara = gChgInfo.money;
                        system_info.isStartCharge[gChgInfo.current_usr_gun_id - 1].isStartChargeSN = pMsg->head.SerialNum;
                        memcpy(system_info.isStartCharge[gChgInfo.current_usr_gun_id - 1].order, pCardAuth->order, sizeof(pCardAuth->order));
                        
                        system_info.isStartCharge[gun_id - 1].isNeedToStartChargeFlag = 1;  //需要开启充电
			
						FlashWriteSystemInfoData();
                        gun_info->startMode = SWIPE_CARD;
                        return;
    				}
    			}
                else {
    				CL_LOG("card req fail.\n");
    			}
    			gCardReqFlag = 0;
            break;
            case 2: //账户余额不足或次数已经用完
                //ACK
                CardAuthAckResult_BTRF2_4G(0, pCardAuth->gun_id - system_info.baseGunSn + 1, pMsg->head.SerialNum);
                MuxSempTake(&gChgInfo.gProtoMuxType.gSc8042bMux);
                //osDelay(800);   //上条语音没有播报完就被这条抢报了
        		Sc8042bSpeech(VOIC_CARD_RECHARGER);
    			MuxSempGive(&gChgInfo.gProtoMuxType.gSc8042bMux);
        		if (pCardAuth->cardType == 0)
                {
        			gChgInfo.user_card_balance = pCardAuth->user_momey;
                    Sc8042bSpeech(VOIC_CARD_RECHARGER);
        			//gui显示卡余额
        		}
                else {
        			CL_LOG("month card charge times err.\n");
        		}
                
            break;
        	case 3: //正在使用,不能同时使用
        	 	//ACK
                CardAuthAckResult_BTRF2_4G(0, pCardAuth->gun_id - system_info.baseGunSn + 1, pMsg->head.SerialNum);
                MuxSempTake(&gChgInfo.gProtoMuxType.gSc8042bMux);
                //osDelay(800);   //上条语音没有播报完就被这条抢报了
                Sc8042bSpeech(VOIC_CARD_INVALID);
                MuxSempGive(&gChgInfo.gProtoMuxType.gSc8042bMux);
                CL_LOG("card identify failure, result=%d,error.\n",pCardAuth->result);
            break;
            case 1: //password error
            case 4: //账户无效
            case 5: //其他原因
            default:
            {
                //ACK
                CardAuthAckResult_BTRF2_4G(0, pCardAuth->gun_id - system_info.baseGunSn + 1, pMsg->head.SerialNum);
                MuxSempTake(&gChgInfo.gProtoMuxType.gSc8042bMux);
                //osDelay(800);       //上条语音没有播报完就被这条抢报了
                Sc8042bSpeech(VOIC_CARD_INVALID);
                MuxSempGive(&gChgInfo.gProtoMuxType.gSc8042bMux);
                CL_LOG("card identify failure, result=%d,error.\n",pCardAuth->result);
            }
            return;
        }
    }
    else{
        CL_LOG("序列号相同 SerialNum = %d.\n", SerialNum);
    }
    
   return;
}

/*****************************************************************************
** Function name:       StartChargingAckProc_24G
** Descriptions:        //充电请求应答
** input parameters:    result: 
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void StartChargingAckProc_24G(uint8_t gun_id, uint8_t result, uint8_t SendFlag)
{
    BLUE_RF2_4G_PKT_STR pkt;
    memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
    PROTO_STR_BTRF24G_TYPE *proto = (void *)pkt.data;
    START_CHARGING_ACK_STR_24G *startChargingACK =  (void*)proto->data;
	uint32_t i = 0;
	
    startChargingACK->failReason = result;
	startChargingACK->gun_id = gun_id + system_info.baseGunSn -1;
	startChargingACK->result = (result == 0 ? 0 : 1) ;
    SerialNumInfo.StartChargingAckSN++;

	if (0xab == SerialNumInfo.ScanCodeChargerFlag[gun_id - 1])
    {
        SerialNumInfo.ScanCodeChargerFlag[gun_id - 1] = 0;
        return;
    }
	SerialNumInfo.ScanCodeChargerFlag[gun_id - 1] = 0;
    for(i = 0; i < (RESENT_24G_MSG_CNT * 3); i++)
    {
        if (0 == (i % RESENT_24G_MSG_CNT))
        {
			SendProtoPkt_BTRF2_4G(CMD_START_CHARGER_24G,&pkt,sizeof(START_CHARGING_ACK_STR_24G), SerialNumInfo.StartChargingAckSN);
			PrintfData("给R6发送扫码开启充电请求应答", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(START_CHARGING_ACK_STR_24G));
			if(ONE_TIMES == SendFlag)
            {
                return;
            }
		}
		
		vTaskDelay(RETRY_24G_DELAY_TIME);
        if(0xab == SerialNumInfo.ScanCodeChargerFlag[gun_id - 1])
        {
        	SerialNumInfo.ScanCodeChargerFlag[gun_id - 1] = 0;
            break;
        }
	}
}

/*****************************************************************************
** Function name:       StartChargingProc_BTRF2_4G
** Descriptions:        //开启充电请求
** input parameters:    pMsg: 内层数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void StartChargingProc_BTRF2_4G(PROTO_STR_BTRF24G_TYPE *pMsg)
{
	uint8_t result = 1;
	START_CHARGING_REQ_STR_24G *startChargingReq = (void*)pMsg->data;
	uint8_t gun_id = startChargingReq->gun_id;
    static uint8_t StartChargingSerialNum[1] = {0};
    PrintfData("扫码开启充电请求接收报文", (uint8_t *)pMsg->data, sizeof(START_CHARGING_REQ_STR_24G));
    //PrintfData("接收订单号", startChargingReq->order, sizeof(startChargingReq->order));
    
    //CL_LOG("枪头号 gun_id = %d\n", gun_id);
    gun_id = gun_id - system_info.baseGunSn + 1;
    //CL_LOG("枪头号1 gun_id = %d\n", gun_id);
    //CL_LOG("起始枪头号 baseGunSn = %d\n", system_info.baseGunSn);
	if((StartChargingSerialNum[0] != pMsg->head.SerialNum) || (((uint32_t)60)*SERIAL_NUM_DELAY_TIME < (GetRtcCount() - gChgInfo.gStartChargeTimeTick[gun_id - 1])))
    {
    	gChgInfo.gStartChargeTimeTick[gun_id - 1] = GetRtcCount();
        if(GetHistoryOrderNum() >= RECORD_MAX_BLOCK_NUM)
        {
        	OptFailNotice(106);
    		CL_LOG("HistoryOrderNum is to many:%d \n",GetHistoryOrderNum());
    		//ACK
            //StartChargingAckProc_24G(gun_id, 1);
            return;
        }
		if ((GUN_STATE_IDLE != gun_info[gun_id - 1].gun_state)
			//&& (GUN_STATE_OFF != &gun_info[gun_id - 1]->gun_state)
			)
		{
			CL_LOG("插座被占用state[%d] \n", gun_info[gun_id - 1].gun_state);
	        return;
	    }
    	if (CL_OK != (result = CheckStartPara(gun_id)))
        {
			//StartChargingAckProc_24G(gun_id, result);   //返回错误
			CL_LOG("插座被占用 result = [%d]\n", result);
			return;
    	}
        gChgInfo.gun_id[gun_id - 1] = 0xae;
        //ACK
        StartChargingAckProc_24G(gun_id, 0, ONE_TIMES);
        printf("StartChargingSerialNum[gun_id - 1] = %d, pMsg->head.SerialNum = %d, 枪头号 gun_id = %d\n", StartChargingSerialNum[0], pMsg->head.SerialNum, gun_id);
        //StartChargingSerialNum[gun_id - 1] = pMsg->head.SerialNum;
        StartChargingSerialNum[0] = pMsg->head.SerialNum;
		
        system_info.isStartCharge[gun_id - 1].gun_id = gun_id;
        system_info.isStartCharge[gun_id - 1].mode = startChargingReq->mode;
        system_info.isStartCharge[gun_id - 1].chargingPara = startChargingReq->chargingPara;
        gChgInfo.money = startChargingReq->chargingPara;
        if(0 != memcmp(startChargingReq->order,gun_info[gChgInfo.current_usr_gun_id-1].order,sizeof(startChargingReq->order)))
        {
            //SerialNumInfo.StartChargeSN++;
            system_info.isStartCharge[gun_id - 1].isStartChargeSN = pMsg->head.SerialNum;//SerialNumInfo.StartChargeSN;
            memcpy(system_info.isStartCharge[gun_id - 1].order, startChargingReq->order, sizeof(startChargingReq->order));
        }
        
        system_info.isStartCharge[gun_id - 1].isNeedToStartChargeFlag = 1;  //需要开启充电
        printf("充电标志 isNeedToStartChargeFlag = %d\n",system_info.isStartCharge[gun_id - 1].isNeedToStartChargeFlag);
		
		FlashWriteSystemInfoData();
		//result = BTRF2_4gStartCharging(gun_id, startChargingReq->chargingPara, startChargingReq->order, pMsg->head.SerialNum);
	}
    else
	{
 //       StartChargingAckProc_24G(gun_id, 1);
        printf("StartChargingSerialNum[gun_id - 1] = %d, error\n",StartChargingSerialNum[0]);
        printf("pMsg->head.SerialNum = %d, error\n",pMsg->head.SerialNum);
    }
}

/*****************************************************************************
** Function name:       isStartChargingBTRF2_4G
** Descriptions:        //查询是否需要开启充电
** input parameters:    pMsg: 内层数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void isStartChargingBTRF2_4G(void)
{
    uint8_t result = 1;
    static uint8_t gun_id = 1;
	
    if(1 == system_info.isStartCharge[gun_id - 1].isNeedToStartChargeFlag)  //如果需要开启充电
    {
        system_info.isStartCharge[gun_id - 1].isNeedToStartChargeFlag = 0;  //需要开启充电
        gChgInfo.money = system_info.isStartCharge[gun_id - 1].chargingPara;
        gChgInfo.current_usr_gun_id = gun_id;
        gChgInfo.mode = system_info.isStartCharge[gun_id - 1].mode;
        result = BTRF2_4gStartCharging(gun_id, gChgInfo.money, system_info.isStartCharge[gun_id - 1].order, 
                                        system_info.isStartCharge[gun_id - 1].isStartChargeSN);
        CL_LOG("result = %d\n", result);
        if(0 == result)
        {
			SpeechChargeing(gun_id + system_info.baseGunSn - 1);
			SerialNumInfo.StartChargingNotice.StartChargeFlag[gun_id - 1] = 0xab;
			//SendStartChargingNotice();
        }
		
		FlashWriteSystemInfoData();
    }
    if(gun_id++ >= 2)
    {
        gun_id = 1;
    }
}

/*****************************************************************************
** Function name:       StopChargingAckBTRF2_4G
** Descriptions:        //停止充电请求应答
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void StopChargingAckBTRF2_4G(uint8_t result, uint8_t gun_id)
{
    //ACK
    BLUE_RF2_4G_PKT_STR pkt;
	memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
	PROTO_STR_BTRF24G_TYPE *proto = (void *)pkt.data;
	STOP_CHARGING_ACK_STR_24G *stopChargingACK =  (void*)proto->data;

	stopChargingACK->result = result;
	stopChargingACK->gun_id = gun_id;
    SerialNumInfo.StopChargingAckSN++;
	SendProtoPkt_BTRF2_4G(CMD_STOP_CHARGER_24G,&pkt,sizeof(STOP_CHARGING_ACK_STR_24G), SerialNumInfo.StopChargingAckSN);
    PrintfData("停止充电请求应答", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(STOP_CHARGING_ACK_STR_24G));
}

/*****************************************************************************
** Function name:       StopChargingProc_BTRF2_4G
** Descriptions:        //停止充电请求
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void StopChargingProc_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
	uint8_t result = 0;
	STOP_CHARGING_REQ_STR_24G* stopChargingReq = (void*)pMsg->data;
	uint8_t gun_id = stopChargingReq->gun_id;
    
    gun_id = gun_id - system_info.baseGunSn + 1;
    
    //if(result == 0)
    {
        gun_info[gun_id-1].stopReason = stopChargingReq->stopReason;
        Proto2_4gStopCharging(gun_id);
        SaveStopChargingNotice_BTRF2_4G(gun_id);
    }
    
    //ACK
    StopChargingAckBTRF2_4G(result, gun_id);
#if 0
    //发送充电停止完成通知
    if(result == 0) {
        SendStopChargingNotice_BTRF2_4G(gun_id);
    }
#endif
}

/*****************************************************************************
** Function name:       StartChargingNoticeACK_BTRF2_4G
** Descriptions:        //清除启动充电通知响应标志
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void StartChargingNoticeACK_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
    START_CHARGING_ACK_STR_24G_TYPE *StartChargingACK = (void*)pMsg->data;
    uint8_t  gun_id = StartChargingACK->gunId;
    static uint8_t SerialNum = 255;
    
    gun_id = gun_id - system_info.baseGunSn + 1;
    
    if(0 == StartChargingACK->result)   //表示发送成功
    {
        if((SerialNum != pMsg->head.SerialNum) || (((uint32_t)60)*SERIAL_NUM_DELAY_TIME < (GetRtcCount() - gChgInfo.gStartChargeNoticeTick[gun_id - 1])))
        {
        	gChgInfo.gStartChargeNoticeTick[gun_id - 1] = GetRtcCount();
            SerialNum = pMsg->head.SerialNum;
            CleanStartChargingNotice_BTRF2_4G(gun_id);//清除标志
            PrintfData("清除启动充电通知响应标志", (uint8_t *)StartChargingACK, sizeof(StartChargingACK));
        }
    }
}

/*****************************************************************************
** Function name:       R6X6BindACK_BTRF2_4G
** Descriptions:        //给r6发送x6绑定桩号确认
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void R6X6BindACK_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  * pMsg)
{
	//ACK
    BLUE_RF2_4G_PKT_STR pkt;
	memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
	PROTO_STR_BTRF24G_TYPE *proto = (void *)pkt.data;
	R6X6_BIND_ACK_STR_24G_TYPE *R6X6_BindACK =  (void*)proto->data;

	memcpy(R6X6_BindACK->deviceID, &gSystemInfo1.station_id[3], sizeof(R6X6_BindACK->deviceID));
	R6X6_BindACK->baseGunSn = system_info.baseGunSn;
	SendProtoPkt_BTRF2_4G(CMD_ENUM_BIND_ACK,&pkt,sizeof(R6X6_BIND_ACK_STR_24G_TYPE), pMsg->head.SerialNum);//返回R6发送过来的序列号,以防绑定时不同的X6返回相同的序列号
    PrintfData("给R6发送绑定应答", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(R6X6_BIND_ACK_STR_24G_TYPE));
}

/*****************************************************************************
** Function name:       R6X6BindREQ_BTRF2_4G
** Descriptions:        //r6给x6发送过来绑定桩号请求确认命令
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  	None
** Author:              quqian
*****************************************************************************/
void R6X6BindREQ_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
    R6X6_BIND_REQ_STR_24G_TYPE *R6X6_BindReq = (void*)pMsg->data;

	if(0 != R6X6_BindReq->ServerTime)
	{
		SetRtcCount(R6X6_BindReq->ServerTime + 2);
	}
	
	R6X6BindACK_BTRF2_4G(pMsg);
}

/*****************************************************************************
** Function name:       X6ToR6HeartBatACK_BTRF2_4G
** Descriptions:        //心跳响应
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void X6ToR6HeartBatACK_BTRF2_4G(void)
{
    BLUE_RF2_4G_PKT_STR pkt;
    uint8_t gunId = 1;
    PROTO_STR_BTRF24G_TYPE *proto = (void *)pkt.data;
    HEART_BEAT_ACK_STR_BTRF2_4G_TYPE *HeartBatACK =  (void*)proto->data;
    GUN_STATUS_STR gunStatus;
    uint16_t PktLen = 0;
    uint16_t PktTotolLen = 0;
    uint8_t *pHeartPkt;
    uint8_t gunChargeStatus[GUN_NUM_MAX];
	
    memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
    GetGunStatus(gunId, &gunStatus);
    
    HeartBatACK->temperature = gunStatus.temp;
    HeartBatACK->includeVer = FW_VERSION;//(FW_VERSION << 4) | (FW_VERSION_SUB);
    HeartBatACK->timestamp = 2;     //GetChargingGunCnt();
    for(gunId = 1; gunId <= GUN_NUM_MAX; gunId++)
    {
        pHeartPkt = (&(HeartBatACK->Data[PktLen]));

		if((GUN_STATE_IDLE == gun_info[gunId - 1].gun_state) || (GUN_STATE_OFF == gun_info[gunId - 1].gun_state))
		{
			gunChargeStatus[gunId - 1] = 0;
		}
		else if((GUN_STATE_WAIT_PLUG_IN == gun_info[gunId - 1].gun_state) || (GUN_STATE_ON == gun_info[gunId - 1].gun_state))
		{
			gunChargeStatus[gunId - 1] = 1;
		}
        if(GUN_CHARGING_IDLE < (gun_info[gunId - 1].is_load_on))
        {
            GetGunStatus(gunId, &gunStatus);
            ((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->gun_id = gunId + system_info.baseGunSn - 1;
            ((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->ChargeStatus = gunChargeStatus[gunId - 1];
            ((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->errStatus = (gunStatus.status == 0)?0:1;
            ((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->voltage = gunStatus.voltage / 10;
            ((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->current = gunStatus.current / 100;
            ((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->power = gunStatus.power / 10;
            ((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->elec = gun_info[gunId - 1].chargingElec;//gunStatus.elec;
            ((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->ChargeCost = gun_info[gunId - 1].current_usr_money;
            PktLen = sizeof(HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE);
            PktTotolLen += PktLen;
	//		CL_LOG("心跳包原始数据发送功率:[%d, %d, %d, %d, %d, %d].\n", 
	//					((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->gun_id,
	//					gunStatus.power, 
	//					gunStatus.voltage, 
	//					gunStatus.current, 
	//					gun_info[gunId - 1].chargingElec, 
	//					gun_info[gunId - 1].current_usr_money);
        }
        else
        {
            ((HEART_BEAT_CHARGE_PORT_BTRF2_4G_TYPE*)pHeartPkt)->gun_id = gunId + system_info.baseGunSn - 1;
            ((HEART_BEAT_CHARGE_PORT_BTRF2_4G_TYPE*)pHeartPkt)->ChargeStatus = 0;//gunChargeStatus[gunId];
            ((HEART_BEAT_CHARGE_PORT_BTRF2_4G_TYPE*)pHeartPkt)->errStatus = (gunStatus.status == 0)?0:1;
            PktLen = sizeof(HEART_BEAT_CHARGE_PORT_BTRF2_4G_TYPE);
            PktTotolLen += PktLen;
        }
		
	//	CL_LOG("心跳包发送功率:[%d, %d, %d, %d, %d, %d].\n", 
	//					((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->gun_id,
	//					((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->power, 
	//					((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->voltage, 
	//					((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->current, 
	//					((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->elec, 
	//					((HEART_BEAT_CHARGE_INFO_BTRF2_4G_TYPE*)pHeartPkt)->ChargeCost);
    }
    SerialNumInfo.HeartBatAckSN++;
	SendProtoPkt_BTRF2_4G(CMD_HEART_BEAT_24G,&pkt,PktTotolLen + 2, SerialNumInfo.HeartBatAckSN);
    
    PrintfData("心跳响应", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + PktTotolLen + 2);
}

/*****************************************************************************
** Function name:       HeartBatACK_BTRF2_4G
** Descriptions:        //心跳响应
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void HeartBatACK_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
	uint8_t gunIdFlag = 1;
	static uint8_t timesFlag[GUN_NUM_MAX];
	static uint8_t  is_load_on[GUN_NUM_MAX] = {0,};
	HEART_BEAT_REQ_STR_BTRF2_4G_TYPE *heartACK = (void*)pMsg->data;
	int cnt;
	
    if(0 == heartACK->ServerTime)
    {
        CL_LOG("服务器时间为零，返回 heartACK->ServerTime = %x  .\n",heartACK->ServerTime);
		//if(gChgInfo.UpperMonitorTestFlag == 0xa5)
		{
			X6ToR6HeartBatACK_BTRF2_4G();
		}
        return;
    }
	time_t now = GetRtcCount();
	cnt = (now > heartACK->ServerTime) ? now - heartACK->ServerTime : heartACK->ServerTime - now;

    //PrintfData("HeartBatACK_BTRF2_4G, 接收心跳包数据", (void*)pMsg, sizeof(HEART_BEAT_REQ_STR_BTRF2_4G_TYPE) + sizeof(PROTO_HEAD_BTRF24G_TYPE));
    Feed_WDT();
    //如果和服务器的时间误差大于30s并且充电的枪头数目是0 , 那么更新本地时间
	if (((30 < cnt) && (0 == GetChargingGunCnt())) || (0 == gChgInfo.heartReceiveFlag))
    {
        CL_LOG("服务器时间 heartACK->ServerTime = %x .\n",heartACK->ServerTime);
        CL_LOG("本地时间 heartACK->ServerTime = %x .\n", now);
        if(0 != heartACK->ServerTime)
        {
            //系统对时
            SetRtcCount(heartACK->ServerTime + 2);
            gChgInfo.heartReceiveFlag = 1;
            CL_LOG("set rtc time.\n");
        }
        else{
            //R6下发时间为0
            CL_LOG("R6下发时间为: %x  .\n",heartACK->ServerTime);
			return;
        }
    }
    //PrintfData("本地mac system_info._24gMacAddr", system_info._24gMacAddr, sizeof(system_info._24gMacAddr));
    //PrintfData("接收R6mac heartACK->gwMAC", heartACK->gwMAC, sizeof(heartACK->gwMAC));

    //更新起始枪头
	if(system_info.baseGunSn != heartACK->baseGunSn)    //如果起始枪头和下发的不一样要及时更新起始枪头
    {
		//枪头起始号
		CL_LOG("心跳包设置起始枪头号! \n");
		system_info.baseGunSn = heartACK->baseGunSn;
		
		FlashWriteSystemInfoData();
	}
    //如果计费模板ID不对要更新计费模板, 请求下发计费模板
    if ((heartACK->cost_modeID != gCostTemplateInfo.cost_template.template_id) || 
        ((COST_POWER != GetCostMode()) && (COST_UNIFY != GetCostMode())))
    {
        CL_LOG("请求计费模板 ,GetCostMode() = %d , heartACK->cost_modeID = %d , system_info.cost_template.template_id = %d.\n",GetCostMode(), heartACK->cost_modeID, gCostTemplateInfo.cost_template.template_id);
	    SendReqCostTemplate_BTRF2_4G(system_info.baseGunSn);
        return;
    }
	#if 1
	for(gunIdFlag = 1; gunIdFlag <= GUN_NUM_MAX; gunIdFlag++)
	{
		if(((gun_info[gunIdFlag - 1].is_load_on == GUN_CHARGING_IDLE) && (is_load_on[gunIdFlag - 1] != GUN_CHARGING_IDLE)) ||
			((gun_info[gunIdFlag - 1].is_load_on != GUN_CHARGING_IDLE) && (is_load_on[gunIdFlag - 1] != GUN_CHARGING_UNKNOW_POWER)))
		{
			CL_LOG("充电枪头状态有变化！[%d], [%d], [%d], [%d]",gunIdFlag, timesFlag[gunIdFlag - 1], gun_info[gunIdFlag - 1].is_load_on, is_load_on[gunIdFlag - 1]);
			X6ToR6HeartBatACK_BTRF2_4G();
			if(4 > timesFlag[gunIdFlag - 1]++)
			{
				//发送心跳ACK
				return;
			}
		}
		if(gun_info[gunIdFlag - 1].is_load_on == GUN_CHARGING_IDLE)
		{
			is_load_on[gunIdFlag - 1] = GUN_CHARGING_IDLE;
		}
		else
		{
			is_load_on[gunIdFlag - 1] = GUN_CHARGING_UNKNOW_POWER;
		}
		timesFlag[gunIdFlag - 1] = 0;
	}
	#endif
	
	if(0 == SendOrderReport())
	{
		return;
	}
	
    #if 1
    //固件版本不相同, 请求升级
    if((0 == GetBlueToothUpgradeFlag()) && (0 == GetUARTUpgradeFlag())) //如果蓝牙和串口都不在升级状态
    {
        if (0 == GetUpgradeReqestFlag())
        {
            if(((0 != heartACK->fwVer) /*&& (heartACK->fwVer != system_info.fw_version)*/))
            {
                CL_LOG("心跳包 请求固件升级 ,GetUpgradeReqestFlag = %d \n", GetUpgradeReqestFlag());
                CL_LOG("升级版本号 fwVer = %d ,老版本号 fw_version = %d \n", heartACK->fwVer, (FW_VERSION) /*system_info.fw_version*/);
                ClearUpgradeWaitTimer();
                FWUpgradeREQ_BTRF2_4G(1);
                return;
            }
        }
    }
    #endif
	SendHeartBat_BTRF2_4G();
}

/*****************************************************************************
** Function name:       RemoteCtrlBtRf2_4g
** Descriptions:        //远程控制枪头
** input parameters:    
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
static uint8_t RemoteCtrlBtRf2_4g(uint8_t cmd,uint32_t gunId)
{
	uint8_t result = 0;
	switch(cmd)
    {
		case 1:
        break;
		case 2:
			GunTurnOn(gunId);
        break;
		case 3:
			GunTurnOff(gunId);
        break;
	}
	
	return result;
}

/*****************************************************************************
** Function name:       RemoteCtrlAck_BTRF2_4G
** Descriptions:        //远程控制应答
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void RemoteCtrlAck_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg, uint8_t result)
{
	//ACK
	BLUE_RF2_4G_PKT_STR pkt;
	memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
	PROTO_STR_BTRF24G_TYPE *proto = (void *)pkt.data;
	REMO_CTRL_ACK_STR_24G* remoteACK = (void*)proto->data;
    
	remoteACK->gun_id = 0;
	remoteACK->result = result;
    SerialNumInfo.RemoteCtrlAckSN++;
	SendProtoPkt_BTRF2_4G(CMD_REMOTE_CTRL_24G,&pkt,sizeof(REMO_CTRL_ACK_STR_24G), SerialNumInfo.RemoteCtrlAckSN);
    
    PrintfData("//远程控制应答", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(REMO_CTRL_ACK_STR_24G));
}

/*****************************************************************************
** Function name:       RemoteCtrlProc_BTRF2_4G
** Descriptions:        //远程控制
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void RemoteCtrlProc_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
	REMO_CTRL_REQ_STR_24G *remoteCtrl = (void*)pMsg->data;
	uint8_t result = 0;
	
	switch (remoteCtrl->optCode)
    {
        case SYSTEM_REBOOT:
            CL_LOG("复位系统！\n");
            //ACK
            RemoteCtrlAck_BTRF2_4G(pMsg, result);
            Sc8042bSpeech(VOIC_DEVICE_REBOOT);
            vTaskDelay(2000);
            //重启系统
            NVIC_SystemReset();
        break;
        case START_CHARGING:
            CL_LOG("start charge.\n");
            result = RemoteCtrlBtRf2_4g(remoteCtrl->optCode,remoteCtrl->para);
        break;
        case STOP_CHARGING:
            CL_LOG("stop charge.\n");
            result = RemoteCtrlBtRf2_4g(remoteCtrl->optCode,remoteCtrl->para);
            //Save 发送结束充电通知 to flash
            SaveStopChargingNotice_BTRF2_4G(remoteCtrl->para);
        break;
        default:
            result = 1;
        break;
    }

    //ACK
    RemoteCtrlAck_BTRF2_4G(pMsg, result);
}

/*****************************************************************************
** Function name:       CostTemplateAckBTRF2_4G
** Descriptions:        //计费模板下发应答
** input parameters:    
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void CostTemplateAckBTRF2_4G(uint8_t gun_id, uint8_t result)
{
    //ACK
    BLUE_RF2_4G_PKT_STR pkt;
    memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
    PROTO_STR_BTRF24G_TYPE *proto = (void *)pkt.data;
    COST_TEMPLATE_ACK_STR_24G *templateACK = (void*)proto->data; 
    
    templateACK->gunId = gun_id;
    templateACK->result = result;
    SerialNumInfo.CostTemplateAckSN++;
    SendProtoPkt_BTRF2_4G(CMD_COST_DOWN_24G,&pkt,sizeof(COST_TEMPLATE_ACK_STR_24G),SerialNumInfo.CostTemplateAckSN);
    
    PrintfData("//计费模板下发应答", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(COST_TEMPLATE_ACK_STR_24G));
}

/*****************************************************************************
** Function name:       CostTemplateProc_BTRF2_4G
** Descriptions:        //计费模板下发
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void CostTemplateProc_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
	int ret = CL_OK;
	COST_TEMPLATE_REQ_STR_24G *p_mqtt_cost_down_req = (void*)pMsg->data;
	uint8_t gun_id = p_mqtt_cost_down_req->gunId;
    gun_id = gun_id - system_info.baseGunSn + 1;
    
	segment_str_24g *pSegment = NULL;

	CL_LOG("cost mode = [%d] .\n",p_mqtt_cost_down_req->mode);

	//分功率计费
	if (COST_POWER == p_mqtt_cost_down_req->mode)
    {
    	CL_LOG("分功率计费功率段数目segmentCnt = [%d] .\n", p_mqtt_cost_down_req->Data.powerInfo.segmentCnt);
        if (COST_TEMP_CNT < p_mqtt_cost_down_req->Data.powerInfo.segmentCnt)
		{
            CL_LOG("segmentCnt=%d,error.\n", p_mqtt_cost_down_req->Data.powerInfo.segmentCnt);
            OptFailNotice(203);
            ret = 1;
        }
        else
		{
            for (int i = 0; i < p_mqtt_cost_down_req->Data.powerInfo.segmentCnt; i++) 
            {
                pSegment = &p_mqtt_cost_down_req->Data.powerInfo.segmet[i];
				CL_LOG("i = [%d], startPower = [%d],endPower = [%d],price = [%d],duration = [%d].\n", i, pSegment->startPower, pSegment->endPower, pSegment->price, pSegment->duration);
                if (0 == pSegment->duration) 
                {
                    CL_LOG("duration=%d,error.\n",pSegment->duration);
                    OptFailNotice(204);
                    ret = 2;
                    break;
                }
            }
            if (CL_OK == ret) 
			{
                memcpy(&gCostTemplateInfo.cost_template.Data.powerInfo, &p_mqtt_cost_down_req->Data.powerInfo, sizeof(multiPower_t));
                gCostTemplateInfo.cost_template.mode = COST_POWER;
                gCostTemplateInfo.cost_template.template_id = p_mqtt_cost_down_req->template_id;
                ret = CL_OK;
            }
        }
    }
    else if (COST_UNIFY == p_mqtt_cost_down_req->mode) 
    {//统一计费
        if (0 == p_mqtt_cost_down_req->Data.unifyInfo.duration) 
		{
            CL_LOG("duration=%d,error.\n",pSegment->duration);
            OptFailNotice(204);
            ret = 3;
        }
		else
		{
            memcpy(&gCostTemplateInfo.cost_template.Data.unifyInfo, &p_mqtt_cost_down_req->Data.unifyInfo, sizeof(unify_t));
            gCostTemplateInfo.cost_template.mode = COST_UNIFY;
            gCostTemplateInfo.cost_template.template_id = p_mqtt_cost_down_req->template_id;
            ret = CL_OK;
			CL_LOG("统一计费 price = [%d],duration = [%d].\n", p_mqtt_cost_down_req->Data.unifyInfo.price, p_mqtt_cost_down_req->Data.unifyInfo.duration);
        }
    }
    else
	{
        CL_LOG("cost mode=%d,error.\n",p_mqtt_cost_down_req->mode);
        OptFailNotice(205);
        ret = 4;
    }
	if (ret == CL_OK) 
    {
        CL_LOG("保存计费模板.\n");
        gCostTemplateInfo.cost_template.template_id = p_mqtt_cost_down_req->template_id;
        gCostTemplateInfo.chargerStartingGold = p_mqtt_cost_down_req->chargerStartingGold;
		gCostTemplateInfo.chargerMethod = p_mqtt_cost_down_req->cost_mode;
		CL_LOG("cost_template.template_id = %d,system_info.chargerStartingGold = %d, 1固定收费2实时收费3起步金收费 = %d,mode = %d.\n", gCostTemplateInfo.cost_template.template_id, gCostTemplateInfo.chargerStartingGold, gCostTemplateInfo.chargerMethod, gCostTemplateInfo.cost_template.mode);
		
		FlashWriteCostTemplateData();
    }
	
	//ACK
    CostTemplateAckBTRF2_4G(gun_id, ret);
    
    return;
}


/*****************************************************************************
** Function name:       FWUpgradeAckProc_BTRF2_4G
** Descriptions:        //固件升级应答状态
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void FWUpgradeAckProc_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg, uint8_t upgradeState)
{
	//ACK
	BLUE_RF2_4G_PKT_STR pkt;
	memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
	PROTO_STR_BTRF24G_TYPE *proto = (void *)pkt.data;
	FW_UPGRADE_NOTI_ACK_STR_24G *upgeadeACK = (void*)proto->data;
    
	upgeadeACK->UpgeadeState = upgradeState;   // 0：可以升级； 1：不能升级
	SerialNumInfo.FWUpgradeAckSN++;
	SendProtoPkt_BTRF2_4G(CMD_FW_UPGRADE_NOTI_24G,&pkt,sizeof(FW_UPGRADE_NOTI_ACK_STR_24G),SerialNumInfo.FWUpgradeAckSN);
    
    PrintfData("固件升级应答状态", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(FW_UPGRADE_NOTI_ACK_STR_24G));
}

/*****************************************************************************
** Function name:       FWUpgradeNotiProc_BTRF2_4G
** Descriptions:        //固件升级开始通知
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void FWUpgradeNotiProc_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
	FW_UPGRADE_NOTI_REQ_STR_24G *upgradeReq = (void*)pMsg->data;
    static uint8_t SerialNum = 255;

    PrintfData("FWUpgradeNotiProc_BTRF2_4G接收R6数据", (void*)pMsg, sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(FW_UPGRADE_NOTI_REQ_STR_24G) + 3);
    if(SerialNum == pMsg->head.SerialNum)
    {
        CL_LOG("固件升级开始通知, 收到R6序列号相同的包, 返回! \n");
        return;
    }
    CL_LOG("校验码 Sum = 0x%x, 文件大小 size = %d, 分片总数 num = %d, 固件版本 fwver = %d \n", upgradeReq->checkSum, upgradeReq->file_size, upgradeReq->package_num, upgradeReq->fwverson);
    if(0 != (upgradeReq->file_size % PIECE_PKT_LEN_24G_SEND_TO_X6))
    {
        if(upgradeReq->package_num != ((upgradeReq->file_size / PIECE_PKT_LEN_24G_SEND_TO_X6) + 1))
        {
            CL_LOG("固件升级开始应答, 接收数据错误1 .\n");
            FWUpgradeAckProc_BTRF2_4G(pMsg, 1);
            return;
        }
    }
    else{
        if(upgradeReq->package_num != ((upgradeReq->file_size / PIECE_PKT_LEN_24G_SEND_TO_X6)))
        {
            CL_LOG("固件升级开始应答, 接收数据错误2 .\n");
            FWUpgradeAckProc_BTRF2_4G(pMsg, 1);
            return;
        }
    }
    
	if((0 == GetBlueToothUpgradeFlag()) && (0 == GetUARTUpgradeFlag())) //如果蓝牙和串口都不在升级状态
    {
        //ACK
    	FWUpgradeAckProc_BTRF2_4G(pMsg, 0);
    	Upgrade_Init(upgradeReq->file_size,(uint16_t)upgradeReq->checkSum,upgradeReq->package_num,upgradeReq->fwverson);
    }
    else{
        //ACK
    	FWUpgradeAckProc_BTRF2_4G(pMsg, 1);
        
        //FWDownLoadAckBTRF2_4G(pMsg, 2);     //判断到蓝牙或串口要升级，直接发送停止升级
    }
    CL_LOG("固件升级开始应答.\n");
    
    ClearUpgradeWaitTimer();
}

/*****************************************************************************
** Function name:       FWDownLoadAckBTRF2_4G
** Descriptions:        //固件下发应答
** input parameters:    pMsg: 内层协议数据头指针
                        result:   0：接收成功；1：接收失败；2：停止升级
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void FWDownLoadAckBTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg, uint8_t result)
{
	//ACK
	BLUE_RF2_4G_PKT_STR pkt;
	memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
	PROTO_STR_BTRF24G_TYPE *proto = (void *)pkt.data;
	DOWN_FW_ACK_STR_24G *fwACK = (void*)proto->data;
    Feed_WDT();
	fwACK->result = result;
    fwACK->package_num = fw_info.current_package;
    fwACK->gunId = system_info.baseGunSn;;
    CL_LOG("当前包序号 package_num = %d \n", fwACK->package_num);
	SerialNumInfo.FWDownLoadSN++;
	SendProtoPkt_BTRF2_4G(CMD_FW_DOWN_LOAD_24G,&pkt,sizeof(DOWN_FW_ACK_STR_24G), SerialNumInfo.FWDownLoadSN);
    
    PrintfData("固件下发应答", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(DOWN_FW_ACK_STR_24G) + 3);
}

/*****************************************************************************
** Function name:       FWDownLoadProc_BTRF2_4G
** Descriptions:        //固件下发
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void FWDownLoadProc_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
    uint8_t ret = 0;
	uint8_t checksum = 0;
	DOWN_FW_REQ_STR_24G *pfwInfo = (void*)pMsg->data;
    uint32_t i = 0;
    static uint8_t SerialNum = 255;
	
    if((0 == GetUpgradeReqestFlag())) //如果不在升级状态
    {
        CL_LOG("固件下发, 2.4G不在升级状态! \n");
        FWDownLoadAckBTRF2_4G(pMsg, 2);    //停止升级
        return;
    }
    else if((GetUpgradeCurrentPackage() != pfwInfo->package_num)) //如果R6发送过来的包不是我想要的包
    {
        //if(SerialNum != pMsg->head.SerialNum)
        {
            SerialNum = pMsg->head.SerialNum;
            CL_LOG("固件下发, 收到R6的包不是X6想要的包, 请求X6再次发送! package_num = %06d \n", pfwInfo->package_num);
            FWDownLoadAckBTRF2_4G(pMsg, 0);    //
            
            return;
        }
    }
    else{
        if(SerialNum == pMsg->head.SerialNum)
        {
            CL_LOG("序列号相同, 重发! \n");
            FWDownLoadAckBTRF2_4G(pMsg, 0);    //
            
            return;
        }
        CL_LOG("固件下发包序号 package_num = %d \n", pfwInfo->package_num);
    }
    CL_LOG("升级了 [%d%%]\n", (pfwInfo->package_num*100) / fw_info.package_num);
	for(i = 0;i < pfwInfo->len ; i++)
    {
		checksum += pfwInfo->data[i];
	}
	if((checksum == pfwInfo->checkSum))
    {
        MuxSempTake(&gChgInfo.gProtoMuxType.gUpgradeReportMux);
        Flash_SetPos_w(FIRMWARE_PART, UpgradeRecord.AddressOffset);
		ret = Upgrade_Write(pfwInfo->data, pfwInfo->len);
		#if 0
		if(8 == ret)
		{
			while(1)
			{
				#if 1
				UpgradeFlashErasePage();
				#else
				uint32_t mPage = 0;
				uint32_t currentPageLenth = 0;
	
				mPage = (UpgradeRecord.AddressOffset / 1024);       //计算写了多少页
			    currentPageLenth = UpgradeRecord.AddressOffset % 1024;       //
			    CL_LOG("写了的页数为 mPage =%d, 不足一页的长度 currentPageLenth = %d ! \n", mPage, currentPageLenth);
			    if(mPage >= flash_partition[FIRMWARE_PART].s_count)
			    {
			        CL_LOG("flash超了flash超了flash超了flash超了");
			        fw_info.current_package = 1;
			        UpgradeRecord.current_package = 1;
			    }
			    Flash_SetPos_r(FIRMWARE_PART, mPage * FLASH_PAGE_SIZE);    //以页首地址为偏移, 读取没有写完的这一页，然后再擦除这页，擦除后再写进去

			    MuxSempTake(&gChgInfo.gProtoMuxType.gBuff1024Byte);
			    Flash_Read(FIRMWARE_PART, gFlashCacheBuff, currentPageLenth);  //不需要读整页
			    //PrintfData("升级不足一页数据 Upgrade_Init", gFlashCacheBuff, currentPageLenth);
			    //MuxSempGive(&gChgInfo.gProtoMuxType.gBuff1024Byte);
			    while(1)
			  	{
					for(i = 0; i < (flash_partition[FIRMWARE_PART].s_count - mPage); i++)
					{	//擦除固件备份区
						//HT_Flash_PageErase(FIRMWARE_PART + (mPage + i)*FLASH_PAGE_SIZE);
						Flash_ErasePage(FIRMWARE_PART, (mPage + i));
					}
					Flash_SetPos_w(FIRMWARE_PART, mPage * FLASH_PAGE_SIZE);    //擦除了整页, 也要以首地址为偏移再写进去
					//MuxSempTake(&gChgInfo.gProtoMuxType.gBuff1024Byte);
					if(0 == Flash_Write(FIRMWARE_PART,&gFlashCacheBuff[0], currentPageLenth))	//不要写整页, 后面还没有升级
					{
						break;
					}
					else
					{
						Feed_WDT();
						CL_LOG("flash[%d]写数据! \n", FIRMWARE_PART);
						//osDelay(100);
					}
				}
			    MuxSempGive(&gChgInfo.gProtoMuxType.gBuff1024Byte);
				#endif
				ret = Upgrade_Write(pfwInfo->data, pfwInfo->len);
				if(8 != ret)
				{
					break;
				}
				else
				{
					osDelay(100);
				}
			}
		}
		#endif
        UpgradeRecord.AddressOffset += pfwInfo->len;
        UpgradeRecord.current_package = fw_info.current_package;
        MuxSempGive(&gChgInfo.gProtoMuxType.gUpgradeReportMux);
        
        if(ret == 2)
        {
            PlayVoice(VOIC_DEVICE_REBOOT);
            OS_DELAY_MS(500);
            MuxSempTake(&gChgInfo.gProtoMuxType.gUpgradeReportMux);
            UpgradeRecord.AddressOffset = UPDATE_HEAD_POS;
            UpgradeRecord.current_package = 1;
			FlashWriteData(UPGRADE_PART, (void*)&UpgradeRecord, sizeof(UpgradeRecord));
            //ACK
            FWDownLoadAckBTRF2_4G(pMsg, 2);    //升级完成
            CL_LOG("升级完成复位系统！\n");
            NVIC_SystemReset();
            MuxSempGive(&gChgInfo.gProtoMuxType.gUpgradeReportMux);
        }
        else if(ret == 0)   //校验错误, 重新从头升级
        {
            MuxSempTake(&gChgInfo.gProtoMuxType.gUpgradeReportMux);
            UpgradeRecord.AddressOffset = UPDATE_HEAD_POS;
            UpgradeRecord.current_package = 1;
            //ACK
	        FWDownLoadAckBTRF2_4G(pMsg, 1);
            MuxSempGive(&gChgInfo.gProtoMuxType.gUpgradeReportMux);
        }
        else if(ret == 1)
        {
            MuxSempTake(&gChgInfo.gProtoMuxType.gUpgradeReportMux);
            //ACK
	        FWDownLoadAckBTRF2_4G(pMsg, 0);
            MuxSempGive(&gChgInfo.gProtoMuxType.gUpgradeReportMux);
        }
		
		FlashWriteData(UPGRADE_PART, (void*)&UpgradeRecord, sizeof(UpgradeRecord));
	}
}

/*****************************************************************************
** Function name:       CloseUpgradeReqestFlag
** Descriptions:        //1: 2.4G正在升级
** input parameters:    pMsg
** output parameters:   None
** Returned value:		None
** Author:              quqian
*****************************************************************************/
void CloseUpgradeReqestFlag(void)
{
    gChgInfo.RF24GCardFlag.isNeedSendUpgradeFlag = 1; 
}

/*****************************************************************************
** Function name:       OpenUpgradeReqestFlag
** Descriptions:        //0: 2.4G不在升级状态
** input parameters:    pMsg
** output parameters:   None
** Returned value:		None
** Author:              quqian
*****************************************************************************/
void OpenUpgradeReqestFlag(void)
{
    gChgInfo.RF24GCardFlag.isNeedSendUpgradeFlag = 0; 
}

/*****************************************************************************
** Function name:       GetUpgradeReqestFlag
** Descriptions:        //获取发送升级请求标志
** input parameters:    pMsg
** output parameters:   None
** Returned value:		None
** Author:              quqian
*****************************************************************************/
int GetUpgradeReqestFlag(void)
{
    return gChgInfo.RF24GCardFlag.isNeedSendUpgradeFlag; 
}

/*****************************************************************************
** Function name:       CloseBlueToothUpgradeFlag
** Descriptions:        //关闭蓝牙升级标志
** input parameters:    pMsg
** output parameters:   None
** Returned value:		None
** Author:              quqian
*****************************************************************************/
void CloseBlueToothUpgradeFlag(void)
{
    gChgInfo.RF24GCardFlag.isBT_UpgradeFlag = 0; 
}

/*****************************************************************************
** Function name:       OpenBlueToothUpgradeFlag
** Descriptions:        //open蓝牙升级标志
** input parameters:    pMsg
** output parameters:   None
** Returned value:		None
** Author:              quqian
*****************************************************************************/
void OpenBlueToothUpgradeFlag(void)
{
    gChgInfo.RF24GCardFlag.isBT_UpgradeFlag = 1; 
}

/*****************************************************************************
** Function name:       GetBlueToothUpgradeFlag
** Descriptions:        //获取蓝牙升级标志
** input parameters:    pMsg
** output parameters:   None
** Returned value:		1：表示在升级 0：表示不在升级状态
** Author:              quqian
*****************************************************************************/
int GetBlueToothUpgradeFlag(void)
{
    return gChgInfo.RF24GCardFlag.isBT_UpgradeFlag; 
}

/*****************************************************************************
** Function name:       CloseUARTUpgradeFlag
** Descriptions:        //关闭uart升级标志
** input parameters:    pMsg
** output parameters:   None
** Returned value:		None
** Author:              quqian
*****************************************************************************/
void CloseUARTUpgradeFlag(void)
{
    gChgInfo.RF24GCardFlag.isUART_UpgradeFlag = 0; 
}

/*****************************************************************************
** Function name:       OpenUARTUpgradeFlag
** Descriptions:        //open串口升级标志
** input parameters:    pMsg
** output parameters:   None
** Returned value:		None
** Author:              quqian
*****************************************************************************/
void OpenUARTUpgradeFlag(void)
{
    gChgInfo.RF24GCardFlag.isUART_UpgradeFlag = 1; 
}

/*****************************************************************************
** Function name:       GetUARTUpgradeFlag
** Descriptions:        //获取串口升级标志
** input parameters:    pMsg
** output parameters:   None
** Returned value:		1：表示在升级 0：表示不在升级状态
** Author:              quqian
*****************************************************************************/
int GetUARTUpgradeFlag(void)
{
    return gChgInfo.RF24GCardFlag.isUART_UpgradeFlag; 
}

/*****************************************************************************
** Function name:       FWUpgradeAck_BTRF2_4G
** Descriptions:        //固件升级请求响应
** input parameters:    pMsg
** output parameters:   None
** Returned value:		None
** Author:              quqian
*****************************************************************************/
void FWUpgradeAck_BTRF2_4G(PROTO_STR_BTRF24G_TYPE  *pMsg)
{
    UPGRADE_ACK_STR_BTRF2_4G_TYPE *UpgradeAck = (void*)pMsg->data;

    if(0 == UpgradeAck->result)
    {
        if((0 == GetBlueToothUpgradeFlag()) && (0 == GetUARTUpgradeFlag())) //如果蓝牙和串口都不在升级状态
        {
            //可以升级
            CloseUpgradeReqestFlag();   //关闭发送升级请求标志,也就是打开2.4G升级标志位
            CL_LOG("R6回应的固件升级请求响应，允许升级,GetUpgradeReqestFlag = %d", GetUpgradeReqestFlag());
            ClearUpgradeWaitTimer();
        }
        else{
            CL_LOG("串口或蓝牙在升级, GetUARTUpgradeFlag = %d, GetBlueToothUpgradeFlag = %d", GetUARTUpgradeFlag(), GetBlueToothUpgradeFlag());
        }
    }
    else{
        CL_LOG("R6不允许升级! ");
    }
}

/*****************************************************************************
** Function name:       FWUpgradeREQ_BTRF2_4G
** Descriptions:        //固件升级请求响应
** input parameters:    pMsg
** output parameters:   None
** Returned value:		None
** Author:              quqian
*****************************************************************************/
void FWUpgradeREQ_BTRF2_4G(uint8_t gun_id)
{
    BLUE_RF2_4G_PKT_STR pkt;
    memset(&pkt,0,sizeof(BLUE_RF2_4G_PKT_STR));
    PROTO_STR_BTRF24G_TYPE *proto = (void *)pkt.data;
    UPGRADE_REQ_STR_BTRF2_4G_TYPE *UpgradeReque =  (void*)proto->data;

    UpgradeReque->gun_id = gun_id + system_info.baseGunSn -1;
    CL_LOG("请求升级枪头号 gun_id= %d \n", UpgradeReque->gun_id);
    UpgradeReque->ReqFwVer = (FW_VERSION);	//system_info.fw_version;
    UpgradeReque->AddrOffset = UpgradeRecord.AddressOffset - UPDATE_HEAD_POS;
    SendProtoPkt_BTRF2_4G(CMD_FW_UPGRADE_REQ_24G,&pkt,sizeof(UPGRADE_REQ_STR_BTRF2_4G_TYPE), SerialNumInfo.UpgradeReqSN);
    SerialNumInfo.UpgradeReqSN++;
    
    PrintfData("心跳包发送固件升级请求", (void*)&pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + sizeof(PROTO_HEAD_BTRF24G_TYPE) + sizeof(UPGRADE_REQ_STR_BTRF2_4G_TYPE));
}


/*****************************************************************************
** Function name:       _24gRecvProc
** Descriptions:        根据cmd做相应的处理事件
** input parameters:    pbuff;外层协议除去2_4G模块头部分的数据部分
                        len:2_4G内层协议的报文长度, 除了校验域不包含,包含起始域，长度域，版本域，数据域
** output parameters:   None
** Returned value:		None
** Author:              quqian
*****************************************************************************/
void _24gRecvProc(uint8_t *pbuff, uint16_t len)
{
	PROTO_STR_BTRF24G_TYPE *pMsg = (void *)pbuff;

	gChgInfo.ReceiveR6Ticks = GetRtcCount(); 
	gChgInfo.ReceiveR624gDataFlag = 0xa5;
	switch(pMsg->head.cmd)
	{
        case CMD_CARD_AUTH_24G:		//卡鉴权ACK    R6-> X6
			CardAuthAck_BTRF2_4G(pMsg);	
        break;
        case CMD_CARD_AUTH_RESULT_RETURN_24G:   //鉴权结果返回
        	if(gChgInfo.isOverTimeFlag == 0)
			{
				CL_LOG("已经开启充电 或 开启充电已经超时 .\n");
				break;
			}
            CloseCheckTimeOver();
			CardAuthResult_BTRF2_4G(pMsg);
        break;
		case CMD_START_CHARGER_24G:		//请求启动充电  R6-> X6
			StartChargingProc_BTRF2_4G(pMsg);
            CL_LOG("请求启动充电R6->X6 .\n");
        break;
		case CMD_STOP_CHARGER_24G:	//请求结束充电  R6-> X6
			CL_LOG("收到R6的结束充电请求R6->X6 .\n");
			StopChargingProc_BTRF2_4G(pMsg);	
        break;
        case CMD_CHARGER_STARTED_24G:	//充电开始通知 
			StartChargingNoticeACK_BTRF2_4G(pMsg);
        break;
        case CMD_CHARGER_STOPED_24G:	//充电结束通知
			CL_LOG("充电结束通知ack .\n");
            R6ToX6StopChargingAckBTRF2_4G(pMsg);
        break;
		case CMD_HEART_BEAT_24G:		//心跳
			HeartBatACK_BTRF2_4G(pMsg);
        break;
		case CMD_COST_DOWN_24G:			//计费模板下发  R6 -> X6
			CL_LOG("X6收到R6的计费模板 .\n");
			CostTemplateProc_BTRF2_4G(pMsg);	
        break;
		case CMD_FW_UPGRADE_NOTI_24G:	//固件升级
		    ClearUpgradeWaitTimer();
			FWUpgradeNotiProc_BTRF2_4G(pMsg);
        break;
		case CMD_FW_DOWN_LOAD_24G:		//固件下发
		    ClearUpgradeWaitTimer();
			FWDownLoadProc_BTRF2_4G(pMsg);
        break;
        case CMD_FW_UPGRADE_REQ_24G:	//请求固件升级
		    FWUpgradeAck_BTRF2_4G(pMsg);
        break;
		case CMD_REMOTE_CTRL_24G:		//远控控制
			RemoteCtrlProc_BTRF2_4G(pMsg);
        break;
		case CMD_ENUM_BIND_ACK:
			R6X6BindREQ_BTRF2_4G(pMsg);	//绑定确认
        break;
		case CMD_SCAN_CODE_START_CHARGE:
			R6ToX6ScanCodeStartChargerAck(pMsg);	
        break;
		default:
			CL_LOG(".\n");
        break;
	}
}


