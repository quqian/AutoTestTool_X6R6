#include "charger.h"
#include "system_info.h"
#include "emu_info.h"
#include "flash_usr.h"
#include "sc8042b.h"
#include "rtc.h"
#include "main_handle.h"
#include "upgrade.h"
#include "delay.h"

/**
*枪头信息
*/
gun_info_t gun_info[GUN_NUM_MAX];

/**
*当前正在充电信息
*/
CHG_INFO_STR gChgInfo;

/**
*各个枪头充电时间
*/
uint32_t chargingOldTime = 0;


//1：按功率段计费 2按统一收费 #define COST_POWER 1  #define COST_UNIFY 2
int GetCostMode(void)
{
    return gCostTemplateInfo.cost_template.mode;
}


int GetChargingTime(int gunId, int power)
{
    uint32_t i = 0;
    gun_info_t *pGunInfo = &gun_info[gunId-1];
    segment_str *pSegMent = pGunInfo->powerInfo.segmet;

    if (COST_POWER == pGunInfo->mode)
    {
        for (i = 0; i < pGunInfo->powerInfo.segmentCnt; i++)
        {
            if ((pSegMent->startPower <= (power/10)) && ((power/10) <= pSegMent->endPower))
            {
                pGunInfo->powerSemenIndex = i;
                pGunInfo->charger_time_plan = pGunInfo->current_usr_money*pSegMent->duration/pSegMent->price;
                if (CHARGING_START == pGunInfo->chargerMethod) 
                {
					//起步金消耗时间
					pGunInfo->startGoldTime = pGunInfo->chargerStartingGold * pSegMent->duration / pSegMent->price;
				}
                CL_LOG("i=%d, current_usr_money=%d,price=%d, duration=%d,charger_time_plan=%d min,startGoldTime=%d min,gunId=%d.\n",i,pGunInfo->current_usr_money,pSegMent->price,pSegMent->duration,pGunInfo->charger_time_plan,pGunInfo->startGoldTime,gunId);
                pGunInfo->getPowerFlag++;
                return CL_OK;
            }

            if (i == (pGunInfo->powerInfo.segmentCnt - 1)) 
            {
                //如果实时功率超出了计费模版的功率段的最大功率，就按最大那段功率计费
                if (pSegMent->endPower < (power/10)) 
                {
                    pGunInfo->powerSemenIndex = i;
                    pGunInfo->charger_time_plan = pGunInfo->current_usr_money*pSegMent->duration/pSegMent->price;
                    CL_LOG("i=%d, current_usr_money=%d,price=%d, duration=%d,charger_time_plan=%d min,gunId=%d.\n",i,pGunInfo->current_usr_money,pSegMent->price,pSegMent->duration,pGunInfo->charger_time_plan,gunId);
                    pGunInfo->getPowerFlag++;
                    return CL_OK;
                }
            }
            pSegMent++;
        }
        CL_LOG("can not find power segment,power=%d w,gunid=%d.\n",power/10,gunId);
        return CL_FAIL;
    }
    else
    {
        pGunInfo->getPowerFlag++;
    }
    
    return CL_OK;
}


