
#include "at24c0x.h"
#include "delay.h"

static I2C_NUM_DEF  i2c_num;
static uint8_t IIC_ERR;


uint8_t get_i2c_num(uint16_t ee_addr, uint16_t num);
void delay8(uint8_t t);



void delay8(uint8_t t)
{
  while(--t);
}



uint8_t get_i2c_num(uint16_t ee_addr, uint16_t num)
{
    uint8_t first_page_left;
    uint16_t left_num;

    if(num==0)      return FALSE;
    if((ee_addr+num)>EE_SIZE)   return FALSE;
    first_page_left=EE_PAGE_SIZE-(ee_addr%EE_PAGE_SIZE);
    if(num<=first_page_left)
    {
        i2c_num.page_one_num=num;
        i2c_num.pages=0;
        i2c_num.page_last_num=0;
        return TRUE;
    }
    else
    {
        left_num=num-first_page_left;
        i2c_num.page_one_num=first_page_left;
        i2c_num.pages=(left_num>>EE_PAGE_BIT);
        i2c_num.page_last_num=left_num&(EE_PAGE_SIZE-1);
        return TRUE;
    }
}


uint8_t IIC_ReadByte(uint8_t nums)
{
    uint8_t i;
    uint8_t datas;

    datas=0;

    SET_SDA_IN();
    for(i=0;i<8;i++)
    {
        datas=datas<<1;
        SCL_OUT_0();
        delay8(2);
        SCL_OUT_1();
        if(SDA)
            datas|=0x01;
        delay8(2);
    }
    SCL_OUT_0();
    SET_SDA_OUT();
    if(nums==1)
        SDA_OUT_1();				 //NoACK
    else
        SDA_OUT_0();				 //ACK
    delay8(2);
    SCL_OUT_1();
    delay8(2);
    SCL_OUT_0();

    return datas;
}


void IIC_SendByte(uint8_t datas)
{
    uint8_t i;

    for(i=0;i<8;i++)
    {
        SCL_OUT_0();
        if(datas&0x80)
            SDA_OUT_1();
        else
            SDA_OUT_0();
        datas=datas<<1;
        delay8(2);
        SCL_OUT_1();
        delay8(2);
    }
    SCL_OUT_0();
    SET_SDA_IN();
    delay8(2);
    SCL_OUT_1();
    if(SDA)		    //ACK
    {
        IIC_ERR=1;
    }
    else
    {
        IIC_ERR=0;
    }
    delay8(2);
    SCL_OUT_0();
    SET_SDA_OUT();
}



uint8_t IIC_page_write(uint8_t *source, uint16_t ee_addr, uint8_t nums, uint8_t clr_write)
{
    uint8_t try_time,k;

    if(nums==0)
    return TRUE;
    if(nums>EE_PAGE_SIZE)
    {
        return FALSE;
    }
    try_time=0;

    while(try_time++<2)
    {
        SDA_OUT_1();
        SCL_OUT_1();
        delay8(10);
        SDA_OUT_0();		 //Start
        delay8(2);
        IIC_SendByte(E2PROM_WRITE);    //Control Byte
        IIC_SendByte(ee_addr>>8);    //Address High
        IIC_SendByte(ee_addr&0xFF);    //Address Low
        for(k=0;k<nums;k++)
        {
        if(clr_write==CLR_EE)
        IIC_SendByte(0);
        else
        IIC_SendByte(*(source+k));
        if(IIC_ERR)
        break;
    }
    SDA_OUT_0();
    delay8(2);			 //Stop
    SCL_OUT_1();
    delay8(2);
    SDA_OUT_1();

    Delay_mSec(10);
    if(IIC_ERR==0)
    {
        break;
    }
    }
    if(IIC_ERR)
    {
        return FALSE;
    }
    else
    {
    return TRUE;
    }
}



uint8_t EEPROM_Write(uint8_t *source, uint16_t ee_addr, uint16_t write_num)
{
    uint8_t i;
    uint8_t *ptr;
    WP_OUT_0();

    if(get_i2c_num(ee_addr,write_num)==FALSE)
    {
        WP_OUT_1();
        return FALSE;
    }
    ptr=source+i2c_num.page_one_num;
    if(IIC_page_write(source, ee_addr, i2c_num.page_one_num, WRITE_EE)==FALSE)
    {
        WP_OUT_1();
        return FALSE;
    }
    for(i=0;i<i2c_num.pages;i++)
    {
        if(IIC_page_write(ptr+((uint16_t)i<<EE_PAGE_BIT), ee_addr+i2c_num.page_one_num+((uint16_t)i<<EE_PAGE_BIT), EE_PAGE_SIZE, WRITE_EE)==FALSE)
        {
            WP_OUT_1();
            return FALSE;
        }
    }
    if(IIC_page_write(ptr+((uint16_t)i<<EE_PAGE_BIT), ee_addr+i2c_num.page_one_num+((uint16_t)i<<EE_PAGE_BIT), i2c_num.page_last_num, WRITE_EE)==FALSE)
    {
        WP_OUT_1();
        return FALSE;
    }

    WP_OUT_1();
    return TRUE;
}

uint8_t EEPROM_Read(uint8_t *source, uint16_t ee_addr, uint16_t nums)
{
	 if(nums==0)      return FALSE;
	 if((ee_addr+nums)>EE_SIZE)   return FALSE;

	 WP_OUT_0();

	 SDA_OUT_1();
	 SCL_OUT_1();
	 delay8(10);
	 SDA_OUT_0();		 //Start

	 IIC_SendByte(E2PROM_WRITE);    //Control Byte
	 IIC_SendByte(ee_addr>>8);     //Address High
	 IIC_SendByte(ee_addr&0xFF);    //Address Low

	 SDA_OUT_1();		  //Restart
	 delay8(2);
	 SCL_OUT_1();
	 delay8(2);
	 SDA_OUT_0();
	 delay8(2);

	 IIC_SendByte(E2PROM_READ);    //Control Byte

	 for(;nums>0;nums--)
	 {
		*source=IIC_ReadByte(nums);
		source++;
	 }
	 SDA_OUT_0();
	 delay8(2);			 //Stop
	 SCL_OUT_1();
	 delay8(2);
	 SDA_OUT_1();

	 WP_OUT_1();
	 return TRUE;
}


void AT24C0x_Init(void)
{
    EnWr_WPREG();

    HT_GPIOE->IOCFG &= 0xFFC7;                //PE3-PE5 as GPIO
	SET_WP_OUT();
	SET_SCL_OUT();
	SET_SDA_OUT();
	WP_OUT_1();
	SDA_OUT_1();
	SCL_OUT_1();
}


