#ifndef __USER_I2C_H__
#define __USER_I2C_H__

void my_i2c_Init(void);
void I2C_Read_Ir_Data(unsigned char * pBuffer,unsigned char cmd,unsigned short numByteToRead);
void I2C_Read_FromSlave(unsigned char * pBuffer,unsigned char cmd,unsigned char numByteToRead);
void I2C_Write_ToSlave(unsigned char * pBuffer,unsigned char cmd,unsigned char numByteToWrite);


#endif