void GunChargingProc(uint8_t gunId, uint16_t power, gun_info_t *pGunInfo)
{
    uint16_t powerDiff;
    uint16_t time;
	static uint8_t inCntFlag[GUN_NUM_MAX];
    uint32_t rtcTime = GetRtcCount();
    GUN_CHARGING_STR *pGunCharging = &gChgInfo.gunCharging[gunId-1];

	#if 1
    uint32_t i = 0;
    uint32_t powerAll;
	//如果充电功率6.6分钟平均小于3.5w，就认为枪头拔出
    if (0 == ((inCntFlag[gunId-1]++) % 2))
	{
        pGunCharging->power[pGunCharging->powerIndex] = power;
		CL_LOG("pGunCharging->powerIndex = %d\n", pGunCharging->powerIndex);
        if (1 == pGunCharging->isFull) 
		{
            powerAll = 0;
            for (i = 0; i < POWER_CHECK_CNT; i++) 
			{
                powerAll += pGunCharging->power[i];
            }
            powerAll = powerAll/POWER_CHECK_CNT;
            if ((powerAll < 20))
			{
				if ((480 > (uint32_t)(rtcTime - pGunInfo->start_time))/* && (240 < (uint32_t)(rtcTime - pGunInfo->start_time)) */)
				{ //充电8分钟之内检测到枪头断开都结束订单
                    CL_LOG("check gun pull out,stop charging,maxPower=%d,gunid=%d.\n",pGunInfo->maxPower,gunId);
                    pGunInfo->stopReason = STOP_PULL_OUT;
                    pGunInfo->reasonDetail = REASON_AVERAGE_POWER_LOW;
                    return;
                }
				else
				{  //大于8分钟之后，实时计费和过了起步金时间之后才检测枪头断开
					CL_LOG("pGunCharging->isRebootMCU = %d,pGunCharging->isTimesFlag = %d.\n", pGunCharging->isRebootMCU, pGunCharging->isTimesFlag);
                    if ((CHARGING_ACTUAL == pGunInfo->chargerMethod) || ((0xae == pGunCharging->isRebootMCU) && (20 < pGunCharging->isTimesFlag++)) ||
						((CHARGING_START == pGunInfo->chargerMethod) && ((uint32_t)(rtcTime - pGunInfo->start_time) > (pGunInfo->startGoldTime*60)))) 
					{
                        CL_LOG("check gun pull out,stop charging,maxPower=%d,gunid=%d.\n",pGunInfo->maxPower,gunId);
                        pGunInfo->stopReason = STOP_PULL_OUT;
                        pGunInfo->reasonDetail = REASON_AVERAGE_POWER_LOW;
                        return;
                    }
                }
				if(pGunInfo->stopReason == STOP_PULL_OUT)
				{
					CL_LOG("check gun pull out,stop charging,gunid=%d.\n",(gunId + system_info.baseGunSn - 1));
					system_info.NoticeEvent[gunId-1][5].gun_id = gunId;
					system_info.NoticeEvent[gunId-1][5].code = EVENT_PLUG_PULL_OUT;
					system_info.NoticeEvent[gunId-1][5].para1 = 0;
					system_info.NoticeEvent[gunId-1][5].para2 = 0;
					system_info.NoticeEvent[gunId-1][5].status = EVENT_OCCUR;
					system_info.NoticeEvent[gunId-1][5].level = EVENT_ALARM;
					
					FlashWriteSystemInfoData();
				}
			}
        }
        if (POWER_CHECK_CNT <= ++pGunCharging->powerIndex) 
		{
            pGunCharging->powerIndex = 0;
            pGunCharging->isFull = 1;
        }
    }
	#endif
	
	//CL_LOG("实时功率wwwwwwwwwwwwwwwwwwwwwwww  power = %d\n", power);
    if (power <= PUT_OUT_GUN_POWER) 
	{
        if (GUN_CHARGING_GUN_PULL != pGunInfo->is_load_on) 
		{
            pGunInfo->is_load_on = GUN_CHARGING_GUN_PULL;
            pGunCharging->beginTime = rtcTime;
        }
		else
		{
            time = rtcTime - pGunCharging->beginTime;
            if (PUT_OUT_GUN_TIME <= time)
			{
				//CL_LOG("time = %d, pGunInfo->start_time = %d\n", time, (rtcTime - pGunInfo->start_time));
                if ((480 > (uint32_t)(rtcTime - pGunInfo->start_time))/* && (240 < (uint32_t)(rtcTime - pGunInfo->start_time)) */)
				{ //充电8分钟之内检测到枪头断开都结束订单
                    pGunInfo->stopReason = STOP_PULL_OUT;
                    CL_LOG("check gun pull out,stop charging,maxPower=%d,gunid=%d.\n",pGunInfo->maxPower,gunId);
                }
				else
				{  //大于8分钟之后，实时计费和过了起步金时间之后才检测枪头断开
					//CL_LOG("pGunCharging->isRebootMCU = %d,pGunCharging->isTimesFlag = %d.\n", pGunCharging->isRebootMCU, pGunCharging->isTimesFlag);
                    if ((CHARGING_ACTUAL == pGunInfo->chargerMethod) || ((0xae == pGunCharging->isRebootMCU)/* && (100 < pGunCharging->isTimesFlag++)*/) ||
					((CHARGING_START == pGunInfo->chargerMethod) && ((uint32_t)(rtcTime - pGunInfo->start_time) > (pGunInfo->startGoldTime*60)))) 
					{
                        pGunInfo->stopReason = STOP_PULL_OUT;
                        CL_LOG("check gun pull out,stop charging,maxPower=%d,gunid=%d.\n",pGunInfo->maxPower,gunId);
                    }
                }
				if(pGunInfo->stopReason == STOP_PULL_OUT)
				{
					CL_LOG("check gun pull out,stop charging,gunid=%d.\n",(gunId + system_info.baseGunSn - 1));
					system_info.NoticeEvent[gunId-1][5].gun_id = gunId;
					system_info.NoticeEvent[gunId-1][5].code = EVENT_PLUG_PULL_OUT;
					system_info.NoticeEvent[gunId-1][5].para1 = 0;
					system_info.NoticeEvent[gunId-1][5].para2 = 0;
					system_info.NoticeEvent[gunId-1][5].status = EVENT_OCCUR;
					system_info.NoticeEvent[gunId-1][5].level = EVENT_ALARM;
					
					FlashWriteSystemInfoData();
				}
            }
			else
			{
				//CL_LOG("计费方式pGunInfo->chargerMethod = %d\n", pGunInfo->chargerMethod);
                if ((CHARGING_ACTUAL == pGunInfo->chargerMethod) || 
					((CHARGING_START == pGunInfo->chargerMethod) && ((uint32_t)(rtcTime - pGunInfo->start_time) > (pGunInfo->startGoldTime*60)))) 
				{
					//CL_LOG("time = %d\n", time);
                    if ((58 <= time && time <= 60) || ((58+120) <= time && time <= (60+120))) 
					{
						//CL_LOG("关闭继电器\n");
                        GunTurnOff(gunId);
                        pGunCharging->stopTime = rtcTime;
                        pGunCharging->stopFlag = 1;
                    }
                }
            }
        }
    }
	else 
	{//充满或充电中处理
        if (power < 200) 
		{  //这个值最好根据电池充电功率来定
            if (GUN_CHARGING_FULL != pGunInfo->is_load_on) 
			{
                pGunInfo->is_load_on = GUN_CHARGING_FULL;
                pGunCharging->currentPower = power;
                pGunCharging->beginTime = rtcTime;
                pGunInfo->powerCheckcnt = 90;  //如果是充电开始的时候还没有进入充电中状态就先进来,需要把功率检测时间加长，避免出现未进入充电中状态就开始确定充电功率
                CL_LOG("power=%d,开始充满判断,gunid=%d,chargingFullTime=%d sec,chargerMethod=%d.\n", power, gunId, system_info.chargingFullTime, pGunInfo->chargerMethod);
            }
			else
			{
                //固定计费下只有月卡才做充电自停功能，实时计费和起步金模式下支持充满自停功能。
                if (((0 == pGunInfo->chargingMode) && (CHARGING_FIXED == pGunInfo->startMode)) || 
					(CHARGING_ACTUAL == pGunInfo->chargerMethod) || ((CHARGING_START == pGunInfo->chargerMethod) && ((uint32_t)(rtcTime - pGunInfo->start_time) > (pGunInfo->startGoldTime*60)))) 
				{
					powerDiff = (power > pGunCharging->currentPower) ? power - pGunCharging->currentPower : pGunCharging->currentPower - power;
					if (30 < powerDiff) 
					{
						CL_LOG("powerDiff>3w,power=%d,currentPower=%d,gunid=%d.\n",power,pGunCharging->currentPower,gunId);
						pGunCharging->currentPower = power;
						pGunCharging->beginTime = rtcTime;
					}
					else
					{
						if (system_info.chargingFullTime <= (uint16_t)(rtcTime - pGunCharging->beginTime)) 
						{
							pGunInfo->stopReason = STOP_CHARGING_FULL;
							CL_LOG("充满自停, 充满自停, 充满自停check charging full,stop charging,gunid=%d.\n",gunId);
						}
					}
				}
            }
        }
		else 
		{ //充电中
            pGunInfo->is_load_on = GUN_CHARGING_WORK;
            if (GUN_STATE_WAIT_PLUG_IN == pGunInfo->gun_state) 
			{
                pGunInfo->gun_state = GUN_STATE_ON;
                pGunInfo->powerCheckcnt = 45;
                pGunInfo->getPowerFlag = 0;
            }
        }

		//确定充电功率
        if (0 == pGunInfo->getPowerFlag) 
		{
            if (pGunInfo->powerCheckcnt) 
			{
                pGunInfo->powerCheckcnt--;
                if (pGunInfo->chargingPower < power) 
				{
                    if (4 <= ++pGunCharging->checkPowerCnt) 
					{
                        pGunCharging->checkPowerCnt = 0;
                        pGunInfo->chargingPower = power;
                        if (0 == pGunInfo->powerCheckcnt) 
						{ //如果到了限定时间，功率还在增加，就再延长功率确认时间
                            pGunInfo->powerCheckcnt = 4;
                        }
                    }
                }
				else if (power < pGunInfo->chargingPower) 
				{
                    pGunCharging->checkPowerCnt = 0;
                }

                if ((15*60) < (uint32_t)(rtcTime - pGunInfo->start_time)) 
				{ //如果超过15分钟功率还没有确定，就强制确定一次功率
                    pGunInfo->chargingPower = power;
                    GetChargingTime(gunId, power);
					
					FlashWriteGunInfoData();
                }
            }
			else
			{ //如果在限定时间内，功率不再增加，就开始按该功率计费
                GetChargingTime(gunId, pGunInfo->chargingPower);
				
				FlashWriteGunInfoData();
            }
        }
		else if (pGunInfo->getPowerFlag < 15) 
		{
            if (power > pGunInfo->chargingPower) 
			{
                if (150 <= (power - pGunInfo->chargingPower)) 
				{ //如果功率持续升高大于8分钟，则进行功率调节
                    if (254 <= ++pGunInfo->powerCheckcnt) 
					{
                        pGunInfo->chargingPower = power;
                        GetChargingTime(gunId, pGunInfo->chargingPower);
						
						FlashWriteGunInfoData();
                        pGunInfo->powerCheckcnt = 0;
                    }
                }
            }
			else if (power < pGunInfo->chargingPower) 
			{
                pGunInfo->powerCheckcnt = 0;
            }
        }
    }
}

