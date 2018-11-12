/*
*********************************************************************************************************
*                                              CKB24_UM模块通信任务
*                                         
* File         : 
* By           : 
* Version      : 
* Description  : 蓝牙/2.4G消息接受及处理
*********************************************************************************************************
*/
#include "ckb24_um_handle.h"
#include "system_info.h"
#include "rtc.h"
#include "blue.h"
#include "CardAuth_handle.h"


CKB_CTRL_STR gCKBCtrl;



uint8_t gBlueRxData[64];


uint8_t gBtRxBuff[64];
uint8_t g24GRxBuff[256];


/*****************************************************************************
** Function name:       PackSendOuterBT24GData
** Descriptions:        打包外层数据包
** input parameters:    nodeType:用来区分走蓝牙还是走蓝牙RF2.4G
                        packt:外层协议的包的起始地址
                        len:内层协议数据包整个包的长度, 包含头，data部分和校验码 , 即外层协议data部分
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void PackSendOuterBT24GData(uint8_t nodeType, BLUE_RF2_4G_PKT_STR *packt, uint16_t len)
{
    BLUE_RF2_4G_PKT_STR *pkt = packt;
    //外层协议
    pkt->head.ab = 0xab;
	pkt->head.cd = 0xcd;
    pkt->head.target = nodeType;
    if(nodeType == NODE_24G)
    {
        memcpy(pkt->head.addr, gGunInfo2._24gMacAddr, MAC_LEN);
    }
    //pkt->head.addr[0] = 0x01;
    //pkt->head.addr[1] = 0x02;
    //pkt->head.addr[2] = 0x03;
    //pkt->head.addr[3] = 0x04;
    //pkt->head.addr[4] = 0xc0;
    pkt->head.addr[5] = 0x00;
//    CL_LOG("节点类型 nodeType = %d \n", nodeType);
//    PrintfData("PackSendOuterBT24GData, 2.4G地址 _24gMacAddr", (void*)system_info._24gMacAddr, sizeof(system_info._24gMacAddr));
    pkt->head.len = len;
    pkt->data[len] = CheckSum((void*)pkt, sizeof(BLUE_RF2_4G_HEAD_STR)+len);
    pkt->data[len+1] = '\r';
    pkt->data[len+2] = '\n';
    
    return;
}

int CheckIdlePin(void)
{
    uint32_t i = 0;
    
    while (GET_BLUE_IRQ())
    {
        vTaskDelay(10);
        if (1000 <= ++i)
        {
            CL_LOG("CKB24 wait idle timeout, 24G IRQ引脚没有拉低,错误\n");

            return CL_FAIL;
        }
    }
    vTaskDelay(50);
    while (GET_BLUE_IRQ())
    {
        vTaskDelay(10);
        if (1000 <= ++i)
        {
            CL_LOG("CKB24 wait idle timeout, 24G IRQ引脚没有拉低,错误\n");

            return CL_FAIL;
        }
    }
	
    return CL_OK;
}

/*****************************************************************************
** Function name:       SendBlueTooth2_4GPkt
** Descriptions:        发送外层数据包
** input parameters:    nodeType:用来区分走蓝牙还是走蓝牙RF2.4G
                        pkt:外层协议的包的起始地址
                        len:内层协议数据包整个包的长度, 包含头，data部分和校验码 , 即外层协议data部分
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int SendBlueTooth2_4GPkt(uint8_t nodeType, BLUE_RF2_4G_PKT_STR *pkt, uint16_t len)
{
    MuxSempTake(&gChgInfo.gProtoMuxType.gBlueSendMux);
    CKB24_CD_HIGH();
    //vTaskDelay(2);
    
    if(CL_OK != CheckIdlePin())
    {
        MuxSempGive(&gChgInfo.gProtoMuxType.gBlueSendMux);
        return CL_FAIL;
    }
    CL_LOG("len = %d \n", len);
	PackSendOuterBT24GData(nodeType, pkt, len);

    Uart_Send(&gUart4Handle,  (void*)pkt, sizeof(BLUE_RF2_4G_HEAD_STR) + len + OUTER_CHECK_SUM + OUTER_R_N);
    MuxSempGive(&gChgInfo.gProtoMuxType.gBlueSendMux);
    PrintfData("SendBlueTooth2_4GPkt", (void*)pkt, sizeof(BLUE_RF2_4G_HEAD_STR)+len+1+2);
    //vTaskDelay(300);
	
    return CL_OK;
}


/*****************************************************************************
** Function name:       UnpackRecv24GData
** Descriptions:        解析内层数据包
** input parameters:    pBuff:数据包首地址
                        packLen:2_4G内层的报文长度, 除了校验域不包含,包含起始域，长度域，版本域，数据域
** output parameters:   None
** Returned value:	  0:接收到正确报文 1:报文接收错误
** Author:              quqian
*****************************************************************************/
uint8_t UnpackRecv24GPack(void)
{
    uint8_t g24GRouteRxData[256];
    uint8_t *pBuff = &g24GRouteRxData[sizeof(BLUE_RF2_4G_HEAD_STR)];
    static uint8_t  sum = 0;
    uint8_t data = 0;
    static uint8_t  step = ENUM_FIND_EE;
    static uint16_t pktLen = 0;
    static uint16_t len = 0;
    
    Feed_WDT();
	
    //CL_LOG("解析内层数据包, 写指针%d, 读指针%d \n", ((FIFO_S_t*)(&gCKBCtrl.rx24GBuff))->writeIndex, ((FIFO_S_t*)(&gCKBCtrl.rx24GBuff))->readIndex);
    while (CL_OK == FIFO_S_Get(&gCKBCtrl.rx24GBuff, &data))
	{
		//Feed_WDT();
		//vTaskDelay(20);
        switch (step) {
            case ENUM_FIND_EE:
                if (0xee == data) {
                    pktLen = 0;
                    pBuff[pktLen++] = data;
                    step = ENUM_FIND_LEN;
                }
            break;
            case ENUM_FIND_LEN:
                pBuff[pktLen++] = data;
                len = data;
                if (CKB_MAX_PKT_LEN <= len) {
                    CL_LOG("len=%d,error.\n",len);
                    step = ENUM_FIND_EE;
                }else{
                    step = ENUM_FIND_VER;
                    sum = 0;
                }
            break;
            case ENUM_FIND_VER:
                pBuff[pktLen++] = data;
                sum += data;
                step = ENUM_FIND_CMD;
            break;
            case ENUM_FIND_CMD:
                pBuff[pktLen++] = data;
                sum += data;
                len = (len >= 3) ? (len - 3) : 0;
                step = len ? ENUM_RX_DATA : ENUM_FIND_CHK;
            break;
            case ENUM_RX_DATA:
                pBuff[pktLen++] = data;
                sum += data;
                if (0 == --len) {
                    step = ENUM_FIND_CHK;
                }
            break;
            case ENUM_FIND_CHK:
                step = ENUM_FIND_EE;
                if (sum == data)
                {
                    PrintfData("24GRecvTask", pBuff, pktLen);
                    _24gRecvProc((void*)pBuff, pktLen);
                }else{
                    CL_LOG("sum=%02x,pkt sum=%02x,error,drop pkt.\n",sum,data);
                }               
            break;
        }
    }
    
    return 0;
}


