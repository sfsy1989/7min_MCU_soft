#include"user_temperature_measure.h"
#include"user_type.h"
#include"stm32f0xx_adc.h"
#include"stm32f0xx_rcc.h"
#include"stm32f0xx_gpio.h"
#include"stm32f0xx_dma.h"


#define TEMP_MINI_VALUE     (-30.00)
#define TEMP_MAX_VALUE      (60.00)
#define GET_INDEX_TEMP(x)   ((float)x+TEMP_MINI_VALUE)

float Temperature=0;
float Humidity = 0;


u16 const TempTable[90]=
{
332,349,365,383,401,419,439,458,479,522,
545,568,592,617,642,668,695,722,750,779,
808,838,869,900,931,964,996,1030,1064,1098,
1133,1168,1204,1240,1276,1313,1351,1388,1426,1464,
1502,1541,1580,1619,1658,1697,1736,1775,1814,1854,
1893,1932,1971,2009,2048,2086,2125,2163,2200,2238,
2275,2311,2347,2383,2419,2454,2489,2523,2557,2590,
2623,2655,2687,2718,2748,2779,2808,2837,2866,2894,
2922,2949,2975,3001,3027,3052,3076,3100,3123,3146
};



void Delay(u16 T)
{
    u16 i;
    while(T--)
    {
        for(i=0;i<2000;i++);
    }
}

float GetTemp(u16 Adc)
{
   u16 i=0;
   float Temp=0;
   for(i=0;i<90;i++)
   {
        if((TempTable[i])>=Adc)break;
   }
   if(i==90)return (60.00);
   if(i== 0)return (-30.00);
   Temp =(float)((float)Adc-TempTable[i-1])/(TempTable[i]-TempTable[i-1])+GET_INDEX_TEMP(i);
   return(Temp);
}
#if 0

#define ADC1_DR_Address                0x40012440
__IO uint16_t RegularConvData_Tab = 0;

void Temperature_Measure_Init(void)
{
  ADC_InitTypeDef     ADC_InitStruct;
  DMA_InitTypeDef     DMA_InitStruct;
	GPIO_InitTypeDef    GPIO_InitStruct;
	  /* ADC1 DeInit */  
  ADC_DeInit(ADC1);

	/* Enable  GPIOA clock */
	  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  /* ADC1 Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  /* DMA1 clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);

  /* Configure PA.01  as analog input */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStruct);				// PA1,输入时不用设置速率

 
  /* DMA1 Channel1 Config */
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
  DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&RegularConvData_Tab;
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStruct.DMA_BufferSize =4;
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStruct.DMA_Priority = DMA_Priority_High;
  DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStruct);
  
  /* DMA1 Channel1 enable */
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
//   /* ADC DMA request in circular mode */
  ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);
  
  /* Enable ADC_DMA */
  ADC_DMACmd(ADC1, ENABLE);  
  
  /* Initialize ADC structure */
  ADC_StructInit(&ADC_InitStruct);
  
  /* Configure the ADC1 in continous mode withe a resolutuion equal to 12 bits  */
  ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStruct.ADC_ContinuousConvMode = ENABLE; 
  ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStruct.ADC_ScanDirection = ADC_ScanDirection_Backward;
  ADC_Init(ADC1, &ADC_InitStruct); 

  
  /* Convert the ADC1 Vref  with 55.5 Cycles as sampling time */ 
  ADC_ChannelConfig(ADC1, ADC_Channel_0  , ADC_SampleTime_55_5Cycles); 
//   ADC_VrefintCmd(ENABLE);
  
  /* ADC Calibration */
  ADC_GetCalibrationFactor(ADC1);
   ADC_DMACmd(ADC1, ENABLE);
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);     
  
  /* Wait the ADCEN falg */
//  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN)); 
  
  /* ADC1 regular Software Start Conv */ 
//  ADC_StartOfConversion(ADC1);
}

#else
void Temperature_Measure_Init(void)
{
	ADC_InitTypeDef	  ADC_InitStructure;
	GPIO_InitTypeDef	  GPIO_InitStructure;

	/* GPIOC Periph clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	/* ADC1 Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* ADCs DeInit */  
	ADC_DeInit(ADC1);

	/* Initialize ADC structure */
	ADC_StructInit(&ADC_InitStructure);

	/* Configure the ADC1 in continuous mode withe a resolution equal to 12 bits  */
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;    
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
	ADC_Init(ADC1, &ADC_InitStructure); 

	/* Convert the ADC1 Channel 11 with 239.5 Cycles as sampling time */ 
	ADC_ChannelConfig(ADC1, ADC_Channel_0, ADC_SampleTime_239_5Cycles);


	/* ADC Calibration */
	ADC_GetCalibrationFactor(ADC1); 

	ADC_ITConfig(ADC1,ADC_IT_EOC,DISABLE);    

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);   

}
#endif
void Temperature_Measure_Process(void)
{
    static u16 TempTimeCnt=0;
    if(++TempTimeCnt<1000)return;
    TempTimeCnt=0;
    
    if(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) != RESET/*(DMA_GetFlagStatus(DMA1_FLAG_TC1)) != RESET*/)
    {
    //	DMA_ClearFlag(DMA1_FLAG_TC1);
        ADC_ClearFlag(ADC1,ADC_FLAG_EOC);
		Temperature =GetTemp(ADC_GetConversionValue(ADC1)/*RegularConvData_Tab*/);
//        ADC_StartOfConversion(ADC1);
    }
	if(ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN))
    {
		ADC_StartOfConversion(ADC1);
    }
}