void ChargingProc(void)
{
    uint8_t cnt = 0;
    segment_str *pPowerSement = NULL;
	gun_info_t *pGunInfo = NULL;
	GUN_CHARGING_STR *pGunCharging = NULL;
	
	for (int i = 1; i <= GUN_NUM_MAX; i++)
	{
        pGunInfo = &gun_info[i-1];
		if (pGunInfo->is_load_on)
		{
			pGunCharging = &gChgInfo.gunCharging[i-1];
            if (pGunCharging->stopFlag) 
			{
                if (60 < (uint32_t)(GetRtcCount() - pGunCharging->stopTime)) 
				{
					CL_LOG("打开继电器");
                    GunTurnOn(i);
                    pGunCharging->stopFlag = 0;
                }
            }
			
            cnt = 1;
			//CL_LOG("计费模式pGunInfo->mode = %d, pGunInfo->getPowerFlag = %d", pGunInfo->mode, pGunInfo->getPowerFlag);
            if ((COST_UNIFY == pGunInfo->mode) || ((COST_POWER == pGunInfo->mode) && pGunInfo->getPowerFlag))
            {
                pPowerSement = &pGunInfo->powerInfo.segmet[pGunInfo->powerSemenIndex];
    			//pGunInfo->money = (GetRtcCount() - pGunInfo->start_time) * pPowerSement->price / pPowerSement->duration / 60;
				pGunInfo->money = pGunInfo->realChargingTime * pPowerSement->price / pPowerSement->duration;
				CL_LOG("实际充电时间pGunInfo->realChargingTime = %d分钟, 已充金额pGunInfo->money = %d分, 总共金额pGunInfo->current_usr_money = %d分\n", pGunInfo->realChargingTime, pGunInfo->money, pGunInfo->current_usr_money);
				//if (pGunInfo->chargingMode != 0)
				{//非智能充满充电模式
					//充电费用大于用户充电金额,停止充电
					if (pGunInfo->money >= pGunInfo->current_usr_money)
					{
						CL_LOG("money=%d >= current_usr_money=%d,stop charging,charging time=%d min,gunId=%d.\n",pGunInfo->money,pGunInfo->current_usr_money,(GetRtcCount()-pGunInfo->start_time)/60,i);
						pGunInfo->money = pGunInfo->current_usr_money;
						pGunInfo->stopReason = STOP_NORMAL;
                        pGunInfo->reasonDetail = REASON_MONEY_ENOUGH;
						Proto2_4gStopCharging(i);
                        //Save 发送结束充电通知 to flash
                        SaveStopChargingNotice_BTRF2_4G(i);
                        CL_LOG("Save 发送结束充电通知 to flash3.\n");
						continue;
					}
					#if 0
                    else
					{
						//if (GetRtcCount() > (pGunInfo->start_time + pGunInfo->charger_time_plan*60)) 
						if (pGunInfo->realChargingTime > pGunInfo->charger_time_plan)
                        {
							pGunInfo->money = pGunInfo->current_usr_money;
							pGunInfo->stopReason = STOP_NORMAL;
                            pGunInfo->reasonDetail = REASON_TIME_ENOUGH;
							CL_LOG("now=%d > (start_time=%d + cplan_time=%d), stop charging,gunId=%d.\n",GetRtcCount(),pGunInfo->start_time,pGunInfo->charger_time_plan*60,i);
							Proto2_4gStopCharging(i);
                            //Save 发送结束充电通知 to flash
                            SaveStopChargingNotice_BTRF2_4G(i);
                            CL_LOG("Save 发送结束充电通知 to flash4.\n");
						}
					}
					#endif
				}
                
				//if (GetRtcCount() > (pGunInfo->start_time + CHARGING_MAX_TIME))
				if (pGunInfo->realChargingTime > CHARGING_MAX_TIME/60) 
				{//智能充满超过最大充电时长12小时
				    pGunInfo->money = CHARGING_MAX_TIME * pPowerSement->price / pPowerSement->duration / 60;
					pGunInfo->stopReason = STOP_OTHER;
                    pGunInfo->reasonDetail = REASON_OVER_23H;
					CL_LOG("charger time over 24h, stop charging,gunId=%d,start_time=%d.\n",i,pGunInfo->start_time);
					Proto2_4gStopCharging(i);
                    SaveStopChargingNotice_BTRF2_4G(i);
				}
            }
		}
	}

    if (cnt) 
	{
        if (20*60 < (GetRtcCount() - chargingOldTime))
		{
            chargingOldTime = GetRtcCount();
			
			FlashWriteGunInfoData();
        }
    }
}


