
#include "blue.h"
#include "ckb24_um_handle.h"
#include "system_info.h"
#include "flash_usr.h"
#include "charger.h"
#include "sc8042b.h"
#include "emu_info.h"
#include "upgrade.h"

//心跳  设备登陆后 周期发送心跳数据
void BlueSendHeartBeat(void)
{
    uint8_t gunId = 1;
    GUN_STATUS_STR GunStatus;
	BLUE_RF2_4G_PKT_STR stk;
	uint8_t gunChargeStatus[GUN_NUM_MAX];
	memset(&stk, 0, sizeof(BLUE_RF2_4G_PKT_STR));
	GetGunStatus(gunId, &GunStatus);
	
    BLUE_PROTO_STR *pBluePkt = (void*)stk.data;
    HEART_BEAT_STR *pHeartACK = (void*)pBluePkt->data;
    //BLUE_GUN_HEART_BEAT_STR *pGunHeart = pHeart->gunStatus;
//    gun_info_t *pGunInfo = NULL;

    pHeartACK->simSignal = 0;
    pHeartACK->temp = GetCpuTemp() + 50;
    pHeartACK->portCnt = GUN_NUM_MAX;
	for(gunId = 1; gunId <= GUN_NUM_MAX; gunId++)
    {
    	if((GUN_STATE_IDLE == gun_info[gunId - 1].gun_state) || (GUN_STATE_OFF == gun_info[gunId - 1].gun_state))
		{
			CL_LOG("插头[%d]空闲\n ", gunId);
			gunChargeStatus[gunId - 1] = 0;
		}
		else if((GUN_STATE_WAIT_PLUG_IN == gun_info[gunId - 1].gun_state) || (GUN_STATE_ON == gun_info[gunId - 1].gun_state))
		{
			CL_LOG("插头[%d]占用\n ", gunId);
			gunChargeStatus[gunId - 1] = 1;
		}

		pHeartACK->gunStatus[gunId - 1].port = gunId + system_info.baseGunSn - 1;
		pHeartACK->gunStatus[gunId - 1].status = gunChargeStatus[gunId - 1];
		pHeartACK->gunStatus[gunId - 1].errCode = (GunStatus.status == 0)?0:1;
	}
	
    pBluePkt->head.start_ee = 0xee;
    pBluePkt->head.cmd = BT_HEART_BEAT;
    pBluePkt->head.len = sizeof(HEART_BEAT_STR);
    pBluePkt->data[pBluePkt->head.len] = CheckSum((void*)pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt->head.len);
	
	SendBlueTooth2_4GPkt(NODE_BLUE, &stk, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt->head.len + 1);
	CL_LOG("给手机发送蓝牙心跳包\n ");
}

