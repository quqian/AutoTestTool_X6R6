
#include "sc8042b.h"
#include "delay.h"
#include "system_info.h"



int PlayVoice(unsigned char cnt)
{
//    int count = 0;
//    //判忙  2s超时
//    while(READ_AU_BUSY() != 0){
//        Delay_mSec(10);
//        if(count++ >200){
//            return -1;
//        }
//    }
    unsigned char i = 0;

    AU_RST_EN();
    Delay_100uSec(1);
    AU_RST_DIS();
    Delay_100uSec(1);

    for(i = 0;i < cnt; i++){
        Feed_WDT();
        AU_DATA_HIHG();
        Delay_100uSec(1);
        AU_DATA_LOW();
        Delay_100uSec(1);
    }
	
    return 0;
}


//提示操作失败及具体代码
void OptFailNotice(int code)
{
    uint8_t codeVal[3];

    Feed_WDT();
    CL_LOG("error code=%03d.\n",code);
    codeVal[0] = code / 100 % 10;
    codeVal[1] = code / 10 % 10;
    codeVal[2] = code / 1 % 10;
    Sc8042bSpeech(codeVal[0]+31);
    OS_DELAY_MS(250);
    Sc8042bSpeech(codeVal[1]+31);
    OS_DELAY_MS(250);
    Sc8042bSpeech(codeVal[2]+31);
    OS_DELAY_MS(250);
}


//提示操作成功及具体代码
void OptSuccessNotice(int code)
{
    uint8_t codeVal[3];

    Feed_WDT();
    CL_LOG("success code=%03d.\n",code);
    codeVal[0] = code / 100 % 10;
    codeVal[1] = code / 10 % 10;
    codeVal[2] = code / 1 % 10;
    Sc8042bSpeech(VOIC_SUCCESS);
    OS_DELAY_MS(1000);
    Sc8042bSpeech(codeVal[0]+31);
    OS_DELAY_MS(250);
    Sc8042bSpeech(codeVal[1]+31);
    OS_DELAY_MS(250);
    Sc8042bSpeech(codeVal[2]+31);
    OS_DELAY_MS(250);
}


void TempNotice(uint16_t temp)
{
    uint8_t codeVal[3];
    Feed_WDT();
    CL_LOG("temp=%d.\n",temp);
    codeVal[0] = temp / 100 % 10;
    codeVal[1] = temp / 10 % 10;
    codeVal[2] = temp / 1 % 10;
    Sc8042bSpeech(VOIC_TEM);
    OS_DELAY_MS(1000);
    Sc8042bSpeech(codeVal[0]+31);
    OS_DELAY_MS(250);
    Sc8042bSpeech(codeVal[1]+31);
    OS_DELAY_MS(250);
    Sc8042bSpeech(codeVal[2]+31);
    Sc8042bSpeech(VOIC_DEGREE);
    OS_DELAY_MS(250);
}


void CardBalanceSpeech(uint32_t balance)
{
	uint8_t tmp=0;

	if (balance>99999) {
		return;
	}
	//Sc8042bSpeech(VOIC_CARD_BANLANCE);//卡片余额
	//OS_DELAY_MS(1000);
	tmp = balance / 10000;
	if (tmp != 0)
	{
		Sc8042bSpeech(tmp+31);
		OS_DELAY_MS(250);
		Sc8042bSpeech(VOIC_HUNDRED);//百
		OS_DELAY_MS(250);
		tmp = (balance/1000)%10;
		if (tmp != 0)
        {
			Sc8042bSpeech(tmp+31);
			OS_DELAY_MS(250);
			Sc8042bSpeech(VOIC_TEN);//十
			OS_DELAY_MS(250);
			tmp = (balance/100)%10;
			if(tmp != 0)
			{
				Sc8042bSpeech(tmp+31);
				OS_DELAY_MS(250);
			}
		} else {
			tmp = (balance/100)%10;
			if(tmp != 0) {
				Sc8042bSpeech(31);
				OS_DELAY_MS(250);
				Sc8042bSpeech(tmp+31);
				OS_DELAY_MS(250);
			}
		}

		tmp = (balance/10)%10;
		if (tmp!=0) {
			Sc8042bSpeech(VOIC_POINT);//点
			OS_DELAY_MS(250);
			Sc8042bSpeech(tmp+31);
			OS_DELAY_MS(250);
		}
		Sc8042bSpeech(VOIC_YUAN);//元
	} else {
		tmp = balance/1000;
		if (tmp != 0) {
			if (tmp != 1) {
				Sc8042bSpeech(tmp+31);
				OS_DELAY_MS(250);
			}
			Sc8042bSpeech(VOIC_TEN);
			OS_DELAY_MS(250);
			tmp = (balance/100)%10;
			if(tmp != 0) {
				Sc8042bSpeech(tmp+31);
				OS_DELAY_MS(250);
			}
			tmp = (balance/10)%10;
			if (tmp!=0) {
				Sc8042bSpeech(VOIC_POINT);
				OS_DELAY_MS(250);
				Sc8042bSpeech(tmp+31);
				OS_DELAY_MS(250);
			}
			Sc8042bSpeech(VOIC_YUAN);
		} else {
			tmp = balance/100;
			Sc8042bSpeech(tmp+31);
			OS_DELAY_MS(250);
			tmp = (balance/10)%10;
			if (tmp!=0) {
				Sc8042bSpeech(VOIC_POINT);
				OS_DELAY_MS(250);
				Sc8042bSpeech(tmp+31);
				OS_DELAY_MS(250);
			}
			Sc8042bSpeech(VOIC_YUAN);
		}
	}
}

