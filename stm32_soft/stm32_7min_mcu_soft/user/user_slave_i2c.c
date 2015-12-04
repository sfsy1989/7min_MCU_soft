#include "user_slave_i2c.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_i2c.h"
#include "stm32f0xx_gpio.h"
#include "user_ir.h"
#include "user_power_measure.h"
#include "user_temperature_measure.h"
#include<string.h>
#include "user_type.h"
#include "stm32f0xx_misc.h"
#include"uart.h"


I2C_STRUCT  stI2c;
extern IR_STRUCT stIr;
u8 Int_Event = 0;

#define IR_SEND_REQ         0x01        //红外发送请求
#define IR_CODE_ADAPT       0x02        //遥控器适配请求
#define IR_RESP_LEARING     0x03        //响应红外学习结果
#define IR_LEN_LIMIT        0x04        //红外码长度限制
#define IR_GET_TEMP_HUM_RQ  0x05        //获取温湿度请求
#define IR_GET_TEMP_HUM     0x06        //获取温度和湿度
#define IR_OPERATE_RELAY    0x07        //操作寄存器
#define IR_READ_INT_EVENT   0x08       //读取中断事件



void I2C_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStruct; 

	/* Enable GPIOA clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	
		/* Connect PXx to I2C_SCL*/
	GPIO_PinAFConfig( GPIOA , GPIO_PinSource9, GPIO_AF_4); 
	/* Connect PXx to I2C_SDA*/
	GPIO_PinAFConfig( GPIOA ,GPIO_PinSource10, GPIO_AF_4);
	/*!< GPIO configuration */ 
	/*!< Configure sEE_I2C pins: SCL */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;//GPIO_Mode_IN
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;//open-drain
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA , &GPIO_InitStruct);

	/*!< Configure sEE_I2C pins: SDA */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA , &GPIO_InitStruct);


}

void I2C_NVIC_Config()
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = I2C1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void I2c_Slave_Variable_Init()
{
	stI2c.I2c_ReciveAddrMatchFlag = 0;
    stI2c.I2c_SendAddrMatchFlag = 0;
    stI2c.CmdCode = 0;
    stI2c.ReciveIndex = 0;
    stI2c.SendIndex = 0;
    stI2c.SendLen = 0;
    stI2c.ReciveLen =0;
}
#if 0

static void Restore_Configuration(void)
{
  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;
  
  /* SYSCLK, HCLK, PCLK configuration ----------------------------------------*/    
  /* Enable HSE */    
  RCC_HSEConfig(RCC_HSE_ON);
 
  /* Wait till HSE is ready and if Time out is reached exit */
  HSEStatus = RCC_WaitForHSEStartUp();

  if (HSEStatus == (uint32_t)0x01)
  {
    /* Enable Prefetch Buffer */
    FLASH_SetLatency(FLASH_Latency_1);
    
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
    
    /* PCLK = HCLK */
    RCC_PCLKConfig(RCC_HCLK_Div1);
        
    /*  PLL configuration:  = HSE *  6 = 48 MHz */
    RCC_PREDIV1Config(RCC_PREDIV1_Div1);
    RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_CFGR_PLLMULL6);
    
    /* Enable PLL */
    RCC_PLLCmd(ENABLE);
    
    /* PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  } 
}



#define SLAVESTRUCTURE            I2C1_DevStructure


void I2C_Slave_Config2(void)
{
  RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);
	  /* Deinitialize I2Cx Device */ 
  CPAL_I2C_DeInit(&SLAVESTRUCTURE); 
  
  /* Initialize CPAL I2C structure parameters values */
  CPAL_I2C_StructInit(&SLAVESTRUCTURE);

  SLAVESTRUCTURE.wCPAL_Options =  CPAL_OPT_I2C_NACK_ADD | CPAL_OPT_I2C_WAKEUP_STOP;
  SLAVESTRUCTURE.CPAL_ProgModel = CPAL_PROGMODEL_INTERRUPT;
    /* Configure Own address 1 */
  SLAVESTRUCTURE.pCPAL_I2C_Struct->I2C_OwnAddress1 = 0xe0;
  
  /* Set I2C Speed */
  SLAVESTRUCTURE.pCPAL_I2C_Struct->I2C_Timing = 0x00630000;
  
  /* Select Slave Mode */ 
  SLAVESTRUCTURE.CPAL_Mode = CPAL_MODE_SLAVE; 
  
  /* Initialize I2Cx Device*/
  CPAL_I2C_Init(&SLAVESTRUCTURE);
  I2C_ITConfig(I2C1,I2C_IT_ERRI|I2C_IT_ADDRI|I2C_IT_NACKI|I2C_IT_STOPI|I2C_IT_TCI|I2C_IT_RXI|I2C_IT_TXI,ENABLE);
  I2C_Cmd(I2C1,ENABLE);
}

