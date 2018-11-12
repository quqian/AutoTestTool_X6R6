#include "ckb24_um.h"
#include "uart.h"
#include "system_info.h"
#include "flash_usr.h"


void CKB24_UMGPIO_Conf(void);
void CKB24_UM_HardReset(void);
int BuleCheckReset(char ok, uint8_t retry);
int BuleModeGetName(char ok, uint8_t retry);
int BlueSendCmd(char *cmd, char *ack, uint16_t waittime);
int BuleModePair(char ok, uint8_t retry);
int BuleModeCons(char ok, uint8_t retry);
int BuleModeGWMac(char ok, uint8_t retry);
int BuleModeVers(char ok, uint8_t retry);
int BuleModeAddr(char ok, uint8_t retry);
int BuleSetName(char ok, uint8_t retry);
int BuleModeEcho(char ok, uint8_t retry);
int BuleModeDevt(char ok, uint8_t retry);
int BuleSetDevt(char ok, uint8_t retry);
int BuleSetGWMac(char ok, uint8_t retry);


uint8_t gBlueInitStep = 0;

int isDevt = 0;
uint8_t gwMac[5];


const BlUE_INITTAB gBlueInitTab[]={
    {NULL,              "OK", 3500, 0, BuleCheckReset},
	{"AT+ECHO=1\r\n",   "OK", 3500, 10, BuleModeEcho},
	{"AT+VERS?\r\n",    "OK", 3500, 10, BuleModeVers},
	{"AT+NAME?\r\n",    "OK", 3500, 10, BuleModeGetName},
	{NULL,              "OK", 3500, 10, BuleSetName},
	{"AT+DEVT?\r\n",    "OK", 3500, 10, BuleModeDevt},
	{NULL,              "OK", 3500, 10, BuleSetDevt},
	//{"AT+ADDR?\r\n",    "OK", 3500, 10, BuleModeGWMac},
	{NULL,              "OK", 3500, 10, BuleSetGWMac},
	{"AT+GWID?\r\n",    "OK", 3500, 10, BuleModeGWMac},
	{"AT+PAIR=1\r\n",   "OK", 3500, 10, BuleModePair},
};

