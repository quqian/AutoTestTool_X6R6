#ifndef __CMU_H__
#define __CMU_H__

#include "includes.h"

void SystemClock_Config(void);

void SwitchTo_Fhrc(void);

void SwitchTo_Flrc(void);

void close_hrc(void);

void run_osc(void);

#endif