#endif

void I2C_Slave_Config(void)
{	
	I2C_InitTypeDef I2C_InitStructure;	
    I2C_DeInit(I2C1);
	RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter = 0x01;
	I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDR;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_Timing = 0x00610000;//0x10950000;//0x00630000;
	//I2C_InitStructure.I2C_Timing = 0x10815E89;        //Master 100khz 48Mhz A_Filter_off R_time=F_time=100ns
    //I2C_InitStructure.I2C_Timing = 0x00702F3E;        //Master 400khz 48Mhz A_Filter_off R_time=F_time=50ns
	I2C_Init(I2C1, &I2C_InitStructure);
	I2C_ITConfig(I2C1,I2C_IT_ERRI|I2C_IT_ADDRI|I2C_IT_NACKI|I2C_IT_STOPI|I2C_IT_RXI|I2C_IT_TXI|I2C_IT_TCI,ENABLE);
    I2C_Cmd(I2C1,ENABLE);	
}

void I2c_Slave_Init(void)
{
	I2c_Slave_Variable_Init();
	I2C_NVIC_Config();
	I2C_GPIO_Configuration();
	I2C_Slave_Config(); 	
}


static void I2C_Get_Answer_Data(void)
{
    u8 Block=0;
    u16 Temp;
    switch(stI2c.CmdCode)
    {
        case IR_RESP_LEARING:
			if(stIr.IrSendIdx<stIr.IrSendLen)
            {
                Temp = stIr.IrSendLen - stIr.IrSendIdx;
                Block=Temp/(I2C_RX_DATA_SIZE-2);
                if(((Temp%(I2C_RX_DATA_SIZE-2))==0)&&(Block>0))
                        Block -=1;
                stIr.RemainFrame= Block;
                stIr.BigDataStartFlag =1;
                
            }
            break;
        case IR_GET_TEMP_HUM:
            stI2c.SendIndex = 0;
            stI2c.SendLen = 12;
			stI2c.Buf[0] = 0x80;//获取数据成功
            memcpy(&stI2c.Buf[1],(u8 *)(&Temperature),4);
            memcpy(&stI2c.Buf[5],(u8 *)(&Humidity),4);
            memcpy(&stI2c.Buf[9],(u8 *)(&U16_AC_P),2);
			stI2c.Buf[11] = (((GPIO_ReadOutputData(GPIOA)&GPIO_Pin_5)>>5)<<1)|(((GPIO_ReadOutputData(GPIOA)&GPIO_Pin_6)>>6));
			break;
        case IR_READ_INT_EVENT:
            stI2c.SendIndex = 0;
            stI2c.SendLen = 4;
            stI2c.Buf[0] = 0x80;
			stI2c.Buf[1] = Int_Event;
			if(Int_Event == 0x1){
				if(stIr.IrSendLen != 0){
					//stI2c.Buf[2] = (u8)(stIr.IrSendLen>>8);
					//stI2c.Buf[3] = (u8)(stIr.IrSendLen);
					memcpy(&stI2c.Buf[2],(u8 *)(&stIr.IrSendLen),2);
				}else{
					stI2c.Buf[2] = 0;
					stI2c.Buf[3] = 0;
				}
			}else if(Int_Event == 0x02){
				stI2c.Buf[2] = 0;
				stI2c.Buf[3] = (((GPIO_ReadOutputData(GPIOA)&GPIO_Pin_5)>>5)<<1)|(((GPIO_ReadOutputData(GPIOA)&GPIO_Pin_6)>>6));	
			}else{
				stI2c.Buf[2] = 0;
				stI2c.Buf[3] = 0;
			}
			Int_Event = 0;
            break;	
        default:break;
    }
}


