#include "comm_proto.h"
#include "system_info.h"
#include "uart.h"
#include "flash_usr.h"
#include "sc8042b.h"
#include "upgrade.h"
#include "system_info.h"
#include "emu_info.h"
#include "gun.h"



int SendDebugPkt(uint8_t nodeIndex, OUT_PKT_STR *pFrame, uint16_t len)
{
    pFrame->head.aa = 0xaa;
    pFrame->head.five = 0x55;
    pFrame->head.dest[0] = nodeIndex;
    pFrame->head.len = 4 + len;
    pFrame->head.ver = 1;
    pFrame->data[len] = CheckSum((void*)&pFrame->head.ver, len + 3);
	Uart_Send(&gUart3Handle,(void*)pFrame, sizeof(OUT_NET_HEAD_STR) + len + 1);
	//PrintfData("升级发送数据",&pFrame->head.aa,sizeof(OUT_NET_HEAD_STR) + len + 1);
	return CL_OK;
}

void SendTestPktAck(uint8_t cmd, uint8_t *pkt, uint8_t len)
{
	uint8_t TestPktStrAck[128] = {0};
    OUT_PKT_STR *pFrame = (void*)TestPktStrAck;

    pFrame->head.cmd = cmd;
    memcpy(pFrame->data, (void*)pkt, len);
	SendDebugPkt(0, pFrame, len);
}

