/*
*********************************************************************************************************
*                                              主任务
*                                       
* File         : 
* By           : 
* Version      : 
* Description  : 系统主任务处理
*********************************************************************************************************
*/
#include "main_handle.h"
#include "system_info.h"
#include "charger.h"
#include "ckb24_um_handle.h"
#include "com_handle.h"
#include "m1_card.h"
#include "emu_info.h"
#include "timer.h"
#include "sc8042b.h"
#include "simuart.h"
#include "flash_usr.h"
#include "delay.h"
#include "rtc.h"
#include "pmu.h"
#include "aes.h"
#include "upgrade.h"
#include "uart.h"
#include "CardAuth_handle.h"
#include "blue.h"
#include "comm_proto.h"


/**
*系统配置信息
*/
system_info_t system_info;	
sys_info1 gSystemInfo1;
gun_info_str1 gGunInfo1;
gun_info_str2 gGunInfo2;
gun_info_str3 gGunInfo3;
cost_template_str gCostTemplateInfo;

/**
*内层报文序列号
*/
PACK_SERIAL_NUM_TYPE SerialNumInfo;


/**
*系统错误代码
*/
uint32_t ErrorCode = 0;


/**
*日志缓存区
*/
uint8_t  gPrintBuff[128];
uint16_t gWrite = 0;

uint32_t isCheckTimeOverFlag = 0;

/**
*蓝牙发送心跳时间
*/


	
static unsigned char SecretCardKEY_A[6] = {'6', 'f', '7', 'd', '2', 'k'};
static unsigned char SeedKeyA[16]="chargerlink1234";


	


#ifdef __GNUC__
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
    if(system_info.printSwitch == 0) 
	{
		return ch; 
	}

    if (gWrite < sizeof(gPrintBuff))
    {
        gPrintBuff[gWrite++] = ch;
        if ('\n' == ch)
        {
            SimuUartSendData(&gSimUartCtrl[0], gPrintBuff, gWrite);
            gWrite = 0;
        }
    }
    else
	{
        SimuUartSendData(&gSimUartCtrl[0], gPrintBuff, sizeof(gPrintBuff));
        gWrite = 0;
    }
	
    return ch;
}


int CheckSum(uint8_t *pData, uint16_t len)
{
    int i;
    uint8_t sum = 0; 

    for (i=0; i<len; i++) {
        sum += pData[i];
    }
    return sum;
}

void PrintfData(void *pfunc, uint8_t *pdata, int len)
{
	int i;
    
	CL_LOG("call by %s,len=%d,pdata:",(char*)pfunc,len);
	for (i=0; i<len; i++) {
		printf("%02x",pdata[i]);
	}
	printf("\n");
}


int GetCpuTemp(void)
{
    uint16_t temp = HT_TBS->TMPDAT;
    int values;
    if(temp >= 0x8000){
        values = temp - 65536;
    }else{
        values = temp;
    }
    
    int CPUTemp = (int)((12.9852-0.0028*values));
    
    return CPUTemp;
}

unsigned char Asc2Int(char ch) 
{
    unsigned char val = 0;
	
    if ((ch >= '0') && (ch <= '9')) 
	{
        val = ch - '0';
    } 
	else if ((ch >= 'A') && (ch <= 'F')) 
	{
        val = ch - 'A' + 10;
    } 
	else if ((ch >= 'a') && (ch <= 'f')) 
	{
        val = ch - 'a' + 10;
    }
	
    return val;
}


int StringToBCD(unsigned char *BCD, const char *str) 
{
    unsigned char chh, chl;
    int length = strlen(str);
    int index = 0;

    for (index = 0; index < length; index += 2)
    {
        chh = Asc2Int(str[index]);
        chl = Asc2Int(str[index + 1]);
       
        BCD[index / 2] = (chh << 4) | chl;
    }
    return (length / 2);
}

char *BCDToString(char *dest, unsigned char *BCD, int bytes) 
{
    char  temp[] = "0123456789ABCDEF";
    int index = 0;
    int length = 0;
    if (BCD == NULL || bytes <= 0)
        return NULL;
    
    for (index = 0; index < bytes; index++) {
        dest[length++] = temp[(BCD[index] >> 4) & 0x0F];
        dest[length++] = temp[BCD[index] & 0x0F];
    }
    dest[length] = '\0';
    return dest;
}

int isArraryEmpty(uint8_t *pAddr, int len)
{
	int returnData = CL_FAIL;
	uint32_t i = 0;
	uint32_t sum = 0;
	uint32_t sum1 = 0;
	
	for(i = 0; i < len; i++)
	{
		if(0 == pAddr[i])
		{
			sum++;
		}
		if(0xff == pAddr[i])
		{
			sum1++;
		}
	}
	if((sum1 == len) || (sum == len))
	{
		returnData = CL_OK;
	}

	return returnData;
}

//int BCDToBCD(unsigned char *BCD1, const char *BCD2, int len) 
//{
//    int i = 0;
//    
//    for(i = 0; i < len; i++)
//    {
//        BCD1[len - i -1] = (BCD2[len - i -1] >> 4);
//        BCD1[len - i -1] = (BCD2[i + 1] >> 4);
//    }
//}

	
uint8_t flag1=0,flag2=0; 