static void I2C_Get_Request_Data(void)
{
    switch(stI2c.CmdCode)
    {
        case IR_SEND_REQ:
            if(stI2c.Buf[0]==0x00)
                stIr.IrSendIdx =0;
            memcpy((u8*)(&stIr.pIrSendBuf[stIr.IrSendIdx]),(u8 *)(&stI2c.Buf[1]),stI2c.ReciveIndex-1);
            stIr.IrSendIdx += (stI2c.ReciveIndex-1);
            if(stI2c.Buf[0]==0xff)//红外数据接收完，启动红外发送
            {
                stIr.IrSendLen = stIr.IrSendIdx ;
                stIr.IrSendIdx =0;
                //memcpy(stIr.IrData,stI2c.Buf, stI2c.ReciveLen);
                stIr.IrStatus = IR_SEDN_START;
            }
            break;
        case IR_CODE_ADAPT:  //请求遥控器适配
         	stIr.IrStatus = IR_FORCE_LEARN;
            break;
		case IR_LEN_LIMIT:
			stIr.LearnIrDataLenMin = (stI2c.Buf[0]<<8)|stI2c.Buf[1];
			stIr.LearnIrDataLenMax = (stI2c.Buf[2]<<8)|stI2c.Buf[3];
			break;
        case IR_OPERATE_RELAY:
			switch((stI2c.Buf[0])&0x07){
				case 0x0:
					GPIO_ResetBits(GPIOA,GPIO_Pin_5);
					GPIO_ResetBits(GPIOA,GPIO_Pin_6);
					break;
				case 0x1:
					GPIO_SetBits(GPIOA,GPIO_Pin_5);
					GPIO_ResetBits(GPIOA,GPIO_Pin_6);
					break;
				case 0x2:
					GPIO_ResetBits(GPIOA,GPIO_Pin_5);
					GPIO_SetBits(GPIOA,GPIO_Pin_6);
					break;
				case 0x3:
					GPIO_SetBits(GPIOA,GPIO_Pin_5);
					GPIO_SetBits(GPIOA,GPIO_Pin_6);
					break;
				default:
					break;
			}
            break;
        default:break;
    }
}


void I2C1_Interrupt_process(void)
{
	u8 DataBuf;  
	//i2c rx 
	//if((I2C1->ISR & 0xff00) != 0){
	//	I2C1->ISR &= 0xffff00ff;
	//}
	if((I2C1->ISR&I2C_FLAG_ADDR) != RESET)// 匹配接收地址
	{
		I2C1->ISR |= 0x01;
		stI2c.I2c_ReciveAddrMatchFlag = 1;
		
	}else if((I2C1->ISR&I2C_FLAG_STOPF) != RESET)//检测到停止信号
	{
		I2C_ClearFlag(I2C1,I2C_FLAG_STOPF);
		I2C_GenerateSTOP(I2C1,DISABLE);
		I2C_Get_Request_Data();
	}
	else if((I2C1->ISR&I2C_FLAG_RXNE) != RESET)//接收到数据
	{
		DataBuf = I2C_ReceiveData(I2C1);
		if(stI2c.I2c_ReciveAddrMatchFlag)
		{
			stI2c.I2c_ReciveAddrMatchFlag = 0;
			stI2c.CmdCode = DataBuf;
			stI2c.ReciveIndex = 0;
			stI2c.SendIndex = 0;
			I2C_Get_Answer_Data();
		}
		else
		{
			stI2c.Buf[stI2c.ReciveIndex++] = DataBuf;
			if(stI2c.ReciveIndex >= I2C_RX_DATA_SIZE)
				stI2c.ReciveIndex = I2C_RX_DATA_SIZE-1;
		}
	}
	else if((I2C1->ISR&I2C_FLAG_TXIS != RESET)||(I2C1->ISR&I2C_FLAG_ADDR != RESET))
	{
		if(stI2c.CmdCode != IR_RESP_LEARING)
		{
			if((stI2c.SendIndex < I2C_RX_DATA_SIZE)&&(stI2c.SendIndex<stI2c.SendLen))
			   I2C_SendData(I2C1,stI2c.Buf[stI2c.SendIndex++]);
			
			else 
			{
					stI2c.SendIndex = 0;
					stI2c.SendLen = 0;				
					I2C_SendData(I2C1,0);
			}
		}
		else if((stIr.IrSendIdx<stIr.IrSendLen))
		{
				if(stIr.BigDataStartFlag)
				{
					if((stIr.IrSendIdx > 60)&&(stIr.IrSendIdx%60 == 1))
						stIr.IrSendIdx--;
					I2C_SendData(I2C1,stIr.RemainFrame);
					stIr.BigDataStartFlag =0;
				}
				else
				{
					I2C_SendData(I2C1,stIr.pIrSendBuf[stIr.IrSendIdx++]);
					if(stIr.IrSendIdx>=stIr.IrSendLen)
					{
						stIr.IrSendIdx = 0;
						stIr.IrSendLen=0;
					}
				}
				
		}
		else
		{
			I2C_SendData(I2C1,1);
		}
	}
	else if((I2C1->ISR&I2C_FLAG_NACKF) != RESET)//应答信号失败
	{
		I2C_ClearFlag(I2C1,I2C_FLAG_NACKF);
	}

}