//充电处理
void ChargingCtrlProc(void)
{
    GUN_STATUS_STR gunStatus;
    gun_info_t *pGunInfo = NULL;
	GUN_CHARGING_STR *pGunCharging = NULL;
    int i;
    uint16_t  chargingPower = 0;

	//电流大于10A
	if(100 <= ((uint32_t)gun[0].current/100 + (uint32_t)gun[1].current/100)) 	//电流最大为10A
	{
		//if (5 <= ++pEmu->overcurcount)
		{
			setbit(gun[0].status, 4);
			setbit(gun[1].status, 4);
		}
	}
	else
	{
		//pEmu->overcurcount = 0;
		clrbit(gun[0].status, 4);
		clrbit(gun[1].status, 4);
	}
	
    for (i = 1; i <= GUN_NUM_MAX; i++)
    {
        pGunInfo = &gun_info[i-1];
		pGunCharging = &gChgInfo.gunCharging[i-1];
        GetGunStatus(i, &gunStatus);
		
        //CL_LOG("gun %d,currentPower=%d,current=%d.\n",i,gunStatus.power,gunStatus.current);
        if (pGunInfo->is_load_on)
        {
            if ((gunStatus.status))
			{
                if (gunStatus.status & 0x01)
				{
                    pGunInfo->stopReason = STOP_CHARGER_FAULT;
                    pGunInfo->reasonDetail = REASON_COMM_ERR;
                }
				else
				{
					//过压
					if(gunStatus.status & 0x04)
					{
						pGunInfo->stopReason = STOP_OVER_VOLTAGE;
						pGunInfo->reasonDetail = pGunInfo->stopReason;
						Sc8042bSpeech(VOIC_POWER_TOO_LARGE);
					}
					//过流
					if((gunStatus.status & 0x02))
					{
						pGunInfo->stopReason = STOP_OVER_LOAD;
						pGunInfo->reasonDetail = pGunInfo->stopReason;
						Sc8042bSpeech(VOIC_POWER_TOO_LARGE);
					}
					//电流最大为10A
					if((gunStatus.status & 0x10))
					{
						if(100 <= ((uint32_t)gun[0].current/100 + (uint32_t)gun[1].current/100))
						{
							Sc8042bSpeech(VOIC_POWER_TOO_LARGE);
							//osDelay(600);
							Delay_mSec(600);
							CL_LOG("gun[0].current = %d,gun[1].current=%d.\n", gun[0].current, gun[1].current);
							if(gun_info[0].start_time < gun_info[1].start_time) //第二个插口后开起充电
							{
								gun_info[1].stopReason = STOP_OVER_LOAD;
								gun_info[1].reasonDetail = pGunInfo->stopReason;
							}
							else
							{
								gun_info[0].stopReason = STOP_OVER_LOAD;
								gun_info[0].reasonDetail = pGunInfo->stopReason;
							}
						}
					}
					//欠压
					if(gunStatus.status & 0x08)
					{
						printf("设备欠压!\n");
						pGunInfo->stopReason = STOP_LOW_VOLTAGE;
						pGunInfo->reasonDetail = pGunInfo->stopReason;
					}
                }
                CL_LOG("check emu or current or voltage hight,stop charging,gunid=%d,status=%d.\n",i,gunStatus.status);
                //SendEventNotice(i, EVENT_CHIP_FAULT, CHIP_EMU, gunStatus.status, EVENT_OCCUR,NULL);
                //if(5 < timesFlag++)
				{
					system_info.NoticeEvent[i-1][4].gun_id = i;
					system_info.NoticeEvent[i-1][4].code = EVENT_CHIP_FAULT;
					system_info.NoticeEvent[i-1][4].para1 = CHIP_EMU;
					system_info.NoticeEvent[i-1][4].para2 = gunStatus.status;
					system_info.NoticeEvent[i-1][4].status = EVENT_OCCUR;
					system_info.NoticeEvent[i-1][4].level = EVENT_ALARM;
					
					FlashWriteSystemInfoData();
				}
            }
            else
			{
                if (gunStatus.elec > pGunInfo->startElec)
                {
                	#if 0
                    pGunInfo->chargingElec += gunStatus.elec - pGunInfo->startElec;
                    pGunInfo->startElec = gunStatus.elec;
					#else
					pGunInfo->chargingElec = gunStatus.elec - pGunInfo->startElec;
					#endif
					CL_LOG("startElec = %d, elec = %d, pGunInfo->chargingElec = [%d].\n", pGunInfo->startElec, gunStatus.elec, pGunInfo->chargingElec);
                }
                else if (gunStatus.elec < pGunInfo->startElec)  //判满
                {
                    //pGunInfo->chargingElec += 0xffffffff - pGunInfo->startElec + gunStatus.elec;
                    CL_LOG("startElec=%d,elec=%d,error.\n",pGunInfo->startElec,gunStatus.elec);
                    pGunInfo->startElec = gunStatus.elec;
                }
                GunChargingProc(i, gunStatus.power, pGunInfo);
            }
            
            if (STOP_UNKNOW != pGunInfo->stopReason)
            {
                CL_LOG("发送结束充电通知 to flash6 ,pGunInfo->is_load_on = %d ,pGunInfo->stopReason = %d.\n", pGunInfo->is_load_on , pGunInfo->stopReason);
                Proto2_4gStopCharging(i);
                //Save 发送结束充电通知 to flash
                //SaveStopChargingNotice_BTRF2_4G(i);
            }
			else
			{
                if ((CHARGING_POWER_LIMIT <= (chargingPower + pGunCharging->currentPower/10)))	//两个插口最大总电流 10A
				{
                    pGunInfo->stopReason = STOP_POWER_TOO_BIG;
                    pGunInfo->reasonDetail = REASON_TOTAL_POWER_LARGE;
					//Sc8042bSpeech(VOIC_POWER_TOO_LARGE);
                    Proto2_4gStopCharging(i);
                    //Save 发送结束充电通知 to flash
                    SaveStopChargingNotice_BTRF2_4G(i);
                    CL_LOG("Save 发送结束充电通知 to flash7.\n");
                }
				else
				{
                    chargingPower = pGunCharging->currentPower/10;
                }
            }

            if (60 < (uint32_t)(GetRtcCount() - pGunCharging->chargingTime))
			{
                pGunCharging->chargingTime = GetRtcCount();
				pGunInfo->realChargingTime++;
        	    CL_LOG("gun %d, chargingPower=%d mW, currentPower=%d mW, current=%d mA, money=%d 分, elec=[%d] (0.01kwh), chg mini=%d 分钟, realChargingTime=%d.\n",
                    i, pGunInfo->chargingPower*100, gunStatus.power*100, gunStatus.current, pGunInfo->money,
                    pGunInfo->chargingElec, (GetRtcCount()-pGunInfo->start_time)/60, pGunInfo->realChargingTime);
            }
            OS_DELAY_MS(10);
        }
    }
    gChgInfo.chargingTotalPower = chargingPower;
}

