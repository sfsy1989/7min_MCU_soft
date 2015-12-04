#include"include.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "uart.h"
#include<stdio.h>

//#define TEST_CLK_EN
void test_sys_clock(void)
{
	TIM_TimeBaseInitTypeDef timeBaseInit;
	GPIO_InitTypeDef   GPIO_InitStructure;

	NVIC_InitTypeDef   NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM14_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_DeInit(TIM14);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	timeBaseInit.TIM_Prescaler = 63;
	timeBaseInit.TIM_Period = 1000;
	timeBaseInit.TIM_ClockDivision = TIM_CKD_DIV1;
	timeBaseInit.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM14,&timeBaseInit);
	TIM_ITConfig(TIM14,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM14,ENABLE);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);

	/* Configure PA0 pin as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	while(1);
}

void test_led(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;
	int i;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	while(1){
		GPIO_SetBits(GPIOA,GPIO_Pin_4);
		for(i = 0; i < 32000;i++){
			;
		}
		GPIO_ResetBits(GPIOA,GPIO_Pin_4);
		for(i = 0; i < 32000;i++){
			;
		}
	}
}
void IWDG_Config(void)
{
  /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  
  /* IWDG counter clock: LSI/256 */
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  
  IWDG_SetReload((unsigned char)(0xff));//512ms
  
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
  
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}

void User_System_Init(void)
{
	int i ;
    GPIO_Default_Init();
    I2c_Slave_Init();
    Temperature_Measure_Init();
//USART_Configuration();
	for(i = 0; i < 320000;i++){
			;
	}
	Ir_Data_Init();
    Power_Measure_Init();
    Timer14_Init();//用来产生38KHZ载波
    IWDG_Config();//看门狗配置
}

#if 0
static void Restore_Configuration(void)
{
#if 0
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
 #endif
   /* At this stage the HSI is already enabled */
 	RCC_HSICmd(ENABLE);
   /* Enable Prefetch Buffer and set Flash Latency */
   FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
  
   /* HCLK = SYSCLK */
   RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;
	   
   /* PCLK = HCLK */
   RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE_DIV1;
 
   /* PLL configuration = (HSI/2) * 12 = ~48 MHz */
   RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
   RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSI_Div2 | RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLMULL4);
			 
   /* Enable PLL */
   RCC->CR |= RCC_CR_PLLON;
 
   /* Wait till PLL is ready */
   while((RCC->CR & RCC_CR_PLLRDY) == 0)
   {
   }

 /* Select PLL as system clock source */
  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
  RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;    

  /* Wait till PLL is used as system clock source */
  while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL)
  {
  }
}

#endif
void Event_Poll(void)
{
    Temperature_Measure_Process();
    HLW8012_Measure_P();
    Process_KeyPress_Event();
	Ir_Event_Poll();
}

 int main(void)
{
//	Restore_Configuration();

//#ifdef TEST_CLK_EN
//	test_sys_clock();
//#else
    User_System_Init();
    while(1)
    {
      Event_Poll();
      IWDG_ReloadCounter();
    }
//#endif
}




