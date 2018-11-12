/*
*********************************************************************************************************
*                                              HT6XXX
*                                           Library Function
*
*                                   Copyright 2013, Hi-Trend Tech, Corp.
*                                        All Rights Reserved
*
*
* Project      : HT6xxx
* File         : ht6xxx_flash.c
* By           : Hitrendtech_SocTeam
* Version      : V1.0.4
* Description  :
*********************************************************************************************************
*/

#define  __HT6XXX_FLASH_C

#include "ht6xxx_flash.h"

/*
*********************************************************************************************************
*                                           本地宏/结构体
*********************************************************************************************************
*/
#define M8(adr)     (*((volatile uint8_t *)  (adr)))
#define M16(adr)    (*((volatile uint16_t *) (adr)))
#define M32(adr)    (*((volatile uint32_t *) (adr)))

static const uint32_t RegisterWriteProtect[]={CMU_WPREG_Protected, CMU_WPREG_UnProtected};

/*
*********************************************************************************************************
*                                             本地变量
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           本地函数申明
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FLASH BYTE WRITE
*
* 函数说明: Flash字节写
*
* 入口参数: pWriteByte    指向被写数据的首地址
*
*           Address       数据将要写到Flash的地址(请用户确保地址合法性以及加上数据总长度后的最后一个地址)
*
*           Num           写数据长度，以字节为单位
*
* 返回参数: 无
*
* 特殊说明: 用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*********************************************************************************************************
*/
#if defined ( __ICCARM__ )
void HT_Flash_ByteWrite(const uint8_t* pWriteByte, uint32_t Address, uint32_t Num)
{
asm(
    "PUSH   {R4-R6}\n"
    "LDR    R6, IAR_FLASH_BWR_WPREG_UL_VAL\n"
    "MOVS   R5, #1\n"
    "LDR    R3, IAR_FLASH_BWR_WPREG_REG_ADDR\n"
    "LDR    R4, [R3]\n"
    "ANDS   R4, R4, R5\n"
    "BEQ    IAR_FLASH_BWR_CONFIG_REG_SET\n"
    "MOVS   R4, R6\n"
"IAR_FLASH_BWR_CONFIG_REG_SET:\n"
    "STR    R6, [R3]\n"
    "LDR    R6, IAR_FLASH_BWR_CONFIG_UL_VAL\n"
    "STR    R6, [R3, #0x38]\n"
    "STR    R5, [R3, #0x34]\n"
    "CMP    R2, #0\n"
    "BEQ    IAR_FLASH_BWR_END\n"
"IAR_FLASH_BWR_LOOP:\n"
    "LDRB   R5, [R0]\n"
    "NOP\n"
    "STRB   R5, [R1]\n"
"IAR_FLASH_BWR_FLAG_CHK_LOOP:\n"
    "LDR    R5, [R3, #0x34]\n"
    "LSLS   R5, R5, #29\n"
    "BMI    IAR_FLASH_BWR_FLAG_CHK_LOOP\n"
    "ADDS   R0, R0, #1\n"
    "ADDS   R1, R1, #1\n"
    "SUBS   R2, R2, #1\n"
    "BNE    IAR_FLASH_BWR_LOOP\n"
"IAR_FLASH_BWR_END:\n"
    "MOVS   R0, #0\n"
    "STR    R0, [R3, #0x34]\n"
    "STR    R0, [R3, #0x38]\n"
    "STR    R4, [R3]\n"
    "POP    {R4-R6}\n"
    "BX     LR\n"
"IAR_FLASH_BWR_WPREG_REG_ADDR: DC32    0x4000F000\n"
"IAR_FLASH_BWR_WPREG_UL_VAL:   DC32    0x0000A55A\n"
"IAR_FLASH_BWR_CONFIG_UL_VAL:  DC32    0x00007A68\n"
      );
}

