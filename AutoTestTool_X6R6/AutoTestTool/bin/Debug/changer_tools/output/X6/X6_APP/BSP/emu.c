#include "emu.h"

EMUC emu_ecr_reg;
uint32_t emu_var_checksum;


uint32_t Read_EPADR(uint16_t address);
uint16_t Read_ECADR(uint16_t address);
void Write_ECADR(uint16_t address, uint16_t data);

void emu_var_cal(void);
void get_emu_var_default(void);


void EMU_Init(void)
{
    EnWr_WPREG();

	HT_GPIOF->IOCFG |= 0x0001;             //PF0 as PFOUT
	HT_EMUECA->EMUIF = 0x0000;

	if((HT_PMU->PMUSTA&0x01) != 0)            //normal
	{
		HT_CMU->CLKCTRL0 |= 0x8000;            //enable EMU

        //加载默认参数
        get_emu_var_default();

        //写参数到寄存器
		emu_var_cal();

		emu_var_checksum = Read_EPADR(EPR_Checksum);
	}
}

void EMU_Proc(void)
{
	if(Read_EPADR(EPR_Checksum)!=emu_var_checksum){
		//写参数到寄存器
		emu_var_cal();
		
		emu_var_checksum = Read_EPADR(EPR_Checksum);
	}
}

