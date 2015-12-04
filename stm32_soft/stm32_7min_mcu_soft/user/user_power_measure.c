#include"user_power_measure.h"
#include"stm32f0xx_tim.h"
#include"stm32f0xx_rcc.h"
#include"user_type.h"
#include"stm32f0xx_exti.h"
#include"stm32f0xx_misc.h"
#include"stm32f0xx_gpio.h"
#include"stm32f0xx_syscfg.h"


#define       POWER_TEST_PORT           GPIOD
#define         POWER_TEST_PIN              GPIO_PIN_4
#define       POWER_EXIT_PORT           EXTI_PORT_GPIOD

//uint32_t SystemCoreClock    = 48000000;


//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//Time1��ʱ����ʱ,ʱ����� = 1ms
#define D_TIME1_20MS                20      
#define D_TIME1_100MS               100 
#define D_TIME1_150MS               150 
#define D_TIME1_200MS               200 
#define D_TIME1_400MS               400 
#define D_TIME1_500MS               500 
#define D_TIME1_1S              1000        //Time1��ʱ����ʱ1Sʱ�䳣��
#define D_TIME1_2S              2000    
#define D_TIME1_3S              2000    
#define D_TIME1_4S              4000    
#define D_TIME1_6S              6000
#define D_TIME1_8S              8000
#define D_TIME1_9S              9000
#define D_TIME1_10S             10000
#define D_TIME1_20S             20000


//#define D_TIME1_V_OVERFLOW                      500        //Time1��ʱ��,��ѹ��������趨Ϊ500mS,���˵���������ڴ���500mS
//#define D_TIME1_I_OVERFLOW            8000       //Time1��ʱ��,������������趨Ϊ10S,���˵���������ڴ���10S
#define D_TIME1_P_OVERFLOW          12000      //Time1��ʱ��,������������趨Ϊ10S(Լ0.5W��Сֵ),���˵���������ڴ���10S
//#define D_TIME1_P_OVERFLOW            40000      //Time1��ʱ��,������������趨Ϊ40S(Լ0.2W��Сֵ)
#define D_TIME1_CAL_TIME            36000      //У��ʱ�䣬��¼�ڴ�ʱ���ڵ���������1000W�������õ�36Sʱ���ںķ�0.01�ȵ�
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------


//HLW 8012 IO����
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
#define IO_HLW8012_CF1_S      PD_ODR_ODR3       //HLW8012 PIN8      
#define IO_HLW8012_CF1        PA_IDR_IDR2       //HLW8012 PIN7  
#define IO_HLW8012_CF         PC_IDR_IDR5       //HLW8012 PIN6
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------  


//����ģʽ
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
#define D_ERR_MODE                  0x00        //������ʾģʽ
#define D_NORMAL_MODE               0x10        //��������ģʽ
#define D_CAL_START_MODE        0x21        //У��ģʽ������
#define D_CAL_END_MODE              0x23        //У��ģʽ�����
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

u16 U16_P_TotalTimes;           //��ǰ���� ���ʲ�����ʱ��
//u16   U16_V_TotalTimes;           //��ǰ���� ��ѹ������ʱ��
//u16   U16_I_TotalTimes;           //��ǰ���� ����������ʱ��

u16 U16_P_OneCycleTime;         //���ʲ���ʱ�����
//u16   U16_V_OneCycleTime;         //��ѹ����ʱ�����
//u16   U16_I_OneCycleTime;         //��������ʱ�����

u16 U16_P_Last_OneCycleTime;        //���ʲ���ʱ���������һ������ֵ
//u16   U16_V_Last_OneCycleTime;        //��ѹ����ʱ���������һ������ֵ
//u16   U16_I_Last_OneCycleTime;        //��������ʱ���������һ������ֵ

u16 U16_P_CNT;              //���ʲ�����������
//u16   U16_V_CNT;              //��ѹ������������
//u16   U16_I_CNT;              //����������������

u16 U16_P_Last_CNT;             //���ʲ���������������һ������ֵ
//u16   U16_V_Last_CNT;             //��ѹ����������������һ������ֵ
//u16   U16_I_Last_CNT;             //��������������������һ������ֵ

BOOL    B_P_TestOneCycle_Mode;          //���ʲ���ģʽ 1:�����ڲ�����0:1S��ʱ����
//BOOL  B_V_TestOneCycle_Mode;
//BOOL  B_I_TestOneCycle_Mode;

BOOL    B_P_Last_TestOneCycle_Mode;
//BOOL  B_V_Last_TestOneCycle_Mode;
//BOOL  B_I_Last_TestOneCycle_Mode;
            
