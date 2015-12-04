#include"user_i2c.h"
#include"stm32f0xx_i2c.h"
#include"stm32f0xx_rcc.h"


#define I2C_MY_ADDR          0xA8
#define EEPROM_ADDR             0xA4
#define I2C_SLAVE_ADDR       0xe0

extern void UART_Send(unsigned char* Data,unsigned char len);
extern void delay_time(int i);
void my_i2c_Init(void)
{
  I2C_InitTypeDef I2C_InitStructure;
 
  RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);
 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
 
  I2C_InitStructure.I2C_Mode = I2C_Mode_SMBusHost;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
  I2C_InitStructure.I2C_DigitalFilter = 0x01;
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_Timing = 0x6100000;
  I2C_Init(I2C1,&I2C_InitStructure);
 
  I2C_Cmd(I2C1, ENABLE);
}

void I2C_Read_Ir_Data(unsigned char * pBuffer,unsigned char cmd,unsigned short numByteToRead)
{
	int read_Num;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET);
	I2C_TransferHandling(I2C1, I2C_SLAVE_ADDR, numByteToRead,  I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
	 for(read_Num = 0; read_Num < numByteToRead; read_Num++)
  	{
   		 while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);
    	 pBuffer[read_Num] = I2C_ReceiveData(I2C1);
  	}
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) == RESET);
}

void I2C_Read_FromSlave(unsigned char * pBuffer,unsigned char cmd,unsigned char numByteToRead)
{
	int read_Num;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET);
	I2C_TransferHandling(I2C1,I2C_SLAVE_ADDR, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);
	I2C_SendData(I2C1, cmd);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TC) == RESET);
	I2C_TransferHandling(I2C1, I2C_SLAVE_ADDR, numByteToRead,  I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
	 for(read_Num = 0; read_Num < numByteToRead; read_Num++)
  	{
   		 while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);
    	 pBuffer[read_Num] = I2C_ReceiveData(I2C1);
  	}
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) == RESET);
}



void I2C_Write_ToSlave(unsigned char * pBuffer,unsigned char cmd,unsigned char numByteToWrite)
{

	int write_Num;
#if 1
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET);
	I2C_TransferHandling(I2C1, I2C_SLAVE_ADDR, 1, I2C_Reload_Mode, I2C_Generate_Start_Write);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);
	 I2C_SendData(I2C1, cmd);
	 while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TCR) == RESET);
	 	I2C_TransferHandling(I2C1, I2C_SLAVE_ADDR, numByteToWrite, I2C_AutoEnd_Mode, I2C_No_StartStop);
		for(write_Num = 0; write_Num < numByteToWrite; write_Num++)
  		{
	   	 	while(I2C_GetFlagStatus(I2C1,I2C_FLAG_TXIS) == RESET);
			I2C_SendData(I2C1, pBuffer[write_Num]);
    	}
	
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) == RESET);
#else
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET);
	I2C_TransferHandling(I2C1,I2C_SLAVE_ADDR,numByteToWrite+1,I2C_AutoEnd_Mode,I2C_Generate_Start_Write);
	UART_send_byte('2');
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);
	//I2C_SlaveAddressConfig(I2C1,I2C_SLAVE_ADDR);
	//I2C_MasterRequestConfig(I2C1,I2C_Direction_Transmitter);
	I2C_SendData(I2C1,cmd);
	UART_send_byte('4');
	while(I2C_GetFlagStatus(I2C1, I2C_ISR_TXIS) != RESET);
	UART_send_byte('5');
	for(write_Num = 0; write_Num < numByteToWrite; write_Num++)
  	{
		I2C_SendData(I2C1, pBuffer[write_Num]);
	    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);
    }
	UART_send_byte('6');
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) == RESET);
#endif
}


