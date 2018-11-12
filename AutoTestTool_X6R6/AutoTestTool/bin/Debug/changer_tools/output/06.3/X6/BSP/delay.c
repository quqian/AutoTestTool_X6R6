#include "ht6xxx_lib.h"
#include "delay.h"





void Delay_mSec(int mSec)
{
	uint16_t	i;
	while (mSec--)
	{
		for (i=0; i<(2200); i++)
		{
			__NOP();
			__NOP();
			__NOP();
			__NOP();
		}
	}
}

void Delay_100uSec(int count)
{
    uint16_t	i;
    for (i=0; i<192; i++)
    {
            __NOP();
			__NOP();
			__NOP();
			__NOP();
    }
}