BOOL    B_P_OVERFLOW;                   // ������������ �����־λ 
//BOOL      B_V_OVERFLOW;                   // ��ѹ�������� �����־λ
//BOOL      B_I_OVERFLOW;                   // ������������ �����־λ

BOOL    B_P_Last_OVERFLOW;              // ������������ �����־λ 
//BOOL      B_V_Last_OVERFLOW;              // ��ѹ�������� �����־λ
//BOOL      B_I_Last_OVERFLOW;              // ������������ �����־λ

//BOOL      B_VI_Test_Mode;             //��ѹ����ģʽ;0:��������ģʽ
//u16       U16_VI_Test_Times;              
u16     U16_Cal_Times;  

u16     U16_AC_P;               //����ֵ 1000.0W
//u16       U16_AC_V;               //��ѹֵ 220.0V
//u16       U16_AC_I;               //����ֵ 4.545A
u32     U32_AC_E;               //�õ���   0.01��
//u32       U32_AC_BACKUP_E;            //�õ�������    
//u8        U8_AC_COS;              //�������� 0.00

u16     U16_REF_001_E_Pluse_CNT;            //0.01�ȵ����������ο�ֵ
u16     U16_E_Pluse_CNT;                //��������Ĵ���

u32     U32_Cal_Times;                      //У��ʱ��

u32     U32_P_REF_PLUSEWIDTH_TIME;          //�ο����� ��������
//u32       U32_V_REF_PLUSEWIDTH_TIME;          //�ο���ѹ ��������
//u32       U32_I_REF_PLUSEWIDTH_TIME;          //�ο����� ��������

u32     U32_P_CURRENT_PLUSEWIDTH_TIME;          //��ǰ���� ��������
//u32       U32_V_CURRENT_PLUSEWIDTH_TIME;          //��ǰ��ѹ ��������
//u32       U32_I_CURRENT_PLUSEWIDTH_TIME;          //��ǰ���� ��������

u16     U16_P_REF_Data;             //�ο�����ֵ,����1000WУ����1000.0W
//u16       U16_V_REF_Data;             //�ο���ѹ  220.0V
//u16       U16_I_REF_Data;             //�ο�����  1000W,220V��������4.545A

u8      U8_CURR_WorkMode;



/*=====================================================
 * ��������: void Read_CalData_EEPROM(void)
 * ��������: 
 * �������:
 * �������: 
 * ��    ��: 
=====================================================*/
void Read_CalData_EEPROM(void)
{
    //u8  u8_temp;
    
    //u8_temp = EEPROM_Read_Byte(ADDR_CAL_FLAG);
   
    //U32_P_REF_PLUSEWIDTH_TIME = EEPROM_Read_Word(ADDR_REF_P_PLUSEWIDTH_TIME);
    //U32_V_REF_PLUSEWIDTH_TIME = EEPROM_Read_Word(ADDR_REF_V_PLUSEWIDTH_TIME); 
    //U32_I_REF_PLUSEWIDTH_TIME = EEPROM_Read_Word(ADDR_REF_I_PLUSEWIDTH_TIME); 
    
   // U16_REF_001_E_Pluse_CNT = EEPROM_Read_Inte(ADDR_REF_001_E);
    //U32_AC_BACKUP_E = EEPROM_Read_Word(ADDR_AC_BACKUP_E);  
    //U32_AC_E = U32_AC_BACKUP_E;
        
              
   // if (u8_temp != 0x55)  //�ж�EEPROM���Ƿ����У��ֵ,��û�У��򽫲�������ΪĬ��ֵ
    {
      //����Ĭ��ֵ
      U16_P_REF_Data = 10000;       // 1000.0W
     // U16_V_REF_Data = 2200;      // 220.0V
      //U16_I_REF_Data = 4545;      // 4.545A
  

      U32_P_REF_PLUSEWIDTH_TIME = 4975;     // 4975us
      //U32_V_REF_PLUSEWIDTH_TIME = 1666;      // 1666us
      //U32_I_REF_PLUSEWIDTH_TIME = 1666;      // 1666us
      //U32_AC_BACKUP_E = 0;
      U32_AC_E = 0;
    }
}



