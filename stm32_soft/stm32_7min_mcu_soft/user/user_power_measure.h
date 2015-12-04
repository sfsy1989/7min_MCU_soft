#ifndef __USER_POWER_MEASURE_H__
#define __USER_POWER_MEASURE_H__

#define    _POWER_MEASURE

extern unsigned short     U16_AC_P;               //¹¦ÂÊÖµ 1000.0W
extern void Power_Measure_Init(void);
extern void  PowerTimerInt(void);
extern void PowerIntProcess(void);
extern void HLW8012_Measure_P(void);


#endif
