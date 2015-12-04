#include"uart.h"
#include"stm32f0xx_gpio.h"
#include"stm32f0xx_rcc.h"
#include"user_i2c.h"
#include"stm32f0xx_adc.h"
#include"stm32f0xx_tim.h"
#include"stm32f0xx_misc.h"



typedef struct testData{
	float Temperature;
	float Humidity;
	unsigned char * pReadStr;
	unsigned char strLen;	
}TESTDATA;

unsigned char recvBuf[63] = {0};
unsigned char printBuf[32] = {0};
unsigned char irData[1280] = {0};
TESTDATA m_testData;

volatile unsigned char startFlag = 0;
volatile unsigned char testCount = 0;
volatile unsigned char currentTestItem = 1;
volatile unsigned char startKeyStatus = 0;
volatile unsigned char nextKeyStatus = 0;
unsigned char resetKeyStatus = 0;
unsigned short irDataIndex = 0;
unsigned short capacity = 0;
unsigned short irDataLen = 0;
unsigned char keyStatus = 0;





unsigned char itoa(unsigned int i, unsigned char *string)  
{  
        unsigned int power=1,j=0;  
  		unsigned char *p = string;
        j=i;  
        for( ;j>10;j/=10)  
                power*=10;  
  
        for(;power>0;power/=10)  
        {  
                *string++='0'+i/power;  
                i%=power;  
        }  
		*string = '\0';
		j = 0;
		while(p[j] != '\0')
			j++;
		return j;
}  

unsigned char my_ftoa(double number,unsigned char *buf)  
{  
 	int int_part,i = 0,len;
	float float_part;
	int_part = (int)number;
	float_part = number - int_part;
	itoa(int_part,buf);
	while(buf[i] != '\0')
		i++;
	buf[i] = '.';
	int_part = (int)(float_part*100);
	len = itoa(int_part,&buf[i+1]);
	return i+len+1;
}  


void my_memcpy(unsigned char *dest, unsigned char *src, int len)
{
      unsigned char *p = dest;
      unsigned char *q = src;
	  int i;
      if( dest == NULL ||src == NULL)
      {
          return;
      }
 
      for (i=0; i<len; i++)
     {
            *p++ = *q++;
     }
}

static unsigned char * my_memset(unsigned char * dst,int val,unsigned long ulcount)
{
	char *pchdst = NULL;
	if(!dst)return 0;
	pchdst = (char*)dst;
	while(ulcount--)
		*pchdst++ = (char)val;
	return dst;
}

void delay_time(int i)
{
	int j,k;
	for(j = 0; j < 1000;j++)
		for(k = 0; k < i; k++)
			;
}


void Init_Var(void)
{
	m_testData.Temperature = 0;
	m_testData.Humidity = 0;
	m_testData.strLen = 0;
	m_testData.pReadStr = recvBuf;	
}

void get_temp_data(void)
{
	my_memset(recvBuf,0,sizeof(recvBuf));
	I2C_Read_FromSlave(recvBuf,0x06,12);
	my_memcpy((unsigned char *)&(m_testData.Temperature),&recvBuf[1],4);
	my_memcpy((unsigned char *)&(m_testData.Humidity),&recvBuf[5],4);
	my_memcpy((unsigned char *)&(capacity),&recvBuf[9],2);
	my_memcpy((unsigned char *)&(keyStatus),&recvBuf[11],1);
}