/*****************************************************************************
** Function name:       OpenCheckTimeOver
** Descriptions:        打开检测刷卡开启充电服务器返回是否超时, 超时播报开启失败
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void OpenCheckTimeOver(void)
{
    
    if((GUN_CHARGING_IDLE < (gun_info[0].is_load_on)) && (GUN_CHARGING_IDLE < (gun_info[1].is_load_on)))
    {
        return;
    }
    gChgInfo.isOverTimeFlag = 1;
    isCheckTimeOverFlag = osGetTimes();

    return;
}

/*****************************************************************************
** Function name:       CloseCheckTimeOver
** Descriptions:        关闭检测刷卡开启充电服务器返回是否超时
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void CloseCheckTimeOver(void)
{
    gChgInfo.isOverTimeFlag = 0;

    return;
}


/*****************************************************************************
** Function name:       SetReadCardFlag
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void SetReadCardFlag(void)
{
    gChgInfo.RF24GCardFlag.isReadCardFlag = 1;    //读卡标志
}

/*****************************************************************************
** Function name:       CleanReadCardFlag
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void CleanReadCardFlag(void)
{
    gChgInfo.RF24GCardFlag.isReadCardFlag = 0;    //清除读到卡标志
}

/*****************************************************************************
** Function name:       GetReadCardFlag
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int GetReadCardFlag(void)
{
    return gChgInfo.RF24GCardFlag.isReadCardFlag;    //读到标志
}

/*****************************************************************************
** Function name:       isTheSameCard
** Descriptions:        卡不相同返回 1
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int isTheSameCard(void)
{
    static uint8_t usrCardID[5] = {0,};

    if(((usrCardID[0] & 0x0f) != (gChgInfo.current_usr_card_id[0] & 0x0f)) ||
        (usrCardID[1]!= gChgInfo.current_usr_card_id[1]) ||
        (usrCardID[2]!= gChgInfo.current_usr_card_id[2]) ||
        (usrCardID[3]!= gChgInfo.current_usr_card_id[3]) ||
        (usrCardID[4]!= gChgInfo.current_usr_card_id[4]))
    {
        usrCardID[0] = gChgInfo.current_usr_card_id[0];
        usrCardID[1] = gChgInfo.current_usr_card_id[1];
        usrCardID[2] = gChgInfo.current_usr_card_id[2];
        usrCardID[3] = gChgInfo.current_usr_card_id[3];
        usrCardID[4] = gChgInfo.current_usr_card_id[4];
        
        CL_LOG("不相同的卡片");
        
        return 1;
    }

    return 0;
}

/*****************************************************************************
** Function name:       SetReadCardTime
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void SetReadCardTime(void)
{
    gChgInfo.RF24GCardFlag.ReadCardTimeLenth = 1;    //读卡标志
}

/*****************************************************************************
** Function name:       CleanReadCardTime
** Descriptions:        
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void CleanReadCardTime(void)
{
    gChgInfo.RF24GCardFlag.ReadCardTimeLenth = 0;    //清除读到卡标志
}

/*****************************************************************************
** Function name:       GetReadCardTime
** Descriptions:        如果为 1 表示 n 秒没有读到卡, 则判断卡已经移开，如果是同一张卡，
                        说明是刷卡第二次，做这个标志的目的就是为了防止同一张卡刷两次的时候要等待 T 秒
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
int GetReadCardTime(void)
{
    return gChgInfo.RF24GCardFlag.ReadCardTimeLenth;    //读到标志
}

/*****************************************************************************
** Function name:       CheckCardTimeLenth
** Descriptions:        检测两次刷卡时间长度超过n秒
** input parameters:    None
** output parameters:   None
** Returned value:	  None
** Author:              quqian
*****************************************************************************/
void CheckCardTimeLenth(void)
{
    static uint32_t authReadCardTicks = 0;
    if(osGetTimes() > authReadCardTicks)
    {
        if (osGetTimes() - authReadCardTicks >= 5000)    //5 秒没有检测到卡说明卡已经拿开
        {
            SetReadCardTime();
        }
    }
    else{
        if (0xffffffff - authReadCardTicks + osGetTimes() >= 5000)    //5 秒没有检测到卡说明卡已经拿开
        {
            SetReadCardTime();
        }
    }
    
    authReadCardTicks = osGetTimes();
}

