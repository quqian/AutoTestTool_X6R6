#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ht6xxx_lib.h"
#include "cmsis_os.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int  uint32_t;
typedef unsigned long long uint64_t;

typedef unsigned int time_t;

#include "rtc.h"

#define FW_VERSION                     (6)
#define FW_VERSION_SUB                  3


/*
2.8   	1. 红色LED：常亮表示已开启充电/正常充电中，熄灭表示无充电或充电已完成;绿色LED：快闪烁(1s/次)表示设备运行正常，常亮表示1分钟内没有收到R6的数据包就8s闪烁一次；
		2. 合并2.4g和串口升级的Upgrade_Write函数
		3. 增加欠压过压检测，增加电流检测，单路最大电流允许为6A，双路最大允许电流为10A(这个10A体现在双路功率最大为2200W)
		4. 以用户刷卡开始计时，若X6设备25s内未收到“R6发送的开启充电(鉴权成功）指令”，则默认本次开启充电失败，并播报“开启充电失败”语音
		5. 增加保险丝熔断检测与事件上报
		6. 每路插头独立的计量，终端将充电中的插头的电压、电流、功率及累计用电量等发送R6
		7. 计费模板增加起步金计费
		8. 增加DFI蓝牙升级功能
		9. 充满自停判断，实时充电功率小于30w并且在45分钟之内，功率变化小于3w，同时满足则判断充电已完成，停止充电
		10. MCU每20分钟保存当前充电状态插头的“已充电时间”，当停电恢复来电后，继续充电，剩余充电时间=总充电时间-已充电时间，停电时间不计入充电时间
		11. 修改枪头拔出检测，“枪头拔出”检测判断时间持续5分钟：充电过程中检测到实时充电功率持续1分钟小于1.5w，则关闭继电器1分钟，1分钟后打开继电器持续充电1分钟；以上关闭/开启继电器操作共循环2次：
    		(1)该5分钟之内，开启继电器期间只要检测到实时充电功率大于1.5w，则判断枪头未拔出，继续充电；
    		(2)该5分钟之内，开启继电器期间只要检测到实时充电功率小于1.5w，则判断枪头拔出。
		12. 修改R6给X6升级单个升级功能
		
2.7   	1. 修改R6不给X6升级的bug

2.6     1. 修改X6扫码一直显示离线的问题，但由于2.4g通信比较慢，一般停止充电后2分钟左右显示和实际对不上属于正常现象

2.5     1. 增加电压过压和欠压检测，增加电流过流检测
		2. 将报开启充电失败时间由40s改为28s

2.4     1.修改R6给X6发送后台鉴权后, X6没有开启充电的问题
		2.修改卡片余额一千，语音播报3百多的问题
2.3     1.修改RTC复位不通过的问题
1.9     1.修改X6给R6发送停止充电请求，然后X6没有及时收到，然后X6收到心跳包又发送了一遍停止充电通知，这时R6就又发送了一遍停止充电应答，就造成 rIndex 加了两次的错误，改正方法是判断是不是同一个序列号
        2.修改X6给R6发送开启充电通知，然后X6没有及时收到，然后X6又发送了一遍开启充电通知，这时R6就又发送了一遍开启充电通知应答，就造成打印了两遍清除开启充电通知标志，改正方法是判断是不是同一个序列号
        3.修改刷卡后没有在没有开启充电成功的情况下，不播报 请插入几号插孔 也不播报的开启充电失败的问题
        
1.8     1.修改心跳同步R6服务器的时间，如果R6的时间为 0则不更新时间 , 也不置位恢复充电接口标志位 , 等R6 同步了服务器时间后再同步时间 置位恢复充电接口标志位 
        2.修改刷卡逻辑, 如果是同一张卡，刷卡时没有移开，那么等待15秒会再刷一次 (这样做的目的就是为了防止一个用户在刷卡的时候没来的及将卡移开而没必要的开启第二个插口)
        3.修改刷卡逻辑, 如果是第二张卡 , 也就是不同的第二张卡来刷卡(可以认为是第二用户 , 但第二个用户不想等待刷卡时间) , 这里改为在前面一张卡充电过程完成后是可以立即刷卡的，不用再等待15s
        4.修改刷卡逻辑, 如果检测到3秒钟没有都到卡，表示前面的用户已经刷卡完毕, 这时再刷卡的话 , 就不用管是不是同一张卡 , 只要是卡就可以刷 , 这是表示用户想开启另外一个插口
        5.刷卡逻辑改成500ms读一次卡，只要卡放在上面就能读到
        6.回复给R6的心跳包增加 固件版本 号字段
        
1.7     1.修改刷卡后播报语音 请将插头插入n号插座 被 刷卡鉴权里面的 卡片无效语音枪播
        2.修改从R6同步过来的时间
        3.添加刷卡鉴权20s服务器没有应答，就播报开启充电失败
        
1.6     1.缩小外层数据包和内层数据包data区域的size
        2.修改刷卡鉴权数据发送方式，改为发送RESENT_24G_MSG_CNT (40)次，每4s发一次，在这4s内每200ms检测一次数据有没有成功发送,如果发送成功就break退出
        3.添加订单保存发送功能，在结束充电控制里面保存，在心跳里面检测发送
        4.修改刷卡语音播报不全的错误
        5.添加刷卡鉴权12s服务器没有应答，就播报开启充电失败
        6.修改刷卡成功后15s内不能再次刷卡，防止用户在开启一个枪口的时候没有将卡马上拿走而开启第二个枪口
        7.重新分配flash保存数据地址
        
1.2     1.添加串口升级功能
        2.添加扫码充电功能
        3.优化刷卡充电功能
        4.修改RTC定时器初始化和get时间函数与R6的时间匹配
000     初始版本
*/


