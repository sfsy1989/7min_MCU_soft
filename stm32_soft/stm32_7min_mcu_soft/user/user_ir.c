#include"user_ir.h"
#include"stm32f0xx_tim.h"
#include<string.h>
#include"stm32f0xx_rcc.h"
#include"user_keypress.h"
#include"stm32f0xx_i2c.h"
#include"stm32f0xx_gpio.h"
#include"stm32f0xx_misc.h"
#include"uart.h"
#include"user_timer.h"

IR_STRUCT stIr;
extern u8 Int_Event;


u8 IrData1[IR_BUF_SIZE] = {0};
u8 IrData2[IR_BUF_SIZE] = {0};



void Ir_Data_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct; 

	stIr.IrStatus =IR_LEARN_START;
    stIr.IrSendIdx = 0;
    stIr.IrSendLen = 0;
	stIr.IrLearnIdx = 0;
	stIr.IrLearnLen = 0;
    stIr.BigDataStartFlag =0;
    stIr.IrLearnResult = 0x02;
    stIr.IrFre = 38000;//hz	


	stIr.pIrLearnBuf = IrData1;
	stIr.pIrSendBuf = IrData2;


	stIr.LearnIrDataLenMax = IR_BUF_SIZE;
	stIr.LearnIrDataLenMin = 50;

	/* Enable GPIOA clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_PinAFConfig( GPIOB ,GPIO_PinSource1, GPIO_AF_1);
	GPIO_PinAFConfig( GPIOA , GPIO_PinSource4, GPIO_AF_4); 

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;//GPIO_Mode_IN
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;//open-drain
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOB , &GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA , &GPIO_InitStruct);
	Ir_Learn_Init();
	Ir_Learn_Enable();
}


void Ir_Learn_Init(void)
{
	TIM_TimeBaseInitTypeDef timeBaseInit;
	NVIC_InitTypeDef   NVIC_InitStructure;
	TIM_ICInitTypeDef icInit;
	stIr.IrLearnIdx =0;//启动学习
	stIr.IrLearnResult = 0x01;//学习中
	stIr.IrLearnLen = 0;
	stIr.IrLearninExTimerCnt =0;
	TIM_DeInit(TIM3);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	timeBaseInit.TIM_Prescaler = 63;
	timeBaseInit.TIM_Period = 65535;
	timeBaseInit.TIM_CounterMode = TIM_CounterMode_Up;
	timeBaseInit.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3,&timeBaseInit);
	icInit.TIM_Channel = TIM_Channel_4;
	icInit.TIM_ICFilter = 0x0;
	icInit.TIM_ICPolarity = TIM_ICPolarity_Falling;
	icInit.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	icInit.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInit(TIM3,&icInit);
}

void Ir_Learn_Enable(void)
{
	TIM_GenerateEvent(TIM3,TIM_EventSource_Update);
    TIM_ClearFlag(TIM3,TIM_FLAG_CC4|TIM_FLAG_Update);
    TIM_ITConfig(TIM3, TIM_IT_CC4|TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3,ENABLE);	
}


void Ir_Learn_Disable(void)
{
    TIM_ITConfig(TIM3, TIM_IT_CC4|TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM3,DISABLE);		
}


void Ir_Send_Enable(void)
{
	u16 Period;
	u32 Buf=0;
	TIM_TimeBaseInitTypeDef timeBaseInit;
	TIM_OCInitTypeDef ocInit;
	float SysFre;	
	NVIC_InitTypeDef   NVIC_InitStructure;
	u16 *ptr=(u16 *)stIr.pIrSendBuf;
	SysFre = SystemCoreClock;
	stIr.IrStatus = IR_SENDING;
	stIr.IrSendIdx =2;//启动发送
	stIr.IrSendLen /=2;

	Period=(u16)(SysFre/stIr.IrFre);
	stIr.H_TimerLoadValue = Period/4;
	TIM_SetAutoreload(TIM14,Period);
	Buf = ptr[1];
	Buf += ptr[2];
	Buf>>=2;	
	TIM_DeInit(TIM1);
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_UP_TRG_COM_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	timeBaseInit.TIM_Prescaler = 63;
	timeBaseInit.TIM_Period = Buf;
	timeBaseInit.TIM_CounterMode = TIM_CounterMode_Up;
	timeBaseInit.TIM_RepetitionCounter = 0;
	timeBaseInit.TIM_ClockDivision = 0;
	TIM_TimeBaseInit(TIM1,&timeBaseInit);
	TIM_ARRPreloadConfig(TIM1,DISABLE);
	ocInit.TIM_OCIdleState = TIM_OCIdleState_Set;
	ocInit.TIM_OCMode = TIM_OCMode_Timing;
	ocInit.TIM_OCNIdleState = TIM_OCNIdleState_Set;
	ocInit.TIM_OCNPolarity = TIM_OCNPolarity_Low;
	ocInit.TIM_OCPolarity = TIM_OCPolarity_Low;
	ocInit.TIM_OutputNState = TIM_OutputNState_Disable;
	ocInit.TIM_OutputState = TIM_OutputState_Disable;
	ocInit.TIM_Pulse = ptr[1]>>2;
	TIM_OC1Init(TIM1,&ocInit);
	TIM_GenerateEvent(TIM1,TIM_EventSource_Update);
	TIM_ClearITPendingBit(TIM1,TIM_IT_Update|TIM_IT_CC1);
	TIM_ITConfig(TIM1,TIM_IT_Update|TIM_IT_CC1,ENABLE);
	TIM_SetCompare1(TIM14,stIr.H_TimerLoadValue);
	TIM_Cmd(TIM1,ENABLE);	
}


void Ir_Send_Disable(void)
{
	TIM_ITConfig(TIM1,TIM_IT_Update|TIM_IT_CC1,DISABLE);
	TIM_SetCompare1(TIM1,0);
	TIM_Cmd(TIM1,DISABLE);	
}


void IrCapture(void)
{
    u32 TimeCnt=TIM_GetCapture4(TIM3);
    u32 TimerVal;
    u16 *Irdata= (u16 *)stIr.pIrLearnBuf;
    u8 ExTimercnt=stIr.IrLearninExTimerCnt;
    stIr.IrLearninExTimerCnt =0;
    if((stIr.IrLearnIdx&0x01) != 0)
        TIM3->CCER |= TIM_CCER_CC4P;//下降沿有效
    else
        TIM3->CCER &= (uint16_t)(~TIM_CCER_CC4P);//上升沿有效

    if(stIr.IrLearnIdx == 0)
    {
        TIM3->EGR |= (uint16_t)TIM_EventSource_Update;
        Irdata[stIr.IrLearnIdx++] = 38000;//设定默认载波频率
        TimeCnt=0;
    }
    else
    {   
        if(ExTimercnt)
            TimerVal = (u32)(ExTimercnt)*65535 - stIr.LearningPreTimes+TimeCnt;
        else
            TimerVal = TimeCnt - stIr.LearningPreTimes;
        TimerVal <<= 2;
        
       // if(TimerVal>=65000)
       // {
        //    Irdata[stIr.IrLearnIdx++] = 0xffff;
       //     Irdata[stIr.IrLearnIdx++] = (u16)(TimerVal);
       //     Irdata[stIr.IrLearnIdx++] = (u16)(TimerVal>>16);
      //  }
       if(stIr.IrLearnIdx<(IR_BUF_SIZE/2))//防止数据溢出
        {
            Irdata[stIr.IrLearnIdx++] = (u16)(TimerVal);
        }
    }
    stIr.LearningPreTimes = TimeCnt;
}



void Ir_Event_Poll(void)
{
	u8 * temp = 0;
	static u8 learnCnt = 0;
	if(stIr.IrStatus == IR_LEARNING_END){
		if((stIr.IrLearnLen < stIr.LearnIrDataLenMin)||(stIr.IrLearnLen > stIr.LearnIrDataLenMax)){
			if((stIr.IrStatus != IR_SEDN_START)&&(stIr.IrStatus != IR_FORCE_LEARN)){
				stIr.IrStatus = IR_LEARN_START;	
				Ir_Learn_Enable();
			}
		}else{
			stIr.pIrLearnBuf[stIr.IrLearnLen++] = 0xFF;
			stIr.pIrLearnBuf[stIr.IrLearnLen++] = 0xFF;
			stIr.pIrLearnBuf[stIr.IrLearnLen++] = 0x3d;
			stIr.pIrLearnBuf[stIr.IrLearnLen++] = 0x86;
			stIr.pIrLearnBuf[stIr.IrLearnLen++] = 0x01;
			stIr.pIrLearnBuf[stIr.IrLearnLen++] = 0x00;
			stIr.IrSendLen = stIr.IrLearnLen;
			stIr.IrSendIdx= stIr.IrLearnIdx;
			temp = stIr.pIrLearnBuf;
			stIr.pIrLearnBuf = stIr.pIrSendBuf;
			stIr.pIrSendBuf = temp;
			//r(i = 0; i < stIr.IrSendLen; i++){
			//USART_send_byte(stIr.pIrSendBuf[i]);
			//
			if((stIr.IrStatus != IR_SEDN_START)&&(stIr.IrStatus != IR_FORCE_LEARN)){
				stIr.IrStatus = IR_LEARN_START;
				Int_Event = 0x01;
				Send_Int_Event();
				Ir_Learn_Enable();
			}
		}
	}else if(stIr.IrStatus == IR_FORCE_LEARN){
		Ir_Send_Disable();
		Ir_Learn_Enable();
	}else if(stIr.IrStatus == IR_FORCE_LEARN_END){
		if((stIr.IrLearnLen < stIr.LearnIrDataLenMin)||(stIr.IrLearnLen > stIr.LearnIrDataLenMax)){
			learnCnt++;
			if(learnCnt == 5){				
				I2C_ITConfig(I2C1,I2C_IT_ERRI|I2C_IT_ADDRI|I2C_IT_NACKI|I2C_IT_STOPI|I2C_IT_RXI|I2C_IT_TXI|I2C_IT_TCI,ENABLE);
				I2C_Cmd(I2C1,ENABLE);
				if((stIr.IrStatus != IR_SEDN_START)&&(stIr.IrStatus != IR_FORCE_LEARN)){
					stIr.IrStatus = IR_LEARN_START;
				}
				learnCnt = 0;
			}else{
				stIr.IrStatus = IR_FORCE_LEARN;
			}
			Ir_Learn_Enable();
			
		}else{
			I2C_ITConfig(I2C1,I2C_IT_ERRI|I2C_IT_ADDRI|I2C_IT_NACKI|I2C_IT_STOPI|I2C_IT_RXI|I2C_IT_TXI|I2C_IT_TCI,ENABLE);
			I2C_Cmd(I2C1,ENABLE);
			stIr.pIrLearnBuf[stIr.IrLearnLen++] = 0xFF;
			stIr.pIrLearnBuf[stIr.IrLearnLen++] = 0xFF;
			stIr.pIrLearnBuf[stIr.IrLearnLen++] = 0x3d;
			stIr.pIrLearnBuf[stIr.IrLearnLen++] = 0x86;
			stIr.pIrLearnBuf[stIr.IrLearnLen++] = 0x01;
			stIr.pIrLearnBuf[stIr.IrLearnLen++] = 0x00;
			stIr.IrSendLen = stIr.IrLearnLen;
			stIr.IrSendIdx= stIr.IrLearnIdx;
			temp = stIr.pIrLearnBuf;
			stIr.pIrLearnBuf = stIr.pIrSendBuf;
			stIr.pIrSendBuf = temp;
			if((stIr.IrStatus != IR_SEDN_START)&&(stIr.IrStatus != IR_FORCE_LEARN)){
				stIr.IrStatus = IR_LEARN_START;
				Ir_Learn_Enable();
				Int_Event = 0x01;
				Send_Int_Event();
			}
		}
	}else if(stIr.IrStatus == IR_SEDN_START){
		Ir_Learn_Disable();
		Ir_Send_Enable();
	}else if(stIr.IrStatus == IR_SENDING_END){
		if((stIr.IrStatus != IR_SEDN_START)&&(stIr.IrStatus != IR_FORCE_LEARN)){
			stIr.IrStatus = IR_LEARN_START;
			//for(i = 0; i < 3200000;i++){
			//	IWDG_ReloadCounter();
			//}
			Ir_Learn_Enable();
		}
	}
}


void IrLearnEnd(void)
{	
	Ir_Learn_Disable();	    
    if(stIr.IrLearnIdx > 4)
    {
        stIr.IrLearnLen = stIr.IrLearnIdx<<1;
        stIr.IrLearnResult = 0x00;//学习成功
    }
    else
    {
        stIr.IrLearnResult = 0x02;//学习失败
        stIr.IrLearnLen = 0;
    }
    stIr.IrLearnIdx = 0;
    stIr.IrLearninExTimerCnt =0;
	if(stIr.IrStatus == IR_FORCE_LEARNING){
		stIr.IrStatus = IR_FORCE_LEARN_END;
	}else{
		if((stIr.IrStatus != IR_SEDN_START)&&(stIr.IrStatus != IR_FORCE_LEARN)){
			stIr.IrStatus = IR_LEARNING_END;	
		}
	}
}


void Ir_Test_Learn_End(void)
{
    static u16 IrLearnNum=0;
    static u16 MaxWaitTime=0;
    
    if(((stIr.IrStatus == IR_LEARNING)||(stIr.IrStatus == IR_FORCE_LEARNING))&&(IrLearnNum == stIr.IrLearnIdx))
    {
        MaxWaitTime++;
        if(((MaxWaitTime>100)&&(stIr.IrLearnIdx>0))||
            ((MaxWaitTime>10000)&&(stIr.IrLearnIdx==0)))
            IrLearnEnd();
    }
    else
    {
        IrLearnNum = stIr.IrLearnIdx;
        MaxWaitTime =0;
    }
}