#elif defined ( __CC_ARM )
__ASM void HT_Flash_ByteWrite(const uint8_t* pWriteByte, uint32_t Address, uint32_t Num)
{
    PUSH    {R4-R6}
    LDR     R6, |KEIL_FLASH_BWR_WPREG_UL_VAL|
    MOVS    R5, #1
    LDR     R3, |KEIL_FLASH_BWR_WPREG_REG_ADDR|
    LDR     R4, [R3]
    ANDS    R4, R4, R5
    BEQ     KEIL_FLASH_BWR_CONFIG_REG_SET
    MOVS    R4, R6
KEIL_FLASH_BWR_CONFIG_REG_SET
    STR     R6, [R3]
    LDR     R6, |KEIL_FLASH_BWR_CONFIG_UL_VAL|
    STR     R6, [R3, #0x38]
    STR     R5, [R3, #0x34]
    CMP     R2, #0
    BEQ     KEIL_FLASH_BWR_END
KEIL_FLASH_BWR_LOOP
    LDRB    R5, [R0]
    NOP
    STRB    R5, [R1]
KEIL_FLASH_BWR_FLAG_CHK_LOOP
    LDR     R5, [R3, #0x34]
    LSLS    R5, R5, #29
    BMI     KEIL_FLASH_BWR_FLAG_CHK_LOOP
    ADDS    R0, R0, #1
    ADDS    R1, R1, #1
    SUBS    R2, R2, #1
    BNE     KEIL_FLASH_BWR_LOOP
KEIL_FLASH_BWR_END
    MOVS    R0, #0
    STR     R0, [R3, #0x34]
    STR     R0, [R3, #0x38]
    STR     R4, [R3]
    POP     {R4-R6}
    BX      LR
|KEIL_FLASH_BWR_WPREG_REG_ADDR| DCD    0x4000F000
|KEIL_FLASH_BWR_WPREG_UL_VAL|   DCD    0x0000A55A
|KEIL_FLASH_BWR_CONFIG_UL_VAL|  DCD    0x00007A68
}
#endif

/*
*********************************************************************************************************
*                                         FLASH BYTE READ
*
* 函数说明: Flash字节读
*
* 入口参数: pReadByte     指向存储读到的数据的首地址
*
*           Address       读Flash的首地址
*
*           Num           读数据长度，以字节为单位
*
* 返回参数: 无
*
* 特殊说明: 用户需注意读的数据不要超出保存的范围，同时访问超出flash有效范围时不返回数据
*********************************************************************************************************
*/
void HT_Flash_ByteRead(uint8_t* pReadByte, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */
    uint32_t i;
    uint32_t writeProtect;

    if (Address >= FLASH_MAX_ADDRESS)
    {
        return;
    }

    writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];             /*!< 保存当前写保护状态     */
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

#if  defined  CMU_FLASHCON2
    HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                            /*!< Flash读                */
#else
    HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                             /*!< Flash读                */
#endif

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */

    for(i=0; i<Num; i++)
    {
        if ((Address+i) >= FLASH_MAX_ADDRESS)
        {
            return;
        }
        pReadByte[i] = M8(Address+i);                                      /*!< 执行Flash读            */
    }
}

/*
*********************************************************************************************************
*                                         FLASH HALF WORD WRITE
*
* 函数说明: Flash半字写
*
* 入口参数: pWriteHalfWord    指向被写数据的首地址
*
*           Address           数据将要写到Flash的地址(请用户确保地址合法性以及加上数据总长度后的最后一个地址)
*
*           Num               写数据长度，以半字为单位
*
* 返回参数: 无
*
* 特殊说明: 1)用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*           2)用户应保证传递给函数的地址为至少半字对齐
*********************************************************************************************************
*/
#if defined ( __ICCARM__ )
void HT_Flash_HalfWordWrite(const uint16_t* pWriteHalfWord, uint32_t Address, uint32_t Num)
{
asm(
    "PUSH   {R4-R6}\n"
    "LDR    R6, IAR_FLASH_HWWR_WPREG_UL_VAL\n"
    "MOVS   R5, #1\n"
    "LDR    R3, IAR_FLASH_HWWR_WPREG_REG_ADDR\n"
    "LDR    R4, [R3]\n"
    "ANDS   R4, R4, R5\n"
    "BEQ    IAR_FLASH_HWWR_CONFIG_REG_SET\n"
    "MOVS   R4, R6\n"
"IAR_FLASH_HWWR_CONFIG_REG_SET:\n"
    "STR    R6, [R3]\n"
    "LDR    R6, IAR_FLASH_HWWR_CONFIG_UL_VAL\n"
    "STR    R6, [R3, #0x38]\n"
    "STR    R5, [R3, #0x34]\n"
    "CMP    R2, #0\n"
    "BEQ    IAR_FLASH_HWWR_END\n"
    "MOVS   R5, #1\n"
    "BICS   R1, R1, R5\n"
"IAR_FLASH_HWWR_LOOP:\n"
    "LDRH   R5, [R0]\n"
    "NOP\n"
    "STRH   R5, [R1]\n"
"IAR_FLASH_HWWR_FLAG_CHK_LOOP:\n"
    "LDR    R5, [R3, #0x34]\n"
    "LSLS   R5, R5, #29\n"
    "BMI    IAR_FLASH_HWWR_FLAG_CHK_LOOP\n"
    "ADDS   R0, R0, #2\n"
    "ADDS   R1, R1, #2\n"
    "SUBS   R2, R2, #1\n"
    "BNE    IAR_FLASH_HWWR_LOOP\n"
"IAR_FLASH_HWWR_END:\n"
    "MOVS   R0, #0\n"
    "STR    R0, [R3, #0x34]\n"
    "STR    R0, [R3, #0x38]\n"
    "STR    R4, [R3]\n"
    "POP    {R4-R6}\n"
    "BX     LR\n"
"IAR_FLASH_HWWR_WPREG_REG_ADDR: DC32    0x4000F000\n"
"IAR_FLASH_HWWR_WPREG_UL_VAL:   DC32    0x0000A55A\n"
"IAR_FLASH_HWWR_CONFIG_UL_VAL:  DC32    0x00007A68\n"
      );
}

#elif defined ( __CC_ARM )
__ASM void HT_Flash_HalfWordWrite(const uint16_t* pWriteHalfWord, uint32_t Address, uint32_t Num)
{
    PUSH    {R4-R6}
    LDR     R6, |KEIL_FLASH_HWWR_WPREG_UL_VAL|
    MOVS    R5, #1
    LDR     R3, |KEIL_FLASH_HWWR_WPREG_REG_ADDR|
    LDR     R4, [R3]
    ANDS    R4, R4, R5
    BEQ     KEIL_FLASH_HWWR_CONFIG_REG_SET
    MOVS    R4, R6
KEIL_FLASH_HWWR_CONFIG_REG_SET
    STR     R6, [R3]
    LDR     R6, |KEIL_FLASH_HWWR_CONFIG_UL_VAL|
    STR     R6, [R3, #0x38]
    STR     R5, [R3, #0x34]
    CMP     R2, #0
    BEQ     KEIL_FLASH_HWWR_END
    MOVS    R5, #1
    BICS    R1, R1, R5
KEIL_FLASH_HWWR_LOOP
    LDRH    R5, [R0]
    NOP
    STRH    R5, [R1]
KEIL_FLASH_HWWR_FLAG_CHK_LOOP
    LDR     R5, [R3, #0x34]
    LSLS    R5, R5, #29
    BMI     KEIL_FLASH_HWWR_FLAG_CHK_LOOP
    ADDS    R0, R0, #2
    ADDS    R1, R1, #2
    SUBS    R2, R2, #1
    BNE     KEIL_FLASH_HWWR_LOOP
KEIL_FLASH_HWWR_END
    MOVS    R0, #0
    STR     R0, [R3, #0x34]
    STR     R0, [R3, #0x38]
    STR     R4, [R3]
    POP     {R4-R6}
    BX      LR
|KEIL_FLASH_HWWR_WPREG_REG_ADDR| DCD    0x4000F000
|KEIL_FLASH_HWWR_WPREG_UL_VAL|   DCD    0x0000A55A
|KEIL_FLASH_HWWR_CONFIG_UL_VAL|  DCD    0x00007A68
}
#endif

/*
*********************************************************************************************************
*                                         FLASH HALF WORD READ
*
* 函数说明: Flash半字读
*
* 入口参数: pReadHalfWord 指向存储读到的数据的首地址
*
*           Address       读Flash的首地址
*
*           Num           读数据长度，以半字为单位
*
* 返回参数: 无
*
* 特殊说明: 用户需注意读的数据不要超出保存的范围，同时访问超出flash有效范围时不返回数据
*********************************************************************************************************
*/
void HT_Flash_HalfWordRead(uint16_t* pReadHalfWord, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */
    uint32_t i;
    uint32_t writeProtect;

    if (Address >= FLASH_MAX_ADDRESS)
    {
        return;
    }

    writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];             /*!< 保存当前写保护状态     */
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

#if  defined  CMU_FLASHCON2
    HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                            /*!< Flash读                */
#else
    HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                             /*!< Flash读                */
#endif

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */

    Address &= 0xFFFFFFFE;                                                 /*!< 保证半字对齐           */
    for(i=0; i<Num; i++)
    {
        if ((Address+i*2) >= FLASH_MAX_ADDRESS)
        {
            return;
        }
        pReadHalfWord[i] = M16(Address+i*2);                               /*!< 执行Flash读            */
    }
}

/*
*********************************************************************************************************
*                                         FLASH WORD WRITE
*
* 函数说明: Flash字写
*
* 入口参数: pWriteWord    指向被写数据的首地址
*
*           Address       数据将要写到Flash的地址(请用户确保地址合法性以及加上数据总长度后的最后一个地址)
*
*           Num           写数据长度，以字为单位
*
* 返回参数: 无
*
* 特殊说明: 1)用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*           2)用户应保证传递给函数的地址为至少字对齐
*********************************************************************************************************
*/
#if defined ( __ICCARM__ )
void HT_Flash_WordWrite(const uint32_t* pWriteWord, uint32_t Address, uint32_t Num)
{
asm(
    "PUSH   {R4-R6}\n"
    "LDR    R6, IAR_FLASH_WWR_WPREG_UL_VAL\n"
    "MOVS   R5, #1\n"
    "LDR    R3, IAR_FLASH_WWR_WPREG_REG_ADDR\n"
    "LDR    R4, [R3]\n"
    "ANDS   R4, R4, R5\n"
    "BEQ    IAR_FLASH_WWR_CONFIG_REG_SET\n"
    "MOVS   R4, R6\n"
"IAR_FLASH_WWR_CONFIG_REG_SET:\n"
    "STR    R6, [R3]\n"
    "LDR    R6, IAR_FLASH_WWR_CONFIG_UL_VAL\n"
    "STR    R6, [R3, #0x38]\n"
    "STR    R5, [R3, #0x34]\n"
    "CMP    R2, #0\n"
    "BEQ    IAR_FLASH_WWR_END\n"
    "MOVS   R5, #3\n"
    "BICS   R1, R1, R5\n"
"IAR_FLASH_WWR_LOOP:\n"
    "LDR    R5, [R0]\n"
    "NOP\n"
    "STR    R5, [R1]\n"
"IAR_FLASH_WWR_FLAG_CHK_LOOP:\n"
    "LDR    R5, [R3, #0x34]\n"
    "LSLS   R5, R5, #29\n"
    "BMI    IAR_FLASH_WWR_FLAG_CHK_LOOP\n"
    "ADDS   R0, R0, #4\n"
    "ADDS   R1, R1, #4\n"
    "SUBS   R2, R2, #1\n"
    "BNE    IAR_FLASH_WWR_LOOP\n"
"IAR_FLASH_WWR_END:\n"
    "MOVS   R0, #0\n"
    "STR    R0, [R3, #0x34]\n"
    "STR    R0, [R3, #0x38]\n"
    "STR    R4, [R3]\n"
    "POP    {R4-R6}\n"
    "BX     LR\n"
"IAR_FLASH_WWR_WPREG_REG_ADDR: DC32    0x4000F000\n"
"IAR_FLASH_WWR_WPREG_UL_VAL:   DC32    0x0000A55A\n"
"IAR_FLASH_WWR_CONFIG_UL_VAL:  DC32    0x00007A68\n"
      );
}

#elif defined ( __CC_ARM )
__ASM void HT_Flash_WordWrite(const uint32_t* pWriteWord, uint32_t Address, uint32_t Num)
{
    PUSH    {R4-R6}
    LDR     R6, |KEIL_FLASH_WWR_WPREG_UL_VAL|
    MOVS    R5, #1
    LDR     R3, |KEIL_FLASH_WWR_WPREG_REG_ADDR|
    LDR     R4, [R3]
    ANDS    R4, R4, R5
    BEQ     KEIL_FLASH_WWR_CONFIG_REG_SET
    MOVS    R4, R6
KEIL_FLASH_WWR_CONFIG_REG_SET
    STR     R6, [R3]
    LDR     R6, |KEIL_FLASH_WWR_CONFIG_UL_VAL|
    STR     R6, [R3, #0x38]
    STR     R5, [R3, #0x34]
    CMP     R2, #0
    BEQ     KEIL_FLASH_WWR_END
    MOVS    R5, #3
    BICS    R1, R1, R5
KEIL_FLASH_WWR_LOOP
    LDR     R5, [R0]
    NOP
    STR     R5, [R1]
KEIL_FLASH_WWR_FLAG_CHK_LOOP
    LDR     R5, [R3, #0x34]
    LSLS    R5, R5, #29
    BMI     KEIL_FLASH_WWR_FLAG_CHK_LOOP
    ADDS    R0, R0, #4
    ADDS    R1, R1, #4
    SUBS    R2, R2, #1
    BNE     KEIL_FLASH_WWR_LOOP
KEIL_FLASH_WWR_END
    MOVS    R0, #0
    STR     R0, [R3, #0x34]
    STR     R0, [R3, #0x38]
    STR     R4, [R3]
    POP     {R4-R6}
    BX      LR
|KEIL_FLASH_WWR_WPREG_REG_ADDR| DCD    0x4000F000
|KEIL_FLASH_WWR_WPREG_UL_VAL|   DCD    0x0000A55A
|KEIL_FLASH_WWR_CONFIG_UL_VAL|  DCD    0x00007A68
}
#endif

/*
*********************************************************************************************************
*                                         FLASH WORD READ
*
* 函数说明: Flash字读
*
* 入口参数: pReadWord     指向存储读到的数据的首地址
*
*           Address       读Flash的首地址
*
*           Num           读数据长度，以字为单位
*
* 返回参数: 无
*
* 特殊说明: 用户需注意读的数据不要超出保存的范围，同时访问超出Flash有效范围时不返回数据
*********************************************************************************************************
*/
void HT_Flash_WordRead(uint32_t* pReadWord, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */
    uint32_t i;
    uint32_t writeProtect;

    if (Address >= FLASH_MAX_ADDRESS)
    {
        return;
    }

    writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];             /*!< 保存当前写保护状态     */
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

#if  defined  CMU_FLASHCON2
    HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                            /*!< Flash读                */
#else
    HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                             /*!< Flash读                */
#endif

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */

    Address &= 0xFFFFFFFC;                                                 /*!< 保证字对齐             */
    for(i=0; i<Num; i++)
    {
        if ((Address+i*4) >= FLASH_MAX_ADDRESS)
        {
            return;
        }
        pReadWord[i] = M32(Address+i*4);                                   /*!< 执行Flash读            */
    }
}

/*
*********************************************************************************************************
*                                         FLASH PAGE ERASE
*
* 函数说明: Flash页擦除
*
* 入口参数: EraseAddress    页擦除地址(请用户确保地址合法性)
*
* 返回参数: 无
*
* 特殊说明: 用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*          HT501x 总共128k bytes - 1k bytes/page
*          HT502x 总共256k bytes - 1k bytes/page
*          HT6x1x 总共128k bytes - 1k bytes/page
*          HT6x2x 总共256k bytes - 1k bytes/page
*          HT6x3x 总共512k bytes - 1k bytes/page
*********************************************************************************************************
*/
#if defined ( __ICCARM__ )
void HT_Flash_PageErase(uint32_t EraseAddress)
{
asm(
    "PUSH   {R4}\n"
    "LDR    R3, IAR_FLASH_PGES_WPREG_UL_VAL\n"
    "LDR    R1, IAR_FLASH_PGES_WPREG_REG_ADDR\n"
    "LDR    R2, [R1]\n"
    "LSLS   R2, R2, #31\n"
    "BPL    IAR_FLASH_PGES_CLEAR\n"
    "MOVS   R2, R3\n"
    "B      IAR_FLASH_PGES_CONFIG_REG_SET\n"
"IAR_FLASH_PGES_CLEAR:\n"
    "MOVS   R2, #0\n"
"IAR_FLASH_PGES_CONFIG_REG_SET:\n"
    "STR    R3, [R1]\n"
    "LDR    R3, IAR_FLASH_PGES_CONFIG_UL_VAL\n"
    "STR    R3, [R1, #0x38]\n"
    "MOVS   R3, #2\n"
    "STR    R3, [R1, #0x34]\n"
    "MOVS   R3, #255\n"
    "MOVS   R4, #3\n"
    "BICS   R0, R0, R4\n"
    "NOP\n"
    "STR    R3, [R0]\n"
"IAR_FLASH_PGES_FLAG_CHK_LOOP:\n"
    "LDR    R0, [R1, #0x34]\n"
    "LSLS   R0, R0, #29\n"
    "BMI    IAR_FLASH_PGES_FLAG_CHK_LOOP\n"
    "MOVS   R0, #0\n"
    "STR    R0, [R1, #0x34]\n"
    "STR    R0, [R1, #0x38]\n"
    "STR    R2, [R1]\n"
    "POP    {R4}\n"
    "BX     LR\n"
"IAR_FLASH_PGES_WPREG_REG_ADDR: DC32    0x4000f000\n"
"IAR_FLASH_PGES_WPREG_UL_VAL:   DC32    0x0000A55A\n"
"IAR_FLASH_PGES_CONFIG_UL_VAL:  DC32    0x00007A68\n"
  );
}

#elif defined ( __CC_ARM )
__ASM void HT_Flash_PageErase(uint32_t EraseAddress)
{
    PUSH    {R4}
    LDR     R3, |KEIL_FLASH_PGES_WPREG_UL_VAL|
    LDR     R1, |KEIL_FLASH_PGES_WPREG_REG_ADDR|
    LDR     R2, [R1]
    LSLS    R2, R2, #31
    BPL     KEIL_FLASH_PGES_CLEAR
    MOVS    R2, R3
    B       KEIL_FLASH_PGES_CONFIG_REG_SET
KEIL_FLASH_PGES_CLEAR
    MOVS    R2, #0
KEIL_FLASH_PGES_CONFIG_REG_SET
    STR     R3, [R1]
    LDR     R3, |KEIL_FLASH_PGES_CONFIG_UL_VAL|
    STR     R3, [R1, #0x38]
    MOVS    R3, #2
    STR     R3, [R1, #0x34]
    MOVS    R3, #255
    MOVS    R4, #3
    BICS    R0, R0, R4
    NOP
    STR     R3, [R0]
KEIL_FLASH_PGES_FLAG_CHK_LOOP
    LDR     R0, [R1, #0x34]
    LSLS    R0, R0, #29
    BMI     KEIL_FLASH_PGES_FLAG_CHK_LOOP
    MOVS    R0, #0
    STR     R0, [R1, #0x34]
    STR     R0, [R1, #0x38]
    STR     R2, [R1]
    POP     {R4}
    BX      LR
|KEIL_FLASH_PGES_WPREG_REG_ADDR| DCD    0x4000F000
|KEIL_FLASH_PGES_WPREG_UL_VAL|   DCD    0x0000A55A
|KEIL_FLASH_PGES_CONFIG_UL_VAL|  DCD    0x00007A68
}
#endif

/*
*********************************************************************************************************
*                                         FLASH CHIP ERASE
*
* 函数说明: Flash全擦除
*
* 入口参数: 无
*
* 返回参数: 无
*
* 特殊说明: 用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*********************************************************************************************************
*/
void HT_Flash_ChipErase(void)
{
    /*  assert_param  */
    uint32_t writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];    /*!< 保存当前写保护状态     */
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /*!< Flash解锁              */
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_CHIPERASE;                         /*!< Flash擦除              */

    M32(0x00000000) = 0xFF;                                                /*!< 执行Flash全擦除        */
    while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                           /*!< 等待写完成             */

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash读                */
    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /*!< Flash锁定              */

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */
}

#if defined  HT6x3x
/*
*********************************************************************************************************
*                                         FLASH BLOCK CHIP ERASE
*
* 函数说明: Flash 256K擦除 (for HT6x3x)
*
* 入口参数: EraseBlock    块擦除区域
*
* 返回参数: 无
*
* 特殊说明: 用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*********************************************************************************************************
*/
void HT_Flash_BlockChipErase(FLASH_BlockEraseTypeDef EraseBlock)
{
    /*  assert_param  */
    uint32_t writeProtect;

    if (EraseBlock > FlashBlock2)
    {
        return;
    }

    writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];             /*!< 保存当前写保护状态     */
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

    if (EraseBlock == FlashBlock1)
    {
        HT_CMU->FLASHLOCK_L256 = CMU_FLASHLOCK_L256_UnLocked;              /*!< Flash解锁              */
    }
    else
    {
        HT_CMU->FLASHLOCK_H256 = CMU_FLASHLOCK_H256_UnLocked;              /*!< Flash解锁              */
    }
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_CHIPERASE;                         /*!< Flash擦除              */

    if (EraseBlock == FlashBlock1)
    {
        M32(0x00000000) = 0xFF;                                            /*!< 执行Flash 256k擦除     */
    }
    else
    {
        M32(0x00040000) = 0xFF;                                            /*!< 执行Flash 256k擦除     */
    }
    while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                           /*!< 等待写完成             */

    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_READ;                              /*!< Flash读                */
    if (EraseBlock == FlashBlock1)
    {
        HT_CMU->FLASHLOCK_L256 = CMU_FLASHLOCK_L256_Locked;                /*!< Flash锁定              */
    }
    else
    {
        HT_CMU->FLASHLOCK_H256 = CMU_FLASHLOCK_H256_Locked;                /*!< Flash锁定              */
    }

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */
}
#endif

/*
*********************************************************************************************************
*                                         FLASH PAGE ERASE WITH SECTION UNLOCK
*
* 函数说明: Flash页擦除(仅Section解锁)    (for HT6x3x, HT502x)
*
* 入口参数: EraseAddress    页擦除地址
*
* 返回参数: 无
*
* 特殊说明: 用户应保证函数执行过程中寄存器写保护状态以及Flash解锁状态不变
*********************************************************************************************************
*/
#if defined  HT6x3x
void HT_Flash_PageEraseWithSectionUnlock(uint32_t EraseAddress)
{
    /*  assert_param  */
    uint32_t writeProtect;

    if (EraseAddress >= FLASH_MAX_ADDRESS)
    {
        return;
    }

    writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];              /*!< 保存当前写保护状态     */
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                  /*!< 关闭写保护功能         */

    if (EraseAddress <= 0x00001FFF )                                        /*!< Flash解锁              */
    {
        HT_CMU->FSA1LOCK = CMU_FSA1LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00002000) && (EraseAddress <= 0x00007FFF ))
    {
        HT_CMU->FSA2LOCK = CMU_FSA2LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00008000) && (EraseAddress <= 0x0000FFFF ))
    {
        HT_CMU->FSA3LOCK = CMU_FSA3LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00010000) && (EraseAddress <= 0x00017FFF ))
    {
        HT_CMU->FSA4LOCK = CMU_FSA4LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00018000) && (EraseAddress <= 0x0001FFFF ))
    {
        HT_CMU->FSA5LOCK = CMU_FSA5LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00020000) && (EraseAddress <= 0x00027FFF ))
    {
        HT_CMU->FSA6LOCK = CMU_FSA6LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00028000) && (EraseAddress <= 0x0002FFFF ))
    {
        HT_CMU->FSA7LOCK = CMU_FSA7LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00030000) && (EraseAddress <= 0x00037FFF ))
    {
        HT_CMU->FSA8LOCK = CMU_FSA8LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00038000) && (EraseAddress <= 0x0003FBFF ))
    {
        HT_CMU->FSA9LOCK = CMU_FSA9LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x0003FC00) && (EraseAddress <= 0x0003FFFF ))
    {
        HT_CMU->FSAALOCK = CMU_FSAALOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00040000) && (EraseAddress <= 0x00041FFF ))
    {
        HT_CMU->FSB1LOCK = CMU_FSB1LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00042000) && (EraseAddress <= 0x00047FFF ))
    {
        HT_CMU->FSB2LOCK = CMU_FSB2LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00048000) && (EraseAddress <= 0x0004FFFF ))
    {
        HT_CMU->FSB3LOCK = CMU_FSB3LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00050000) && (EraseAddress <= 0x00057FFF ))
    {
        HT_CMU->FSB4LOCK = CMU_FSB4LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00058000) && (EraseAddress <= 0x0005FFFF ))
    {
        HT_CMU->FSB5LOCK = CMU_FSB5LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00060000) && (EraseAddress <= 0x00067FFF ))
    {
        HT_CMU->FSB6LOCK = CMU_FSB6LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00068000) && (EraseAddress <= 0x0006FFFF ))
    {
        HT_CMU->FSB7LOCK = CMU_FSB7LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00070000) && (EraseAddress <= 0x00077FFF ))
    {
        HT_CMU->FSB8LOCK = CMU_FSB8LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00078000) && (EraseAddress <= 0x0007FBFF ))
    {
        HT_CMU->FSB9LOCK = CMU_FSB9LOCK_UnLocked;
    }
    else
    {
        HT_CMU->FSBBLOCK = CMU_FSBBLOCK_UnLocked;
    }