//--------------------------------------------------------------------------------------------
/*=====================================================
 * Function : void Init_Rom(void)
 * Describe : rom�ռ��ʼ��
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2014/04/14
=====================================================*/
void Init_Rom(void)
{
    U8_CURR_WorkMode = D_NORMAL_MODE;
    //U8_DspMode = 0;
    
    U16_AC_P = 0;
    //U16_AC_V = 0;
    //U16_AC_I = 0;
    //U8_AC_COS = 0;
  
//У�����ʲο�ֵ(У������1000W)
  U16_P_REF_Data = 10000;       // 1000.0W
  //U16_V_REF_Data = 2200;      // 220.0V
  //U16_I_REF_Data = 4545;      // 4.545A
  
  
    
//У�����ʲο�ֵ(У������250W)
//  U16_P_REF_Data = 2500;      // 250.0W
//  U16_V_REF_Data = 2200;      // 220.0V
//  U16_I_REF_Data = 1136;      // 1.136A


 
  U16_P_TotalTimes = 0;
  //U16_V_TotalTimes = 0;
  //U16_I_TotalTimes = 0;

  
  U16_P_OneCycleTime = 0;
  //U16_V_OneCycleTime = 0;
  //U16_I_OneCycleTime = 0;
  U16_P_Last_OneCycleTime = 0;
  //U16_V_Last_OneCycleTime = 0;
  //U16_I_Last_OneCycleTime = 0;
  
  U16_P_CNT = 0;
  //U16_V_CNT = 0;
  //U16_I_CNT = 0;
  U16_P_Last_CNT = 0;
  //U16_V_Last_CNT = 0;
  //U16_I_Last_CNT = 0;
  
//��ʼ�������ڲ���ģʽ
  B_P_TestOneCycle_Mode = 1;
  //B_V_TestOneCycle_Mode = 1;
  //B_I_TestOneCycle_Mode = 1;
  B_P_Last_TestOneCycle_Mode = 1;
  //B_V_Last_TestOneCycle_Mode = 1;
  //B_I_Last_TestOneCycle_Mode = 1;

//��ʼ�������������־λΪ1  
  B_P_OVERFLOW = 1;
  //B_V_OVERFLOW = 1;
  //B_I_OVERFLOW = 1;
  
  B_P_Last_OVERFLOW = 1;
  //B_V_Last_OVERFLOW = 1;
  //B_I_Last_OVERFLOW = 1;
  
//�ϵ��ʼ��Ϊ��ѹ����ģʽ 
  //B_VI_Test_Mode = 1;
  //IO_HLW8012_CF1_S = 1;
 // U16_VI_Test_Times = D_TIME1_2S;
  
//��ʾ��ʼ��
  //U8_DspMode = 0;
  //B_DIS_EN = FALSE;
  //U16_SendUart_Time = 0;

  Read_CalData_EEPROM();
  
}


void  PowerTimerInt(void)
{
//У��ģʽ
    if (U8_CURR_WorkMode == D_CAL_START_MODE)
    {
        U32_Cal_Times++;
        if (U32_Cal_Times == D_TIME1_CAL_TIME)
        {
            U8_CURR_WorkMode = D_CAL_END_MODE;
            U16_REF_001_E_Pluse_CNT = U16_E_Pluse_CNT;      //��¼36Sʱ���ڵ�������������������ʾ0.01���õ���
        }
    }
    
//���ʲ���
    if (U16_P_CNT != 0)
    {
        U16_P_OneCycleTime++;
        U16_P_TotalTimes++;
    }  
    if (U16_P_TotalTimes >= D_TIME1_P_OVERFLOW)
    {
        B_P_OVERFLOW = TRUE; 
        B_P_Last_OVERFLOW = B_P_OVERFLOW;
        //��״̬����,���¿�ʼ����
        U16_P_TotalTimes = 0;       //������Ĵ���
        U16_P_OneCycleTime = 0;
        U16_P_CNT = 0;              //�ȴ���һ���жϿ�ʼ����
        B_P_TestOneCycle_Mode = 0;   //��ʼ��Ϊ�����������ģʽ      
    }
    else if (U16_P_OneCycleTime == D_TIME1_100MS)
    {
      if (U16_P_CNT < 2)
      {
        // 100ms��ֻ��һ���жϣ�˵������>100ms,���õ����ڲ���ģʽ 
        B_P_TestOneCycle_Mode = 1;
      }
      else
      {
         // 100ms����2�λ������������壬˵������<100ms�����ü����������ģʽ
         B_P_TestOneCycle_Mode = 0;   
      }
    }
     
}



