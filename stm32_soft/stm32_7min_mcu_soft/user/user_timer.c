#include"user_timer.h"
#include"user_type.h"
#include"stm32f0xx_rcc.h"
#include"stm32f0xx_tim.h"
#include"stm32f0xx_gpio.h"
#include"stm32f0xx_misc.h"



void Timer14_Init(void)
{

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	u16 TimerPeriod;
	NVIC_InitStructure.NVIC_IRQChannel = TIM14_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
#if 0	
	TIM_DeInit(TIM16);
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_TIM16EN, ENABLE);
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_Period = 421;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM16,&TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OC1Init(TIM16, &TIM_OCInitStructure);
	TIM_ITConfig(TIM16,TIM_IT_Update, DISABLE);

	TIM_Cmd(TIM16, ENABLE);
	
	TIM_CtrlPWMOutputs(TIM16, ENABLE);
#else
	TimerPeriod = (SystemCoreClock / 38000 );

	TIM_DeInit(TIM14);
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM14EN, ENABLE);
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM14,&TIM_TimeBaseStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
  	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
 	 TIM_OCInitStructure.TIM_Pulse = 0;
 	 TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
// 	 TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	 TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
//  	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

	TIM_OC1Init(TIM14, &TIM_OCInitStructure);
	TIM_ITConfig(TIM14,TIM_IT_Update,DISABLE);
	TIM_Cmd(TIM14, ENABLE);
	TIM_CtrlPWMOutputs(TIM14, ENABLE);
#endif


}