void emu_var_cal(void)
{
	Write_ECADR(VAR_EMUSR,            emu_ecr_reg.ECR.EMUSR);
	Write_ECADR(VAR_EMUIE,            emu_ecr_reg.ECR.EMUIE);
	Write_ECADR(VAR_EMUIF,            emu_ecr_reg.ECR.EMUIF);
	Write_ECADR(VAR_GP1,              emu_ecr_reg.ECR.GP1);
	Write_ECADR(VAR_GQ1,              emu_ecr_reg.ECR.GQ1);
	Write_ECADR(VAR_GS1,              emu_ecr_reg.ECR.GS1);
	Write_ECADR(VAR_GPhs1,            emu_ecr_reg.ECR.GPhs1);
	Write_ECADR(VAR_GP2,              emu_ecr_reg.ECR.GP2);
	Write_ECADR(VAR_GQ2,              emu_ecr_reg.ECR.GQ2);
	Write_ECADR(VAR_GS2,              emu_ecr_reg.ECR.GS2);
	Write_ECADR(VAR_GPhs2,            emu_ecr_reg.ECR.GPhs2);
	Write_ECADR(VAR_QPhsCal,          emu_ecr_reg.ECR.QPhsCal);
	Write_ECADR(VAR_I2Gain,           emu_ecr_reg.ECR.I2Gain);
	Write_ECADR(VAR_I1Off,            emu_ecr_reg.ECR.I1Off);
	Write_ECADR(VAR_I2Off,            emu_ecr_reg.ECR.I2Off);
	Write_ECADR(VAR_UOff,             emu_ecr_reg.ECR.UOff);
	Write_ECADR(VAR_PStart,           emu_ecr_reg.ECR.PStart);
	Write_ECADR(VAR_QStart,           emu_ecr_reg.ECR.QStart);
	Write_ECADR(VAR_SStart,           emu_ecr_reg.ECR.SStart);
	Write_ECADR(VAR_HFconst,          emu_ecr_reg.ECR.HFconst);
	Write_ECADR(VAR_ADCCFG,           emu_ecr_reg.ECR.ADCCFG);
	Write_ECADR(VAR_CHNLCR,           emu_ecr_reg.ECR.CHNLCR);
	Write_ECADR(VAR_EMCON,            emu_ecr_reg.ECR.EMCON);
	Write_ECADR(VAR_PFCnt,            emu_ecr_reg.ECR.PFCnt);
	Write_ECADR(VAR_QFCnt,            emu_ecr_reg.ECR.QFCnt);
	Write_ECADR(VAR_SFCnt,            emu_ecr_reg.ECR.SFCnt);
	Write_ECADR(VAR_ADCCON,           emu_ecr_reg.ECR.ADCCON);
	Write_ECADR(VAR_IPTAMP,           emu_ecr_reg.ECR.IPTAMP);
	Write_ECADR(VAR_ICHK,             emu_ecr_reg.ECR.ICHK);
	Write_ECADR(VAR_EMUCTRL,          emu_ecr_reg.ECR.EMUCTRL);
	Write_ECADR(VAR_P1OFFSET,         emu_ecr_reg.ECR.P1OFFSET);
	Write_ECADR(VAR_P2OFFSET,         emu_ecr_reg.ECR.P2OFFSET);
	Write_ECADR(VAR_Q1OFFSET,         emu_ecr_reg.ECR.Q1OFFSET);
	Write_ECADR(VAR_Q2OFFSET,         emu_ecr_reg.ECR.Q2OFFSET);
	Write_ECADR(VAR_I1RMSOFFSET,      emu_ecr_reg.ECR.I1RMSOFFSET);
	Write_ECADR(VAR_I2RMSOFFSET,      emu_ecr_reg.ECR.I2RMSOFFSET);
	Write_ECADR(VAR_URMSOFFSET,       emu_ecr_reg.ECR.URMSOFFSET);
	Write_ECADR(VAR_RosiCtrl,         emu_ecr_reg.ECR.RosiCtrl);
	Write_ECADR(VAR_ANA_control,      emu_ecr_reg.ECR.ANA_control);
	Write_ECADR(VAR_UCONST,           emu_ecr_reg.ECR.UCONST);
	Write_ECADR(VAR_LpIdleTime,       emu_ecr_reg.ECR.LpIdleTime);
	Write_ECADR(VAR_USAGLVL,          emu_ecr_reg.ECR.USAGLVL);
	Write_ECADR(VAR_USagCyc,          emu_ecr_reg.ECR.USagCyc);
	Write_ECADR(VAR_UOVLVL,           emu_ecr_reg.ECR.UOVLVL);
	Write_ECADR(VAR_OvCyc,            emu_ecr_reg.ECR.OvCyc);
	Write_ECADR(VAR_IOVLVL,           emu_ecr_reg.ECR.IOVLVL);
	Write_ECADR(VAR_ZXILVL,           emu_ecr_reg.ECR.ZXILVL);
	Write_ECADR(VAR_PDataCpH,         emu_ecr_reg.ECR.PDataCpH);
	Write_ECADR(VAR_PDataCpL,         emu_ecr_reg.ECR.PDataCpL);
	Write_ECADR(VAR_QDataCpH,         emu_ecr_reg.ECR.QDataCpH);
	Write_ECADR(VAR_QDataCpL,         emu_ecr_reg.ECR.QDataCpL);
	Write_ECADR(VAR_SDataCpH,         emu_ecr_reg.ECR.SDataCpH);
	Write_ECADR(VAR_SDataCpL,         emu_ecr_reg.ECR.SDataCpL);
	Write_ECADR(VAR_FilterCtrl,       emu_ecr_reg.ECR.FilterCtrl);
	Write_ECADR(VAR_TUgain,           emu_ecr_reg.ECR.TUgain);
	Write_ECADR(VAR_TI1gain,          emu_ecr_reg.ECR.TI1gain);
	Write_ECADR(VAR_TI2gain,          emu_ecr_reg.ECR.TI2gain);
	Write_ECADR(VAR_UTCcoffA,         emu_ecr_reg.ECR.UTCcoffA);
	Write_ECADR(VAR_UTCcoffB,         emu_ecr_reg.ECR.UTCcoffB);
	Write_ECADR(VAR_UTCcoffC,         emu_ecr_reg.ECR.UTCcoffC);
	Write_ECADR(VAR_I1TCcoffA,        emu_ecr_reg.ECR.I1TCcoffA);
	Write_ECADR(VAR_I1TCcoffB,        emu_ecr_reg.ECR.I1TCcoffB);
	Write_ECADR(VAR_I1TCcoffC,        emu_ecr_reg.ECR.I1TCcoffC);
	Write_ECADR(VAR_I2TCcoffA,        emu_ecr_reg.ECR.I2TCcoffA);
	Write_ECADR(VAR_I2TCcoffB,        emu_ecr_reg.ECR.I2TCcoffB);
	Write_ECADR(VAR_I2TCcoffC,        emu_ecr_reg.ECR.I2TCcoffC);
	Write_ECADR(VAR_LoadDataCp,       emu_ecr_reg.ECR.LoadDataCp);

}