int BlueCheckRes(char *cmd, char *res, uint16_t tmo, uint8_t *pbuff, uint16_t size)
{
    char *ret=NULL;
    uint8_t c;
    int cnt = 0;
    int retv = CL_FAIL;

    memset(pbuff, 0, size);

    for(uint16_t time = 0; time < tmo; time += 50)
    {
        osDelay(50);
        while(FIFO_S_Get(&gUart4Handle.rxFIFO, &c) == CL_OK)
        {
            if ((c))
            {
                if (size > cnt)
                {
                    pbuff[cnt++] = c;
                    //CL_LOG("%d \n", c);
                }
                else
				{
                    CL_LOG("cnt=%d , c = %d ,error.\n",cnt , c);
                }
            }
            if((RECEIVE_PKT_LEN - 2) == cnt)
            {
                CL_LOG("cnt = %d\n", cnt);
                break;
            }
        }

        ret = strstr((const char*)pbuff, (const char*)res);
        if (ret)
        {
			//获取版本号
			if(strstr(cmd,"AT+VERS?"))
            {
				char  temp[21];
				memset(temp,0,21);
				sscanf((char*)pbuff,"+VERS: %s",temp);
				if(strlen(temp) >= 20)
				{
					memcpy(system_info.btVersion,temp,20);
				}
				else
				{
					strcpy((char*)system_info.btVersion,temp);
				}
				
				CL_LOG("blue ver:%s.\n",system_info.btVersion);
			}
			//获取蓝牙名称
			if(strstr(cmd,"AT+NAME?"))
            {
                ret = strstr((const char*)pbuff, (const char*)"+NAME:");
                if(ret)
                {
                    //if((0x0d == pbuff[10]) && (0x0a == pbuff[11]))
                    {
                        //CL_LOG("蓝牙名字1 = %x.\n", pbuff[7]);
                        //CL_LOG("蓝牙名字2 = %x.\n", pbuff[8]);
                        //CL_LOG("蓝牙名字3 = %x.\n", pbuff[9]);
                        //CL_LOG("蓝牙名字4 = %x.\n", pbuff[10]);
                        //CL_LOG("蓝牙名字5 = %x.\n", pbuff[11]);
                        //CL_LOG("蓝牙名字6 = %x.\n", pbuff[12]);
                        //CL_LOG("蓝牙名字7 = %x.\n", pbuff[13]);
                        //CL_LOG("蓝牙名字8 = %x.\n", pbuff[14]);
                        //CL_LOG("蓝牙名字9 = %x.\n", pbuff[15]);
                        //CL_LOG("蓝牙名字10 = %x.\n", pbuff[16]);
                        //CL_LOG("蓝牙名字11 = %x.\n", pbuff[17]);
                        //CL_LOG("蓝牙名字12 = %x.\n", pbuff[18]);
                        //CL_LOG("蓝牙名字 blue name:%s.\n",pbuff);
                        memcpy(gChgInfo.blue_name,&pbuff[sizeof("+NAME:")],10);
                        CL_LOG("蓝牙名字 gChgInfo.blue_name :%s.\n",gChgInfo.blue_name);
						if((isArraryEmpty(&gSystemInfo1.station_id[3], MAC_LEN) == CL_OK))
						{
							StringToBCD(&gSystemInfo1.station_id[3], (const char *)gChgInfo.blue_name);
							
                            FlashWriteDataToSystemInfo1();
							PrintfData("设备号", gSystemInfo1.station_id, sizeof(gSystemInfo1.station_id));
						}
						FlashWriteSystemInfoData();
                    }
                }
			}
			
            #if 1
            //获取本地mac
			if(strstr(cmd,"AT+GWID?"))
            {
				uint8_t num = sizeof("+GWID:");

                ret = strstr((const char*)pbuff, (const char*)"+GWID:");
                if(ret)
                {
                    //if((0x0d == pbuff[10]) && (0x0a == pbuff[11]))
                    {
                        //#if 0
                        //CL_LOG("本地24G地址1 = %x.\n", pbuff[num]);
                        //CL_LOG("本地24G地址2 = %x.\n", pbuff[num + 1]);
                        //CL_LOG("本地24G地址3 = %x.\n", pbuff[num + 2]);
                        //CL_LOG("本地24G地址4 = %x.\n", pbuff[num + 3]);
                        //CL_LOG("本地24G地址5 = %x.\n", pbuff[num + 4]);
                        //CL_LOG("本地24G地址6 = %x.\n", pbuff[num + 5]);
                        //CL_LOG("本地24G地址7 = %x.\n", pbuff[num + 6]);
                        //CL_LOG("本地24G地址8 = %x.\n", pbuff[num + 7]);
                        //CL_LOG("本地24G地址9 = %x.\n", pbuff[num + 8]);
                        //CL_LOG("本地24G地址10 = %x.\n", pbuff[num + 9]);
                        //#endif
                        CL_LOG("24G网关地址GWID :%s.\n",pbuff);
                        //StringToBCD(gwMac, &pbuff[num]);
                        //PrintfData("24G网关地址3", gwMac, 5);
                        //CL_LOG("24G网关地址  2:%s.\n",gwMac);
                        if((isArraryEmpty(gGunInfo2.Local24gMacAddr, MAC_LEN) == CL_OK) && (isArraryEmpty(gGunInfo2._24gMacAddr, MAC_LEN) == CL_OK))
						{
							StringToBCD(gGunInfo2.Local24gMacAddr, (const char *)&pbuff[num]);
							gGunInfo2._24gMacAddr[0] = gGunInfo2.Local24gMacAddr[4];
							gGunInfo2._24gMacAddr[1] = gGunInfo2.Local24gMacAddr[3];
							gGunInfo2._24gMacAddr[2] = gGunInfo2.Local24gMacAddr[2];
							gGunInfo2._24gMacAddr[3] = gGunInfo2.Local24gMacAddr[1];
							gGunInfo2._24gMacAddr[4] = gGunInfo2.Local24gMacAddr[0] & (~0x3f);
							
							FlashWriteGunInfo_2_Data();
							PrintfData("本地2.4g地址", (void*)gGunInfo2.Local24gMacAddr, sizeof(gGunInfo2.Local24gMacAddr));
						}
                    }
                }
			}
            #endif 
            
            #if 0
            //获取网关mac
			if(strstr(cmd,"AT+ADDR?"))
            {
				//memset(temp,0,20);
				//sscanf((char*)pbuff,"+ADDR: %s",temp);
				//StringToBCD(gwMac,temp);
                
				//CL_LOG("gwmac:%s.\n",temp);
                ret = strstr((const char*)pbuff, (const char*)"+ADDR:");
                if(ret)
                {
                    //if((0x0d == pbuff[10]) && (0x0a == pbuff[11]))
                    {
                        CL_LOG("24G网关地址ADDR :%s.\n",pbuff);
                        //StringToBCD(gwMac, &pbuff[num]);
                        //PrintfData("24G网关地址3", gwMac, 5);
                        //CL_LOG("24G网关地址  2:%s.\n",gwMac);
                    }
                }
			}
            #endif 
            
			//查询是否终端设备
			if(strstr(cmd,"AT+DEVT?")){
				sscanf((char*)pbuff,"+DEVT: %d",&isDevt);
			}
            retv = CL_OK;
            break;
        }
    }

    #if (1 == BLUE_DEBUG)
    CL_LOG("<<< [%s].\n",pbuff);
    #endif
    return retv;
}