#define ICCID_LEN                       20
#define TX_FAIL_MAX_CNT                 5

#define SystemFsysClock                 19660800

#define EnWr_WPREG()		            HT_CMU->WPREG = 0xA55A			//则关闭写保护功能，用户可以写操作被保护的寄存器
#define DisWr_WPREG()		            HT_CMU->WPREG = 0x0000			//则开启写保护功能，用户禁止写操作被保护的寄存器

#define	Feed_WDT()			            HT_WDT->WDTCLR = 0xAA3E
#define	Feed_WDT16S()					HT_WDT->WDTCLR = 0xAAFF			//FeedWDT per 16s


#define Enable_Int()		__enable_irq()					//ê1?ü?D??
#define Disable_Int()		__disable_irq()					//???1?D??

#define Goto_Sleep()		{SCB->SCR = 0x0004;__WFI();__NOP();}		//??è?Sleep?￡ê?
#define Goto_Hold()			{__WFI();__NOP();}		//??è?Sleep?￡ê?
#define Reset_CPU()			NVIC_SystemReset()



#define setbit(x,y) x|=(1<<y)           //将X的第Y位置1
#define clrbit(x,y) x&=~(1<<y)          //将X的第Y位清0
#define revbit(x,y) x^=(1<<y)           //将X的第Y位反转


#define THIS_LINE                       __LINE__
#define CL_LINE                         THIS_LINE
#define CL_FUNC                         __func__
#define CL_OK                           0
#define CL_FAIL                         (-1)
#define CL_TRUE                         1
#define CL_FALSE                        0


#define MAGIC_NUM_BASE                  0xcd3456ab
#define STATION_MACHINE_TYPE            "DEVICE_CHARGER_CL_XA"
#define REGISTER_CODE					"928698494847"

//TYPE -- 1:x9；2:x10；3:x6  4:x6P
#define CHARGER_TYPE                    3  
#define CHARGER_NAME					"X6"



#define GUN_NUM_MAX				        2
#define ORDER_SECTION_LEN               10
#define CHARGER_SN_LEN			        8   //bcd的长度
#define EVEN_DISCRI_LEN                 32




#define osGetTimes()                    (xTaskGetTickCount()*portTICK_PERIOD_MS)
#define OS_DELAY_MS                     osDelay
//#define GetRtcCount 	                GetTimeStamp /*(xTaskGetTickCount()*portTICK_PERIOD_MS/1000)*/

//extern char* GetCurrentTime(void);

#define CL_LOG(fmt,args...) do {    \
    printf("[%s][%s]:%s (%d) "fmt,GetCurrentTime(),CHARGER_NAME,__func__, __LINE__, ##args); \
}while(0)


#endif