void get_emu_var_default(void)
{
	emu_ecr_reg.ECR.EMUSR              =Read_ECADR(VAR_EMUSR);
	emu_ecr_reg.ECR.EMUIE              =Read_ECADR(VAR_EMUIE);
	emu_ecr_reg.ECR.EMUIF              =Read_ECADR(VAR_EMUIF);
	emu_ecr_reg.ECR.GP1                =Read_ECADR(VAR_GP1);
	emu_ecr_reg.ECR.GQ1                =Read_ECADR(VAR_GQ1);
	emu_ecr_reg.ECR.GS1                =Read_ECADR(VAR_GS1);
	emu_ecr_reg.ECR.GPhs1              =Read_ECADR(VAR_GPhs1);
	emu_ecr_reg.ECR.GP2                =Read_ECADR(VAR_GP2);
	emu_ecr_reg.ECR.GQ2                =Read_ECADR(VAR_GQ2);
	emu_ecr_reg.ECR.GS2                =Read_ECADR(VAR_GS2);
	emu_ecr_reg.ECR.GPhs2              =Read_ECADR(VAR_GPhs2);
	emu_ecr_reg.ECR.QPhsCal            =Read_ECADR(VAR_QPhsCal);
	emu_ecr_reg.ECR.I2Gain             =Read_ECADR(VAR_I2Gain);
	emu_ecr_reg.ECR.I1Off              =Read_ECADR(VAR_I1Off);
	emu_ecr_reg.ECR.I2Off              =Read_ECADR(VAR_I2Off);
	emu_ecr_reg.ECR.UOff               =Read_ECADR(VAR_UOff);
	emu_ecr_reg.ECR.PStart             =Read_ECADR(VAR_PStart);
	emu_ecr_reg.ECR.QStart             =Read_ECADR(VAR_QStart);
	emu_ecr_reg.ECR.SStart             =Read_ECADR(VAR_SStart);
	emu_ecr_reg.ECR.HFconst            =Read_ECADR(VAR_HFconst);
	emu_ecr_reg.ECR.ADCCFG             =Read_ECADR(VAR_ADCCFG);
	emu_ecr_reg.ECR.CHNLCR             =Read_ECADR(VAR_CHNLCR);
	emu_ecr_reg.ECR.EMCON              =Read_ECADR(VAR_EMCON);
	emu_ecr_reg.ECR.PFCnt              =Read_ECADR(VAR_PFCnt);
	emu_ecr_reg.ECR.QFCnt              =Read_ECADR(VAR_QFCnt);
	emu_ecr_reg.ECR.SFCnt              =Read_ECADR(VAR_SFCnt);
	emu_ecr_reg.ECR.ADCCON             =Read_ECADR(VAR_ADCCON);
	emu_ecr_reg.ECR.IPTAMP             =Read_ECADR(VAR_IPTAMP);
	emu_ecr_reg.ECR.ICHK               =Read_ECADR(VAR_ICHK);
	emu_ecr_reg.ECR.EMUCTRL            =Read_ECADR(VAR_EMUCTRL);
	emu_ecr_reg.ECR.P1OFFSET           =Read_ECADR(VAR_P1OFFSET);
	emu_ecr_reg.ECR.P2OFFSET           =Read_ECADR(VAR_P2OFFSET);
	emu_ecr_reg.ECR.Q1OFFSET           =Read_ECADR(VAR_Q1OFFSET);
	emu_ecr_reg.ECR.Q2OFFSET           =Read_ECADR(VAR_Q2OFFSET);
	emu_ecr_reg.ECR.I1RMSOFFSET        =Read_ECADR(VAR_I1RMSOFFSET);
	emu_ecr_reg.ECR.I2RMSOFFSET        =Read_ECADR(VAR_I2RMSOFFSET);
	emu_ecr_reg.ECR.URMSOFFSET         =Read_ECADR(VAR_URMSOFFSET);
	emu_ecr_reg.ECR.RosiCtrl           =Read_ECADR(VAR_RosiCtrl);
	emu_ecr_reg.ECR.ANA_control        =Read_ECADR(VAR_ANA_control);
	emu_ecr_reg.ECR.UCONST             =Read_ECADR(VAR_UCONST);
	emu_ecr_reg.ECR.LpIdleTime         =Read_ECADR(VAR_LpIdleTime);
	emu_ecr_reg.ECR.USAGLVL            =Read_ECADR(VAR_USAGLVL);
	emu_ecr_reg.ECR.USagCyc            =Read_ECADR(VAR_USagCyc);
	emu_ecr_reg.ECR.UOVLVL             =Read_ECADR(VAR_UOVLVL);
	emu_ecr_reg.ECR.OvCyc              =Read_ECADR(VAR_OvCyc);
	emu_ecr_reg.ECR.IOVLVL             =Read_ECADR(VAR_IOVLVL);
	emu_ecr_reg.ECR.ZXILVL             =Read_ECADR(VAR_ZXILVL);
	emu_ecr_reg.ECR.PDataCpH           =Read_ECADR(VAR_PDataCpH);
	emu_ecr_reg.ECR.PDataCpL           =Read_ECADR(VAR_PDataCpL);
	emu_ecr_reg.ECR.QDataCpH           =Read_ECADR(VAR_QDataCpH);
	emu_ecr_reg.ECR.QDataCpL           =Read_ECADR(VAR_QDataCpL);
	emu_ecr_reg.ECR.SDataCpH           =Read_ECADR(VAR_SDataCpH);
	emu_ecr_reg.ECR.SDataCpL           =Read_ECADR(VAR_SDataCpL);
	emu_ecr_reg.ECR.FilterCtrl         =Read_ECADR(VAR_FilterCtrl);
	emu_ecr_reg.ECR.TUgain             =Read_ECADR(VAR_TUgain);
	emu_ecr_reg.ECR.TI1gain            =Read_ECADR(VAR_TI1gain);
	emu_ecr_reg.ECR.TI2gain            =Read_ECADR(VAR_TI2gain);
	emu_ecr_reg.ECR.UTCcoffA           =Read_ECADR(VAR_UTCcoffA);
	emu_ecr_reg.ECR.UTCcoffB           =Read_ECADR(VAR_UTCcoffB);
	emu_ecr_reg.ECR.UTCcoffC           =Read_ECADR(VAR_UTCcoffC);
	emu_ecr_reg.ECR.I1TCcoffA          =Read_ECADR(VAR_I1TCcoffA);
	emu_ecr_reg.ECR.I1TCcoffB          =Read_ECADR(VAR_I1TCcoffB);
	emu_ecr_reg.ECR.I1TCcoffC          =Read_ECADR(VAR_I1TCcoffC);
	emu_ecr_reg.ECR.I2TCcoffA          =Read_ECADR(VAR_I2TCcoffA);
	emu_ecr_reg.ECR.I2TCcoffB          =Read_ECADR(VAR_I2TCcoffB);
	emu_ecr_reg.ECR.I2TCcoffC          =Read_ECADR(VAR_I2TCcoffC);
	emu_ecr_reg.ECR.LoadDataCp         =Read_ECADR(VAR_LoadDataCp);

	emu_ecr_reg.ECR.CHNLCR = 0x0607;
    emu_ecr_reg.ECR.ADCCON = 0x0028;      //8倍增益
	//emu_ecr_reg.ECR.ADCCON = 0x0014;		//2倍增益
    //emu_ecr_reg.ECR.ADCCON = 0x003C;		//16倍增益
	emu_ecr_reg.ECR.EMUCTRL = 0x03F2;
	emu_ecr_reg.ECR.ANA_control = 0xA7A6;
	//emu_ecr_reg.ECR.ADCCFG = 0x2003;//通道1计量
	emu_ecr_reg.ECR.ADCCFG = 0x2013;	//通道2计量
	emu_ecr_reg.ECR.RosiCtrl = 0x4000;
    emu_ecr_reg.ECR.I2Gain = 0;    //通道2自动补偿系数

	//初始化值
	emu_ecr_reg.ECR.HFconst   = 74;	//脉频输出设置

	emu_ecr_reg.ECR.Emu_Krms = 0.000114893;		//电压系数
	emu_ecr_reg.ECR.Emu_Kp   = 1.13382E-05;		//功率系数初始化 3.69746E-06  1.3066E-05
	emu_ecr_reg.ECR.Emu_Kims = 2.95606E-06;		//电流系数 6.7379E-07

	//通道1
	emu_ecr_reg.ECR.GP1 = 0xFDA4;
	emu_ecr_reg.ECR.GQ1 = 0xFDA4;
	emu_ecr_reg.ECR.GS1	= 0xFDA4;
	emu_ecr_reg.ECR.GPhs1 = 0xFE43;
    emu_ecr_reg.ECR.P1OFFSET = 0xFFFB;

	//通道2
	emu_ecr_reg.ECR.GP2 = 0xFE69;
	emu_ecr_reg.ECR.GQ2 = 0xFE69;
	emu_ecr_reg.ECR.GS2	= 0xFE69;
	emu_ecr_reg.ECR.GPhs2 = 0xFE3B;
    emu_ecr_reg.ECR.P2OFFSET = 0xFFFA;
}