int BlueSendCmd(char *cmd, char *ack, uint16_t waittime)
{
	uint8_t gBlueRecvData[RECEIVE_PKT_LEN];
	uint8_t res = 1;

    CKB24_CD_LOW();
    osDelay(2);
    //CL_LOG("strlen(cmd) = %d\n", strlen(cmd));
	Uart_Send(&gUart4Handle,(void*)cmd, strlen(cmd));

    #if (1 == BLUE_DEBUG)
	CL_LOG(">>> %s.\n",cmd);
    #endif

    if ((ack == NULL) || (waittime == 0)) 
	{
        CKB24_CD_HIGH();
        return CL_OK;
    }
    if (CL_OK == BlueCheckRes(cmd, ack, waittime, gBlueRecvData, (uint16_t)RECEIVE_PKT_LEN)) 
	{
        res = CL_OK; /*check success, retrun 0*/
    }
    CKB24_CD_HIGH();
	
	return res;
}


int BuleCheckReset(char ok, uint8_t retry)
{
    CKB24_UM_HardReset();
		
    for (int i=0; i<2; i++)
    {
        if (BlueSendCmd("AT\r\n", "OK", gBlueInitTab[gBlueInitStep].wait)==0)
        {
            CL_LOG("blue test ok.\n");
            gBlueInitStep++;
            return CL_OK;
        }
        //CL_LOG("blue reset fail,retry.\n");
        OS_DELAY_MS(2000);
    }
    CL_LOG("blue reset fail.\n");
    return CL_FAIL;
}


int CheckBlueModeCmd(char ok, uint8_t retry, uint16_t delay)
{
    if (ok==0) {
        gBlueInitStep++;
        return CL_OK;
    }else if (retry > 10) {
        gBlueInitStep = BLUE_RESET;
	}
    return CL_FAIL;
}



int BuleSetGWMac(char ok, uint8_t retry)
{
    if(isArraryEmpty(gGunInfo2.Local24gMacAddr, MAC_LEN) == CL_OK)
	{
		CL_LOG("Local24gMacAddr is all zero, no need to set gw mac.\n");
		gBlueInitStep++;
        return CL_OK;
	}
	
	if(memcmp(gwMac, gGunInfo2.Local24gMacAddr, MAC_LEN) == 0)
	{
		CL_LOG("gwmac is ok, no need to set gwmac.\n");
		gBlueInitStep++;
        return CL_OK;
	}
	
	char str_mac[STR_MAC_LENTH];
    BCDToString(str_mac, gGunInfo2.Local24gMacAddr, MAC_LEN);
	if(SetGateWay24GMacAddr(str_mac, USER_CMD_GWID) == CL_OK)
    {
		CL_LOG("set gwmac success.\n");
		gBlueInitStep++;
        return CL_OK;
	}
	CL_LOG("set gwmac fail.\n");
    return CL_FAIL;
}