//检测充电状态
int CheckGunStatu(uint8_t gunId)
{
	GUN_STATUS_STR gunStatus;

	if (CL_OK != GetGunStatus(gunId, &gunStatus))
	{
		CL_LOG("call GetGunStatus fail.\n");
		return CL_FAIL;
	}

	if (gunStatus.status) 
	{
        CL_LOG("status=%d,error.\n",gunStatus.status);
		return CL_FAIL;
	} 
	else 
	{
		return CL_OK;
	}
}

//返回正在充电的枪头数目 0:就是没有在充电的枪头
int GetChargingGunCnt(void)
{
    int i;
    int cnt = 0;

    for (i=0; i<GUN_NUM_MAX; i++) {
        if (gun_info[i].is_load_on) {
            cnt++;
        }
    }
    return cnt;
}


//检测枪头状态
int CheckStartPara(uint8_t gun_id)
{
    gun_info_t *pGunInfo = &gun_info[gun_id-1];

    if ((gun_id > GUN_NUM_MAX) || (gun_id < 1))
	{
        CL_LOG("gun_id=%d,error.\n",gun_id);
        OptFailNotice(102);
		return START_FAIL_PARA_ERR;
	}

    if ((GUN_STATE_IDLE != pGunInfo->gun_state) && (GUN_STATE_OFF != pGunInfo->gun_state))
	{
        CL_LOG("插座被占用gun %d busy,gun_state=%d.\n", gun_id,pGunInfo->gun_state);
        //OptFailNotice(103);
        Sc8042bSpeech(VOIC_SOCKET_OCCUPID);
        return START_FAIL_CHARGING;
    }

    if ((COST_POWER != GetCostMode()) && (COST_UNIFY != GetCostMode()))
	{
        CL_LOG("GetCostMode()=%d,error.\n", GetCostMode());
        OptFailNotice(104);
        return START_FAIL_NO_TEMPLATE;
    }

    if (CL_OK != CheckGunStatu(gun_id))
	{
        CL_LOG("CheckGunStatu fail,error.\n");
        OptFailNotice(105);
        return START_FAIL_FAULT;
    }
	
    return CL_OK;
}