/*****************************************************************************
** Function name:       CKBRecvProc
** Descriptions:        解析外层数据包得到内层数据包
** input parameters:    None
** output parameters:   None
** Returned value:	  
** Author:              zhoumin
*****************************************************************************/
void CKBRecvProc(void)
{
    uint8_t gCK24RxData[256];
	uint8_t  *pktBuff = (void*)gCK24RxData;
    static uint8_t  step = BLUE_AB;
    uint8_t  data;
    static uint8_t  sum;
    static uint16_t pktLen;
    static uint16_t len;
	static uint8_t  target;
	uint8_t pktNum = 0;
    BLUE_RF2_4G_PKT_STR *pPkt24g = NULL;
	BLUE_TOOTH_RECEIVE_PKT_STR *pPktBlueTooth = NULL;

	while (CL_OK == FIFO_S_Get(&gUart4Handle.rxFIFO, &data))
    {
		//printf("data = 0x%x ", data);
		Feed_WDT();
		vTaskDelay(10);
		switch (step)
		{
			case BLUE_AB:
				if (data == 0xAB)
				{
					step = BLUE_CD;
					pktBuff[0] = 0xAB;
					pktLen = 1;
					sum = 0xAB;
				}
				break;

			case BLUE_CD:
				if (data == 0xcd)
				{
					step = BLUE_TARGET;
					pktBuff[1] = 0xcd;
					pktLen++;
					sum += data;
				}
				else if (data == 0xAB)
				{
					step = BLUE_CD;
					pktBuff[0] = 0xAB;
					pktLen = 1;
					sum = 0xAB;
				}
				else{
					step = BLUE_AB;
				}
				break;

			case BLUE_TARGET:
				pktBuff[pktLen++] = data;
				len = 0;
				if(data == NODE_BLUE)
					step = BLUE_LEN;
				else
					step = BLUE_ADDR;
				sum += data;
				target = data;
				break;

			case BLUE_ADDR:
				pktBuff[pktLen++] = data;
                //CL_LOG("BLUE_ADDR data[%d] = 0x%x.\n",len,data);
				sum += data;
				if (6 == ++len)
				{
					len = 0;
					step = BLUE_LEN;
				}
				break;

			case BLUE_LEN:
				pktBuff[pktLen++] = data;
				sum += data;
				if (2 == ++len)
				{
					len = (pktBuff[pktLen-1]<<8) | pktBuff[pktLen-2];
					//msgLen = len;
					if ((256-16) < len)
					{
						step = BLUE_AB;
						CL_LOG("len=%d,接收数据包的长度错误 \n", len);
					}
					else{
						step = BLUE_RX_DATA;
					}
				}
				break;
			case BLUE_RX_DATA:
				pktBuff[pktLen++] = data;
				sum += data;
				if (0 == --len)
				{
					step = BLUE_CHK;
				}
				break;

			case BLUE_CHK:
				pktBuff[pktLen++] = data;
				if (data == sum)
                {
					step = BLUE_END;
					len = 0;
				}
				else{
					CL_LOG("接收数据校验错误,sum = 0x%x,pkt sum = 0x%x.\n",sum,data);
					PrintfData("24GRecvTask", pktBuff, pktLen);
					step = BLUE_AB;
				}
				break;

			case BLUE_END:
				if (2 == ++len) 
                {
					if (NODE_24G == target)
                    {
                    	pPkt24g = (void*)pktBuff;
                        if(256 < pPkt24g->head.len)
                        {
                            CL_LOG("2.4G数据长度接收错误！\n");
                            return;
                        }
                        for (len = 0; len < pPkt24g->head.len; ) 
                        {
                            //CL_LOG("len = %d, 接收包长 %d 写指针%d, 读指针%d \n", len, pPkt->head.len, ((FIFO_S_t*)(&gCKBCtrl.rx24GBuff))->writeIndex, ((FIFO_S_t*)(&gCKBCtrl.rx24GBuff))->readIndex);
                            if (CL_OK == FIFO_S_Put(&gCKBCtrl.rx24GBuff, pPkt24g->data[len]))
                            {
                                len++;
                            }
                            else{
                                CL_LOG("2.4g buff over flow error.\n");
                                vTaskDelay(2);
                            }
                        }
					}
                    else{
						pPktBlueTooth = (void*)pktBuff;
                        if(256 < pPktBlueTooth->head.len)
                        {
                            CL_LOG("蓝牙数据长度接收错误！\n");
                            return;
                        }
                        PrintfData("BlueTask bt recv", pPktBlueTooth->data, pPktBlueTooth->head.len);
                        for (len = 0; len < pPktBlueTooth->head.len; )
                        {
                            if (CL_OK == FIFO_S_Put(&gCKBCtrl.rxBtBuff, pPktBlueTooth->data[len]))
                            {
                                len++;
                            }
                            else{
                                CL_LOG("bt buff over flow error.\n");
                                vTaskDelay(2);
                            }
                        }
					}
					step = BLUE_AB;
                    if(RECEIVE_MAX_PKT_24G <= ++pktNum)
                    {
                        pktNum = 0;
                        return;
                    }
				}
				break;

			default:
				step = BLUE_AB;
				break;
		}
	}
}

