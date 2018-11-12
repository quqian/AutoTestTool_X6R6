/*
*********************************************************************************************************
*                                              HT6XXX
*                                          Library Function
*
*                                   Copyright 2013, Hi-Trend Tech, Corp.
*                                        All Rights Reserved
*
*
* Project      : HT6xxx
* File         : ht6xxx_flash.h
* By           : Hitrendtech_SocTeam
* Version      : V1.0.4
* Description  :
**********************************************************************************************************
*/

#ifndef __HT6XXX_FLASH_H__
#define __HT6XXX_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ht6xxx.h"

/*
*********************************************************************************************************
*                                           全局宏/结构体
*********************************************************************************************************
*/
#define   FLASH_EWR_KEY         (FLASH_EWR_KEY_4 | FLASH_EWR_KEY_3 | FLASH_EWR_KEY_2 | FLASH_EWR_KEY_1)
#define   FLASH_EWR_KEY_1       (0x000000B4UL)
#define   FLASH_EWR_KEY_1_AREA  (0x000000FFUL)
#define   FLASH_EWR_KEY_2       (0x0000C300UL)
#define   FLASH_EWR_KEY_2_AREA  (0x0000FF00UL)
#define   FLASH_EWR_KEY_3       (0x00A50000UL)
#define   FLASH_EWR_KEY_3_AREA  (0x00FF0000UL)
#define   FLASH_EWR_KEY_4       (0x96000000UL)
#define   FLASH_EWR_KEY_4_AREA  (0xFF000000UL)

typedef enum
{
    FlashSucc    = 1,                               /*!< success status                       */
    FlashNone    = 0,                               /*!< no error status                      */
    FlashErrKey  = -1,                              /*!< input key error status               */
    FlashErrAddr = -2,                              /*!< input address error status           */
    FlashErrMod  = -3,                              /*!< input work method error status       */
    FlashErrNum  = -4,                              /*!< input number of buffer status        */
    FlashErrBuf  = -5                               /*!< input address of buffer status       */
}FLASH_RetStaTypeDef;                               /*!< end of group FLASH_RetStaTypeDef     */

typedef enum
{
    FlashErase   = 0x3C,                            /*!< erase flash                          */
    FlashWrite   = 0x5A,                            /*!< write flash                          */
    FlashRead    = 0x69,                            /*!< read  flash                          */
}FLASH_WorkMethodTypeDef;                           /*!< end of group FLASH_WorkMethodTypeDef */

/*
* @brief  flash H256K and L256K block
*/
#if  defined  HT6x3x
typedef enum
{
    FlashBlock1   = 0,                           /*!< First  256k byte                 */
    FlashBlock2   = 1,                           /*!< Second 256k byte                 */
}FLASH_BlockEraseTypeDef;                        /*!< end of group FLASH_BlockEraseTypeDef      */
#endif

#if  defined  HT501x || defined  HT502x || defined  HT6x1x || defined  HT6x2x || defined  HT6x3x

#define FLASH_MIN_ADDRESS       (HT_FLASH_BASE)                  /*!<常量FLASH_MIN_ADDRESS暂未被使用，如需要可自行添加 */
#define FLASH_MAX_ADDRESS       (HT_FLASH_BASE+HT_FLASH_SIZE)
#define INFO_MIN_ADDRESS        (HT_INFO_BASE)
#define INFO_MAX_ADDRESS        (HT_INFO_BASE+HT_INFO_SIZE)
#define BUF_MIN_ADDRESS         (HT_RAM_BASE)
#define BUF_MAX_ADDRESS         (HT_RAM_BASE+HT_RAM_SIZE)
#else
#define FLASH_MIN_ADDRESS       (0x00000000)
#define FLASH_MAX_ADDRESS       (0x00000000)
#define INFO_MIN_ADDRESS        (0x00000000)
#define INFO_MAX_ADDRESS        (0x00000000)
#define BUF_MIN_ADDRESS         (0x00000000)
#define BUF_MAX_ADDRESS         (0x00000000)
#endif

/*
*********************************************************************************************************
*                                             全局变量
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           全局函数申明
*********************************************************************************************************
*/

void HT_Flash_ByteWrite(const uint8_t* pWriteByte, uint32_t Address, uint32_t Num);
void HT_Flash_HalfWordWrite(const uint16_t* pWriteHalfWord, uint32_t Address, uint32_t Num);
void HT_Flash_WordWrite(const uint32_t* pWriteWord, uint32_t Address, uint32_t Num);
void HT_Flash_ByteRead(uint8_t* pReadByte, uint32_t Address, uint32_t Num);
void HT_Flash_HalfWordRead(uint16_t* pReadHalfWord, uint32_t Address, uint32_t Num);
void HT_Flash_WordRead(uint32_t* pReadWord, uint32_t Address, uint32_t Num);
void HT_Flash_PageErase(uint32_t EraseAddress);
void HT_Flash_ChipErase(void);
#if defined  HT6x3x
void HT_Flash_BlockChipErase(FLASH_BlockEraseTypeDef EraseBlock);
#endif
#if defined  HT6x3x  ||  defined  HT502x
void HT_Flash_PageEraseWithSectionUnlock(uint32_t EraseAddress);
#endif
#if defined  HT6x1x  ||  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT502x
FLASH_RetStaTypeDef HT_Flash_EWR_Pro(uint32_t Address, FLASH_WorkMethodTypeDef Method, uint8_t* Buff, uint32_t Num, uint32_t Key);
#endif

void HT_Info_ByteWrite(const uint8_t* pWriteByte, uint32_t Address, uint32_t Num);
void HT_Info_HalfWordWrite(const uint16_t* pWriteHalfWord, uint32_t Address, uint32_t Num);
void HT_Info_WordWrite(const uint32_t* pWriteWord, uint32_t Address, uint32_t Num);
void HT_Info_ByteRead(uint8_t* pReadByte, uint32_t Address, uint32_t Num);
void HT_Info_HalfWordRead(uint16_t* pReadHalfWord, uint32_t Address, uint32_t Num);
void HT_Info_WordRead(uint32_t* pReadWord, uint32_t Address, uint32_t Num);
void HT_Info_PageErase(uint32_t EraseAddress);
#if defined  HT6x1x  ||  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT502x
FLASH_RetStaTypeDef HT_Info_EWR_Pro(uint32_t Address, FLASH_WorkMethodTypeDef Method, uint8_t* pBuff, uint32_t Num, uint32_t Key);
#endif


#ifdef __cplusplus
}
#endif

#endif /* __HT60xx_FLASH_H__ */