/*****************************************************************************
** Function name:       COM_UpgradeNotiProc_BTRF2_4G
** Descriptions:        //固件升级请求
** input parameters:    pMsg: 内层协议数据头指针
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void COM_UpgradeNotiProc_BTRF2_4G(OUT_PKT_STR *pMsg)
{
	DEBUG_DOWN_FW_STR *upgradeReq = (void*)pMsg->data;
	printf("upgradeReq->fw_size = %d,upgradeReq->checkSum = %d,upgradeReq->package_num = %d,upgradeReq->fw_version = %d",
        upgradeReq->fw_size,upgradeReq->checkSum,upgradeReq->package_num,upgradeReq->fw_version);
	OpenUpgradeReqestFlag();
	CloseBlueToothUpgradeFlag();
	OpenUARTUpgradeFlag();
	Upgrade_Init(upgradeReq->fw_size,(uint16_t)upgradeReq->checkSum,upgradeReq->package_num,upgradeReq->fw_version);
}

int FlashTest(void)
{
	uint8_t data = 0x5A;
	Flash_EraseALL(FIRMWARE_PART);
	
	for(int i = 0;i<1024;i++)
	{
		if(Flash_Write(FIRMWARE_PART,&data,1) != CL_OK)
		{
			return CL_FAIL;
		}
	}
	
	return CL_OK;
}

void DebugPktProc(OUT_PKT_STR *pBuff, uint16_t len)
{
    uint16_t dataLen = 1;
    uint32_t checkSum = 0;

	CL_LOG("comcom cmd=%x\n",pBuff->head.cmd);
    switch (pBuff->head.cmd)
    {
    	case DEBUG_CMD_TEST:	//请求测试
    		if(pBuff->data[0] == 0)
			{
				CL_LOG("打开上位机检测!\n");
				pBuff->data[1] = 0;
				gChgInfo.UpperMonitorTestFlag = 0xa5;
				gChgInfo.UpperMonitorTicks = GetRtcCount();
			}
			else
			{
				CL_LOG("关闭上位机检测!\n");
				pBuff->data[1] = 1;
				gChgInfo.UpperMonitorTestFlag = 0;
			}
			
			dataLen = 2;
			break;
		case DEBUG_CMD_CARD_TEST:
			gChgInfo.TestCardFlag = 0xa5;
			break;
        case DEBUG_CMD_SN:  //设置桩号
            memcpy(gSystemInfo1.station_id, pBuff->data, sizeof(gSystemInfo1.station_id));
            pBuff->data[0] = 0;
			
			FlashWriteDataToSystemInfo1();
			PrintfData("设置设备号", (void*)&gSystemInfo1.station_id[3], 5);
            break;
		case DEBUG_CMD_BLUE:		//蓝牙
			if(0xa5 == gChgInfo.isBlueToothInit)
			{
				pBuff->data[0] = 1;
			}
			else
			{
				pBuff->data[0] = 0;
			}
			dataLen = 1;
			break;
		case DEBUG_CMD_24G_COMMUNICATION:		//24g通信测试
			if((0xa5 == gChgInfo.ReceiveR624gDataFlag))
			{
				gChgInfo.ReceiveR624gDataFlag = 0;
				pBuff->data[0] = 1;
			}
			else
			{
				pBuff->data[0] = 0;
			}
			CL_LOG("24g通信测试 data = [%d] \n",pBuff->data[0]);
			dataLen = 1;
			break;
		case DEBUG_CMD_GETSVER:		//获取版本号
			CL_LOG("DEBUG_CMD_GETSVER data[0]=%d \n",pBuff->data[0]);
			if(pBuff->data[0] == 0)
			{//主版本
				pBuff->data[1] = 0x00;
				pBuff->data[2] = FW_VERSION;
				pBuff->data[3] = FW_VERSION_SUB;
				CL_LOG("版本号 [%d.%d]\n", pBuff->data[2], pBuff->data[3]);
			}
			//else
			//{//副版本
			//	pBuff->data[1] = 0x00;
			//	pBuff->data[2] = 0x00;
			//	pBuff->data[3] = 0x00;
			//}
			
			dataLen = 4;
			break; 
		case DEBUG_CMD_READPCB:		//读PCB编号
			CL_LOG("DEBUG_CMD_READPCB data[0]=%d \n",pBuff->data[0]);
			if(pBuff->data[0] == 0)
			{
				//gun_info_str3 GunInfo3;
				//ReadFlashInfoData(GUNINFO_PART3, (uint8_t*)&gGunInfo3, (uint8_t*)&GunInfo3 , sizeof(gGunInfo3));
				PrintfData("read pcbSN:", gGunInfo3.pcb_sn, 8);
				memcpy(&pBuff->data[1], gGunInfo3.pcb_sn, 8);
				dataLen = 9;
			}
		break;
        case DEBUG_CMD_DEV_TYPE:
            pBuff->data[0] = 0;
            break;
        case DEBUG_CMD_GW_ADDR:
            memcpy(gGunInfo2._24gMacAddr, pBuff->data, sizeof(gGunInfo2._24gMacAddr));
			PrintfData("设置网关2.4g地址", (void*)gGunInfo2._24gMacAddr, sizeof(gGunInfo2._24gMacAddr));
            pBuff->data[0] = 0;
			
			FlashWriteGunInfo_2_Data();
            break;
		case DEBUG_CMD_TER_MINAL:
			gGunInfo2._24gMacAddr[0] = pBuff->data[0];
			gGunInfo2._24gMacAddr[1] = pBuff->data[1];
			gGunInfo2._24gMacAddr[2] = pBuff->data[2];
			gGunInfo2._24gMacAddr[3] = pBuff->data[3];
			gGunInfo2._24gMacAddr[4] = pBuff->data[4] & (~0x3f);
			PrintfData("gw mac addr", gGunInfo2._24gMacAddr, sizeof(gGunInfo2._24gMacAddr));
			gGunInfo2.Local24gMacAddr[0] = pBuff->data[4];
			gGunInfo2.Local24gMacAddr[1] = pBuff->data[3];
			gGunInfo2.Local24gMacAddr[2] = pBuff->data[2];
			gGunInfo2.Local24gMacAddr[3] = pBuff->data[1];
			gGunInfo2.Local24gMacAddr[4] = pBuff->data[0];
			PrintfData("设置本地2.4g地址", (void*)pBuff->data, sizeof(gGunInfo2.Local24gMacAddr));
            //memcpy(system_info.Local24gMacAddr, pBuff->data, sizeof(system_info.Local24gMacAddr));
            
			FlashWriteGunInfo_2_Data();
			pBuff->data[0] = 0;
			
			{
				gChgInfo.RF24GCardFlag.isBTSet24gAddrFlag = 1;
			}
            break;
        case DEBUG_CMD_PRINTF:
            system_info.printSwitch = pBuff->data[0];
            pBuff->data[0] = 0;
			
			FlashWriteSystemInfoData();
            break;
        case DEBUG_CMD_REBOOT:	//重启系统
        	pBuff->data[0] = 0;
            SendDebugPkt(0, pBuff, 1);
			CL_LOG("DEBUG_CMD_REBOOT重启系统！\n");
            OS_DELAY_MS(500);
            NVIC_SystemReset();
            break;
		#if 0
        case DEBUG_CMD_DEVICE_ID:
            system_info.idCode[0] = 0;
            system_info.idCode[1] = 0;
            memcpy(&system_info.idCode[2], pBuff->data, 6);
            pBuff->data[0] = 0;
			
			FlashWriteSystemInfoData();
            break;
		#endif
        case DEBUG_CMD_UPGRADE_CMD:	//请求固件升级
            printf("升级固件版本号 , fw_version = %d\n", ((DEBUG_DOWN_FW_STR *)(pBuff->data))->fw_version);
			printf("之前版本号 fw_version = %d\n", (FW_VERSION));
            if(((DEBUG_DOWN_FW_STR *)(pBuff->data))->fw_version != (FW_VERSION) /*system_info.fw_version*/)
            {
                COM_UpgradeNotiProc_BTRF2_4G(pBuff);
                pBuff->data[0] = 0;
            }
		    else
			{
				pBuff->data[0] = 1;
			}
            //printf("串口debug请求固件升级 , pBuff->data[0] = %d\n", pBuff->data[0]);
        break;
        case DEBUG_CMD_UPGRADE_DOWN_LOAD:		//固件下发
		{
    		uint8_t ret = COM_Upgrade_Write(pBuff->data,pBuff->head.len - 4);
            for(uint32_t i = 0;i < pBuff->head.len - 4;i++)
            {
                checkSum += pBuff->data[i];
            }
    		if(ret == 2)
            {
    			PlayVoice(VOIC_DEVICE_REBOOT);
				CL_LOG("DEBUG_CMD_REBOOT重启系统！\n");
    			pBuff->data[0] = 0;
    			SendDebugPkt(0, pBuff, 1);
                CloseUARTUpgradeFlag();
    			OS_DELAY_MS(500);
    			NVIC_SystemReset();
    		}
    		if(ret == 0)
            {
    			pBuff->data[0] = 1;
    		}
            else
			{
    			pBuff->data[0] = 0;
    		}
            //printf("//固件下发 ,checkSum = %d, pBuff->data[0] = %d\n", checkSum, pBuff->data[0]);
	    }
        break;
		case DEBUG_CMD_TRUMPTE://喇叭测试
			CL_LOG("DEBUG_CMD_TRUMPTE \n");
			PlayVoice(VOIC_WELCOME);
			pBuff->data[0] = 0;
			break;
		case DEBUG_CMD_RELAY:	//继电器
		{
			CL_LOG("DEBUG_CMD_RELAY :data[0]=%d\n",pBuff->data[0]);
			if(pBuff->data[0] == 1)
			{
				GunTurnOff(1);
				GunTurnOff(2);
			}
			else if(pBuff->data[0] == 0)
			{
				GunTurnOn(1);
				GunTurnOn(2);
			}
			pBuff->data[1] = 0;
			int temp = 2;
			OS_DELAY_MS(2000);
			for(int i = 0;i<GUN_NUM_MAX;i++)
			{
				pBuff->data[temp++] = (gun[i].power>>8) & 0xFF;
				pBuff->data[temp++] = gun[i].power & 0xFF;
			}
			//testRelayFlag = 1;
			//testRelayTime = GetRtcCount();
			dataLen = temp;
		}
		break;
		case DEBUG_CMD_WRITEPCB:	//写PCB编号
			CL_LOG("DEBUG_CMD_WRITEPCB data[0]=%d \n",pBuff->data[0]);
			PrintfData("write pcbSN:",&pBuff->data[1],8);
			if(pBuff->data[0] == 0)
			{
				dataLen = 2;
				memcpy(gGunInfo3.pcb_sn, &pBuff->data[1], sizeof(gGunInfo3.pcb_sn));
				FlashWriteGunInfo_3_Data();
				{
					pBuff->data[1] = 0;
				}
			}
		break;
		case DEBUG_CMD_SETRTC:		//设置RTC
		{
			uint32_t time = (uint32_t)(pBuff->data[0] << 24 | pBuff->data[1] << 16 | pBuff->data[2] << 8 | pBuff->data[3]);
			CL_LOG("DEBUG_CMD_SETRTC time=%d \n",time);
			if(time != 0)
			{
				SetRtcCount(time);
				pBuff->data[0] = 0;
			}
			else
			{
				pBuff->data[0] = 1;
			}
		}
		break;
		case DEBUG_CMD_READRTC:		//读RTC
		{
			uint32_t time = GetTimeStamp();
			CL_LOG("DEBUG_CMD_READRTC. cuuuent time=%d\n",time);
			pBuff->data[0] = (time>>24) & 0xFF;
			pBuff->data[1] = (time>>16) & 0xFF;
			pBuff->data[2] = (time>>8) & 0xFF;
			pBuff->data[3] = time & 0xFF;
			dataLen = 4;
		}
		break;
		case DEBUG_CMD_FLASHTEST:	//Flash读写
			pBuff->data[0] = (FlashTest() == CL_OK ? 0:1);
			dataLen = 1;
			break;
		case DEBUG_CMD_LED:	//LED灯测试
			LED_ON(LED1|LED3);
			pBuff->data[0] = 0;
			dataLen = 1;
			break;
		case DEBUG_CMD_AGE_TEST:	//老化测试
			GunTurnOn(1);
			GunTurnOn(2);
			CL_LOG("老机测试请求!\n");
			pBuff->data[0] = 0;
			dataLen = 1;
			break;
        default:
            printf("DebugPktProc error\n");
        break;
    }
    SendDebugPkt(0, pBuff, dataLen);
}