//读卡处理
void Check_M1_Card()
{
	uint8_t PICC_ATQA[2],PICC_SAK[3],PICC_UID[4];
	//uint8_t pCardId[8];
	
	if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
	{
		PrintfData("PICC_UID:",PICC_UID,4);
        PrintfData("PICC_ATQA1:",PICC_ATQA,2);
		//密钥卡
		if(Mifare_Auth(0,2,SecretCardKEY_A,PICC_UID) == OK)
        {
			CL_LOG("this is SecretCard.\n");
			Sc8042bSpeech(VOIC_CARD);
		}
        else
		{//非密钥卡
			uint8_t ucardkeyA[16]={0};
			AES_KEY aes;
			memset(&aes,0,sizeof(AES_KEY));
			AES_set_encrypt_key(SeedKeyA, 128, &aes);
            
            //CL_LOG("瞿嵌aes.rounds = %d.\n",aes.rounds);
            //PrintfData("瞿嵌Check_M1_Card", (uint8_t *)&aes.rd_key[0], 4 *(14 + 1));
            
			uint8_t card[16] ;
			memset(card,0,16);
			memcpy(card,PICC_UID,4);
			AES_encrypt(card, ucardkeyA, &aes);
            //PrintfData("瞿嵌Check_M", ucardkeyA, 16);
            
			if((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
            {
				uint8_t block_data[16];
				memset((void*)block_data,0,16);
			    PrintfData("PICC_ATQA2:",PICC_ATQA,2);
                
				if(Mifare_Auth(0,2,ucardkeyA,PICC_UID) == OK)
				{
					if(Mifare_Blockread(9,block_data) == OK) 
					{
						sprintf((char*)gChgInfo.current_usr_card_id, "%02x%02x%02x%02x%02x",
								block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);
                        CL_LOG("block_data[4] = %02x ,block_data[5] = %02x ,block_data[6] = %02x ,block_data[7] = %02x ,block_data[8] = %02x .\n",
                            block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);

//						StringToBCD(pCardId, (const char *)gChgInfo.current_usr_card_id);
//					    PrintfData("卡号", &pCardId[3], 5);

						gChgInfo.ReadCardFlag = 0xa5;
					}
				}
                else
				{
					CL_LOG("Mifare_Auth error.\n");
				}
			}
		}
		
		TypeA_Halt();
	}
}

//读卡处理
void Check_M0_Card()
{
	uint8_t PICC_ATQA[2],PICC_SAK[3],PICC_UID[4];
	if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
	{
		PrintfData("PICC_UID:",PICC_UID,4);
        PrintfData("PICC_ATQA1:",PICC_ATQA,2);
		//密钥卡
		if(Mifare_Auth(0,2,SecretCardKEY_A,PICC_UID) == OK)
        {
			CL_LOG("this is SecretCard.\n");
			Sc8042bSpeech(VOIC_CARD);
		}
        else{//非密钥卡
			uint8_t ucardkeyA[16]={0};
			AES_KEY aes;
			memset(&aes,0,sizeof(AES_KEY));
			AES_set_encrypt_key(SeedKeyA, 128, &aes);
            
            //CL_LOG("瞿嵌aes.rounds = %d.\n",aes.rounds);
            //PrintfData("瞿嵌Check_M1_Card", (uint8_t *)&aes.rd_key[0], 4 *(14 + 1));
            
			uint8_t card[16] ;
			memset(card,0,16);
			memcpy(card,PICC_UID,4);
			AES_encrypt(card, ucardkeyA, &aes);
            //PrintfData("瞿嵌Check_M", ucardkeyA, 16);
            
			if((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
            {
				uint8_t block_data[16];
				memset((void*)block_data,0,16);
			    PrintfData("PICC_ATQA2:",PICC_ATQA,2);
                
				if(Mifare_Auth(0,2,ucardkeyA,PICC_UID) == OK)
				{
					if(Mifare_Blockread(0,block_data) == OK) 
					{
                        #if 0
                        CL_LOG("block0_data[0] = %02x ,block0_data[1] = %02x ,block0_data[2] = %02x ,block0_data[3] = %02x .\n",
                            block_data[0], block_data[1], block_data[2],block_data[3]);
                        CL_LOG("block0_data[4] = %02x ,block0_data[5] = %02x ,block0_data[6] = %02x ,block0_data[7] = %02x ,block0_data[8] = %02x .\n",
                            block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);
                        
                        CL_LOG("block0_data[9] = %02x ,block0_data[10] = %02x ,block0_data[11] = %02x ,block0_data[12] = %02x ,block0_data[13] = %02x ,block0_data[14] = %02x ,block0_data[15] = %02x .\n",
                            block_data[9],block_data[10],block_data[11],block_data[12],block_data[13],block_data[14],block_data[15]);
                        #endif
					}
				}
                else{
					CL_LOG("Mifare_Auth error.\n");
				}
			}
		}

		TypeA_Halt();
	}
}
//读卡处理
void Check_M2_Card()
{
	uint8_t PICC_ATQA[2],PICC_SAK[3],PICC_UID[4];
	if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
	{
		PrintfData("PICC_UID:",PICC_UID,4);
        PrintfData("PICC_ATQA1:",PICC_ATQA,2);
		//密钥卡
		if(Mifare_Auth(0,2,SecretCardKEY_A,PICC_UID) == OK)
        {
			CL_LOG("this is SecretCard.\n");
			Sc8042bSpeech(VOIC_CARD);
		}
        else{//非密钥卡
			uint8_t ucardkeyA[16]={0};
			AES_KEY aes;
			memset(&aes,0,sizeof(AES_KEY));
			AES_set_encrypt_key(SeedKeyA, 128, &aes);
            
            //CL_LOG("瞿嵌aes.rounds = %d.\n",aes.rounds);
            //PrintfData("瞿嵌Check_M1_Card", (uint8_t *)&aes.rd_key[0], 4 *(14 + 1));
            
			uint8_t card[16] ;
			memset(card,0,16);
			memcpy(card,PICC_UID,4);
			AES_encrypt(card, ucardkeyA, &aes);
            //PrintfData("瞿嵌Check_M", ucardkeyA, 16);
            
			if((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
            {
				uint8_t block_data[16];
				memset((void*)block_data,0,16);
			    PrintfData("PICC_ATQA2:",PICC_ATQA,2);
                
				if(Mifare_Auth(0,2,ucardkeyA,PICC_UID) == OK)
				{
					if(Mifare_Blockread(1,block_data) == OK) 
					{
                        
                    #if 0
                    CL_LOG("block1_data[0] = %02x ,block1_data[1] = %02x ,block1_data[2] = %02x ,block1_data[3] = %02x .\n",
                        block_data[0], block_data[1], block_data[2],block_data[3]);
                    CL_LOG("block1_data[4] = %02x ,block1_data[5] = %02x ,block1_data[6] = %02x ,block1_data[7] = %02x ,block1_data[8] = %02x .\n",
                        block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);
                    
                    CL_LOG("block1_data[9] = %02x ,block1_data[10] = %02x ,block1_data[11] = %02x ,block1_data[12] = %02x ,block1_data[13] = %02x ,block1_data[14] = %02x ,block1_data[15] = %02x .\n",
                        block_data[9],block_data[10],block_data[11],block_data[12],block_data[13],block_data[14],block_data[15]);
                    #endif
					}
				}
                else{
					CL_LOG("Mifare_Auth error.\n");
				}
			}
		}
		TypeA_Halt();
	}
}
//读卡处理
void Check_M3_Card()
{
	uint8_t PICC_ATQA[2],PICC_SAK[3],PICC_UID[4];
	if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
	{
		PrintfData("PICC_UID:",PICC_UID,4);
        PrintfData("PICC_ATQA1:",PICC_ATQA,2);
		//密钥卡
		if(Mifare_Auth(0,2,SecretCardKEY_A,PICC_UID) == OK)
        {
			CL_LOG("this is SecretCard.\n");
			Sc8042bSpeech(VOIC_CARD);
		}
        else{//非密钥卡
			uint8_t ucardkeyA[16]={0};
			AES_KEY aes;
			memset(&aes,0,sizeof(AES_KEY));
			AES_set_encrypt_key(SeedKeyA, 128, &aes);
            
            //CL_LOG("瞿嵌aes.rounds = %d.\n",aes.rounds);
            //PrintfData("瞿嵌Check_M1_Card", (uint8_t *)&aes.rd_key[0], 4 *(14 + 1));
            
			uint8_t card[16] ;
			memset(card,0,16);
			memcpy(card,PICC_UID,4);
			AES_encrypt(card, ucardkeyA, &aes);
            //PrintfData("瞿嵌Check_M", ucardkeyA, 16);
            
			if((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
            {
				uint8_t block_data[16];
				memset((void*)block_data,0,16);
			    PrintfData("PICC_ATQA2:",PICC_ATQA,2);
                
				if(Mifare_Auth(0,2,ucardkeyA,PICC_UID) == OK)
				{
					if(Mifare_Blockread(2,block_data) == OK) 
					{
                            #if 0
                            CL_LOG("block2_data[0] = %02x ,block2_data[1] = %02x ,block2_data[2] = %02x ,block2_data[3] = %02x .\n",
                                block_data[0], block_data[1], block_data[2],block_data[3]);
                            CL_LOG("block2_data[4] = %02x ,block2_data[5] = %02x ,block2_data[6] = %02x ,block2_data[7] = %02x ,block2_data[8] = %02x .\n",
                                block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);
                            
                            CL_LOG("block2_data[9] = %02x ,block2_data[10] = %02x ,block2_data[11] = %02x ,block2_data[12] = %02x ,block2_data[13] = %02x ,block2_data[14] = %02x ,block2_data[15] = %02x .\n",
                                block_data[9],block_data[10],block_data[11],block_data[12],block_data[13],block_data[14],block_data[15]);
                            #endif
                    }
				}
                else{
					CL_LOG("Mifare_Auth error.\n");
				}
			}
		}
		TypeA_Halt();
	}
}
//读卡处理
void Check_M4_Card()
{
	uint8_t PICC_ATQA[2],PICC_SAK[3],PICC_UID[4];
	if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
	{
		PrintfData("PICC_UID:",PICC_UID,4);
        PrintfData("PICC_ATQA1:",PICC_ATQA,2);
		//密钥卡
		if(Mifare_Auth(0,2,SecretCardKEY_A,PICC_UID) == OK)
        {
			CL_LOG("this is SecretCard.\n");
			Sc8042bSpeech(VOIC_CARD);
		}
        else{//非密钥卡
			uint8_t ucardkeyA[16]={0};
			AES_KEY aes;
			memset(&aes,0,sizeof(AES_KEY));
			AES_set_encrypt_key(SeedKeyA, 128, &aes);
            
            //CL_LOG("瞿嵌aes.rounds = %d.\n",aes.rounds);
            //PrintfData("瞿嵌Check_M1_Card", (uint8_t *)&aes.rd_key[0], 4 *(14 + 1));
            
			uint8_t card[16] ;
			memset(card,0,16);
			memcpy(card,PICC_UID,4);
			AES_encrypt(card, ucardkeyA, &aes);
            //PrintfData("瞿嵌Check_M", ucardkeyA, 16);
            
			if((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
            {
				uint8_t block_data[16];
				memset((void*)block_data,0,16);
			    PrintfData("PICC_ATQA2:",PICC_ATQA,2);
                
				if(Mifare_Auth(0,2,ucardkeyA,PICC_UID) == OK)
				{
					if(Mifare_Blockread(3,block_data) == OK) 
					{
                            #if 0
                            CL_LOG("block3_data[0] = %02x ,block3_data[1] = %02x ,block3_data[2] = %02x ,block3_data[3] = %02x .\n",
                                block_data[0], block_data[1], block_data[2],block_data[3]);
                            CL_LOG("block3_data[4] = %02x ,block3_data[5] = %02x ,block3_data[6] = %02x ,block3_data[7] = %02x ,block3_data[8] = %02x .\n",
                                block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);
                            
                            CL_LOG("block3_data[9] = %02x ,block3_data[10] = %02x ,block3_data[11] = %02x ,block3_data[12] = %02x ,block3_data[13] = %02x ,block3_data[14] = %02x ,block3_data[15] = %02x .\n",
                                block_data[9],block_data[10],block_data[11],block_data[12],block_data[13],block_data[14],block_data[15]);
                            #endif
                    }
				}
                else{
					CL_LOG("Mifare_Auth error.\n");
				}
			}
		}
		TypeA_Halt();
	}
}
//读卡处理
void Check_M5_Card()
{
	uint8_t PICC_ATQA[2],PICC_SAK[3],PICC_UID[4];
	if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
	{
		PrintfData("PICC_UID:",PICC_UID,4);
        PrintfData("PICC_ATQA1:",PICC_ATQA,2);
		//密钥卡
		if(Mifare_Auth(0,2,SecretCardKEY_A,PICC_UID) == OK)
        {
			CL_LOG("this is SecretCard.\n");
			Sc8042bSpeech(VOIC_CARD);
		}
        else{//非密钥卡
			uint8_t ucardkeyA[16]={0};
			AES_KEY aes;
			memset(&aes,0,sizeof(AES_KEY));
			AES_set_encrypt_key(SeedKeyA, 128, &aes);
            
            //CL_LOG("瞿嵌aes.rounds = %d.\n",aes.rounds);
            //PrintfData("瞿嵌Check_M1_Card", (uint8_t *)&aes.rd_key[0], 4 *(14 + 1));
            
			uint8_t card[16] ;
			memset(card,0,16);
			memcpy(card,PICC_UID,4);
			AES_encrypt(card, ucardkeyA, &aes);
            //PrintfData("瞿嵌Check_M", ucardkeyA, 16);
            
			if((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
            {
				uint8_t block_data[16];
				memset((void*)block_data,0,16);
			    PrintfData("PICC_ATQA2:",PICC_ATQA,2);
                
				if(Mifare_Auth(0,2,ucardkeyA,PICC_UID) == OK)
				{
					if(Mifare_Blockread(4,block_data) == OK) 
					{
                            #if 0
                            CL_LOG("block4_data[0] = %02x ,block4_data[1] = %02x ,block4_data[2] = %02x ,block4_data[3] = %02x .\n",
                                block_data[0], block_data[1], block_data[2],block_data[3]);
                            CL_LOG("block4_data[4] = %02x ,block4_data[5] = %02x ,block4_data[6] = %02x ,block4_data[7] = %02x ,block4_data[8] = %02x .\n",
                                block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);
                            
                            CL_LOG("block4_data[9] = %02x ,block4_data[10] = %02x ,block4_data[11] = %02x ,block4_data[12] = %02x ,block4_data[13] = %02x ,block4_data[14] = %02x ,block4_data[15] = %02x .\n",
                                block_data[9],block_data[10],block_data[11],block_data[12],block_data[13],block_data[14],block_data[15]);
                            #endif
                    }
				}
                else{
					CL_LOG("Mifare_Auth error.\n");
				}
			}
		}
		TypeA_Halt();
	}
}
//读卡处理
void Check_M6_Card()
{
	uint8_t PICC_ATQA[2],PICC_SAK[3],PICC_UID[4];
	if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
	{
		PrintfData("PICC_UID:",PICC_UID,4);
        PrintfData("PICC_ATQA1:",PICC_ATQA,2);
		//密钥卡
		if(Mifare_Auth(0,2,SecretCardKEY_A,PICC_UID) == OK)
        {
			CL_LOG("this is SecretCard.\n");
			Sc8042bSpeech(VOIC_CARD);
		}
        else{//非密钥卡
			uint8_t ucardkeyA[16]={0};
			AES_KEY aes;
			memset(&aes,0,sizeof(AES_KEY));
			AES_set_encrypt_key(SeedKeyA, 128, &aes);
            
            //CL_LOG("瞿嵌aes.rounds = %d.\n",aes.rounds);
            //PrintfData("瞿嵌Check_M1_Card", (uint8_t *)&aes.rd_key[0], 4 *(14 + 1));
            
			uint8_t card[16] ;
			memset(card,0,16);
			memcpy(card,PICC_UID,4);
			AES_encrypt(card, ucardkeyA, &aes);
            //PrintfData("瞿嵌Check_M", ucardkeyA, 16);
            
			if((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
            {
				uint8_t block_data[16];
				memset((void*)block_data,0,16);
			    PrintfData("PICC_ATQA2:",PICC_ATQA,2);
                
				if(Mifare_Auth(0,2,ucardkeyA,PICC_UID) == OK)
				{
					if(Mifare_Blockread(5,block_data) == OK) 
					{
                            #if 0
                            CL_LOG("block5_data[0] = %02x ,block5_data[1] = %02x ,block5_data[2] = %02x ,block5_data[3] = %02x .\n",
                                block_data[0], block_data[1], block_data[2],block_data[3]);
                            CL_LOG("block5_data[4] = %02x ,block5_data[5] = %02x ,block5_data[6] = %02x ,block5_data[7] = %02x ,block5_data[8] = %02x .\n",
                                block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);
                            
                            CL_LOG("block5_data[9] = %02x ,block5_data[10] = %02x ,block5_data[11] = %02x ,block5_data[12] = %02x ,block5_data[13] = %02x ,block5_data[14] = %02x ,block5_data[15] = %02x .\n",
                                block_data[9],block_data[10],block_data[11],block_data[12],block_data[13],block_data[14],block_data[15]);
                            #endif
                        }
				}
                else{
					CL_LOG("Mifare_Auth error.\n");
				}
			}
		}
		TypeA_Halt();
	}
}
//读卡处理
void Check_M7_Card()
{
	uint8_t PICC_ATQA[2],PICC_SAK[3],PICC_UID[4];
	if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
	{
		PrintfData("PICC_UID:",PICC_UID,4);
        PrintfData("PICC_ATQA1:",PICC_ATQA,2);
		//密钥卡
		if(Mifare_Auth(0,2,SecretCardKEY_A,PICC_UID) == OK)
        {
			CL_LOG("this is SecretCard.\n");
			Sc8042bSpeech(VOIC_CARD);
		}
        else{//非密钥卡
			uint8_t ucardkeyA[16]={0};
			AES_KEY aes;
			memset(&aes,0,sizeof(AES_KEY));
			AES_set_encrypt_key(SeedKeyA, 128, &aes);
            
            //CL_LOG("瞿嵌aes.rounds = %d.\n",aes.rounds);
            //PrintfData("瞿嵌Check_M1_Card", (uint8_t *)&aes.rd_key[0], 4 *(14 + 1));
            
			uint8_t card[16] ;
			memset(card,0,16);
			memcpy(card,PICC_UID,4);
			AES_encrypt(card, ucardkeyA, &aes);
            //PrintfData("瞿嵌Check_M", ucardkeyA, 16);
            
			if((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK) == OK))
            {
				uint8_t block_data[16];
				memset((void*)block_data,0,16);
			    PrintfData("PICC_ATQA2:",PICC_ATQA,2);
                
				if(Mifare_Auth(0,2,ucardkeyA,PICC_UID) == OK)
				{
					if(Mifare_Blockread(6,block_data) == OK) 
					{
                            #if 0
                            CL_LOG("block6_data[0] = %02x ,block6_data[1] = %02x ,block6_data[2] = %02x ,block6_data[3] = %02x .\n",
                                block_data[0], block_data[1], block_data[2],block_data[3]);
                            CL_LOG("block6_data[4] = %02x ,block6_data[5] = %02x ,block6_data[6] = %02x ,block6_data[7] = %02x ,block6_data[8] = %02x .\n",
                                block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);
                            
                            CL_LOG("block6_data[9] = %02x ,block6_data[10] = %02x ,block6_data[11] = %02x ,block6_data[12] = %02x ,block6_data[13] = %02x ,block6_data[14] = %02x ,block6_data[15] = %02x .\n",
                                block_data[9],block_data[10],block_data[11],block_data[12],block_data[13],block_data[14],block_data[15]);
                            #endif
                    }
				}
                else{
					CL_LOG("Mifare_Auth error.\n");
				}
			}
		}
		TypeA_Halt();
	}
}
//读卡处理
void Check_M8_Card()
{
	uint8_t PICC_ATQA[2],PICC_SAK[3],PICC_UID[4];
	if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
	{
		PrintfData("PICC_UID:",PICC_UID,4);
        PrintfData("PICC_ATQA1:",PICC_ATQA,2);
		//密钥卡
		if(Mifare_Auth(0,2,SecretCardKEY_A,PICC_UID) == OK)
        {
			CL_LOG("this is SecretCard.\n");
			Sc8042bSpeech(VOIC_CARD);
		}
        else{//非密钥卡
			uint8_t ucardkeyA[16]={0};
			AES_KEY aes;
			memset(&aes,0,sizeof(AES_KEY));
			AES_set_encrypt_key(SeedKeyA, 128, &aes);
            
            //CL_LOG("瞿嵌aes.rounds = %d.\n",aes.rounds);
            //PrintfData("瞿嵌Check_M1_Card", (uint8_t *)&aes.rd_key[0], 4 *(14 + 1));
            
			uint8_t card[16] ;
			memset(card,0,16);
			memcpy(card,PICC_UID,4);
			AES_encrypt(card, ucardkeyA, &aes);
            //PrintfData("瞿嵌Check_M", ucardkeyA, 16);
            
			if((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
            {
				uint8_t block_data[16];
				memset((void*)block_data,0,16);
			    PrintfData("PICC_ATQA2:",PICC_ATQA,2);
                
				if(Mifare_Auth(0,2,ucardkeyA,PICC_UID) == OK)
				{
					if(Mifare_Blockread(7,block_data) == OK) 
					{
                            #if 0
                            CL_LOG("block7_data[0] = %02x ,block7_data[1] = %02x ,block7_data[2] = %02x ,block7_data[3] = %02x .\n",
                                block_data[0], block_data[1], block_data[2],block_data[3]);
                            CL_LOG("block7_data[4] = %02x ,block7_data[5] = %02x ,block7_data[6] = %02x ,block7_data[7] = %02x ,block7_data[8] = %02x .\n",
                                block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);
                            
                            CL_LOG("block7_data[9] = %02x ,block7_data[10] = %02x ,block7_data[11] = %02x ,block7_data[12] = %02x ,block7_data[13] = %02x ,block7_data[14] = %02x ,block7_data[15] = %02x .\n",
                                block_data[9],block_data[10],block_data[11],block_data[12],block_data[13],block_data[14],block_data[15]);
                            #endif
                    }
				}
                else{
					CL_LOG("Mifare_Auth error.\n");
				}
			}
		}
		TypeA_Halt();
	}
}

//读卡处理
void Check_M9_Card()
{
	uint8_t PICC_ATQA[2],PICC_SAK[3],PICC_UID[4];
	if ((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
	{
		PrintfData("PICC_UID:",PICC_UID,4);
        PrintfData("PICC_ATQA1:",PICC_ATQA,2);
		//密钥卡
		if(Mifare_Auth(0,2,SecretCardKEY_A,PICC_UID) == OK)
        {
			CL_LOG("this is SecretCard.\n");
			Sc8042bSpeech(VOIC_CARD);
		}
        else{//非密钥卡
			uint8_t ucardkeyA[16]={0};
			AES_KEY aes;
			memset(&aes,0,sizeof(AES_KEY));
			AES_set_encrypt_key(SeedKeyA, 128, &aes);
            
            //CL_LOG("瞿嵌aes.rounds = %d.\n",aes.rounds);
            //PrintfData("瞿嵌Check_M1_Card", (uint8_t *)&aes.rd_key[0], 4 *(14 + 1));
            
			uint8_t card[16] ;
			memset(card,0,16);
			memcpy(card,PICC_UID,4);
			AES_encrypt(card, ucardkeyA, &aes);
            //PrintfData("瞿嵌Check_M", ucardkeyA, 16);
            
			if((TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK))
            {
				uint8_t block_data[16];
				memset((void*)block_data,0,16);
			    PrintfData("PICC_ATQA2:",PICC_ATQA,2);
                
				if(Mifare_Auth(0,2,ucardkeyA,PICC_UID) == OK)
				{
					if(Mifare_Blockread(8,block_data) == OK) 
					{
                            #if 0
                            CL_LOG("block8_data[0] = %02x ,block8_data[1] = %02x ,block8_data[2] = %02x ,block8_data[3] = %02x .\n",
                                block_data[0], block_data[1], block_data[2],block_data[3]);
                            CL_LOG("block8_data[4] = %02x ,block8_data[5] = %02x ,block8_data[6] = %02x ,block8_data[7] = %02x ,block8_data[8] = %02x .\n",
                                block_data[4],block_data[5],block_data[6],block_data[7],block_data[8]);
                            
                            CL_LOG("block8_data[9] = %02x ,block8_data[10] = %02x ,block8_data[11] = %02x ,block8_data[12] = %02x ,block8_data[13] = %02x ,block8_data[14] = %02x ,block8_data[15] = %02x .\n",
                                block_data[9],block_data[10],block_data[11],block_data[12],block_data[13],block_data[14],block_data[15]);
                            #endif
                    }
				}
                else{
					CL_LOG("Mifare_Auth error.\n");
				}
			}
		}
		TypeA_Halt();
	}
}







TickType_t _24gHeatbatTicks = 0;
TickType_t ReqCostTempTicks = 0;

void ReadCardHandle(void)
{
	if ((gChgInfo.UpperMonitorTestFlag != 0xa5) || (60 < (GetRtcCount() - gChgInfo.UpperMonitorTicks)))
	{
		gChgInfo.UpperMonitorTestFlag = 0;
		//读卡处理
		Check_M1_Card();
	}

	if (gChgInfo.ReadCardFlag == 0xa5) 
	{
		gChgInfo.ReadCardFlag = 0;
		//检查计费模板
		if ((COST_POWER != gCostTemplateInfo.cost_template.mode)
			&& (COST_UNIFY != gCostTemplateInfo.cost_template.mode))
		{
			Sc8042bSpeech(VOIC_DEVICE_EXCEPTION);
		}
		SetReadCardFlag();	  //读卡标志
		CheckCardTimeLenth();
	}
}

void TestReadCardHandle(void)
{
	if (gChgInfo.TestCardFlag == 0xa5) 
	{
	    //读卡处理
	    Check_M1_Card();
		
		if (gChgInfo.ReadCardFlag == 0xa5) 
		{
			gChgInfo.TestCardFlag = 0;
			gChgInfo.ReadCardFlag = 0;
			Sc8042bSpeech(VOIC_CARD);
			SendTestPktAck(DEBUG_CMD_CARD_TEST, (void*)gChgInfo.current_usr_card_id, sizeof(gChgInfo.current_usr_card_id));
			vTaskDelay(3000);
			return;
		}
    }
}


void SysCfgInit(void)
{
	uint8_t Local24gMacAddr[MAC_LEN];	//本地2.4mac地址
	
    //system_info.fw_version = ((uint32_t)FW_VERSION);			//((uint32_t)FW_VERSION * 10) + FW_VERSION_SUB;
    system_info.printSwitch = 1;
	system_info.card_state = 0;
	system_info.chargingFullTime = CHARGING_FULL_TIME;
	if ((MAGIC_NUM_BASE) == gGunInfo3.magic_number)
	{
		printf("second start.\n");
    }
	else
	{
		printf("first start.\n");
	}
    printf("fw_version  = %03d, subVer=%d.\n", FW_VERSION, FW_VERSION_SUB);
	printf("起始枪头号base gun id =%d.\n", system_info.baseGunSn);
    PrintfData("设备号charger sn", gSystemInfo1.station_id, sizeof(gSystemInfo1.station_id));
    PrintfData("gw mac addr", gGunInfo2._24gMacAddr, sizeof(gGunInfo2._24gMacAddr));
	Local24gMacAddr[0] = gGunInfo2.Local24gMacAddr[4];
	Local24gMacAddr[1] = gGunInfo2.Local24gMacAddr[3];
	Local24gMacAddr[2] = gGunInfo2.Local24gMacAddr[2];
	Local24gMacAddr[3] = gGunInfo2.Local24gMacAddr[1];
	Local24gMacAddr[4] = gGunInfo2.Local24gMacAddr[0];
    PrintfData("Local24gMacAddr", Local24gMacAddr, sizeof(gGunInfo2.Local24gMacAddr));
    printf("systemSize=%d, gunStatusSize=%d, gChgInfo=%d .\n",sizeof(system_info), sizeof(gun_info), sizeof(gChgInfo));
	printf("收费模式 mode =%d.\n", gCostTemplateInfo.cost_template.mode);
	printf("订单数量 rIndex = [%d], wIndex = [%d].\n", gGunInfo1.orderInfo.rIndex, gGunInfo1.orderInfo.wIndex);
}

void LoadSysCfgInfo(void)
{
	gun_info_str1 GunInfo1;
	gun_info_str2 GunInfo2;
	gun_info_str3 GunInfo3;
	cost_template_str CostTemplateInfo;
	sys_info1 SystemInfo1;

	ReadFlashInfoData(GUNINFO_PART1, (uint8_t*)&gGunInfo1, (uint8_t*)&GunInfo1 , sizeof(gGunInfo1));
	ReadFlashInfoData(GUNINFO_PART2, (uint8_t*)&gGunInfo2, (uint8_t*)&GunInfo2 , sizeof(gGunInfo2));
	ReadFlashInfoData(GUNINFO_PART3, (uint8_t*)&gGunInfo3, (uint8_t*)&GunInfo3 , sizeof(gGunInfo3));
	ReadFlashInfoData(COST_TEMPLATE_PART, (uint8_t*)&gCostTemplateInfo, (uint8_t*)&CostTemplateInfo , sizeof(gCostTemplateInfo));
	ReadFlashInfoData(SYSCONF_PART1, (uint8_t*)&gSystemInfo1, (uint8_t*)&SystemInfo1 , sizeof(gSystemInfo1));
	//PrintfData("设备号", SystemInfo1.station_id, sizeof(SystemInfo1.station_id));
	#if 1
	system_info_t systemInfo;
	ReadFlashInfoData(SYSCONF_PART, (uint8_t*)&system_info, (uint8_t*)&systemInfo , sizeof(system_info));
	#else
	system_info_t systemInfo;
	uint8_t timesFlag = 0;
	uint32_t sum[3] = {0, 1, 2};
	uint8_t i = 0;

	while(1)
	{
		for(i = 0; i < 3; i++)
		{
			memset(&systemInfo, 0, sizeof(systemInfo));
			Flash_SetPos_r(SYSCONF_PART,0);
			Flash_Read(SYSCONF_PART,(void*)&systemInfo, sizeof(systemInfo));
			sum[i] = CheckSum((uint8_t*)&systemInfo, sizeof(systemInfo));
			//PrintfData("网关地址", systemInfo._24gMacAddr, sizeof(systemInfo._24gMacAddr));
		}
		
		if((sum[0] == sum[1]) && (sum[0] == sum[2]))
		{
			memcpy(&system_info, &systemInfo, sizeof(systemInfo));
			CL_LOG("system_info 读数据成功! .\n");
			break;
		}
		else
		{
			CL_LOG("system_info 读数据失败! .\n");
		}
		if(20 < timesFlag++)
		{
			memcpy(&system_info, &systemInfo, sizeof(systemInfo));
			CL_LOG("读system_info数据失败! .\n");
			break;
		}
		osDelay(50);
        Feed_WDT();
	}
    #endif
    if ((MAGIC_NUM_BASE) == gGunInfo3.magic_number)
	{
        SysCfgInit();
    }
	else
	{
		SysCfgInit();
        memset((void*)&gGunInfo1, 0, sizeof(gGunInfo1));
		FlashWriteGunInfo_1_Data();
		//memset((void*)&gSystemInfo1, 0, sizeof(gSystemInfo1));
		//FlashWriteDataToSystemInfo1();
		memset((void*)&system_info, 0, sizeof(system_info_t));
		FlashWriteSystemInfoData();
		memset((void*)&gCostTemplateInfo, 0, sizeof(cost_template_str));
		FlashWriteCostTemplateData();
        gGunInfo3.magic_number = MAGIC_NUM_BASE;
		FlashWriteGunInfo_3_Data();
		memset(gun_info, 0, sizeof(gun_info));
		FlashWriteGunInfoData();
    }
}


//系统初始化
void System_Init(void)
{
	//枪头EMU数据初始化
	memset(&gEmuCtrl,0,sizeof(gEmuCtrl));
	
    //读取初始电量
    ReadElec();
    
	memset((void*)&SerialNumInfo, 0, sizeof(PACK_SERIAL_NUM_TYPE));
    memset((void*)&gChgInfo, 0, sizeof(gChgInfo));
    
	//加载系统配置信息
	LoadSysCfgInfo();
	
	//充电接口初始化
	Charger_Init();
    ReadUpgradeInit();
}

//硬件接口初始化
void BSP_Init(void)
{
	system_info.printSwitch = 1;
	//PMU_Init();
	EMU_Init();
	SimuartInit();
	CL_LOG("Application start.\n");
	Flash_Init();
	GunGpioInit();
	SC8042B_Init();
	Timer0_Init();
    CL_LOG("InitRTC start.\n");
	RTC_Init();
    CL_LOG("InitRTC Finish!\n");
	HT_TBSConfig(TBS_TBSCON_TMPEn, ENABLE);
}



/*****************************************************************************
** Function name:       SaveOrderReport
** Descriptions:        //保存订单记录
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
uint8_t gFlashCacheBuff[SPI_FLASH_PAGE_SIZE];
void SaveOrderReport(uint8_t gun_id)
{
    uint32_t nPage = 0;
    uint32_t currentPage = 0;
    ORDER_STR *pOrder = &gGunInfo1.orderInfo;
    ORDER_REPORT_NOTICE_BTRF2_4G_TYPE SaveChargeNoticeReq;
    gun_info_t *pGunInfo = &gun_info[gun_id-1];
    uint16_t datalen;
    
    SaveChargeNoticeReq.gun_id = gun_id + system_info.baseGunSn - 1;
    SaveChargeNoticeReq.stop_reason = pGunInfo->stopReason;                             // 结束原因
    SaveChargeNoticeReq.stopDetal = pGunInfo->reasonDetail;                             // 充电结束原因细节
	SaveChargeNoticeReq.startTime = pGunInfo->start_time;                               // 开始充电时间
	SaveChargeNoticeReq.chargingTime = pGunInfo->charger_time_plan;                     // 充电时长
	SaveChargeNoticeReq.chargingElec = pGunInfo->chargingElec;                          // 充电电量  0.01kwh
	SaveChargeNoticeReq.charger_cost = pGunInfo->money;                                 // 订单费用  分
	SaveChargeNoticeReq.chargingPower = pGunInfo->chargingPower/10;                     // 按功率段计费的功率，单位：w
	memcpy(SaveChargeNoticeReq.order, (void*)pGunInfo->order, ORDER_SECTION_LEN);
    //memset((void  *)&SaveChargeNoticeReq,gun_id,sizeof(SaveChargeNoticeReq));

	if((RECORD_MAX_BLOCK_NUM < pOrder->wIndex) || (RECORD_MAX_BLOCK_NUM < pOrder->rIndex))
    {
    	CL_LOG("wIndex=[%d] != rIndex=[%d] 错误.\n", pOrder->wIndex, pOrder->rIndex);
		pOrder->wIndex = 0;
		pOrder->rIndex = 0;
	}
	
    if (pOrder->wIndex >= pOrder->rIndex)
    {
        datalen = pOrder->wIndex - pOrder->rIndex + 1;
    }
    else
	{
        datalen = pOrder->wIndex + RECORD_MAX_BLOCK_NUM - pOrder->rIndex + 1;
    }
	
    if (RECORD_MAX_BLOCK_NUM > datalen)
    {
        nPage = pOrder->wIndex / PER_PAGE_RECORD_COUNT;           //得到写到的第几页//第pOrder->wIndex指向的订单记录还没有写数据
        currentPage = pOrder->wIndex % PER_PAGE_RECORD_COUNT;     //当前页的第currentPage条记录，0代表一个页里面的第一个订单记录
        CL_LOG("wIndex=%d != rIndex=%d, datalen = %d.\n", pOrder->wIndex, pOrder->rIndex, datalen);
        CL_LOG("nPage = %d , currentPage = %d ，PER_PAGE_RECORD_COUNT= %d\n", nPage, currentPage,PER_PAGE_RECORD_COUNT);
        PrintfData("保存SaveOrderReport", (uint8_t*)&SaveChargeNoticeReq, sizeof(SaveChargeNoticeReq));
		
        //MuxSempTake(&gChgInfo.gProtoMuxType.gBuff1024Byte);	//函数外面有调用
		{
			Flash_SetPos_r(ORDER_PART, nPage * SPI_FLASH_PAGE_SIZE);    //要读整页, 要以也首地址为偏移
	        Flash_Read(ORDER_PART, gFlashCacheBuff, SPI_FLASH_PAGE_SIZE);  //读取整页, 后面可能存的有之前写进去的数据
	        //ReadFlashInfoData(ORDER_PART, (uint8_t*)gFlashCacheBuff, (uint8_t*)gFlashCacheBuff , SPI_FLASH_PAGE_SIZE);  //读取整页, 后面可能存的有之前写进去的数据
	        //PrintfData("全局1gFlashCacheBuff", (uint8_t*)&gFlashCacheBuff, currentPage * RECORD_BLOCK_SIZE_PER_PAGE);
	        memcpy(&gFlashCacheBuff[currentPage * RECORD_BLOCK_SIZE_PER_PAGE], (void*)&SaveChargeNoticeReq, sizeof(SaveChargeNoticeReq));  //先把要写入的数据放在读出来的数据后面，待会一起写进去
	        //PrintfData("全局2gFlashCacheBuff", (uint8_t*)&gFlashCacheBuff, (currentPage + 1) * RECORD_BLOCK_SIZE_PER_PAGE);
	        while(1)
			{
				Flash_ErasePage(ORDER_PART, nPage); //擦除以ORDER_PART指向的地址后面的第nPage页, 新数据就写在这一页
		        Flash_SetPos_w(ORDER_PART, nPage * SPI_FLASH_PAGE_SIZE);    //擦除了整页, 也要以也首地址为偏移
		        if(0 == Flash_Write(ORDER_PART, &gFlashCacheBuff[0], SPI_FLASH_PAGE_SIZE))	//写整页, 后面可能存的有之前写进去的数据
				{
					break;
				}
				else
				{
					Feed_WDT();
					CL_LOG("flash[%d]写数据! \n", ORDER_PART);
					osDelay(50);
				}
			}
		}
		//MuxSempGive(&gChgInfo.gProtoMuxType.gBuff1024Byte);

        if(RECORD_MAX_BLOCK_NUM <= ++pOrder->wIndex)
        {
    		pOrder->wIndex = 0;
    	}

		FlashWriteGunInfo_1_Data();
    }
    else
	{
		CL_LOG("订单数目已超过最大值[%d]! \n", datalen);
        //记录有多少次没有保存订单
    }
    
    return;
}

int CheckOrderData(uint8_t *pAddr, uint8_t len)
{
	int returnData = 0;
	uint32_t i = 0;
	uint32_t sum = 0;
	uint32_t sum1 = 0;
	
	for(i = 0; i < len; i++)
	{
		if(0 == pAddr[i])
		{
			sum++;
		}
		if(0xff == pAddr[i])
		{
			sum1++;
		}
	}
	if((sum1 == len) || (sum == len))
	{
		returnData = 1;
	}

	return returnData;
}

/*****************************************************************************
** Function name:       SendOrderReport
** Descriptions:        //发送记录订单
** input parameters:    None
** output parameters:   None
** Returned value:	    None
** Author:              quqian
*****************************************************************************/
int SendOrderReport(void)
{
    ORDER_STR *pOrder = &gGunInfo1.orderInfo;
	ORDER_REPORT_NOTICE_BTRF2_4G_TYPE StopChargeNoticeReq;
    int returnData = 1;
	
    memset((void*)&StopChargeNoticeReq, 0, sizeof(StopChargeNoticeReq));
    MuxSempTake(&gChgInfo.gProtoMuxType.gOrderReportMux);
	if (pOrder->wIndex != pOrder->rIndex)       //缓冲区非空
    {
        CL_LOG("发送订单记录.\n");
		CL_LOG("wIndex=%d != rIndex=%d, send order.\n", pOrder->wIndex, pOrder->rIndex);	
		if(RECORD_MAX_BLOCK_NUM < pOrder->wIndex)
        {
    		pOrder->wIndex = 0;
			pOrder->rIndex = 0;
			FlashWriteGunInfo_1_Data();
            CL_LOG("发送订单超过最大值!!!\n");
            osDelay(800);
            NVIC_SystemReset();
			return 1;
    	}

        Flash_SetPos_r(ORDER_PART, pOrder->rIndex * RECORD_BLOCK_SIZE_PER_PAGE);
        Flash_Read(ORDER_PART, (void*)&StopChargeNoticeReq, sizeof(StopChargeNoticeReq));
		//ReadFlashInfoData(ORDER_PART, (uint8_t*)&StopChargeNoticeReq, (uint8_t*)&StopChargeNoticeReq, sizeof(StopChargeNoticeReq));
		if(1 == CheckOrderData((void*)&StopChargeNoticeReq, sizeof(StopChargeNoticeReq)))
		{
			if(RECORD_MAX_BLOCK_NUM <= ++pOrder->rIndex)
            {
    			pOrder->rIndex = 0;
				FlashWriteGunInfo_1_Data();
    		}
			returnData = 1;
			CL_LOG("订单号全1或全零.\n");
		}
		else
		{
			CL_LOG("发送订单枪头号 %d .\n", StopChargeNoticeReq.gun_id);
			if((StopChargeNoticeReq.gun_id != system_info.baseGunSn) && (StopChargeNoticeReq.gun_id != (system_info.baseGunSn + 1)))
			{
				pOrder->wIndex = 0;
				pOrder->rIndex = 0;
				CL_LOG("发送订单枪头号错误!!!\n");
				FlashWriteGunInfo_1_Data();
                osDelay(800);
                NVIC_SystemReset();
				return 1;
			}
			SendOrderReportNotice_BTRF2_4G(&StopChargeNoticeReq, SerialNumInfo.OrderReportNoticeSN);
			PrintfData("发送SendOrderReport", (uint8_t*)&StopChargeNoticeReq, sizeof(StopChargeNoticeReq));
        	returnData = 0;
		}
    }
    MuxSempGive(&gChgInfo.gProtoMuxType.gOrderReportMux);
    
    return returnData;
}

void Sc8042bSpeechStartChargerFail(void)
{
	if(1 == gChgInfo.isOverTimeFlag)
    {
        if (osGetTimes() - isCheckTimeOverFlag >= 25000)
        {
            MuxSempTake(&gChgInfo.gProtoMuxType.gSc8042bMux);
            CloseCheckTimeOver();
			CL_LOG("给用户提示开启充电失败 \n");
            Sc8042bSpeech(VOIC_START_CHARGER_FAIL);
            osDelay(800);
            MuxSempGive(&gChgInfo.gProtoMuxType.gSc8042bMux);
        }
    }
}

void GreenLedToggle(void)
{
	static uint32_t ledTicks = 0;
	
	//系统运行 指示灯
	if(GetRtcCount() - gChgInfo.ReceiveR6Ticks > 60)
	{
		if (GetRtcCount() - ledTicks >= 8)
		{
			ledTicks = GetRtcCount();
			GREEN_LED_Toggle(LED2);
			//CL_LOG("time:%d \n",GetRtcCount());
		}
	}
	else
	{
		if (osGetTimes() - ledTicks >= 500)
		{
			ledTicks = osGetTimes();
			GREEN_LED_Toggle(LED2);
			//CL_LOG("time:%d \n",GetRtcCount());
		}
	}
}

void RestoreGunWithChargingCtrlProc(void)
{
	static uint8_t sendState = 0;
	static uint32_t chargerTicks = 0;
	//static uint32_t cTicks1 = 0;
	uint32_t nowTicks = 0;
	
	nowTicks = GetRtcCount();	
	
	//if((1 == gChgInfo.heartReceiveFlag))	//跟R6或手机对时后才可用
	{
		if(0 == sendState)
		{
			{	//还是要R6链接上服务器后再发送心跳包打开 1 == gChgInfo.heartReceiveFlag
				//恢复充电接口
				RestoreGunStatus();
				sendState = 1;
			}
		}
		
		//充电处理 2s调用一次
		if(nowTicks - chargerTicks >= TIME_OF_SECONDS) 
		{
			chargerTicks = nowTicks;
			ChargingCtrlProc();
			ChargingProc();
		}
	}
}


//主任务
void vMainTask(void* argc)
{
    TickType_t CheckChargerTicksTest = 0;
    uint8_t gun_id = 0;
    uint32_t FuseTicks = 0;
	uint8_t timesFlag = 0;
//	uint32_t CheckStopChargerTicks = 0;
	
    osDelay(500);
    BSP_Init();
    CL_LOG("BSP_Init OK.\n");
	System_Init();
	CL_LOG("System_Init OK.\n");
	while(1)
	{
		if(FM175XX_Init() == CL_OK)
	    {
			system_info.card_state = 0;
			CL_LOG("FM175XX_Init OK.\n");
			break;
		}
	    else
	    {
	    	timesFlag++;
			system_info.card_state = 1;
			CL_LOG("FM175XX_Init error.\n");
			if(50 <= timesFlag)
			{
				break;
			}
		}
	}
	
	//语音提示
	PlayVoice(2);       //欢迎使用
    osDelay(900);
    PlayVoice(4);       //共享充电
    osDelay(900);

	gChgInfo.gunCharging[0].isRebootMCU = 0xae;
	gChgInfo.gunCharging[1].isRebootMCU = 0xae;
	#if 1
	//蓝牙2.4G通信
    xTaskCreate((TaskFunction_t)CK24_UMTask,"CK24_UM",356,NULL,2,NULL);
	#endif
	
	#if 1
    //刷卡鉴权
	xTaskCreate((TaskFunction_t)CardAuthTask,"CardAuthTask",384,NULL,1,NULL);
    #endif
	
	#if 1
	//上位机通信任务
	xTaskCreate((TaskFunction_t)ComTask,"ComTask",384,NULL,1,NULL);
	#endif
	
	FuseTicks = GetRtcCount();
	while (1)
    {
		osDelay(200);
        Feed_WDT();
		#if 1
		//掉电检测
		if((HT_PMU->PMUSTA&0x08) == 0)
		{
			printf("[进入Enter_StandBy, vMainTask]\n");
			Enter_StandBy();
		}
        #endif
		SendStartChargingNotice();

        #if 1
        if (osGetTimes() - CheckChargerTicksTest >= 20000)
        {
            GUN_STATUS_STR GunStatus;
            for(gun_id = 1; gun_id <= GUN_NUM_MAX; gun_id++)
            {
                GetGunStatus(gun_id, &GunStatus);
                CL_LOG("枪头号 gun_id = %d.\n",gun_id + system_info.baseGunSn - 1);
                CL_LOG("充电电流 = %.3f A.\n", ((float)GunStatus.current) / 1000);
                CL_LOG("充电电压 = %.3f V.\n", ((float)GunStatus.voltage) /10);
                CL_LOG("充电功率 = %.3f W.\n", ((float)GunStatus.power) / 10);
				//CL_LOG("is_load_on[0] = %d, is_load_on[1] = %d.\n", gun_info[0].is_load_on, gun_info[1].is_load_on);
            }
            CheckChargerTicksTest = osGetTimes();
        }
        #endif
		
		if(GetRtcCount() - FuseTicks > 30)
		{
			FuseTicks = GetRtcCount();
			FuseholderStatusProc();
		}
	}
}