void scanKeyValue(void)
{
	if((GPIO_ReadInputData(GPIOA)&GPIO_Pin_7) != 0){
		nextKeyStatus = 0;	
	}
	if(nextKeyStatus == 0){
		if((GPIO_ReadInputData(GPIOA)&GPIO_Pin_7)==0){
			nextKeyStatus = 1;
			if(startFlag == 1){
				currentTestItem++;	
				testCount = 0;
			}	
		}
	}
	if((GPIO_ReadInputData(GPIOA)&GPIO_Pin_6) != 0){
		startKeyStatus = 0;	
	}
	if(startKeyStatus == 0){
		if((GPIO_ReadInputData(GPIOA)&GPIO_Pin_6)==0){
			startKeyStatus = 1;
			startFlag = 1;
			testCount = 0;
			if(currentTestItem == 6)			
				currentTestItem = 1;
		}
	}
}
void ir_learn_test(void)
{
	//请求学习
	unsigned char block,lastNum,i;
	my_memset(recvBuf,0,sizeof(recvBuf));
	I2C_Write_ToSlave(NULL,0x02,0);
	UART_Send("请按遥控器...",13);
	while(1){	
		while((GPIO_ReadInputData(GPIOA)&GPIO_Pin_1) != 0);
		I2C_Read_FromSlave(recvBuf,0x08,4);
		if((recvBuf[0] == 0x80)&&(recvBuf[1] == 0x01)){
			irDataLen = (recvBuf[3]<<8)|recvBuf[2];
			break;
		}else if(recvBuf[0] == 0x02){
			delay_time(100);
			I2C_Write_ToSlave(NULL,0x02,0);
		}
	}
//获取IR 数据
//	my_memset(printBuf,0,30);
//	itoa(irDataLen,printBuf);
//	UART_Send("长度:",5);
//	UART_Send(printBuf,4);
	delay_time(100);
//	I2C_Write_ToSlave(NULL,0x03,0);
	delay_time(100);
	block = (unsigned char)((irDataLen)/60);
	lastNum = (unsigned char)((irDataLen)%60);
	if((irDataLen)%60 != 0)
		block++;		
	my_memset(irData,0,sizeof(irData));
	for(i = 0; i < block; i++){	
		I2C_Write_ToSlave(NULL,0x03,0);
		if(i == (block-1)){
			I2C_Read_Ir_Data(&irData[i*61],0x03,lastNum+1);
		}
		else{
			I2C_Read_Ir_Data(&irData[i*61],0x03,61);
		}
		delay_time(100);
	}
//	for(i = 0; i < irDataLen;i++){
//		UART_send_byte(irData[i]);
//	}
	UART_Send("红外学习结束!",13);

}

void ir_send_test(void)
{
	unsigned char block,i,last;
	block=(unsigned char)(irDataLen/60);
    if(((irDataLen%60)==0)&&(block>0))
    	block -=1;
	last = (unsigned char)(irDataLen%60);
	//请求发射红外

	for(i = 0; i <= block; i++){
		my_memset(recvBuf,0,sizeof(recvBuf));
		if(i == block){
			recvBuf[0] = 0xff;
			my_memcpy(&recvBuf[1],&irData[irDataIndex+1],last);
			I2C_Write_ToSlave(recvBuf,0x01,last+2);
		}else{
			recvBuf[0] = i;	
			my_memcpy(&recvBuf[1],&irData[irDataIndex+1],60);
			I2C_Write_ToSlave(recvBuf,0x01,62);
		}
		delay_time(50);
		irDataIndex += 61;
	}
	irDataIndex = 0;
	UART_Send("等待红外发射...",15);	
}

void Volt_ADC_Init(void)
{
	ADC_InitTypeDef   ADC_InitStructure;
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

unsigned short Get_Adc_Volt(void)
{
	if(ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN))
    {
		ADC_StartOfConversion(ADC1);
    }
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);
    ADC_ClearFlag(ADC1,ADC_FLAG_EOC);
	return ADC_GetConversionValue(ADC1);
}