//恢复充电状态
void RestoreGunStatus(void)
{
    gun_info_t *pGunInfo;
    
    for(int i = 1; i <= GUN_NUM_MAX; i++)
    {
        pGunInfo = &gun_info[i-1];
        CL_LOG("gunid=%d,is_load_on=%d,gun_state=%d.\n", i, pGunInfo->is_load_on, pGunInfo->gun_state);
        if (pGunInfo->is_load_on)
        {
        	pGunInfo->is_load_on = GUN_CHARGING_UNKNOW_POWER;   //先强制修改为未检测到功率，避免出现马上停止充电
            CL_LOG("restore gun-%d ON.\n", i);
            GunTurnOn(i);
            //pGunInfo->isSync = POWER_RECOVER;
        }
    }
}

/*****************************************************************************
** Function name:       BTRF2_4gStartCharging
** Descriptions:        //开启充电
** input parameters:    gunId: 对于本设备有两个插孔，这里取值1或2
                        pOrder: 订单号
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
uint8_t BTRF2_4gStartCharging(uint8_t gunId, uint32_t money, uint8_t *pOrder,uint8_t SerialNum)
{
    gun_info_t *pGunInfo = &gun_info[gunId-1];
    GUN_STATUS_STR gunStatus;
    static uint8_t SerialNumId[GUN_NUM_MAX] = {255,255};
    GUN_CHARGING_STR *pGunCharging = &gChgInfo.gunCharging[gunId-1];
	
    if((SerialNumId[gunId-1] != SerialNum) || (((uint32_t)60)*SERIAL_NUM_DELAY_TIME < (GetRtcCount() - gChgInfo.gStartChargingNoticeTick[gunId - 1])))
    {
    	gChgInfo.gStartChargingNoticeTick[gunId - 1] = GetRtcCount();
        SerialNumId[gunId-1] = SerialNum;
        memset(pGunInfo, 0, sizeof(gun_info_t));
		//pGunCharging->isPullOutGun = 0;
        CL_LOG("开始充电 ,gun_id = %d\n",gunId);
        //CL_LOG("保存订单号.保存订单号.保存订单号.\n");
        memcpy(pGunInfo->order, pOrder, sizeof(pGunInfo->order));
        pGunInfo->gun_state = GUN_STATE_WAIT_PLUG_IN;
        pGunInfo->is_load_on = GUN_CHARGING_UNKNOW_POWER;
    }
	else
	{
        CL_LOG("开始充电序列号相同.\n");
        return 1; //如果已经有这个订单号了, 表示该枪头已经在充电
    }
    pGunInfo->gun_id = gunId;
    pGunInfo->mode = gCostTemplateInfo.cost_template.mode;
	pGunInfo->chargerMethod = gCostTemplateInfo.chargerMethod;
	pGunInfo->chargerStartingGold = gCostTemplateInfo.chargerStartingGold;
    if (COST_UNIFY == gCostTemplateInfo.cost_template.mode)
    {
        pGunInfo->charger_time_plan = money * gCostTemplateInfo.cost_template.Data.unifyInfo.duration / gCostTemplateInfo.cost_template.Data.unifyInfo.price;
        pGunInfo->powerInfo.segmet[0].price = gCostTemplateInfo.cost_template.Data.unifyInfo.price;
        pGunInfo->powerInfo.segmet[0].duration = gCostTemplateInfo.cost_template.Data.unifyInfo.duration;
		if (pGunInfo->chargerMethod == CHARGING_START)
		{
			//起步价充电时间
			pGunInfo->startGoldTime = pGunInfo->chargerStartingGold * gCostTemplateInfo.cost_template.Data.unifyInfo.duration/ gCostTemplateInfo.cost_template.Data.unifyInfo.price;
		}
        CL_LOG("current_usr_money=%d,charger_time_plan=%d min,startGoldTime=%d min,gunId=%d.\n", money, pGunInfo->charger_time_plan, pGunInfo->startGoldTime, gunId);
    }
    else
	{
        pGunInfo->charger_time_plan = NO_LOAD_TIME;
        memcpy(&pGunInfo->powerInfo, &gCostTemplateInfo.cost_template.Data.powerInfo, sizeof(pGunInfo->powerInfo));
    }
    //pGunInfo->costTempId = gGunInfo1.cost_template.template_id;
    pGunInfo->start_time = GetRtcCount();
    CL_LOG("开始充电时间pGunInfo->start_time = %d\n",pGunInfo->start_time);
    GetGunStatus(gunId, &gunStatus);
	//pGunInfo->isSync = FIRST_START_CHARGING;
    pGunInfo->realChargingTime = 0;
    pGunInfo->startElec = gunStatus.elec;
	pGunInfo->startChargerElec = gunStatus.elec;
    pGunInfo->current_usr_money = money;
    pGunInfo->stopReason = STOP_UNKNOW;
    pGunInfo->reasonDetail = REASON_UNKNOW;
    pGunInfo->chargingMode = gChgInfo.mode;
	//pGunInfo->chargerPararm = gChgInfo.pararm;
	//pGunInfo->subsidyType = gChgInfo.subsidyType;
	//pGunInfo->subsidyPararm = gChgInfo.subsidyPararm;
    GunTurnOn(gunId);
    chargingOldTime = pGunInfo->start_time;
	pGunCharging->chargingTime = chargingOldTime;
    //pGunCharging->isTesting = 0;
    pGunCharging->checkPowerCnt = 0;
    //pGunCharging->inCnt = 0;
    pGunCharging->isFull = 0;
    pGunCharging->powerIndex = 0;
    //pGunCharging->resetEmuChipFlag = 0;
    //pGunCharging->startChargerTimers = 0;
    pGunCharging->stopFlag = 0;
	gChgInfo.gunCharging[gunId-1].isRebootMCU = 0;
	gChgInfo.gunCharging[gunId-1].isTimesFlag = 0;
	
	FlashWriteGunInfoData();
    //SpeechChargeing(gunId + system_info.baseGunSn - 1);
   
    return 0;
}



/*****************************************************************************
** Function name:       Proto2_4gStopCharging
** Descriptions:        //停止充电
** input parameters:    gunId: 对于本设备有两个插孔，这里取值1或2
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void Proto2_4gStopCharging(uint8_t gunId)
{
    gun_info_t *pGunInfo = &gun_info[gunId-1];
	//GUN_CHARGING_STR *pGunCharging = &gChgInfo.gunCharging[gunId-1];
	
    GunTurnOff(gunId);
    //语音充电结束
    //pGunInfo->stop_time = GetRtcCount();
	//pGunCharging->isTesting = 0;
	pGunInfo->is_load_on = GUN_CHARGING_IDLE;
	pGunInfo->gun_state = GUN_STATE_IDLE;
	
	FlashWriteGunInfoData();
    MuxSempTake(&gChgInfo.gProtoMuxType.gOrderReportMux);
    
    MuxSempTake(&gChgInfo.gProtoMuxType.gBuff1024Byte);      
    SaveOrderReport(gunId);    //保存订单
    MuxSempGive(&gChgInfo.gProtoMuxType.gBuff1024Byte);
                    
    MuxSempGive(&gChgInfo.gProtoMuxType.gOrderReportMux);
}

void ShowGunInfo(gun_info_t *pGunInfo)
{
    int i;

    for (i=0; i<GUN_NUM_MAX; i++)
    {
        CL_LOG("ShowGunInfo ,gun_state=%d,is_load_on=%d,i=%d.\n",pGunInfo->gun_state,pGunInfo->is_load_on,i);
        if ((0xff == pGunInfo->gun_state) || (pGunInfo->is_load_on==0xff)) {
            pGunInfo->gun_state = 0;
            pGunInfo->is_load_on = GUN_CHARGING_IDLE;
        }
        if ((0 == pGunInfo->is_load_on) && pGunInfo->gun_state) {
            CL_LOG("gun status error,gun_state=%d,is_load_on=%d,gunid=%d.\n",pGunInfo->gun_state,pGunInfo->is_load_on,i);
            pGunInfo->gun_state = 0;
        }
        pGunInfo++;
    }
}


void ReadFlashInfoData(PARTITION n, uint8_t *data_info, uint8_t *dataInfo, uint32_t len)
{
	uint32_t timesFlag = 0;
	uint32_t sum[3] = {0, 1, 2};
	uint32_t i = 0;

	while(1)
	{
		for(i = 0; i < 3; i++)
		{
			Feed_WDT();
			memset(dataInfo, 0, len);
			Flash_SetPos_r(n, 0);
			Flash_Read(n, (void*)dataInfo, len);
			sum[i] = CheckSum((uint8_t*)dataInfo, len);
			//PrintfData("网关地址", systemInfo._24gMacAddr, sizeof(systemInfo._24gMacAddr));
		}
		
		if((sum[0] == sum[1]) && (sum[0] == sum[2]))
		{
			memcpy(data_info, dataInfo, len);
			//printf("读数据成功[%d]! .\n", n);
			return;
		}
		else
		{
			printf("读数据失败[%d]! .\n", n);
		}
		if(20 < timesFlag++)
		{
			memcpy(data_info, dataInfo, len);
			printf("[%d]读数据失败! .\n", n);
			break;
		}
		osDelay(50);
	}
	return;
}

/*****************************************************************************
** Function name:       ReadUpgradeInit
** Descriptions:        //升级初始化
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void ReadUpgradeInit(void)
{
	UPGRADE_RECORD_STR upgradeRecord;
	ReadFlashInfoData(UPGRADE_PART, (uint8_t*)&UpgradeRecord, (uint8_t*)&upgradeRecord , sizeof(upgradeRecord));
}

//充电接口初始化
void Charger_Init(void)
{
	
	#if 1
	gun_info_t gunInfo[GUN_NUM_MAX];
	ReadFlashInfoData(GUNINFO_PART, (uint8_t*)gun_info, (uint8_t*)gunInfo , sizeof(gunInfo));
	#else
	gun_info_t gunInfo[GUN_NUM_MAX];
	uint8_t timesFlag = 0;
	uint32_t sum[3] = {0, 1, 2};
	uint8_t i = 0;
	
	while(1)
	{
		for(i = 0; i < 3; i++)
		{
			memset(&gunInfo, 0, sizeof(gunInfo));
			Flash_SetPos_r(GUNINFO_PART, 0);
			Flash_Read(GUNINFO_PART, gunInfo, sizeof(gunInfo));
			sum[i] = CheckSum((uint8_t*)&gunInfo, sizeof(gunInfo));
		}
		
		if((sum[0] == sum[1]) && (sum[0] == sum[2]))
		{
			memcpy(&gun_info, &gunInfo, sizeof(gunInfo));
			CL_LOG("gun_info 读数据成功! .\n");
			break;
		}
		else
		{
			CL_LOG("gun_info 读数据失败! .\n");
		}
		if(20 < timesFlag++)
		{
			memcpy(&gun_info, &gunInfo, sizeof(gunInfo));
			CL_LOG("读gun_info数据失败! .\n");
			break;
		}
		osDelay(50);
        Feed_WDT();
	}
	#endif
	
	ShowGunInfo(gun_info);
}

void FuseholderStatusProc(void)
{
	uint8_t status;
	
	//保险丝断掉
	if(gChgInfo.RF24GCardFlag.fuseState == 1)
	{
		CL_LOG("保险丝熔断.\n");
		status = 1;
	}
	else
	{
		status = 0;
	}
	
	if(gChgInfo.lastFuseholderStatus != status)
	{
		if(gChgInfo.RF24GCardFlag.fuseState == 1)
		{
			CL_LOG("保险丝熔断, 将事件发送给R6.\n");
		}
		//else
		//{
		//	CL_LOG("保险丝正常, 将事件发送给R6.\n");
		//}
		system_info.NoticeEvent[0][1].gun_id = system_info.baseGunSn;
        system_info.NoticeEvent[0][1].code = EVENT_FUSE_BREAK;
        system_info.NoticeEvent[0][1].para1 = CHIP_EMU;
        system_info.NoticeEvent[0][1].para2 = 0;
        system_info.NoticeEvent[0][1].status = status ? EVENT_OCCUR : EVENT_RECOVER;
        system_info.NoticeEvent[0][1].level = EVENT_ALARM;
		
		FlashWriteSystemInfoData();
		//if(0 < gChgInfo.sendFuseholderStatus++)
		{
			//gChgInfo.sendFuseholderStatus = 0;
			gChgInfo.lastFuseholderStatus = status;
		}
	}
}