int BuleSetDevt(char ok, uint8_t retry)
{	
	if(isDevt == 0){
		CL_LOG("blue devt=0,no neet to set devt=1.\n");
		gBlueInitStep++;
        return CL_OK;
	}
	
	for (int i=0; i<3; i++) {
        if (BlueSendCmd("AT+DEVT=0\r\n", "OK",3000) == CL_OK) {
			gBlueInitStep++;
            return CL_OK;
        }
        if(retry > 1){
            OS_DELAY_MS(1000);
        }
    }
    return CL_FAIL;	
}


int BuleModePair(char ok, uint8_t retry)
{
    return CheckBlueModeCmd(ok, retry, 0);
}


int BuleModeCons(char ok, uint8_t retry)
{
    return CheckBlueModeCmd(ok, retry, 0);
}


int BuleModeGWMac(char ok, uint8_t retry)
{
    return CheckBlueModeCmd(ok, retry, 0);
}


int BuleModeVers(char ok, uint8_t retry)
{
    return CheckBlueModeCmd(ok, retry, 0);
}


int BuleModeAddr(char ok, uint8_t retry)
{
    return CheckBlueModeCmd(ok, retry, 0);
}


int BuleSetName(char ok, uint8_t retry)
{
	char name[12] = {0};
	
	if(isArraryEmpty(&gSystemInfo1.station_id[3], 5) == CL_OK)
	{
		CL_LOG("charger sn is all zero, no need to set blue name.\n");
		gBlueInitStep++;
        return CL_OK;
	}
	
	BCDToString(name,(void*)&gSystemInfo1.station_id[3], 5);
	#if 1
	CL_LOG("蓝牙名字 :%s.\n", gChgInfo.blue_name);
	CL_LOG("name :%s.\n", name);
	if (0 == memcmp(gChgInfo.blue_name, name, 10))
	{
		CL_LOG("name is ok, no need to set blue name.\n");
        gBlueInitStep++;
        return CL_OK;
	}
	#endif
	if(SetBlueName(name) == CL_OK)
    {
		CL_LOG("set name=%s ok.\n",name);
		gBlueInitStep++;
		return CL_OK;
	}
	
    CL_LOG("set name fail.\n");
    return CL_FAIL;
}


int BuleModeEcho(char ok, uint8_t retry)
{
    return CheckBlueModeCmd(ok, retry, 0);
}


int BuleModeDevt(char ok, uint8_t retry)
{
    CL_LOG("in.\n");
    return CheckBlueModeCmd(ok, retry, 0);
}


int BuleModeGetName(char ok, uint8_t retry)
{
    CL_LOG("BuleModeGetName 获取蓝牙名字\n");
    return CheckBlueModeCmd(ok, retry, 0);
}



int BuleReconnect(void)
{
	char ok;
    uint8_t retry = 0;

	gChgInfo.isBlueToothInit = 0;
	gBlueInitStep = BLUE_RESET;
	while(1) 
    {
        OS_DELAY_MS(gBlueInitTab[gBlueInitStep].nwait*10);
        if (gBlueInitTab[gBlueInitStep].cmd) 
        {
            ok = BlueSendCmd(gBlueInitTab[gBlueInitStep].cmd,gBlueInitTab[gBlueInitStep].res,gBlueInitTab[gBlueInitStep].wait);
        }
        CL_LOG("gBlueInitStep = %d \n", gBlueInitStep);
        if (gBlueInitTab[gBlueInitStep].process) 
        {
            if (CL_OK == gBlueInitTab[gBlueInitStep].process(ok, retry)) 
            {
                retry = 0;
            }
            else
            {
                retry++;
            }
        }

		if (gBlueInitStep == BLUE_STATE_NUM) 
        {
			CL_LOG("blue init ok.\n");
			gChgInfo.isBlueToothInit = 0xa5;
			return CL_OK;
		}
	}
}


//蓝牙测试
int BlueTest(int retry)
{
    for (int i=0; i<retry; i++) {
        if (BlueSendCmd("AT\r\n", "OK",1000) == CL_OK) {
            return CL_OK;
        }
        if(retry > 1){
            OS_DELAY_MS(1000);
        }
    }
    return CL_FAIL;
}



