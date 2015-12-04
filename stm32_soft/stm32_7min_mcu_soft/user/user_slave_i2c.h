#ifndef __USER_SLAVE_I2C_H__
#define __USER_SLAVE_I2C_H__
#include"user_type.h"

#define I2C_SLAVE_ADDR          0xe0
#define I2C_RX_DATA_SIZE        62


typedef struct
{
	u8 Buf[I2C_RX_DATA_SIZE];
	BOOL I2c_ReciveAddrMatchFlag;
    BOOL I2c_SendAddrMatchFlag;
	#ifdef ADDR_16BITS_EN
    	u16 CmdCode;
	#else
    	u8 CmdCode;
	#endif
    u16 ReciveIndex;
    u16 ReciveLen;
    u16 SendIndex;
    u16 SendLen;
}I2C_STRUCT;


void I2c_Slave_Init(void);
void I2C1_Interrupt_process(void);


#endif
