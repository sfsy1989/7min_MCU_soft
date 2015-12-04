#include"user_gpio.h"
#include"stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"

void GPIO_Default_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_DeInit(GPIOA);	
	GPIO_DeInit(GPIOB);

	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;    //要设置的PIN
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN; //输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3;
	GPIO_Init(GPIOA,&GPIO_InitStructure);


	
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz; //输出速度
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;    //要设置的PIN
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT; //输出
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5;    //要设置的PIN
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	GPIO_ResetBits(GPIOA,GPIO_Pin_6);
	GPIO_ResetBits(GPIOA,GPIO_Pin_5);
	GPIO_SetBits(GPIOA,GPIO_Pin_1);
	
	
}