//设置蓝牙名称
int SetBlueName(char *name)
{
    char  buf[32];
    memset(buf,0,32);
    sprintf(buf,"AT+NAME=%s\r\n",name);
    for (int i=0; i<2; i++) {
        if (BlueSendCmd(buf, "OK", 2000)==CL_OK) {
            return CL_OK;
        }
        OS_DELAY_MS(1000);
    }
    return CL_FAIL;
}

//设置2.4G gwmac地址
int SetGateWay24GMacAddr(char *mac, uint8_t cmd)
{
    uint32_t i = 0;
    char buf[32];
    memset(buf, 0, 32);
    if(USER_CMD_ADDR == cmd)
    {
        sprintf(buf,"AT+ADDR=%s\r\n",mac);
    }
    else{
        sprintf(buf,"AT+GWID=%s\r\n",mac);
    }
    
    for (i = 0; i < 2; i++)
    {
        if (BlueSendCmd(buf, "OK", 2000) == CL_OK)
        {
            return CL_OK;
        }
		OS_DELAY_MS(1000);
    }
    return CL_FAIL;
}

//获取2.4G gwmac地址
int GetGateWay24GMacAddr(uint8_t cmd)
{
    uint32_t i = 0;
    char buf[32];
    memset(buf,0,32);
    if(USER_CMD_ADDR == cmd)
    {
        strcpy(buf, "AT+ADDR?\r\n");
    }
    else{
        strcpy(buf,"AT+GWID?\r\n");
    }
    
    for (i = 0; i < 2; i++)
    {
        if (BlueSendCmd(buf, "OK", 2000) == CL_OK)
        {
            return CL_OK;
        }
		OS_DELAY_MS(1000);
    }
    return CL_FAIL;
}

//获取蓝牙名称
int GetBlueName(void)
{
    uint32_t i = 0;
    char buf[32];
    memset(buf,0,32);
    //sprintf(buf,"AT+NAME?\r\n");
    strcpy(buf, "AT+NAME?\r\n");
    for (i = 0; i < 2; i++)
    {
        if (BlueSendCmd(buf, "OK", 2000) == CL_OK)
        {
            gBlueInitStep++;
            return CL_OK;
        }
		OS_DELAY_MS(1000);
    }
    printf("蓝牙名字错误！\n");
    gBlueInitStep = BLUE_RESET;
    return CL_FAIL;
}

//设置蓝牙配对状态
int SetBluePair(int value)
{
    char  buf[32];
    memset(buf,0,32);
    sprintf(buf,"AT+PAIR=%d\r\n",value);
    for (int i=0; i<3; i++) {
        
        if (BlueSendCmd(buf, "OK", 2000)==CL_OK) {
            return CL_OK;
        }
		OS_DELAY_MS(1000);
    }
    return CL_FAIL;
}

void CKB24_UMGPIO_Conf(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//PB1--BT_EN  PB0--BT_RST
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOOUT;
	GPIO_InitStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_Init(HT_GPIOB,&GPIO_InitStruct);
	
	//PD14--BT_C/D
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOOUT;
	GPIO_InitStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_Init(HT_GPIOD,&GPIO_InitStruct);
    
	//PD15--BT_IRQ
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOIN;
	GPIO_InitStruct.GPIO_InputStruct = GPIO_Input_Up;
	HT_GPIO_Init(HT_GPIOD,&GPIO_InitStruct);

    CKB24_POWER_EN();
    
    CKB24_RST_HIGH();
}


void CKB24_UM_HardReset(void)
{
    CKB24_POWER_DIS();
    osDelay(2000);
    CKB24_POWER_EN();
    osDelay(2000);
}

void CKB24_UM_SoftReset(void)
{
    CKB24_RST_LOW();
    osDelay(1000);
    CKB24_RST_HIGH();
    osDelay(100);
}


void CKB24_UM_Init(void)
{
    //GPIO初始化
    CKB24_UMGPIO_Conf();

    //串口初始化
    UART4_Init(115200);
}