uint32_t Read_EPADR(uint16_t address)
{
	uint32_t *ptr;
	ptr = (uint32_t*)(HT_EMU_EPA_BASE + address);
	return ((*ptr)&0xffffffff);
}


uint16_t Read_ECADR(uint16_t address)
{
	uint16_t *ptr;
	ptr = (uint16_t*)(HT_EMU_ECA_BASE + address);
	return ((*ptr)&0xffff);
}


void Write_ECADR(uint16_t address, uint16_t data)
{
	uint16_t *ptr;
	ptr = (uint16_t*)(HT_EMU_ECA_BASE + address);
	*ptr = data;
}


//读取电压 分辨率：0.1V
uint16_t ReadRMSU(void)
{
    uint32_t temp_urms = Read_EPADR(EPR_FastRMSU);

	return temp_urms*(emu_ecr_reg.ECR.Emu_Krms)*10;
}

//读取电流：分辨率mA
uint16_t ReadRMSI(uint8_t channl)
{
    uint32_t temp_i1rms;
	if(channl == 2){
        temp_i1rms = Read_EPADR(EPR_FastRMSI1);
    }
    else if(channl == 1){
        temp_i1rms = Read_EPADR(EPR_FastRMSI2);
    }
	return temp_i1rms*(emu_ecr_reg.ECR.Emu_Kims)*1000;
}


//读取功率：分辨率0.1W
uint16_t ReadPower(uint8_t channl)
{
    uint32_t temp_powerp;

    if(channl == 2){
        temp_powerp = Read_EPADR(EPR_FastPowerP1);
    }
    else if(channl == 1){
        temp_powerp = Read_EPADR(EPR_FastPowerP2);
    }

	if(temp_powerp >= 0x80000000){
		temp_powerp = 0x100000000 - temp_powerp;
	}

	return temp_powerp*(emu_ecr_reg.ECR.Emu_Kp)*10;
}