void SpeechCardBalance(uint32_t balance)
{
	uint8_t tmp=0;
	
	if (balance>999999) {
		return;
	}
	
	Sc8042bSpeech(VOIC_CARD_BANLANCE);//卡片余额
	OS_DELAY_MS(1000);
	tmp = balance / 100000;
	if(tmp != 0)
	{
		Sc8042bSpeech(tmp+31);
		OS_DELAY_MS(250);
		Sc8042bSpeech(VOIC_THOUSAND);//千
		OS_DELAY_MS(250);
		tmp = (balance / 10000)%10;
		if(tmp != 0)
		{
			Sc8042bSpeech(tmp+31);
			OS_DELAY_MS(250);
			Sc8042bSpeech(VOIC_HUNDRED);//百
			OS_DELAY_MS(250);
			tmp = (balance/1000)%10;
			if (tmp != 0)
			{
				Sc8042bSpeech(tmp+31);
				OS_DELAY_MS(250);
				Sc8042bSpeech(VOIC_TEN);//十
				OS_DELAY_MS(250);
				tmp = (balance/100)%10;
				if(tmp != 0)
				{
					Sc8042bSpeech(tmp+31);
					OS_DELAY_MS(250);
				}
			} 
			else
			{
				tmp = (balance/100)%10;
				if(tmp != 0)
				{
					Sc8042bSpeech(VOIC_0);
					OS_DELAY_MS(250);
//					Sc8042bSpeech(31);
//					OS_DELAY_MS(250);
					Sc8042bSpeech(tmp+31);
					OS_DELAY_MS(250);
				}
			}

			tmp = (balance/10)%10;
			if (tmp!=0)
			{
				Sc8042bSpeech(VOIC_POINT);//点
				OS_DELAY_MS(250);
				Sc8042bSpeech(tmp+31);
				OS_DELAY_MS(250);
			}
			Sc8042bSpeech(VOIC_YUAN);//元
		} 
		else {
			tmp = (balance % 100000)/1000;
			if (tmp != 0)
			{
				Sc8042bSpeech(VOIC_0);
				OS_DELAY_MS(250);
				Sc8042bSpeech(tmp+31);
				OS_DELAY_MS(250);
				Sc8042bSpeech(VOIC_TEN);	//十
				OS_DELAY_MS(250);
				tmp = (balance/100)%10;
				if(tmp != 0)
				{
					Sc8042bSpeech(tmp+31);
					OS_DELAY_MS(250);
				}
				tmp = (balance/10)%10;
				if (tmp!=0)
				{
					Sc8042bSpeech(VOIC_POINT);
					OS_DELAY_MS(250);
					Sc8042bSpeech(tmp+31);
					OS_DELAY_MS(250);
				}
				Sc8042bSpeech(VOIC_YUAN);
			} 
			else
			{
				tmp = (balance % 100000)/100;
				if (tmp != 0)
				{
					Sc8042bSpeech(VOIC_0);
					OS_DELAY_MS(250);
					Sc8042bSpeech(tmp+31);
					OS_DELAY_MS(250);
				}
				tmp = (balance/10)%10;
				if (tmp != 0)
				{
					Sc8042bSpeech(VOIC_POINT);
					OS_DELAY_MS(250);
					Sc8042bSpeech(tmp+31);
					OS_DELAY_MS(250);
				}
				Sc8042bSpeech(VOIC_YUAN);
			}
		}
	}
	else{
		CardBalanceSpeech(balance);
	}
}
//Sc8042bSpeech(VOIC_0);


void SpeechChargeing(uint8_t id)
{
	uint8_t tmp=0;
	tmp = id / 10;
	if (tmp==0) {
		tmp = id%10;
		Sc8042bSpeech(tmp+31);
		OS_DELAY_MS(250);
	} else {
		Sc8042bSpeech(VOIC_TEN);
		OS_DELAY_MS(250);
		tmp = id % 10;
		if (tmp != 0) {
			Sc8042bSpeech(tmp+31);
			OS_DELAY_MS(250);
		}
	}
	Sc8042bSpeech(VOIC_HAO_CHAZUO);
	OS_DELAY_MS(800);
	Sc8042bSpeech(VOIC_START_CHARGING);
	OS_DELAY_MS(1000);
	Sc8042bSpeech(VOIC_STOP_CHARGER_TIP);
}


void SC8042B_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//power
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOOUT;
	GPIO_InitStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_Init(HT_GPIOA,&GPIO_InitStruct);
	
    //PA8--AU_DATA
	memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOOUT;
	GPIO_InitStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_Init(HT_GPIOA,&GPIO_InitStruct);
	
    //PD2--AU_RST
    memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOOUT;
	GPIO_InitStruct.GPIO_OutputStruct = GPIO_Output_PP;
	HT_GPIO_Init(HT_GPIOD,&GPIO_InitStruct);

   //PD3--AU_BUSY
    memset(&GPIO_InitStruct,0,sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IOIN;
	GPIO_InitStruct.GPIO_InputStruct = GPIO_Input_Floating;
	HT_GPIO_Init(HT_GPIOD,&GPIO_InitStruct);

   
    AU_POWER_EN();
    AU_DATA_LOW();
    AU_RST_DIS();
}


