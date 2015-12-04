/**
  ******************************************************************************
  * @file    stm32f0xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    23-March-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"
#include "stm32f0xx_conf.h"
#include "user_type.h"
#include "user_ir.h"
#include "user_slave_i2c.h"
#include "user_power_measure.h"
#include"uart.h"


extern IR_STRUCT stIr;

/** @addtogroup STM32F0-Discovery_Demo
  * @{
  */

/** @addtogroup STM32F0XX_IT
  * @brief Interrupts driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
//void SysTick_Handler(void)
//{
 // TimingDelay_Decrement(); 
//}

/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

void I2C1_IRQHandler(void)
{
	I2C1_Interrupt_process();	
	I2C_ClearITPendingBit(I2C1, I2C_IT_ADDR|I2C_IT_NACKF|I2C_IT_STOPF|I2C_IT_BERR|I2C_IT_ARLO|I2C_IT_OVR|I2C_IT_PECERR|I2C_IT_TIMEOUT|I2C_IT_TXIS);

}

void TIM1_CC_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM1,TIM_IT_CC1) != RESET)
    {
    	//if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_4) == 0){
		//	GPIO_SetBits(GPIOA,GPIO_Pin_4);
			//M16->CCR1 = 105;
		//}else{
		//	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
		//	//M16->CCR1 = 0;
		//}
        TIM14->CCR1 = 0;
        TIM1->SR &= (uint16_t)(~(uint16_t)TIM_IT_CC1);
    }
}

void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
	u32 Buf = 0;
	u16 *Ptr=(u16 *)stIr.pIrSendBuf;
	u32 LowLevel,HiLevel;
	TIM1->SR &= (uint16_t)(~(uint16_t)TIM_IT_Update);
	if(stIr.IrStatus == IR_SENDING)
	{
		stIr.IrSendIdx++;
		if(Ptr[stIr.IrSendIdx]==0xffff)
		{
			Buf = Ptr[stIr.IrSendIdx+2];
			Buf <<= 16;
			Buf += Ptr[stIr.IrSendIdx+1];
			LowLevel = Buf>>2;
			stIr.IrSendIdx += 2;
		}
		else
		{
		   LowLevel =  Ptr[stIr.IrSendIdx]>>2;
		}

		stIr.IrSendIdx++;
		if(Ptr[stIr.IrSendIdx]==0xffff)
		{
			Buf = Ptr[stIr.IrSendIdx+2];
			Buf <<= 16;
			Buf += Ptr[stIr.IrSendIdx+1];
			HiLevel = Buf>>2;
			stIr.IrSendIdx += 2;
		}
		else
		{
		   HiLevel =  Ptr[stIr.IrSendIdx]>>2;
		}
		Buf = HiLevel;
		Buf += LowLevel;
		if(stIr.IrSendIdx<=stIr.IrSendLen)	 
		{
			//设置脉冲周期
			TIM1->ARR = (uint32_t)Buf;
			//产生红外载波信号的宽度
			TIM1->CCR1 = (uint32_t)LowLevel;	
			//产生38KHZ红外载波信号
			if(Buf>0)
			{
				TIM14->CCR1 = (uint32_t)stIr.H_TimerLoadValue;
			}
		}
		else//红外数据发送完毕
		{
			if((stIr.IrStatus != IR_SEDN_START)&&(stIr.IrStatus != IR_FORCE_LEARN)){
				stIr.IrStatus = IR_SENDING_END;
			}
			stIr.IrSendIdx =0;
			stIr.IrSendLen = 0;
			Ir_Send_Disable();
		} 
	}
	
}


void TIM14_IRQHandler(void)
{

	TIM_ClearFlag(TIM14,TIM_IT_Update);	
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_CC4) != RESET)
    {
    	if(stIr.IrStatus == IR_FORCE_LEARN){	
			stIr.IrStatus = IR_FORCE_LEARNING;
			I2C_ITConfig(I2C1,I2C_IT_ERRI|I2C_IT_ADDRI|I2C_IT_NACKI|I2C_IT_STOPI|I2C_IT_RXI|I2C_IT_TXI|I2C_IT_TCI,DISABLE);
			I2C_Cmd(I2C1,DISABLE);
		}else if(stIr.IrStatus == IR_LEARN_START){
			if((stIr.IrStatus != IR_SEDN_START)&&(stIr.IrStatus != IR_FORCE_LEARN)){
				stIr.IrStatus = IR_LEARNING;	
			}
		}
        IrCapture();
		TIM_ClearFlag(TIM3,TIM_FLAG_CC4);
    }

	if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET)
	{
		TIM_ClearFlag(TIM3,TIM_IT_Update);
		if((stIr.IrStatus == IR_LEARNING)||(stIr.IrStatus == IR_FORCE_LEARNING))
		{
			stIr.IrLearninExTimerCnt++;
		}
	}
}


void TIM16_IRQHandler(void)
{
#if 0
	if(TIM_GetITStatus(TIM14,TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIM14,TIM_IT_Update);
		if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_4) == 0){
			GPIO_SetBits(GPIOA,GPIO_Pin_4);
			//M16->CCR1 = 105;
		}else{
			GPIO_ResetBits(GPIOA,GPIO_Pin_4);
			//M16->CCR1 = 0;
		}
	}
#else
	if(TIM_GetITStatus(TIM16,TIM_IT_Update) != RESET){
			TIM_ClearITPendingBit(TIM16,TIM_IT_Update);
			PowerTimerInt();
			Ir_Test_Learn_End();
	}
#endif
}




void EXTI2_3_IRQHandler(void)
{
	EXTI_ClearITPendingBit(EXTI_Line2);
	PowerIntProcess();
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