#if  defined  CMU_FLASHCON2
    HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_PAGEERASE;                       /*!< Flash页擦除            */
#else
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_PAGEERASE;                         /*!< Flash页擦除            */
#endif

    EraseAddress &= 0xFFFFFFFC;                                            /*!< 保证字对齐             */
    M32(EraseAddress) = 0xFF;                                              /*!< 执行Flash页擦除        */
    while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                           /*!< 等待写完成             */

#if  defined  CMU_FLASHCON2
    HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                            /*!< Flash读                */
#else
    HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                             /*!< Flash读                */
#endif

    if (EraseAddress <= 0x00001FFF )                                       /*!< Flash解锁              */
    {
        HT_CMU->FSA1LOCK = CMU_FSA1LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00002000) && (EraseAddress <= 0x00007FFF ))
    {
        HT_CMU->FSA2LOCK = CMU_FSA2LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00008000) && (EraseAddress <= 0x0000FFFF ))
    {
        HT_CMU->FSA3LOCK = CMU_FSA3LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00010000) && (EraseAddress <= 0x00017FFF ))
    {
        HT_CMU->FSA4LOCK = CMU_FSA4LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00018000) && (EraseAddress <= 0x0001FFFF ))
    {
        HT_CMU->FSA5LOCK = CMU_FSA5LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00020000) && (EraseAddress <= 0x00027FFF ))
    {
        HT_CMU->FSA6LOCK = CMU_FSA6LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00028000) && (EraseAddress <= 0x0002FFFF ))
    {
        HT_CMU->FSA7LOCK = CMU_FSA7LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00030000) && (EraseAddress <= 0x00037FFF ))
    {
        HT_CMU->FSA8LOCK = CMU_FSA8LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00038000) && (EraseAddress <= 0x0003FBFF ))
    {
        HT_CMU->FSA9LOCK = CMU_FSA9LOCK_Locked;
    }
    else if ((EraseAddress >= 0x0003FC00) && (EraseAddress <= 0x0003FFFF ))
    {
        HT_CMU->FSAALOCK = CMU_FSAALOCK_Locked;
    }
    else if ((EraseAddress >= 0x00040000) && (EraseAddress <= 0x00041FFF ))
    {
        HT_CMU->FSB1LOCK = CMU_FSB1LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00042000) && (EraseAddress <= 0x00047FFF ))
    {
        HT_CMU->FSB2LOCK = CMU_FSB2LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00048000) && (EraseAddress <= 0x0004FFFF ))
    {
        HT_CMU->FSB3LOCK = CMU_FSB3LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00050000) && (EraseAddress <= 0x00057FFF ))
    {
        HT_CMU->FSB4LOCK = CMU_FSB4LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00058000) && (EraseAddress <= 0x0005FFFF ))
    {
        HT_CMU->FSB5LOCK = CMU_FSB5LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00060000) && (EraseAddress <= 0x00067FFF ))
    {
        HT_CMU->FSB6LOCK = CMU_FSB6LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00068000) && (EraseAddress <= 0x0006FFFF ))
    {
        HT_CMU->FSB7LOCK = CMU_FSB7LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00070000) && (EraseAddress <= 0x00077FFF ))
    {
        HT_CMU->FSB8LOCK = CMU_FSB8LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00078000) && (EraseAddress <= 0x0007FBFF ))
    {
        HT_CMU->FSB9LOCK = CMU_FSB9LOCK_Locked;
    }
    else
    {
        HT_CMU->FSBBLOCK = CMU_FSBBLOCK_Locked;
    }

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */
}