void CK24_System_Init(void)
{
	gCKBCtrl.rxSem = osSemaphoreCreate(NULL,10);
	gCKBCtrl.msgIndex = 0;
	gCKBCtrl.recvTimers = 0;
	gCKBCtrl.sendTimers = 0;
    FIFO_S_Init(&gCKBCtrl.rxBtBuff, (void*)gBtRxBuff, sizeof(gBtRxBuff));
	FIFO_S_Init(&gCKBCtrl.rx24GBuff, (void*)g24GRxBuff, sizeof(g24GRxBuff));
}


/*****************************************************************************
** Function name:       CK24_UMTask
** Descriptions:        2_4g TASK
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
void CK24_UMTask(void *argc)
{
	uint32_t blueHeatbatTicks = 0;//osGetTimes();
    CKB24_UM_Init();
	CK24_System_Init();
	
	while(1)
	{
		BuleReconnect();
		gChgInfo.RF24GCardFlag.isBTSet24gAddrFlag = 0;
		while(1)
		{
			osDelay(100);
			if(1 == gChgInfo.RF24GCardFlag.isBTSet24gAddrFlag)
			{
				CL_LOG("复位蓝牙模块标志已打开!\n");
				break;
			}
			#if 1
			if(0xa5 == gChgInfo.isBlueToothInit)
			{
				#if 1
				uint32_t authCardTicks = 0;
				
				if (0 == system_info.card_state &&	(osGetTimes() - authCardTicks) >= 200)
				{
					//读卡处理
					TestReadCardHandle();
					authCardTicks = osGetTimes();
				}
				#endif
				
				CKBRecvProc();
		        UnpackRecv24GPack();
		        RecvBtData();
		        CheckUpgradeWaitTimer();        //升级过程中R6 是否断电或离线了
		        //定时发送心跳
		        if(1 == gChgInfo.blue_connectstate)
				{
					if(osGetTimes() - blueHeatbatTicks >= 1000*6)
					{
						blueHeatbatTicks = osGetTimes();
						gChgInfo.blue_connectstate = 0;
						BlueSendHeartBeat();
						CL_LOG("BlueSendHeartBeat.\n");
					}
				}
			}
			else
			{
				CL_LOG("BuleReconnect error.\n");
			}
			#else
			if(osOK == osSemaphoreWait(gCKBCtrl.rxSem,1000*30)){
				UnpackRecvPackGetInnerProtocol();
				UnpackRecv24GPack();
				RecvBtData();
				CheckUpgradeWaitTimer();        //升级过程中R6 是否断电或离线了
			}else{
				CheckUpgradeWaitTimer();        //升级过程中R6 是否断电或离线了
				if(gChgInfo.isBlueToothInit == 0){
				CL_LOG("ckb24 check failed.\n");
					break;
				}
			}
			#endif
		}
	}
}