void testFunction(unsigned char testItem)
{
	unsigned char printStrLen = 0;
	unsigned short volt;
	float fVolt;
	switch(testItem){
		case 0:
			;
			break;
		case 1:
			if(testCount == 0){
					//电源 温度 功率测试
					get_temp_data();
					my_memset(printBuf,0,sizeof(printBuf));
					my_memcpy(printBuf,"1,温度: ",8);
					printStrLen = my_ftoa(m_testData.Temperature,&printBuf[8]);
					UART_Send(printBuf,printStrLen+8);
					my_memset(printBuf,0,sizeof(printBuf));
					if((capacity >= 24742) &&(capacity <= 25242)){
						my_memcpy(printBuf,"2,功率: ",8);
						printStrLen = itoa(capacity,&printBuf[8]);
						UART_Send(printBuf,printStrLen+8);	
	  				}else{
						UART_Send("2,功率: Error",13);	
					}
					
					volt = Get_Adc_Volt();
					if((volt >= 1985) && (volt <= 2079)){
						my_memcpy(printBuf,"3,电压(3.20V ~ 3.35V): OK  ",27);
						fVolt = volt*2*3.3/4096;
						printStrLen = my_ftoa(fVolt,&printBuf[27]);
						UART_Send(printBuf,printStrLen+27);
					}else{
						UART_Send("3,电压: Error",13);	
					}
					testCount = 1;
					UART_Send("Start or Next ?",15);
				}			
				break;
		case 2:
			//继电器按键测试
			if(testCount == 0){
				get_temp_data();
				my_memset(printBuf,0,sizeof(printBuf));
				UART_Send("继电器状态:",11);
				if((unsigned char)(keyStatus&0x01) == 0x01){
					UART_Send("4,继电器1 : 开",14);	
				}else{
					UART_Send("4,继电器1 : 关",14);	
				}
				if((unsigned char)(keyStatus&0x02) == 0x02){
					UART_Send("5,继电器2 : 开",14);	
				}else{
					UART_Send("5,继电器2 : 关",14);	
				}
				testCount = 1;
				UART_Send("Start or Next ?",15);
			}
			break;
		case 3:
			//reset key test
			if((GPIO_ReadInputData(GPIOB)&GPIO_Pin_1)!=0){
				resetKeyStatus = 0;
			}
			if((GPIO_ReadInputData(GPIOB)&GPIO_Pin_1) == 0){
				if(resetKeyStatus == 0){
					UART_Send("复位键测试OK",12);
					resetKeyStatus = 1;
				}
			}	
			break;
//		case 4:
//			if(testCount == 0){
//				UART_Send("请按MCU板按键:",14);
//				for(IntCount = 0; IntCount < 64000000;IntCount++){
//					if((GPIO_ReadInputData(GPIOA)&GPIO_Pin_1) == 0){
//						UART_Send("中断功能测试 OK",11);	
//						break;
//					}
//				}
//				if(IntCount == 1600000){
//					UART_Send("中断功能测试 FAILED",19);	
//				}
//				testCount = 1;
//				UART_Send("Start or Next ?",15);
//			}
//			break;
		case 4:
			//红外学习测试
			if(testCount == 0){
				UART_Send("8,开始红外学习测试",18);
				ir_learn_test();
				testCount = 1;
				UART_Send("Start or Next ?",15);
			}
		
		break;
		case 5:
			//红外发射测试
			if(testCount == 0){
				UART_Send("9,开始红外发射测试",18);
				ir_send_test();
				testCount = 1;
				UART_Send("Start or Next ?",15);
			}
			
			break;
		default:
			break;
	}
}


void Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_DeInit(GPIOA);	
	GPIO_DeInit(GPIOB);
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;    //要设置的PIN
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN; //输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
		/* Connect PXx to I2C_SCL*/
	GPIO_PinAFConfig( GPIOA , GPIO_PinSource9, GPIO_AF_4); 
	/* Connect PXx to I2C_SDA*/
	GPIO_PinAFConfig( GPIOA ,GPIO_PinSource10, GPIO_AF_4);
	/*!< GPIO configuration */ 
	/*!< Configure sEE_I2C pins: SCL */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//GPIO_Mode_IN
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//open-drain
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA , &GPIO_InitStructure);

	/*!< Configure sEE_I2C pins: SDA */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA , &GPIO_InitStructure);
}

void Power_TIM_Init()
{
	TIM_TimeBaseInitTypeDef timeBaseInit;
	NVIC_InitTypeDef   NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = TIM16_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_DeInit(TIM16);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
	timeBaseInit.TIM_Prescaler = 63;
	timeBaseInit.TIM_Period = 250;
	timeBaseInit.TIM_ClockDivision = TIM_CKD_DIV1;
	timeBaseInit.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM16,&timeBaseInit);
	TIM_ITConfig(TIM16,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM16,ENABLE);
}

int main()
{
	Gpio_Init();
	Power_TIM_Init();
	USART_Configuration();
	my_i2c_Init();
	Volt_ADC_Init();
	Init_Var();
	UART_Send("等待从设备初始化...\r\n",21);
	delay_time(1600);
	UART_Send("请按start键进行测试!\r\n",22);
	delay_time(1000);
	 while(1)
    {	
    	scanKeyValue();
    	if(startFlag == 1){
			if(testCount == 0){
				if(currentTestItem == 3){
					UART_Send("7,开始测试复位按键,请按复位按键!",32);	
					testCount = 1;
				}
			}
			if(currentTestItem == 6){
				UART_Send("测试结束!",9);	
				UART_Send("按start键重新测试!",18);	
				startFlag = 0;
			}
			testFunction(currentTestItem);	
		}

    }
}