#elif  defined  HT502x
void HT_Flash_PageEraseWithSectionUnlock(uint32_t EraseAddress)
{
    /*  assert_param  */
    uint32_t writeProtect;

    if (EraseAddress >= FLASH_MAX_ADDRESS)
    {
        return;
    }

    writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];              /*!< 保存当前写保护状态     */
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                  /*!< 关闭写保护功能         */

    HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                               /*!< Flash锁定              */
    if (EraseAddress <= 0x00001FFF )                                        /*!< Flash解锁              */
    {
        HT_CMU->FS1LOCK = CMU_FS1LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00002000) && (EraseAddress <= 0x00007FFF ))
    {
        HT_CMU->FS2LOCK = CMU_FS2LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00008000) && (EraseAddress <= 0x0000FFFF ))
    {
        HT_CMU->FS3LOCK = CMU_FS3LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00010000) && (EraseAddress <= 0x00017FFF ))
    {
        HT_CMU->FS4LOCK = CMU_FS4LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00018000) && (EraseAddress <= 0x0001FBFF ))
    {
        HT_CMU->FS5LOCK = CMU_FS5LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x0001FC00) && (EraseAddress <= 0x0001FFFF ))
    {
        HT_CMU->FSALOCK = CMU_FSALOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00020000) && (EraseAddress <= 0x00027FFF ))
    {
        HT_CMU->FS6LOCK = CMU_FS6LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00028000) && (EraseAddress <= 0x0002FFFF ))
    {
        HT_CMU->FS7LOCK = CMU_FS7LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00030000) && (EraseAddress <= 0x00037FFF ))
    {
        HT_CMU->FS8LOCK = CMU_FS8LOCK_UnLocked;
    }
    else if ((EraseAddress >= 0x00038000) && (EraseAddress <= 0x0003FBFF ))
    {
        HT_CMU->FS9LOCK = CMU_FS9LOCK_UnLocked;
    }
    else
    {
        HT_CMU->FSBLOCK = CMU_FSBLOCK_UnLocked;
    }

#if  defined  CMU_FLASHCON2
    HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_PAGEERASE;                       /*!< Flash页擦除            */
#else
    HT_CMU->FLASHCON = CMU_FLASHCON_FOP_PAGEERASE;                         /*!< Flash页擦除            */
#endif

    EraseAddress &= 0xFFFFFFFC;                                            /*!< 保证字对齐             */
    M32(EraseAddress) = 0xFF;                                              /*!< 执行Flash页擦除        */
    while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                           /*!< 等待写完成             */

#if  defined  CMU_FLASHCON2
    HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                            /*!< Flash读                */
#else
    HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                             /*!< Flash读                */
#endif

    if (EraseAddress <= 0x00001FFF )                                       /*!< Flash锁定              */
    {
        HT_CMU->FS1LOCK = CMU_FS1LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00002000) && (EraseAddress <= 0x00007FFF ))
    {
        HT_CMU->FS2LOCK = CMU_FS2LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00008000) && (EraseAddress <= 0x0000FFFF ))
    {
        HT_CMU->FS3LOCK = CMU_FS3LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00010000) && (EraseAddress <= 0x00017FFF ))
    {
        HT_CMU->FS4LOCK = CMU_FS4LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00018000) && (EraseAddress <= 0x0001FBFF ))
    {
        HT_CMU->FS5LOCK = CMU_FS5LOCK_Locked;
    }
    else if ((EraseAddress >= 0x0001FC00) && (EraseAddress <= 0x0001FFFF ))
    {
        HT_CMU->FSALOCK = CMU_FSALOCK_Locked;
    }
    else if ((EraseAddress >= 0x00020000) && (EraseAddress <= 0x00027FFF ))
    {
        HT_CMU->FS6LOCK = CMU_FS6LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00028000) && (EraseAddress <= 0x0002FFFF ))
    {
        HT_CMU->FS7LOCK = CMU_FS7LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00030000) && (EraseAddress <= 0x00037FFF ))
    {
        HT_CMU->FS8LOCK = CMU_FS8LOCK_Locked;
    }
    else if ((EraseAddress >= 0x00038000) && (EraseAddress <= 0x0003FBFF ))
    {
        HT_CMU->FS9LOCK = CMU_FS9LOCK_Locked;
    }
    else
    {
        HT_CMU->FSBLOCK = CMU_FSBLOCK_Locked;
    }

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */
}
#endif