void PowerIntProcess(void)
{
//���ʲ���
    U16_P_TotalTimes = 0;       //���һ����Ч�Ĳ���������Ĵ�������    
    U16_P_CNT++;
    if (B_P_OVERFLOW == TRUE)
    {  
        //�����ģʽת��,��ʼ����     
        B_P_TestOneCycle_Mode = 0;  //��ʼ��Ϊ�����������ģʽ
        U16_P_TotalTimes = 0;       //������Ĵ�������
        U16_P_OneCycleTime = 0;     //������Ĵ���
        U16_P_CNT = 1;              
        B_P_OVERFLOW = FALSE;       //�������־λ
    }
    else
    {
        if (B_P_TestOneCycle_Mode == 1)
        {
            if (U16_P_OneCycleTime >= D_TIME1_100MS)
            {
                //�����ڲ���ģʽ 
                U16_P_Last_OneCycleTime = U16_P_OneCycleTime;
                B_P_Last_TestOneCycle_Mode = B_P_TestOneCycle_Mode;
                B_P_OVERFLOW = FALSE;       //�����־λ����
                B_P_Last_OVERFLOW = B_P_OVERFLOW;
                 //��״̬����,���¿�ʼ����
                B_P_TestOneCycle_Mode = 0;  //��ʼ��Ϊ�����������ģʽ
                U16_P_TotalTimes = 0;       //���һ����Ч�Ĳ���������Ĵ�������
                U16_P_OneCycleTime = 0;     //������Ĵ���
                U16_P_CNT = 1;
 
            }
        }
        else
        {
            if (U16_P_OneCycleTime >= D_TIME1_1S)
            {   
                U16_P_Last_OneCycleTime = U16_P_OneCycleTime;
                U16_P_Last_CNT = U16_P_CNT;
                B_P_Last_TestOneCycle_Mode = B_P_TestOneCycle_Mode;
                B_P_OVERFLOW = FALSE;       //�����־λ����
                B_P_Last_OVERFLOW = B_P_OVERFLOW;
                //��״̬����,���¿�ʼ����
                B_P_TestOneCycle_Mode = 0;  //��ʼ��Ϊ�����������ģʽ
                U16_P_TotalTimes = 0;       //���һ����Ч�Ĳ���������Ĵ�������
                U16_P_OneCycleTime = 0;     //������Ĵ���
                U16_P_CNT = 1;
            }
        }
    }
    
    
//У��ģʽ
    if (U8_CURR_WorkMode == D_CAL_START_MODE)
    {
        //��¼��λʱ���ڵ��õ���
        U16_E_Pluse_CNT++;
    }
    
//���ܼ���
    if (U8_CURR_WorkMode == D_NORMAL_MODE)
    {
        U16_E_Pluse_CNT++;
        if (U16_E_Pluse_CNT == U16_REF_001_E_Pluse_CNT )
        {
            U16_E_Pluse_CNT = 0;
            U32_AC_E++;
        }
    }
}




/*=====================================================
 * Function : void HLW8012_Measure_P(void)
 * Describe : 
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2014/04/14
=====================================================*/
void HLW8012_Measure_P(void)
{
    u32 a;
    u32 b;
    u32 u32_P_Period;
    
    if (B_P_Last_TestOneCycle_Mode == 1)
    {
       //�����ڲ���ģʽ
       b = U16_P_Last_OneCycleTime;
       b = b*1000;  //msת����us 
       u32_P_Period = b;   
    }
    else
    {
      //�����������ģʽ
      b = U16_P_Last_OneCycleTime;
      b = b*1000;
      u32_P_Period = b/(U16_P_Last_CNT-1);  
    }
    
    U32_P_CURRENT_PLUSEWIDTH_TIME = u32_P_Period;      // У��ʱȡU32_P_CURRENT_PLUSEWIDTH_TIME������Ϊ�ο�ֵ     
    a = U16_P_REF_Data * U32_P_REF_PLUSEWIDTH_TIME;        
    U16_AC_P = a/U32_P_CURRENT_PLUSEWIDTH_TIME;
    
    if (U16_AC_P == 0xffff)     //����ʱU32_P_CURRENT_PLUSEWIDTH_TIME = 0���������
    {
        U16_AC_P = 0; 
    }
    
    if (B_P_Last_OVERFLOW == TRUE)
    {
        U16_AC_P = 0;
    }
}

void Power_Measure_Init()
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	TIM_TimeBaseInitTypeDef timeBaseInit;
	#if 1
	Init_Rom();
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);

	/* Configure PA2 pin as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	/* Connect EXTI0 Line to PA2 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource2);

	/* Configure EXTI2 line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
	NVIC_InitStructure.NVIC_IRQChannel = TIM16_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x01;
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