//手机时间暂时不处理
void ShakeReqProc(BLUE_PROTO_STR *pMsg)
{
    BLUE_SHAKE_ACK_STR *pAck = (void*)pMsg->data;
    BLUE_RF2_4G_PKT_STR *pkt = (void*)((uint8_t*)pMsg-sizeof(BLUE_HEAD_SEND_STR));
	//uint32_t cnt = 0;
	BLUE_SHAKE_REQ_STR *shakeReq = (void*)pMsg->data;
	
	if(shakeReq->time != 0)
	{
		time_t now = GetRtcCount();
		CL_LOG("本地时间Time = [%d]\n", now);
		//cnt = (now > shakeReq->time) ? now - shakeReq->time : shakeReq->time - now;
		
		//if ((30 < cnt)) && (0 == GetChargingGunCnt()))
		{
			//系统对时
			SetRtcCount(shakeReq->time+1);
			gChgInfo.heartReceiveFlag = 1;
			CL_LOG("set rtc time.\n");
		}
	}
	CL_LOG("phonesSn:[%s] 握手时间Time = [%d]\n",shakeReq->phonesSn,shakeReq->time);
	
    memset(pAck, 0, sizeof(BLUE_SHAKE_ACK_STR));
    memcpy(pAck->name, CHARGER_NAME, strlen(CHARGER_NAME));
    memcpy(pAck->chargerSn, &gSystemInfo1.station_id[3], sizeof(pAck->chargerSn));
    pAck->fwVer = FW_VERSION;
    pAck->portCnt = GUN_NUM_MAX;
    pAck->startNo = system_info.baseGunSn;
    //内层
    //pMsg->head.start_ee = 0xee;
    //pMsg->head.cmd = pMsg->head.cmd;
    pMsg->head.len = sizeof(BLUE_SHAKE_ACK_STR);
    pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
    //外层
	//pkt->head.len = sizeof(BLUE_PROTO_HEAD_STR) + pMsg->head.len + INNER_CHECK_SUM;
    SendBlueTooth2_4GPkt(NODE_BLUE, pkt, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
	CL_LOG("给手机发送蓝牙握手包应答 和 发送心跳包\n ");

	gChgInfo.blue_connectstate = 1;
	//发送心跳
	BlueSendHeartBeat();
}

void BlueToothSet2_4gAddrAck(BLUE_PROTO_STR *pMsg, uint8_t Result)
{
	BLUE_2_4G_ADDR_ACK_STR *pAck = (void*)pMsg->data;
    BLUE_RF2_4G_PKT_STR *pkt = (void*)((uint8_t*)pMsg-sizeof(BLUE_HEAD_SEND_STR));
        
    memset(pAck, 0, sizeof(BLUE_2_4G_ADDR_ACK_STR));
    pAck->Result = Result;
    //内层
    //pMsg->head.start_ee = 0xee;
    //pMsg->head.cmd = pMsg->head.cmd;
    pMsg->head.len = sizeof(BLUE_SHAKE_ACK_STR);
    pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
    //外层
	//pkt->head.len = sizeof(BLUE_PROTO_HEAD_STR) + pMsg->head.len + INNER_CHECK_SUM;
    SendBlueTooth2_4GPkt(NODE_BLUE, pkt, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
	CL_LOG("设置2.4g地址返回应答\n ");
}

void BlueToothSet2_4gAddr(BLUE_PROTO_STR *pMsg)
{
	uint8_t CompareLocal24gMacAddr[MAC_LEN];					    //本地2.4mac地址
	BLUE_2_4G_ADDR_STR *p2_4gAddr = (void*)pMsg->data;
	uint8_t BlueToothResetFlag = 0;
	
	CompareLocal24gMacAddr[0] = p2_4gAddr->Local24gAddr[4];
	CompareLocal24gMacAddr[1] = p2_4gAddr->Local24gAddr[3];
	CompareLocal24gMacAddr[2] = p2_4gAddr->Local24gAddr[2];
	CompareLocal24gMacAddr[3] = p2_4gAddr->Local24gAddr[1];
	CompareLocal24gMacAddr[4] = p2_4gAddr->Local24gAddr[0];

	if(0 != memcmp(gGunInfo2.Local24gMacAddr, CompareLocal24gMacAddr, sizeof(gGunInfo2.Local24gMacAddr)))
	{
		BlueToothResetFlag = 1;
		memcpy(gGunInfo2.Local24gMacAddr, CompareLocal24gMacAddr, sizeof(CompareLocal24gMacAddr));
		//CL_LOG("2.4g地址有改变，擦除所有订单\n");
		//Flash_EraseALL(ORDER_PART);
		gGunInfo1.orderInfo.rIndex = 0;
		gGunInfo1.orderInfo.wIndex = 0;
		FlashWriteGunInfo_1_Data();

		gGunInfo2._24gMacAddr[0] = p2_4gAddr->Local24gAddr[0];
		gGunInfo2._24gMacAddr[1] = p2_4gAddr->Local24gAddr[1];
		gGunInfo2._24gMacAddr[2] = p2_4gAddr->Local24gAddr[2];
		gGunInfo2._24gMacAddr[3] = p2_4gAddr->Local24gAddr[3];
		gGunInfo2._24gMacAddr[4] = p2_4gAddr->Local24gAddr[4] & (~0x3f);
		
		system_info.baseGunSn = (gGunInfo2.Local24gMacAddr[0] & 0x3f) * 2 - 1;
		CL_LOG("设备枪头号baseGunSn = %d, system_info.Local24gMacAddr[0] = %02x \n", system_info.baseGunSn, gGunInfo2.Local24gMacAddr[0]);
		
		FlashWriteGunInfo_2_Data();
	}
	PrintfData("蓝牙设置网关2.4g地址", (void*)gGunInfo2._24gMacAddr, sizeof(gGunInfo2._24gMacAddr));
	PrintfData("蓝牙设置本地2.4g地址", (void*)p2_4gAddr->Local24gAddr, sizeof(gGunInfo2.Local24gMacAddr));
    BlueToothSet2_4gAddrAck(pMsg, 0);
	vTaskDelay(1000);
	if(1 == BlueToothResetFlag)
	{
		gChgInfo.RF24GCardFlag.isBTSet24gAddrFlag = 1;
	}
}

void BlueToothSetDevIDAck(BLUE_PROTO_STR *pMsg, uint8_t Result)
{
	BLUE_DEV_ID_ACK_STR *pAck = (void*)pMsg->data;
    BLUE_RF2_4G_PKT_STR *pkt = (void*)((uint8_t*)pMsg-sizeof(BLUE_HEAD_SEND_STR));

    memset(pAck, 0, sizeof(BLUE_DEV_ID_ACK_STR));
    pAck->Result = Result;
    //内层
    //pMsg->head.start_ee = 0xee;
    //pMsg->head.cmd = pMsg->head.cmd;
    pMsg->head.len = sizeof(BLUE_SHAKE_ACK_STR);
    pMsg->data[pMsg->head.len] = GetPktSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
    //外层
	//pkt->head.len = sizeof(BLUE_PROTO_HEAD_STR) + pMsg->head.len + INNER_CHECK_SUM;
    SendBlueTooth2_4GPkt(NODE_BLUE, pkt, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1);
}

//蓝牙断开
void BlueReqBreak(BLUE_PROTO_STR *pMsg)
{
	//ACK
	BLUE_RF2_4G_PKT_STR stk;
	memset(&stk,0,sizeof(BLUE_RF2_4G_PKT_STR));
	
	BLUE_DISCONNECT_DEV_ACK_STR *pACK = (void*)pMsg->data;
	pACK->status = 0;
	//内层
	pMsg->head.len = sizeof(BLUE_DISCONNECT_DEV_ACK_STR);
    pMsg->data[pMsg->head.len] = CheckSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
	//外层
	stk.head.len = sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1;
	memcpy(stk.data,pMsg,stk.head.len);
	SendBlueTooth2_4GPkt(NODE_BLUE,(BLUE_RF2_4G_PKT_STR*)&stk,stk.head.len);

	OS_DELAY_MS(500);	
	
	//断开蓝牙链接
	SetBluePair(1);
	gChgInfo.blue_connectstate = 0;
	PlayVoice(VOIC_BLUETOOTH_OFFLINE);
}

void BlueToothSetDevID(BLUE_PROTO_STR *pMsg)
{
	BLUE_DEV_ID_STR *pdevID = (void*)pMsg->data;

	memcpy(gSystemInfo1.station_id, pdevID->DeviceID, sizeof(pdevID->DeviceID));
	
	FlashWriteDataToSystemInfo1();
	PrintfData("蓝牙设置设备号", (void*)gSystemInfo1.station_id, 5);
    BlueToothSetDevIDAck(pMsg, 0);
	vTaskDelay(1000);
	//NVIC_SystemReset();
}

uint16_t GetHistoryOrderNum(void)
{
    ORDER_STR *pOrder = &gGunInfo1.orderInfo;
    uint16_t datalen;


	if((pOrder->wIndex > RECORD_MAX_BLOCK_NUM) || (pOrder->rIndex > RECORD_MAX_BLOCK_NUM))
	{
        CL_LOG("历史订单pOrder->wIndex = [%d], pOrder->rIndex = [%d],错误!\n", pOrder->wIndex, pOrder->rIndex);
		pOrder->wIndex = 0;
		pOrder->rIndex = 0;
		FlashWriteGunInfo_1_Data();
	}
	
    if (pOrder->wIndex >= pOrder->rIndex)
    {
        datalen = pOrder->wIndex - pOrder->rIndex + 1;
    }
    else
	{
        datalen = pOrder->wIndex + RECORD_MAX_BLOCK_NUM - pOrder->rIndex + 1;
    }
    
	return datalen;
}

void BlueSendStartChargingAck(BLUE_PROTO_STR *pMsg, uint8_t gunId, uint8_t result, uint8_t reason)
{
    //ACK
	BLUE_RF2_4G_PKT_STR stk;
	BLUE_START_CHARGING_ACK_STR *pStartAck = (void*)pMsg->data;
	gun_info_t *pGunInfo = &gun_info[gunId-1];

	memset(&stk,0,sizeof(BLUE_RF2_4G_PKT_STR));
	
	pStartAck->port = gunId;
    pStartAck->result = (result == 0?0:1);
    pStartAck->reason = (result==0 ? 0 : result);
	pStartAck->startTime = GetRtcCount();//pGunInfo->start_time;
    pStartAck->startElec = pGunInfo->startChargerElec;
	CL_LOG("开启充电应答时间 [%d], [%d] ", pStartAck->startTime, pGunInfo->start_time);
	//内层
    pMsg->head.len = sizeof(BLUE_START_CHARGING_ACK_STR);
    pMsg->data[pMsg->head.len] = CheckSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
	//外层
	stk.head.len = sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1;
	memcpy(stk.data,pMsg,stk.head.len);
	SendBlueTooth2_4GPkt(NODE_BLUE,(BLUE_RF2_4G_PKT_STR*)&stk,stk.head.len);
}

//心跳响应
void BlueSendHeartBeatACK(BLUE_PROTO_STR *pMsg)
{
	BLUE_HEART_BEAT_ACK_STR *pHeartReceive = (BLUE_HEART_BEAT_ACK_STR*)pMsg->data;

	if(pHeartReceive->result == 0)
	{
		if(pHeartReceive->time != 0)
		{
			int cnt;
			time_t now = GetRtcCount();
			CL_LOG("心跳获取本地时间Time = [%d]\n", now);
			CL_LOG("接收心跳时间Time = [%d]\n", pHeartReceive->time);
			cnt = (now > pHeartReceive->time) ? now - pHeartReceive->time : pHeartReceive->time - now;
			
			if ((30 < cnt) && (0 == GetChargingGunCnt()))
			{
				//系统对时
				SetRtcCount(pHeartReceive->time+1);
				gChgInfo.heartReceiveFlag = 1;
				CL_LOG("set rtc time.\n");
			}			
		}
		else{
			CL_LOG("接收蓝牙心跳时间为零.\n");
		}
	}
	else{
		CL_LOG("接收蓝牙心跳失败.\n");
	}




	#if 0
    //ACK
	BLUE_RF2_4G_PKT_STR stk;
	HEART_BEAT_STR *pHeartACK = (void*)pMsg->data;
	uint8_t gunId = 1;
	GUN_STATUS_STR GunStatus;
	uint8_t gunChargeStatus[GUN_NUM_MAX];
	//gun_info_t *pGunInfo = &gun_info[gunId-1];

	memset(&stk,0,sizeof(BLUE_RF2_4G_PKT_STR));
	GetGunStatus(gunId, &GunStatus);
	
	pHeartACK->simSignal = 0;
	pHeartACK->temp = GunStatus.temp;
	pHeartACK->portCnt = 2;
	for(gunId = 1; gunId <= GUN_NUM_MAX; gunId++)
    {
    	if((GUN_STATE_IDLE == gun_info[gunId - 1].gun_state) || (GUN_STATE_OFF == gun_info[gunId - 1].gun_state))
		{
			gunChargeStatus[gunId - 1] = 0;
		}
		else if((GUN_STATE_WAIT_PLUG_IN == gun_info[gunId - 1].gun_state) || (GUN_STATE_ON == gun_info[gunId - 1].gun_state))
		{
			gunChargeStatus[gunId - 1] = 1;
		}

		pHeartACK->gunStatus[gunId - 1].port = gunId + system_info.baseGunSn - 1;
		pHeartACK->gunStatus[gunId - 1].status = gunChargeStatus[gunId - 1];
		pHeartACK->gunStatus[gunId - 1].errCode = (GunStatus.status == 0)?0:1;
	}

	//内层
    pMsg->head.len = sizeof(HEART_BEAT_STR);
    pMsg->data[pMsg->head.len] = CheckSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
	//外层
	stk.head.len = sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1;
	memcpy(stk.data, pMsg, stk.head.len);
	SendBlueTooth2_4GPkt(NODE_BLUE,(BLUE_RF2_4G_PKT_STR*)&stk,stk.head.len);
	#endif
}

void BlueCostTemlteAck(BLUE_PROTO_STR *pMsg)
{
	COST_TEMPLATE_ACK_STR *costAck = (void*)pMsg->data;
	CL_LOG("手机应答计费模板gunid[%d], 结果值[%d] \n", costAck->gunId, costAck->result);
}
//计费模板上传
void BlueCostTemplateUpload(uint8_t gunid)
{
	uint16_t temp;
	BLUE_RF2_4G_PKT_STR stk;
	BLUE_PROTO_STR *pBluePkt = (void*)stk.data;
	
	COST_TEMPLATE_HEAD_STR *cost = (void*)pBluePkt->data;
	uint16_t datalen = 6;

	memset(&stk,0,sizeof(BLUE_RF2_4G_PKT_STR));
	
	cost->gunId = gCostTemplateInfo.cost_template.gunId;
	cost->template_id = gCostTemplateInfo.cost_template.template_id;
	cost->mode = gCostTemplateInfo.cost_template.mode;
	if(gCostTemplateInfo.cost_template.mode == COST_UNIFY)
	{
		temp = sizeof(unify_t);
	}
	else if(gCostTemplateInfo.cost_template.mode == COST_POWER)
	{
		temp = sizeof(segment_str)*gCostTemplateInfo.cost_template.Data.powerInfo.segmentCnt + 1;
	}
	
	memcpy(cost->Data, &gCostTemplateInfo.cost_template.Data, temp);
	cost->Data[temp] = gCostTemplateInfo.chargerMethod;
	cost->Data[temp+1] = gCostTemplateInfo.chargerStartingGold;
	cost->Data[temp+2] = gCostTemplateInfo.chargerStartingGold>>8;
	datalen += temp+1;
	
	pBluePkt->head.start_ee = 0xee;
    pBluePkt->head.cmd = BT_COST_TEMPLATE_UPLOAD;
    pBluePkt->head.len = datalen;
    pBluePkt->data[pBluePkt->head.len] = CheckSum((void*)pBluePkt, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt->head.len);
	
	PrintfData("send Cost_Template data:给手机发送计费模板",(void*)pBluePkt,sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt->head.len+1);
	
	SendBlueTooth2_4GPkt(NODE_BLUE,&stk, sizeof(BLUE_PROTO_HEAD_STR)+pBluePkt->head.len+1);
}

//请求计费模板
void BlueReqCostTemlteProc(BLUE_PROTO_STR *pMsg)
{
	BLUE_COST_TEMPLATE_REQ *cost = (void*)pMsg->data;
	if((cost->template_id != gCostTemplateInfo.cost_template.template_id)  && (gCostTemplateInfo.cost_template.template_id != 0))
	{
		CL_LOG("与手机计费模板ID不一样, 给手机发送计费模板 \n");
		BlueCostTemplateUpload(cost->gun_id);
	}
}

//固件升级通知
void BlueFWUpgradeNotice(BLUE_PROTO_STR *pMsg)
{
	uint8_t result = 0;
	
	BLUE_DOWN_FW_REQ_STR *pFW = (void*)pMsg->data;
	
	if((pFW->fw_version != FW_VERSION)/* || (pFW->fw_version != FW_VERSION_SUB)*/)
	{
		//记录固件信息
		Upgrade_Init(pFW->fw_size,pFW->checkSum,pFW->package_num,pFW->fw_version);
	}
	else
	{
		CL_LOG("fw_version is equal,cann't upgrade.\n");
		result = 1;
	}
	
	//ACK
	BLUE_RF2_4G_PKT_STR stk;
	memset(&stk,0,sizeof(BLUE_RF2_4G_PKT_STR));
	
	BLUE_DOWN_FW_ACK_STR *pACK = (void*)pMsg->data;
	pACK->result = result;
	//内层
	pMsg->head.len = sizeof(BLUE_DOWN_FW_ACK_STR);
	pMsg->data[pMsg->head.len] = CheckSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
	//外层
	stk.head.len = sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1;
	memcpy(stk.data,pMsg,stk.head.len);
	SendBlueTooth2_4GPkt(NODE_BLUE,(BLUE_RF2_4G_PKT_STR*)&stk,stk.head.len);	
}

//固件下发
void BlueFWDownLoad(BLUE_PROTO_STR *pMsg)
{
	BLUE_FW_DOWNLOAD_REQ_STR *pFW = (void*)pMsg->data;
	
	uint8_t index = pFW->data[0];
	
	//保存数据
	uint8_t result = COM_Upgrade_Write(pFW->data,pMsg->head.len);
	
	//ACK
	BLUE_RF2_4G_PKT_STR stk;
	memset(&stk,0,sizeof(BLUE_RF2_4G_PKT_STR));
	
	BLUE_FW_DOWNLOAD_ACK_STR *pACK = (void*)pMsg->data;
	pACK->result = (result == 0 ? 1:0);
	pACK->index = index;
	//内层
	pMsg->head.len = sizeof(BLUE_FW_DOWNLOAD_ACK_STR);
	pMsg->data[pMsg->head.len] = CheckSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len); 
	//外层
	stk.head.len = sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1;
	memcpy(stk.data,pMsg,stk.head.len);
	SendBlueTooth2_4GPkt(NODE_BLUE,(BLUE_RF2_4G_PKT_STR*)&stk,stk.head.len);	
	
	if(result == 2)
	{
		PlayVoice(VOIC_DEVICE_REBOOT);
		CL_LOG("Blue重启系统！\n");
		//CloseBlueToothUpgradeFlag();
		OS_DELAY_MS(500);
		Reset_CPU();
	}
}

//蓝牙停止充电
void BlueStopCharging(BLUE_PROTO_STR *pMsg)
{
	uint8_t result = 0;
	
    BLUE_STOP_CHARGING_STR *pStop = (void*)pMsg->data;
	uint8_t gun_id = pStop->port;
    gun_info_t *pGunInfo = &gun_info[gun_id-1];

    if ((0 == gun_id) || (GUN_NUM_MAX < gun_id))
    {
        CL_LOG("port=%d,error.\n",pStop->port);
		result = 1;
    }
	//ACK
	BLUE_RF2_4G_PKT_STR stk;
	GUN_STATUS_STR gunStatus;
	memset(&stk,0,sizeof(BLUE_RF2_4G_PKT_STR));
	
	BLUE_STOP_CHARGING_ACK_STR *pStopAck = (void*)pMsg->data;
	GetGunStatus(gun_id, &gunStatus);
    pMsg->head.len = sizeof(BLUE_STOP_CHARGING_ACK_STR);
    pStopAck->port = pStop->port;
    pStopAck->result = result;
	pStopAck->stopTime = GetRtcCount();
	pStopAck->start_time = pGunInfo->start_time;
    pStopAck->startElec = pGunInfo->startChargerElec;
	pStopAck->money = pGunInfo->money;
    pStopAck->chargerPower = pGunInfo->chargingPower;
	CL_LOG("手机蓝牙结束充电请求时间Time = [%d]\n", pStopAck->stopTime);
	pStopAck->stopElec = gunStatus.elec;
	pStopAck->stopReason = 1;
	//内层
	pMsg->head.len =sizeof(BLUE_STOP_CHARGING_ACK_STR);
    pMsg->data[pMsg->head.len] = CheckSum((void*)pMsg, sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len);
	//外层
	stk.head.len = sizeof(BLUE_PROTO_HEAD_STR)+pMsg->head.len+1;
	memcpy(stk.data,pMsg,stk.head.len);
	SendBlueTooth2_4GPkt(NODE_BLUE,(BLUE_RF2_4G_PKT_STR*)&stk, stk.head.len);
	
	if(result == 0)
	{
		pGunInfo->stopReason = STOP_BY_BLUETOOTH;
		pGunInfo->reasonDetail = REASON_BLUE;
		CL_LOG("手机蓝牙结束充电\n");
		Proto2_4gStopCharging(pStop->port);
	}
}

//需要判断离线的时候如果已经没有缓存订单空间就不能再开启充电
void BlueStartCharging(BLUE_PROTO_STR *pMsg)
{
    //BLUE_RF2_4G_PKT_STR stk;
    BLUE_START_CHARGING_STR *startChargingReq = (void*)pMsg->data;
    int ret;
	uint8_t result = 0;
    uint8_t gun_id = startChargingReq->port - system_info.baseGunSn + 1;
	gun_info_t *pGunInfo = &gun_info[gun_id-1];
    
    PrintfData("扫码蓝牙开启充电请求接收报文", (uint8_t *)pMsg->data, sizeof(BLUE_START_CHARGING_STR));
	//如果该枪头正在充电，订单号一致 ，认为充电已经开启
	if(pGunInfo->is_load_on)
    {
		CL_LOG("枪头正在充电pGunInfo->is_load_on %d \n", pGunInfo->is_load_on);
        BlueSendStartChargingAck(pMsg, gun_id, 0, 0);
        return;
	}

	//离线订单数量判断
	if(GetHistoryOrderNum() >= RECORD_MAX_BLOCK_NUM)
	{
		CL_LOG("HistoryOrderNum is to many:%d \n",GetHistoryOrderNum());
		BlueSendStartChargingAck(pMsg, gun_id, 1, ret);
        return;
	}

	//充电参数判断
    if (CL_OK != (ret = CheckStartPara(gun_id))) {
        CL_LOG("call CheckStartPara fail.\n");
        BlueSendStartChargingAck(pMsg, gun_id, 1, ret);
        return;
    }

    system_info.isStartCharge[gun_id - 1].gun_id = gun_id;
    system_info.isStartCharge[gun_id - 1].mode = startChargingReq->mode;
    system_info.isStartCharge[gun_id - 1].chargingPara = startChargingReq->chargingPara;
    gChgInfo.money = startChargingReq->chargingPara;
    if(0 != memcmp(startChargingReq->order,gun_info[gChgInfo.current_usr_gun_id-1].order,sizeof(startChargingReq->order)))
    {
        //SerialNumInfo.StartChargeSN++;
        system_info.isStartCharge[gun_id - 1].isStartChargeSN = system_info.isStartCharge[gun_id - 1].isStartChargeSN + 10; //为了SN不同
        memcpy(system_info.isStartCharge[gun_id - 1].order, startChargingReq->order, sizeof(startChargingReq->order));
    }
    
    //system_info.isStartCharge[gun_id - 1].isNeedToStartChargeFlag = 1;  //需要开启充电
    //if(1 == system_info.isStartCharge[gun_id - 1].isNeedToStartChargeFlag)  //如果需要开启充电
    {
        //system_info.isStartCharge[gun_id - 1].isNeedToStartChargeFlag = 0;  //需要开启充电
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
    }
	
	FlashWriteSystemInfoData();
	//ACK
    BlueSendStartChargingAck(pMsg, gun_id, 0, 0);
}

void BluePktProc(BLUE_PROTO_STR *pPkt, uint8_t len)
{
    CL_LOG("接收到蓝牙数据 , cmd = 0x%02X\n", pPkt->head.cmd);

	switch (pPkt->head.cmd)
	{
        case BT_SHAKE_REQ:
			PrintfData("接收到手机蓝牙握手包", (void*)pPkt, len);
            ShakeReqProc(pPkt);
			Sc8042bSpeech(VOIC_BLUETOOTH_ONLINE);
        break;
        case BT_START_CHARGING:
			PrintfData("接收到手机蓝牙开启充电", (void*)pPkt, len);
            BlueStartCharging(pPkt);
        break;
        case BT_STOP_CHARGING:
            PrintfData("接收到手机蓝牙停止充电", (void*)pPkt, len);
			BlueStopCharging(pPkt);
        break;
        case BT_HEART_BEAT:
			PrintfData("接收到手机蓝牙心跳包", (void*)pPkt, len);
			BlueSendHeartBeatACK(pPkt);
        break;
        case BT_COST_TEMPLATE_UPLOAD:
			PrintfData("接收到手机蓝牙的计费模板上传应答", (void*)pPkt, len);
			BlueCostTemlteAck(pPkt);
        break;
        case BT_REQ_COST_TEMPLATE:
			PrintfData("接收到手机蓝牙请求计费模板", (void*)pPkt, len);
			BlueReqCostTemlteProc(pPkt);
        break;
        case BT_FW_UPGRADE_NOTICE:
			OpenUpgradeReqestFlag();
			CloseUARTUpgradeFlag();
			OpenBlueToothUpgradeFlag();
			PrintfData("接收到手机蓝牙固件升级开始通知", (void*)pPkt, len);
			BlueFWUpgradeNotice(pPkt);
        break;
        case BT_FW_DOWN_LOAD:
			PrintfData("接收到手机蓝牙固件下发", (void*)pPkt, len);
			BlueFWDownLoad(pPkt);
        break;
        case BT_REQ_BREAK:
			PrintfData("接收到手机请求蓝牙断开连接", (void*)pPkt, len);
			BlueReqBreak(pPkt);
        break;
        case BT_CMD_SET_CHARGER_SN:
			PrintfData("接收到设置充电桩编号", (void*)pPkt, len);
			//BlueToothSetDevID(pPkt);
        break;
        case BT_CMD_SET_GW_ADDR:
			BlueToothSet2_4gAddr(pPkt);
        break;
        case BT_CMD_REMO_CTRL:
			PrintfData("接收到手机蓝牙远程控制", (void*)pPkt, len);
        break;
    }
    //发送报文应答
}

//手机蓝牙数据接收处理
void RecvBtData(void)
{
    uint8_t  gBtRxData[255] = {0,};
    uint8_t  *pBuff = &gBtRxData[sizeof(BLUE_HEAD_SEND_STR)];
    static uint8_t  len;
    static uint8_t  pktLen;
    uint8_t  data;
    static uint8_t  step = ENUM_FIND_AA;
    static uint8_t  sum;

	while (CL_OK == FIFO_S_Get(&gCKBCtrl.rxBtBuff, &data))
	{
        switch (step) {
            case BT_FIND_EE:
                if (0xee == data)
				{
                    pktLen = 0;
                    pBuff[pktLen++] = data;
                    step = BT_FIND_CMD;
                    sum = 0xee;
                }
                break;

            case BT_FIND_CMD:
                pBuff[pktLen++] = data;
                sum += data;
                step = BT_FIND_LEN;
                break;

            case BT_FIND_LEN:
                pBuff[pktLen++] = data;
                sum += data;
                len = data;
                if (sizeof(gBtRxData) < len)
				{
                    step = BT_FIND_EE;
                }
				else{
                    step = BT_RX_DATA;
                }
                break;

            case BT_RX_DATA:
                pBuff[pktLen++] = data;
                sum += data;
                if (0 == --len) {
                    step = ENUM_FIND_CHK;
                }
                break;

           case ENUM_FIND_CHK:
                pBuff[pktLen++] = data;
                if (sum == data)
                {
                    BluePktProc((void*)pBuff, pktLen);
                }
                else{
                    PrintfData("接收的错误蓝牙数据RecvBtData", pBuff, pktLen);
                    printf("sum=%02x,pkt sum=%02x,error,drop pkt.\n",sum,data);
                }
                step = BT_FIND_EE;
                break;
        }
    }
}

