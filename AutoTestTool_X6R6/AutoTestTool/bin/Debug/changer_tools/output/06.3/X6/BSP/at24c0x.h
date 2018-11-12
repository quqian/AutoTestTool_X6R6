
#ifndef __AT24C04x_H__
#define __AT24C04x_H__


#include "includes.h"

#define         E2PROM                  64

#if E2PROM==32
	#define     EE_PAGE_SIZE            32
	#define     EE_PAGE_BIT             5
	#define     EE_SIZE                 0x0FFF
#elif E2PROM==64
	#define     EE_PAGE_SIZE            32
	#define     EE_PAGE_BIT             5
	#define     EE_SIZE                 0x1FFF
#elif E2PROM==128
	#define     EE_PAGE_SIZE            64
	#define     EE_PAGE_BIT             6
	#define     EE_SIZE                 0x3FFF
#elif E2PROM==256
	#define     EE_PAGE_SIZE            64
	#define     EE_PAGE_BIT             6
	#define     EE_SIZE                 0x7FFF
#elif E2PROM==512
	#define     EE_PAGE_SIZE            128
	#define     EE_PAGE_BIT             7
	#define     EE_SIZE                 0xFFFF
#endif

#define   E2PROM_WRITE                  0xA0
#define   E2PROM_READ                   0xA1
#define   WRITE_EE                      1
#define   CLR_EE				        0




typedef struct{
    unsigned char page_one_num;
    unsigned char pages;
    unsigned char page_last_num;
}I2C_NUM_DEF;

#define SET_SDA_IN()     (HT_GPIOE->PTDIR &= 0xFFF7)
#define SET_SDA_OUT()    (HT_GPIOE->PTDIR |= 0x0008)
#define SET_SCL_OUT()    (HT_GPIOE->PTDIR |= 0x0010)
#define SET_WP_OUT()     (HT_GPIOE->PTDIR |= 0x0020)

#define SDA_OUT_0()      (HT_GPIOE->PTDAT &= 0xFFF7)
#define SDA_OUT_1()      (HT_GPIOE->PTDAT |= 0x0008)
#define SCL_OUT_0()      (HT_GPIOE->PTDAT &= 0xFFEF)
#define SCL_OUT_1()      (HT_GPIOE->PTDAT |= 0x0010)
#define WP_OUT_0()       (HT_GPIOE->PTDAT &= 0xFFDF)
#define WP_OUT_1()       (HT_GPIOE->PTDAT |= 0x0020)

#define SDA              (HT_GPIOE->PTDAT & 0x0008)


extern void AT24C0x_Init(void);
extern uint8_t EEPROM_Write(uint8_t *source, uint16_t ee_addr, uint16_t write_num);
extern uint8_t EEPROM_Read(uint8_t *source, uint16_t ee_addr, uint16_t nums);
int FlashWriteSysInfo(void *pSysInfo, uint16_t size, uint8_t readBack);
int FlashReadSysInfo(void *pInfo, uint16_t size);
int FlashWriteGunInfo(void *pGunInfo, uint16_t size, uint8_t readBack);
int FlashReadGunInfo(void *pGunInfo, uint16_t size);

#endif