#if defined  HT6x1x  ||  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT502x
/*
*********************************************************************************************************
*                                    FLASH BYTE ERASE/WRITE/READ
*
* 函数说明: flash擦/写/读
*
* 入口参数: Address       数据将要写到Flash的地址(请用户确保地址合法性以及加上数据总长度后的最后一个地址)
*
*           Method        Flash操作方式
*                          FlashErase    FLASH擦操作
*                          FlashWrite    FLASH写操作
*                          FlashRead     FLASH读操作
*
*           pBuff         指向存储数据的缓存的首地址(擦操作无效)
*
*           Num           写数据长度，以字节为单位(擦操作无效)
*
*           Key           执行操作的检查控制字
*                          FLASH_EWR_KEY  软件确认操作码
*
* 返回参数: RetSta        该函数操作结果
*                          FlashSucc      FLASH操作成功
*                          FlashNone      FLASH无操作  
*                          FlashErrKey    用户输入操作码有误
*                          FlashErrAddr   用户输入FLASH地址信息有误
*                          FlashErrMod    用户输入FLASH操作方式有误
*                          FlashErrNum    用户输入buffer的操作数据范围有误
*                          FlashErrBuf    用户输入buffer的地址有误
*
* 特殊说明: 
*********************************************************************************************************
*/
FLASH_RetStaTypeDef HT_Flash_EWR_Pro (uint32_t Address, FLASH_WorkMethodTypeDef Method, uint8_t* pBuff, uint32_t Num, uint32_t Key)
{
    uint32_t writeProtect, i;
    FLASH_RetStaTypeDef RetSta = FlashNone;

    if ((Key & FLASH_EWR_KEY_2_AREA) == FLASH_EWR_KEY_2)                                       /* 检查第一级软件关键字是否合法 */
    {
        Key &= (~FLASH_EWR_KEY_2_AREA);                                                        /* 清除当前流程操作字          */

        if (Address < FLASH_MAX_ADDRESS)                                                       /* 检查操作地址非法性          */
        {
            switch (Method)                                                                    /* 检查操作地址非法性          */
            {
                case FlashErase:
                    if ((Key & FLASH_EWR_KEY_1_AREA) == FLASH_EWR_KEY_1)                       /* 检查第二级软件关键字是否合法 */
                    {
                        Key &= (~FLASH_EWR_KEY_1_AREA);

                        writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];             /* 保存当前写保护状态        */
                        HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /* 关闭写保护功能            */
                        HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /* Flash解锁                */
#if  defined  CMU_FLASHCON2
                        HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_PAGEERASE;                       /* Flash页擦除              */
#else
                        HT_CMU->FLASHCON = CMU_FLASHCON_FOP_PAGEERASE;                         /* Flash页擦除              */
#endif
                        HT_CMU->WPREG = CMU_FLASHLOCK_Locked;                                  /* 恢复之前写保护设置        */

                        if (((Key & FLASH_EWR_KEY_3_AREA) == FLASH_EWR_KEY_3) 
                          && ((Key & FLASH_EWR_KEY_4_AREA) == FLASH_EWR_KEY_4))                /* 检查第三级软件关键字是否合法 */
                        {
                            Key &= (~(FLASH_EWR_KEY_3_AREA | FLASH_EWR_KEY_4_AREA));           /* 清除当前流程操作字          */
                            M8(Address) = 0xFF;                                                /* 执行Flash页擦除           */
                            while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                       /* 等待写完成                */
                            RetSta = FlashSucc;
                        } 
                        else
                        {
                            RetSta = FlashErrKey;
                        } 

                        HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /* 关闭写保护功能            */
#if  defined  CMU_FLASHCON2
                        HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                            /* Flash读                  */
#else
                        HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                             /* Flash读                  */
#endif
                        HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /* Flash锁定                */
                        HT_CMU->WPREG = writeProtect;                                          /* 恢复之前写保护设置        */
                    }
                    else
                    {
                        RetSta = FlashErrKey;
                    }
                    break;
                case FlashWrite:
                    if ((Key & FLASH_EWR_KEY_3_AREA) == FLASH_EWR_KEY_3) 
                    {
                        Key &= (~FLASH_EWR_KEY_3_AREA);                                        /* 清除当前流程操作字         */

                        if ((Address + Num - 1) < FLASH_MAX_ADDRESS) 
                        {
                            if (((uint32_t)pBuff >= BUF_MIN_ADDRESS) && ((uint32_t)pBuff < BUF_MAX_ADDRESS))
                            {
                                writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];     /* 保存当前写保护状态        */
                                HT_CMU->WPREG = CMU_WPREG_UnProtected;                         /* 关闭写保护功能            */
                                HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                    /* Flash解锁                */
#if  defined  CMU_FLASHCON2
                                HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_WRITE;                   /* Flash写                  */
#else
                                HT_CMU->FLASHCON = CMU_FLASHCON_FOP_WRITE;                     /* Flash写                  */
#endif
                                HT_CMU->WPREG = CMU_FLASHLOCK_Locked;                          /* 恢复之前写保护设置        */

                                if (((Key & FLASH_EWR_KEY_1_AREA) == FLASH_EWR_KEY_1) 
                                  && ((Key & FLASH_EWR_KEY_4_AREA) == FLASH_EWR_KEY_4)) 
                                {
                                    Key &= (~(FLASH_EWR_KEY_1_AREA | FLASH_EWR_KEY_4_AREA));   /* 清除当前流程操作字         */
                                    for(i = 0; i < Num; i++) 
                                    {
                                        M8(Address + i) = *(pBuff + i);                        /* 执行Flash写              */
                                        while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);           /* 等待写完成               */
                                    }
                                    RetSta = FlashSucc;
                                }
                                else
                                {
                                    RetSta = FlashErrKey;
                                }

                                HT_CMU->WPREG = CMU_WPREG_UnProtected;                         /* 关闭写保护功能         */
#if  defined  CMU_FLASHCON2
                                HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                    /* Flash读                */
#else
                                HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                     /* Flash读                */
#endif
                                HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                      /* Flash锁定              */
                                HT_CMU->WPREG = writeProtect;                                  /* 恢复之前写保护设置     */
                            }
                            else
                            {
                                RetSta = FlashErrBuf;
                            }
                        }
                        else
                        {
                            RetSta = FlashErrNum;
                        }
                    }
                    else
                    {
                        RetSta = FlashErrKey;
                    }
                    break;
                case FlashRead:
                    if ((Key & FLASH_EWR_KEY_4_AREA) == FLASH_EWR_KEY_4) 
                    {
                        Key &= (~(FLASH_EWR_KEY_1_AREA | FLASH_EWR_KEY_3_AREA | FLASH_EWR_KEY_4_AREA)); 
                                                                                               /* 清除所有流程操作字        */
                        if ((Address + Num - 1) < FLASH_MAX_ADDRESS)
                        {
                            if (((uint32_t)pBuff >= BUF_MIN_ADDRESS) && ((uint32_t)pBuff < BUF_MAX_ADDRESS))
                            {
#if  defined  CMU_FLASHCON2
                                if (HT_CMU->FLASHCON2 != CMU_FLASHCON2_FOP_READ)
#else
                                if (HT_CMU->FLASHCON != CMU_FLASHCON_FOP_READ)
#endif
                                {
                                    writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];  /* 保存当前写保护状态      */
                                    HT_CMU->WPREG = CMU_WPREG_UnProtected;                      /* 关闭写保护功能          */
#if  defined  CMU_FLASHCON2
                                    HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                 /* Flash读                */
#else
                                    HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                  /* Flash读                */
#endif
                                    HT_CMU->WPREG = writeProtect;                               /* 恢复之前写保护设置      */
                                }

                            
                                for (i = 0; i < Num; i++) 
                                {
                                    *(pBuff + i) = M8(Address + i);                             /* 执行Flash读            */
                                }
                                RetSta = FlashSucc;
                            }
                            else
                            {
                                RetSta = FlashErrBuf;
                            }
                        }
                        else
                        {
                            RetSta = FlashErrNum;
                        }
                    }
                    else
                    {
                        RetSta = FlashErrKey;
                    }
                    break;
                default:
                    RetSta = FlashErrMod;
                    break;
            }
        }
        else
        {
            RetSta = FlashErrAddr;
        }
    }
    else
    {
        RetSta = FlashErrKey;
    }

    return RetSta;
}
#endif

/*
*********************************************************************************************************
*                                         INFO BYTE WRITE
*
* 函数说明: info字节写
*
* 入口参数: pWriteByte    指向被写数据的首地址
*
*           Address       数据将要写到Info的地址(请用户确保地址合法性以及加上数据总长度后的最后一个地址)
*
*           Num           写数据长度，以半字为单位
*
* 返回参数: 无
*
* 特殊说明: 用户应保证函数执行过程中寄存器写保护状态以及Flash和info解锁状态不变
*********************************************************************************************************
*/
#if defined ( __ICCARM__ )
void HT_Info_ByteWrite(const uint8_t* pWriteByte, uint32_t Address, uint32_t Num)
{
asm(
    "PUSH   {R4-R6}\n"
    "LDR    R6, IAR_INFO_BWR_WPREG_UL_VAL\n"
    "MOVS   R5, #1\n"
    "LDR    R3, IAR_INFO_BWR_WPREG_REG_ADDR\n"
    "LDR    R4, [R3]\n"
    "ANDS   R4, R4, R5\n"
    "BEQ    IAR_INFO_BWR_CONFIG_REG_SET\n"
    "MOVS   R4, R6\n"
"IAR_INFO_BWR_CONFIG_REG_SET:\n"
    "STR    R6, [R3]\n"
    "LDR    R6, IAR_INFO_BWR_CONFIG_UL_VAL\n"
    "STR    R6, [R3, #0x38]\n"
    "LDR    R6, IAR_INFO_BWR_CONFIG2_UL_VAL\n"
    "STR    R6, [R3, #0x50]\n"
    "STR    R5, [R3, #0x34]\n"
    "CMP    R2, #0\n"
    "BEQ    IAR_INFO_BWR_END\n"
"IAR_INFO_BWR_LOOP:\n"
    "LDRB   R5, [R0]\n"
    "NOP\n"
    "STRB   R5, [R1]\n"
"IAR_INFO_BWR_FLAG_CHK_LOOP:\n"
    "LDR    R5, [R3, #0x34]\n"
    "LSLS   R5, R5, #29\n"
    "BMI    IAR_INFO_BWR_FLAG_CHK_LOOP\n"
    "ADDS   R0, R0, #1\n"
    "ADDS   R1, R1, #1\n"
    "SUBS   R2, R2, #1\n"
    "BNE    IAR_INFO_BWR_LOOP\n"
"IAR_INFO_BWR_END:\n"
    "MOVS   R0, #0\n"
    "STR    R0, [R3, #0x34]\n"
    "STR    R0, [R3, #0x50]\n"
    "STR    R0, [R3, #0x38]\n"
    "STR    R4, [R3]\n"
    "POP    {R4-R6}\n"
    "BX     LR\n"
    "NOP\n"
"IAR_INFO_BWR_WPREG_REG_ADDR: DC32    0x4000F000\n"
"IAR_INFO_BWR_WPREG_UL_VAL:   DC32    0x0000A55A\n"
"IAR_INFO_BWR_CONFIG_UL_VAL:  DC32    0x00007A68\n"
"IAR_INFO_BWR_CONFIG2_UL_VAL: DC32    0x0000F998\n"
      );
}

#elif defined ( __CC_ARM )
__ASM void HT_Info_ByteWrite(const uint8_t* pWriteByte, uint32_t Address, uint32_t Num)
{
    PUSH    {R4-R6}
    LDR     R6, |KEIL_INFO_BWR_WPREG_UL_VAL|
    MOVS    R5, #1
    LDR     R3, |KEIL_INFO_BWR_WPREG_REG_ADDR|
    LDR     R4, [R3]
    ANDS    R4, R4, R5
    BEQ     KEIL_INFO_BWR_CONFIG_REG_SET
    MOVS    R4, R6
KEIL_INFO_BWR_CONFIG_REG_SET
    STR     R6, [R3]
    LDR     R6, |KEIL_INFO_BWR_CONFIG_UL_VAL|
    STR     R6, [R3, #0x38]
    LDR     R6, |KEIL_INFO_BWR_CONFIG2_UL_VAL|
    STR     R6, [R3, #0x50]
    STR     R5, [R3, #0x34]
    CMP     R2, #0
    BEQ     KEIL_INFO_BWR_END
KEIL_INFO_BWR_LOOP
    LDRB    R5, [R0]
    NOP
    STRB    R5, [R1]
KEIL_INFO_BWR_FLAG_CHK_LOOP
    LDR     R5, [R3, #0x34]
    LSLS    R5, R5, #29
    BMI     KEIL_INFO_BWR_FLAG_CHK_LOOP
    ADDS    R0, R0, #1
    ADDS    R1, R1, #1
    SUBS    R2, R2, #1
    BNE     KEIL_INFO_BWR_LOOP
KEIL_INFO_BWR_END
    MOVS    R0, #0
    STR     R0, [R3, #0x34]
    STR     R0, [R3, #0x50]
    STR     R0, [R3, #0x38]
    STR     R4, [R3]
    POP     {R4-R6}
    BX      LR
    NOP
|KEIL_INFO_BWR_WPREG_REG_ADDR| DCD     0x4000F000
|KEIL_INFO_BWR_WPREG_UL_VAL|   DCD     0x0000A55A
|KEIL_INFO_BWR_CONFIG_UL_VAL|  DCD     0x00007A68
|KEIL_INFO_BWR_CONFIG2_UL_VAL| DCD     0x0000F998
}
#endif

/*
*********************************************************************************************************
*                                         INFO BYTE READ
*
* 函数说明: Info字节读
*
* 入口参数: pReadByte     指向存储读到的数据的首地址
*
*           Address       读Info的首地址
*
*           Num           读数据长度，以字节为单位
*
* 返回参数: 无
*
* 特殊说明: 用户需注意读的数据不要超出保存的范围，同时访问超出info有效范围时不返回数据
*********************************************************************************************************
*/
void HT_Info_ByteRead(uint8_t* pReadByte, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */
    uint32_t i;
    uint32_t writeProtect;

    if ((Address < INFO_MIN_ADDRESS) || (Address >= INFO_MAX_ADDRESS))
    {
        return;
    }

    writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];             /*!< 保存当前写保护状态     */
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

#if  defined  CMU_FLASHCON2
    HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                            /*!< Flash读                */
#else
    HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                             /*!< Flash读                */
#endif

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */

    for(i=0; i<Num; i++)
    {
        if ((Address+i) >= INFO_MAX_ADDRESS)
        {
            return;
        }
        pReadByte[i] = M8(Address+i);                                      /*!< 执行Flash读            */
    }
}

/*
*********************************************************************************************************
*                                         INFO HALF WORD WRITE
*
* 函数说明: info半字写
*
* 入口参数: pWriteHalfWord    指向被写数据的首地址
*
*           Address           数据将要写到Info的地址(请用户确保地址合法性以及加上数据总长度后的最后一个地址)
*
*           Num               写数据长度，以半字为单位
*
* 返回参数: 无
*
* 特殊说明: 1)用户应保证函数执行过程中寄存器写保护状态以及Flash和info解锁状态不变
*           2)用户应保证传递给函数的地址为至少半字对齐
*********************************************************************************************************
*/
#if defined ( __ICCARM__ )
void HT_Info_HalfWordWrite(const uint16_t* pWriteHalfWord, uint32_t Address, uint32_t Num)
{
asm(
    "PUSH   {R4-R6}\n"
    "LDR    R6, IAR_INFO_HWWR_WPREG_UL_VAL\n"
    "MOVS   R5, #1\n"
    "LDR    R3, IAR_INFO_HWWR_WPREG_REG_ADDR\n"
    "LDR    R4, [R3]\n"
    "ANDS   R4, R4, R5\n"
    "BEQ    IAR_INFO_HWWR_CONFIG_REG_SET\n"
    "MOVS   R4, R6\n"
"IAR_INFO_HWWR_CONFIG_REG_SET:\n"
    "STR    R6, [R3]\n"
    "LDR    R6, IAR_INFO_HWWR_CONFIG_UL_VAL\n"
    "STR    R6, [R3, #0x38]\n"
    "LDR    R6, IAR_INFO_HWWR_CONFIG2_UL_VAL\n"
    "STR    R6, [R3, #0x50]\n"
    "STR    R5, [R3, #0x34]\n"
    "CMP    R2, #0\n"
    "BEQ    IAR_INFO_HWWR_END\n"
    "MOVS   R5, #1\n"
    "BICS   R1, R1, R5\n"
"IAR_INFO_HWWR_LOOP:\n"
    "LDRH   R5, [R0]\n"
    "NOP\n"
    "STRH   R5, [R1]\n"
"IAR_INFO_HWWR_FLAG_CHK_LOOP:\n"
    "LDR    R5, [R3, #0x34]\n"
    "LSLS   R5, R5, #29\n"
    "BMI    IAR_INFO_HWWR_FLAG_CHK_LOOP\n"
    "ADDS   R0, R0, #2\n"
    "ADDS   R1, R1, #2\n"
    "SUBS   R2, R2, #1\n"
    "BNE    IAR_INFO_HWWR_LOOP\n"
"IAR_INFO_HWWR_END:\n"
    "MOVS   R0, #0\n"
    "STR    R0, [R3, #0x34]\n"
    "STR    R0, [R3, #0x50]\n"
    "STR    R0, [R3, #0x38]\n"
    "STR    R4, [R3]\n"
    "POP    {R4-R6}\n"
    "BX     LR\n"
    "NOP\n"
"IAR_INFO_HWWR_WPREG_REG_ADDR: DC32    0x4000F000\n"
"IAR_INFO_HWWR_WPREG_UL_VAL:   DC32    0x0000A55A\n"
"IAR_INFO_HWWR_CONFIG_UL_VAL:  DC32    0x00007A68\n"
"IAR_INFO_HWWR_CONFIG2_UL_VAL: DC32    0x0000F998\n"
      );
}

#elif defined ( __CC_ARM )
__ASM void HT_Info_HalfWordWrite(const uint16_t* pWriteHalfWord, uint32_t Address, uint32_t Num)
{
    PUSH    {R4-R6}
    LDR     R6, |KEIL_INFO_HWWR_WPREG_UL_VAL|
    MOVS    R5, #1
    LDR     R3, |KEIL_INFO_HWWR_WPREG_REG_ADDR|
    LDR     R4, [R3]
    ANDS    R4, R4, R5
    BEQ     KEIL_INFO_HWWR_CONFIG_REG_SET
    MOVS    R4, R6
KEIL_INFO_HWWR_CONFIG_REG_SET
    STR     R6, [R3]
    LDR     R6, |KEIL_INFO_HWWR_CONFIG_UL_VAL|
    STR     R6, [R3, #0x38]
    LDR     R6, |KEIL_INFO_HWWR_CONFIG2_UL_VAL|
    STR     R6, [R3, #0x50]
    STR     R5, [R3, #0x34]
    CMP     R2, #0
    BEQ     KEIL_INFO_HWWR_END
    MOVS    R5, #1
    BICS    R1, R1, R5
KEIL_INFO_HWWR_LOOP
    LDRH    R5, [R0]
    NOP
    STRH    R5, [R1]
KEIL_INFO_HWWR_FLAG_CHK_LOOP
    LDR     R5, [R3, #0x34]
    LSLS    R5, R5, #29
    BMI     KEIL_INFO_HWWR_FLAG_CHK_LOOP
    ADDS    R0, R0, #2
    ADDS    R1, R1, #2
    SUBS    R2, R2, #1
    BNE     KEIL_INFO_HWWR_LOOP
KEIL_INFO_HWWR_END
    MOVS    R0, #0
    STR     R0, [R3, #0x34]
    STR     R0, [R3, #0x50]
    STR     R0, [R3, #0x38]
    STR     R4, [R3]
    POP     {R4-R6}
    BX      LR
    NOP
|KEIL_INFO_HWWR_WPREG_REG_ADDR| DCD     0x4000F000
|KEIL_INFO_HWWR_WPREG_UL_VAL|   DCD     0x0000A55A
|KEIL_INFO_HWWR_CONFIG_UL_VAL|  DCD     0x00007A68
|KEIL_INFO_HWWR_CONFIG2_UL_VAL| DCD     0x0000F998
}
#endif

/*
*********************************************************************************************************
*                                         INFO HALF WORD READ
*
* 函数说明: Info 半字读
*
* 入口参数: pReadHalfWord 指向存储读到的数据的首地址
*
*           Address       读Info的首地址
*
*           Num           读数据长度，以半字为单位
*
* 返回参数: 无
*
* 特殊说明: 用户需注意读的数据不要超出保存的范围，同时访问超出info有效范围时不返回数据
*********************************************************************************************************
*/
void HT_Info_HalfWordRead(uint16_t* pReadHalfWord, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */
    uint32_t i;
    uint32_t writeProtect;

    if ((Address < INFO_MIN_ADDRESS) || (Address >= INFO_MAX_ADDRESS))
    {
        return;
    }

    writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];             /*!< 保存当前写保护状态     */
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

#if  defined  CMU_FLASHCON2
    HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                            /*!< Flash读                */
#else
    HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                             /*!< Flash读                */
#endif

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */

    Address &= 0xFFFFFFFE;                                                 /*!< 保证半字对齐           */
    for(i=0; i<Num; i++)
    {
        if ((Address+i*2) >= INFO_MAX_ADDRESS)
        {
            return;
        }
        pReadHalfWord[i] = M16(Address+i*2);                               /*!< 执行Flash读            */
    }
}

/*
*********************************************************************************************************
*                                         INFO WORD WRITE
*
* 函数说明: info字写
*
* 入口参数: pWriteWord    指向被写数据的首地址
*
*           Address       数据将要写到Info的地址(请用户确保地址合法性以及加上数据总长度后的最后一个地址)
*
*           Num           写数据长度，以字为单位
*
* 返回参数: 无
*
* 特殊说明: 1)用户应保证函数执行过程中寄存器写保护状态以及Flash和info解锁状态不变
*           2)用户应保证传递给函数的地址为至少字对齐
*********************************************************************************************************
*/
#if defined ( __ICCARM__ )
void HT_Info_WordWrite(const uint32_t* pWriteWord, uint32_t Address, uint32_t Num)
{
asm(
    "PUSH   {R4-R6}\n"
    "LDR    R6, IAR_INFO_WWR_WPREG_UL_VAL\n"
    "MOVS   R5, #1\n"
    "LDR    R3, IAR_INFO_WWR_WPREG_REG_ADDR\n"
    "LDR    R4, [R3]\n"
    "ANDS   R4, R4, R5\n"
    "BEQ    IAR_INFO_WWR_CONFIG_REG_SET\n"
    "MOVS   R4, R6\n"
"IAR_INFO_WWR_CONFIG_REG_SET:\n"
    "STR    R6, [R3]\n"
    "LDR    R6, IAR_INFO_WWR_CONFIG_UL_VAL\n"
    "STR    R6, [R3, #0x38]\n"
    "LDR    R6, IAR_INFO_WWR_CONFIG2_UL_VAL\n"
    "STR    R6, [R3, #0x50]\n"
    "STR    R5, [R3, #0x34]\n"
    "CMP    R2, #0\n"
    "BEQ    IAR_INFO_WWR_END\n"
    "MOVS   R5, #3\n"
    "BICS   R1, R1, R5\n"
"IAR_INFO_WWR_LOOP:\n"
    "LDR    R5, [R0]\n"
    "NOP\n"
    "STR    R5, [R1]\n"
"IAR_INFO_WWR_FLAG_CHK_LOOP:\n"
    "LDR    R5, [R3, #0x34]\n"
    "LSLS   R5, R5, #29\n"
    "BMI    IAR_INFO_WWR_FLAG_CHK_LOOP\n"
    "ADDS   R0, R0, #4\n"
    "ADDS   R1, R1, #4\n"
    "SUBS   R2, R2, #1\n"
    "BNE    IAR_INFO_WWR_LOOP\n"
"IAR_INFO_WWR_END:\n"
    "MOVS   R0, #0\n"
    "STR    R0, [R3, #0x34]\n"
    "STR    R0, [R3, #0x50]\n"
    "STR    R0, [R3, #0x38]\n"
    "STR    R4, [R3]\n"
    "POP    {R4-R6}\n"
    "BX     LR\n"
    "NOP\n"
"IAR_INFO_WWR_WPREG_REG_ADDR: DC32    0x4000F000\n"
"IAR_INFO_WWR_WPREG_UL_VAL:   DC32    0x0000A55A\n"
"IAR_INFO_WWR_CONFIG_UL_VAL:  DC32    0x00007A68\n"
"IAR_INFO_WWR_CONFIG2_UL_VAL: DC32    0x0000F998\n"
      );
}

#elif defined ( __CC_ARM )
__ASM void HT_Info_WordWrite(const uint32_t* pWriteWord, uint32_t Address, uint32_t Num)
{
    PUSH    {R4-R6}
    LDR     R6, |KEIL_INFO_WWR_WPREG_UL_VAL|
    MOVS    R5, #1
    LDR     R3, |KEIL_INFO_WWR_WPREG_REG_ADDR|
    LDR     R4, [R3]
    ANDS    R4, R4, R5
    BEQ     KEIL_INFO_WWR_CONFIG_REG_SET
    MOVS    R4, R6
KEIL_INFO_WWR_CONFIG_REG_SET
    STR     R6, [R3]
    LDR     R6, |KEIL_INFO_WWR_CONFIG_UL_VAL|
    STR     R6, [R3, #0x38]
    LDR     R6, |KEIL_INFO_WWR_CONFIG2_UL_VAL|
    STR     R6, [R3, #0x50]
    STR     R5, [R3, #0x34]
    CMP     R2, #0
    BEQ     KEIL_INFO_WWR_END
    MOVS    R5, #3
    BICS    R1, R1, R5
KEIL_INFO_WWR_LOOP
    LDR     R5, [R0]
    NOP
    STR     R5, [R1]
KEIL_INFO_WWR_FLAG_CHK_LOOP
    LDR     R5, [R3, #0x34]
    LSLS    R5, R5, #29
    BMI     KEIL_INFO_WWR_FLAG_CHK_LOOP
    ADDS    R0, R0, #4
    ADDS    R1, R1, #4
    SUBS    R2, R2, #1
    BNE     KEIL_INFO_WWR_LOOP
KEIL_INFO_WWR_END
    MOVS    R0, #0
    STR     R0, [R3, #0x34]
    STR     R0, [R3, #0x50]
    STR     R0, [R3, #0x38]
    STR     R4, [R3]
    POP     {R4-R6}
    BX      LR
    NOP
|KEIL_INFO_WWR_WPREG_REG_ADDR| DCD     0x4000F000
|KEIL_INFO_WWR_WPREG_UL_VAL|   DCD     0x0000A55A
|KEIL_INFO_WWR_CONFIG_UL_VAL|  DCD     0x00007A68
|KEIL_INFO_WWR_CONFIG2_UL_VAL| DCD     0x0000F998
}
#endif

/*
*********************************************************************************************************
*                                         FLASH WORD READ
*
* 函数说明: Info字读
*
* 入口参数: pReadWord     指向存储读到的数据的首地址
*
*           Address       读Info的首地址
*
*           Num           读数据长度，以字为单位
*
* 返回参数: 无
*
* 特殊说明: 用户需注意读的数据不要超出保存的范围，同时访问超出info有效范围时不返回数据
*********************************************************************************************************
*/
void HT_Info_WordRead(uint32_t* pReadWord, uint32_t Address, uint32_t Num)
{
    /*  assert_param  */
    uint32_t i;
    uint32_t writeProtect;

    if ((Address < INFO_MIN_ADDRESS) || (Address >= INFO_MAX_ADDRESS))
    {
        return;
    }

    writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];             /*!< 保存当前写保护状态     */
    HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /*!< 关闭写保护功能         */

#if  defined  CMU_FLASHCON2
    HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                            /*!< Flash读                */
#else
    HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                             /*!< Flash读                */
#endif

    HT_CMU->WPREG = writeProtect;                                          /*!< 恢复之前写保护设置     */

    Address &= 0xFFFFFFFC;                                                 /*!< 保证字对齐             */
    for(i=0; i<Num; i++)
    {
        if ((Address+i*4) >= INFO_MAX_ADDRESS)
        {
            return;
        }
        pReadWord[i] = M32(Address+i*4);                                   /*!< 执行Flash读            */
    }
}

/*
*********************************************************************************************************
*                                         INFO PAGE ERASE
*
* 函数说明: INFO页擦除
*
* 入口参数: EraseAddress    页擦除地址(请用户确保地址合法性)
*
* 返回参数: 无
*
* 特殊说明: 用户应保证函数执行过程中寄存器写保护状态以及Flash和info解锁状态不变，
*          HT501x 总共512 bytes - 256 bytes/page
*          HT502x 总共1k  bytes - 1k  bytes/page
*          HT6x1x 总共512 bytes - 256 bytes/page
*          HT6x2x 总共1k  bytes - 1k  bytes/page
*          HT6x3x 总共1k  bytes - 1k  bytes/page
*********************************************************************************************************
*/
#if defined ( __ICCARM__ )
void HT_Info_PageErase(uint32_t EraseAddress)
{
asm(
    "PUSH   {R4}\n"
    "LDR    R3, IAR_INFO_PGES_WPREG_UL_VAL\n"
    "LDR    R1, IAR_INFO_PGES_WPREG_REG_ADDR\n"
    "LDR    R2, [R1]\n"
    "LSLS   R2, R2, #31\n"
    "BPL    IAR_INFO_PGES_CLEAR\n"
    "MOVS   R2, R3\n"
    "B      IAR_INFO_PGES_CONFIG_REG_SET\n"
"IAR_INFO_PGES_CLEAR:\n"
    "MOVS   R2, #0\n"
"IAR_INFO_PGES_CONFIG_REG_SET:\n"
    "STR    R3, [R1]\n"
    "LDR    R3, IAR_INFO_PGES_CONFIG_UL_VAL\n"
    "STR    R3, [R1, #0x38]\n"
    "LDR    R3, IAR_INFO_PGES_CONFIG2_UL_VAL\n"
    "STR    R3, [R1, #0x50]\n"
    "MOVS   R3, #2\n"
    "STR    R3, [R1, #0x34]\n"
    "MOVS   R3, #255\n"
    "MOVS   R4, #3\n"
    "BICS   R0, R0, R4\n"
    "NOP\n"
    "STR    R3, [R0]\n"
"IAR_INFO_PGES_FLAG_CHK_LOOP:\n"
    "LDR    R0, [R1, #0x34]\n"
    "LSLS   R0, R0, #29\n"
    "BMI    IAR_INFO_PGES_FLAG_CHK_LOOP\n"
    "MOVS   R0, #0\n"
    "STR    R0, [R1, #0x34]\n"
    "STR    R0, [R1, #0x50]\n"
    "STR    R0, [R1, #0x38]\n"
    "STR    R2, [R1]\n"
    "POP    {R4}\n"
    "BX     LR\n"
    "NOP\n"
"IAR_INFO_PGES_WPREG_REG_ADDR: DC32    0x4000F000\n"
"IAR_INFO_PGES_WPREG_UL_VAL:   DC32    0x0000A55A\n"
"IAR_INFO_PGES_CONFIG_UL_VAL:  DC32    0x00007A68\n"
"IAR_INFO_PGES_CONFIG2_UL_VAL: DC32    0x0000F998\n"
  );
}

#elif defined ( __CC_ARM )
__ASM void HT_Info_PageErase(uint32_t EraseAddress)
{
    PUSH    {R4}
    LDR     R3, |KEIL_INFO_PGES_WPREG_UL_VAL|
    LDR     R1, |KEIL_INFO_PGES_WPREG_REG_ADDR|
    LDR     R2, [R1]
    LSLS    R2, R2, #31
    BPL     KEIL_INFO_PGES_CLEAR
    MOVS    R2, R3
    B       KEIL_INFO_PGES_CONFIG_REG_SET
KEIL_INFO_PGES_CLEAR
    MOVS    R2, #0
KEIL_INFO_PGES_CONFIG_REG_SET
    STR     R3, [R1]
    LDR     R3, |KEIL_INFO_PGES_CONFIG_UL_VAL|
    STR     R3, [R1, #0x38]
    LDR     R3, |KEIL_INFO_PGES_CONFIG2_UL_VAL|
    STR     R3, [R1, #0x50]
    MOVS    R3, #2
    STR     R3, [R1, #0x34]
    MOVS    R3, #255
    MOVS    R4, #3
    BICS    R0, R0, R4
    NOP
    STR     R3, [R0]
KEIL_INFO_PGES_FLAG_CHK_LOOP
    LDR     R0, [R1, #0x34]
    LSLS    R0, R0, #29
    BMI     KEIL_INFO_PGES_FLAG_CHK_LOOP
    MOVS    R0, #0
    STR     R0, [R1, #0x34]
    STR     R0, [R1, #0x50]
    STR     R0, [R1, #0x38]
    STR     R2, [R1]
    POP     {R4}
    BX      LR
    NOP
|KEIL_INFO_PGES_WPREG_REG_ADDR| DCD    0x4000F000
|KEIL_INFO_PGES_WPREG_UL_VAL|   DCD    0x0000A55A
|KEIL_INFO_PGES_CONFIG_UL_VAL|  DCD    0x00007A68
|KEIL_INFO_PGES_CONFIG2_UL_VAL| DCD    0x0000F998
}
#endif

#if defined  HT6x1x  ||  defined  HT6x2x  ||  defined  HT6x3x  ||  defined  HT502x
/*
*********************************************************************************************************
*                                    INFO BYTE ERASE/WRITE/READ
*
* 函数说明: info擦/写/读
*
* 入口参数: Address       数据将要写到Info的地址(请用户确保地址合法性以及加上数据总长度后的最后一个地址)
*
*           Method        Info操作方式
*                          FlashErase    INFO擦操作
*                          FlashWrite    INFO写操作
*                          FlashRead     INFO读操作
*
*           pBuff         指向存储数据的缓存的首地址(擦操作无效)
*
*           Num           写数据长度，以字节为单位(擦操作无效)
*
*           Key           执行操作的检查控制字
*                          FLASH_EWR_KEY  软件确认操作码
*
* 返回参数: RetSta        该函数操作结果
*                          FlashSucc      INFO操作成功
*                          FlashNone      INFO无操作  
*                          FlashErrKey    用户输入操作码有误
*                          FlashErrAddr   用户输入INFO地址信息有误
*                          FlashErrMod    用户输入INFO操作方式有误
*                          FlashErrNum    用户输入buffer的操作数据范围有误
*                          FlashErrBuf    用户输入buffer的地址有误
*
* 特殊说明: 
*********************************************************************************************************
*/
FLASH_RetStaTypeDef HT_Info_EWR_Pro (uint32_t Address, FLASH_WorkMethodTypeDef Method, uint8_t* pBuff, uint32_t Num, uint32_t Key)
{
    uint32_t writeProtect, i;
    FLASH_RetStaTypeDef RetSta = FlashNone;

    if ((Key & FLASH_EWR_KEY_2_AREA) == FLASH_EWR_KEY_2)                                       /* 检查第一级软件关键字是否合法 */
    {
        Key &= (~FLASH_EWR_KEY_2_AREA);                                                        /* 清除当前流程操作字          */

        if ((Address >= INFO_MIN_ADDRESS) && (Address < INFO_MAX_ADDRESS))                     /* 检查操作地址非法性          */
        {
            switch (Method)                                                                    /* 检查操作地址非法性          */
            {
                case FlashErase:
                    if ((Key & FLASH_EWR_KEY_1_AREA) == FLASH_EWR_KEY_1)                       /* 检查第二级软件关键字是否合法 */
                    {
                        Key &= (~FLASH_EWR_KEY_1_AREA);                                        /* 清除当前流程操作字          */

                        writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];             /* 保存当前写保护状态          */
                        HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /* 关闭写保护功能              */
                        HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                            /* Flash解锁                  */
#if  defined  HT6x3x
                        HT_CMU->INFOLOCK_A = CMU_INFOLOCK_A_UnLocked;                          /* Info解锁                   */
#else
                        HT_CMU->INFOLOCK = CMU_INFOLOCK_UnLocked;                              /* Info解锁                   */
#endif
#if  defined  CMU_FLASHCON2
                        HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_PAGEERASE;                       /* Flash页擦除                */
#else
                        HT_CMU->FLASHCON = CMU_FLASHCON_FOP_PAGEERASE;                         /* Flash页擦除                */
#endif
                        HT_CMU->WPREG = CMU_FLASHLOCK_Locked;                                  /* 恢复之前写保护设置          */

                        if (((Key & FLASH_EWR_KEY_3_AREA) == FLASH_EWR_KEY_3) 
                          && ((Key & FLASH_EWR_KEY_4_AREA) == FLASH_EWR_KEY_4))                /* 检查第三级软件关键字是否合法 */
                        {
                            Key &= (~(FLASH_EWR_KEY_3_AREA | FLASH_EWR_KEY_4_AREA));           /* 清除当前流程操作字          */
                            M8(Address) = 0xFF;                                                /* 执行Flash页擦除            */
                            while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);                       /* 等待写完成                 */
                            RetSta = FlashSucc;
                        } 
                        else
                        {
                            RetSta = FlashErrKey;
                        } 

                        HT_CMU->WPREG = CMU_WPREG_UnProtected;                                 /* 关闭写保护功能             */
#if  defined  CMU_FLASHCON2
                        HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                            /* Flash读                   */
#else
                        HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                             /* Flash读                   */
#endif
#if  defined  HT6x3x
                        HT_CMU->INFOLOCK_A = CMU_INFOLOCK_A_Locked;                            /* Info解锁                  */
#else
                        HT_CMU->INFOLOCK = CMU_INFOLOCK_Locked;                                 /* Info解锁                  */
#endif
                        HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                              /* Flash锁定                 */
                        HT_CMU->WPREG = writeProtect;                                          /* 恢复之前写保护设置         */
                    }
                    else
                    {
                        RetSta = FlashErrKey;
                    }
                    break;
                case FlashWrite:
                    if ((Key & FLASH_EWR_KEY_3_AREA) == FLASH_EWR_KEY_3) 
                    {
                        Key &= (~FLASH_EWR_KEY_3_AREA);                                        /* 清除当前流程操作字         */

                        if ((Address + Num - 1) < INFO_MAX_ADDRESS) 
                        {
                            if (((uint32_t)pBuff >= BUF_MIN_ADDRESS) && ((uint32_t)pBuff < BUF_MAX_ADDRESS))
                            {
                                writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01];     /* 保存当前写保护状态         */
                                HT_CMU->WPREG = CMU_WPREG_UnProtected;                         /* 关闭写保护功能             */
                                HT_CMU->FLASHLOCK = CMU_FLASHLOCK_UnLocked;                    /* Flash解锁                 */
#if  defined  HT6x3x
                                HT_CMU->INFOLOCK_A = CMU_INFOLOCK_A_UnLocked;                  /* Info解锁                  */
#else
                                HT_CMU->INFOLOCK = CMU_INFOLOCK_UnLocked;                      /* Info解锁                  */
#endif
#if  defined  CMU_FLASHCON2
                                HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_WRITE;                   /* Flash写                   */
#else
                                HT_CMU->FLASHCON = CMU_FLASHCON_FOP_WRITE;                     /* Flash写                   */
#endif
                                HT_CMU->WPREG = CMU_FLASHLOCK_Locked;                          /* 恢复之前写保护设置         */

                            
                                if (((Key & FLASH_EWR_KEY_1_AREA) == FLASH_EWR_KEY_1) 
                                  && ((Key & FLASH_EWR_KEY_4_AREA) == FLASH_EWR_KEY_4)) 
                                {
                                    Key &= (~(FLASH_EWR_KEY_1_AREA | FLASH_EWR_KEY_4_AREA));   /* 清除当前流程操作字         */
                                    for(i = 0; i < Num; i++) 
                                    {
                                        M8(Address + i) = *(pBuff + i);                        /* 执行Flash写               */
                                        while(HT_CMU->FLASHCON & CMU_FLASHCON_BUSY);           /* 等待写完成                */
                                    }
                                    RetSta = FlashSucc;
                                }
                                else
                                {
                                    RetSta = FlashErrKey;
                                }
                            

                                HT_CMU->WPREG = CMU_WPREG_UnProtected;                         /* 关闭写保护功能            */
#if  defined  CMU_FLASHCON2
                                HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                    /* Flash读                  */
#else
                                HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                     /* Flash读                  */
#endif
#if  defined  HT6x3x
                                HT_CMU->INFOLOCK_A = CMU_INFOLOCK_A_Locked;                    /* Info解锁                 */
#else
                                HT_CMU->INFOLOCK = CMU_INFOLOCK_Locked;                         /* Info解锁                 */
#endif
                                HT_CMU->FLASHLOCK = CMU_FLASHLOCK_Locked;                      /* Flash锁定                */
                                HT_CMU->WPREG = writeProtect;                                  /* 恢复之前写保护设置        */
                            }
                            else
                            {
                                RetSta = FlashErrBuf;
                            }
                        }
                        else
                        {
                            RetSta = FlashErrNum;
                        }
                    }
                    else
                    {
                        RetSta = FlashErrKey;
                    }
                    break;
                case FlashRead:
                    if ((Key & FLASH_EWR_KEY_4_AREA) == FLASH_EWR_KEY_4) 
                    {
                        Key &= (~(FLASH_EWR_KEY_1_AREA | FLASH_EWR_KEY_3_AREA | FLASH_EWR_KEY_4_AREA)); 
                                                                                               /* 清除所有流程操作字        */
                        if ((Address + Num - 1) < INFO_MAX_ADDRESS)
                        {
                            if (((uint32_t)pBuff >= BUF_MIN_ADDRESS) && ((uint32_t)pBuff < BUF_MAX_ADDRESS))
                            {
#if  defined  CMU_FLASHCON2
                                if (HT_CMU->FLASHCON2 != CMU_FLASHCON2_FOP_READ)
#else
                                if (HT_CMU->FLASHCON != CMU_FLASHCON_FOP_READ)
#endif
                                {
                                    writeProtect = RegisterWriteProtect[HT_CMU->WPREG & 0x01]; /* 保存当前写保护状态        */
                                    HT_CMU->WPREG = CMU_WPREG_UnProtected;                     /* 关闭写保护功能            */
#if  defined  CMU_FLASHCON2
                                    HT_CMU->FLASHCON2 = CMU_FLASHCON2_FOP_READ;                /* Flash读                  */
#else
                                    HT_CMU->FLASHCON  = CMU_FLASHCON_FOP_READ;                 /* Flash读                  */
#endif
                                    HT_CMU->WPREG = writeProtect;                              /* 恢复之前写保护设置        */
                                }

                            
                                for (i = 0; i < Num; i++) 
                                {
                                    *(pBuff + i) = M8(Address + i);                            /* 执行Flash读              */
                                }
                                RetSta = FlashSucc;
                            }
                            else
                            {
                                RetSta = FlashErrBuf;
                            }
                        }
                        else
                        {
                            RetSta = FlashErrNum;
                        }
                    }
                    else
                    {
                        RetSta = FlashErrKey;
                    }
                    break;
                default:
                    RetSta = FlashErrMod;
                    break;
            }
        }
        else
        {
            RetSta = FlashErrAddr;
        }
    }
    else
    {
        RetSta = FlashErrKey;
    }

    return RetSta;
}
#endif
