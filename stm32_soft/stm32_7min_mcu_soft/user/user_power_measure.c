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
//Time1定时器定时,时间基数 = 1ms
#define D_TIME1_20MS                20      
#define D_TIME1_100MS               100 
#define D_TIME1_150MS               150 
#define D_TIME1_200MS               200 
#define D_TIME1_400MS               400 
#define D_TIME1_500MS               500 
#define D_TIME1_1S              1000        //Time1定时器定时1S时间常数
#define D_TIME1_2S              2000    
#define D_TIME1_3S              2000    
#define D_TIME1_4S              4000    
#define D_TIME1_6S              6000
#define D_TIME1_8S              8000
#define D_TIME1_9S              9000
#define D_TIME1_10S             10000
#define D_TIME1_20S             20000


//#define D_TIME1_V_OVERFLOW                      500        //Time1定时器,电压溢出常数设定为500mS,溢出说明脉宽周期大于500mS
//#define D_TIME1_I_OVERFLOW            8000       //Time1定时器,电流溢出常数设定为10S,溢出说明脉宽周期大于10S
#define D_TIME1_P_OVERFLOW          12000      //Time1定时器,功率溢出常数设定为10S(约0.5W最小值),溢出说明脉宽周期大于10S
//#define D_TIME1_P_OVERFLOW            40000      //Time1定时器,功率溢出常数设定为40S(约0.2W最小值)
#define D_TIME1_CAL_TIME            36000      //校正时间，记录在此时间内的脉冲数，1000W负载在用电36S时间内耗费0.01度电
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------


//HLW 8012 IO设置
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
#define IO_HLW8012_CF1_S      PD_ODR_ODR3       //HLW8012 PIN8      
#define IO_HLW8012_CF1        PA_IDR_IDR2       //HLW8012 PIN7  
#define IO_HLW8012_CF         PC_IDR_IDR5       //HLW8012 PIN6
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------  


//工作模式
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
#define D_ERR_MODE                  0x00        //错误提示模式
#define D_NORMAL_MODE               0x10        //正常工作模式
#define D_CAL_START_MODE        0x21        //校正模式，启动
#define D_CAL_END_MODE              0x23        //校正模式，完成
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

u16 U16_P_TotalTimes;           //当前脉冲 功率测量总时间
//u16   U16_V_TotalTimes;           //当前脉冲 电压测量总时间
//u16   U16_I_TotalTimes;           //当前脉冲 电流测量总时间

u16 U16_P_OneCycleTime;         //功率测量时间参数
//u16   U16_V_OneCycleTime;         //电压测量时间参数
//u16   U16_I_OneCycleTime;         //电流测量时间参数

u16 U16_P_Last_OneCycleTime;        //功率测量时间参数，上一次数量值
//u16   U16_V_Last_OneCycleTime;        //电压测量时间参数，上一次数量值
//u16   U16_I_Last_OneCycleTime;        //电流测量时间参数，上一次数量值

u16 U16_P_CNT;              //功率测量脉冲数量
//u16   U16_V_CNT;              //电压测量脉冲数量
//u16   U16_I_CNT;              //电流测量脉冲数量

u16 U16_P_Last_CNT;             //功率测量脉冲数量，上一次数量值
//u16   U16_V_Last_CNT;             //电压测量脉冲数量，上一次数量值
//u16   U16_I_Last_CNT;             //电流测量脉冲数量，上一次数量值

BOOL    B_P_TestOneCycle_Mode;          //功率测量模式 1:单周期测量，0:1S定时测量
//BOOL  B_V_TestOneCycle_Mode;
//BOOL  B_I_TestOneCycle_Mode;

BOOL    B_P_Last_TestOneCycle_Mode;
//BOOL  B_V_Last_TestOneCycle_Mode;
//BOOL  B_I_Last_TestOneCycle_Mode;
            
BOOL    B_P_OVERFLOW;                   // 功率脉冲周期 溢出标志位 
//BOOL      B_V_OVERFLOW;                   // 电压脉冲周期 溢出标志位
//BOOL      B_I_OVERFLOW;                   // 电流脉冲周期 溢出标志位

BOOL    B_P_Last_OVERFLOW;              // 功率脉冲周期 溢出标志位 
//BOOL      B_V_Last_OVERFLOW;              // 电压脉冲周期 溢出标志位
//BOOL      B_I_Last_OVERFLOW;              // 电流脉冲周期 溢出标志位

//BOOL      B_VI_Test_Mode;             //电压测量模式;0:电流测量模式
//u16       U16_VI_Test_Times;              
u16     U16_Cal_Times;  

u16     U16_AC_P;               //功率值 1000.0W
//u16       U16_AC_V;               //电压值 220.0V
//u16       U16_AC_I;               //电流值 4.545A
u32     U32_AC_E;               //用电量   0.01度
//u32       U32_AC_BACKUP_E;            //用电量备份    
//u8        U8_AC_COS;              //功率因素 0.00

u16     U16_REF_001_E_Pluse_CNT;            //0.01度电脉冲总数参考值
u16     U16_E_Pluse_CNT;                //脉冲个数寄存器

u32     U32_Cal_Times;                      //校正时间

u32     U32_P_REF_PLUSEWIDTH_TIME;          //参考功率 脉冲周期
//u32       U32_V_REF_PLUSEWIDTH_TIME;          //参考电压 脉冲周期
//u32       U32_I_REF_PLUSEWIDTH_TIME;          //参考电流 脉冲周期

u32     U32_P_CURRENT_PLUSEWIDTH_TIME;          //当前功率 脉冲周期
//u32       U32_V_CURRENT_PLUSEWIDTH_TIME;          //当前电压 脉冲周期
//u32       U32_I_CURRENT_PLUSEWIDTH_TIME;          //当前电流 脉冲周期

u16     U16_P_REF_Data;             //参考功率值,如以1000W校正。1000.0W
//u16       U16_V_REF_Data;             //参考电压  220.0V
//u16       U16_I_REF_Data;             //参考电流  1000W,220V条件下是4.545A

u8      U8_CURR_WorkMode;



/*=====================================================
 * 函数名称: void Read_CalData_EEPROM(void)
 * 函数功能: 
 * 输入参数:
 * 输出参数: 
 * 返    回: 
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
        
              
   // if (u8_temp != 0x55)  //判断EEPROM内是否存有校正值,若没有，则将参数设置为默认值
    {
      //设置默认值
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
 * Describe : rom空间初始化
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
  
//校正功率参考值(校正功率1000W)
  U16_P_REF_Data = 10000;       // 1000.0W
  //U16_V_REF_Data = 2200;      // 220.0V
  //U16_I_REF_Data = 4545;      // 4.545A
  
  
    
//校正功率参考值(校正功率250W)
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
  
//初始化单周期测量模式
  B_P_TestOneCycle_Mode = 1;
  //B_V_TestOneCycle_Mode = 1;
  //B_I_TestOneCycle_Mode = 1;
  B_P_Last_TestOneCycle_Mode = 1;
  //B_V_Last_TestOneCycle_Mode = 1;
  //B_I_Last_TestOneCycle_Mode = 1;

//开始测量，置溢出标志位为1  
  B_P_OVERFLOW = 1;
  //B_V_OVERFLOW = 1;
  //B_I_OVERFLOW = 1;
  
  B_P_Last_OVERFLOW = 1;
  //B_V_Last_OVERFLOW = 1;
  //B_I_Last_OVERFLOW = 1;
  
//上电初始化为电压测试模式 
  //B_VI_Test_Mode = 1;
  //IO_HLW8012_CF1_S = 1;
 // U16_VI_Test_Times = D_TIME1_2S;
  
//显示初始化
  //U8_DspMode = 0;
  //B_DIS_EN = FALSE;
  //U16_SendUart_Time = 0;

  Read_CalData_EEPROM();
  
}


void  PowerTimerInt(void)
{
//校正模式
    if (U8_CURR_WorkMode == D_CAL_START_MODE)
    {
        U32_Cal_Times++;
        if (U32_Cal_Times == D_TIME1_CAL_TIME)
        {
            U8_CURR_WorkMode = D_CAL_END_MODE;
            U16_REF_001_E_Pluse_CNT = U16_E_Pluse_CNT;      //记录36S时间内的脉冲数，此脉冲数表示0.01度用电量
        }
    }
    
//功率测量
    if (U16_P_CNT != 0)
    {
        U16_P_OneCycleTime++;
        U16_P_TotalTimes++;
    }  
    if (U16_P_TotalTimes >= D_TIME1_P_OVERFLOW)
    {
        B_P_OVERFLOW = TRUE; 
        B_P_Last_OVERFLOW = B_P_OVERFLOW;
        //清状态参数,重新开始测试
        U16_P_TotalTimes = 0;       //清溢出寄存器
        U16_P_OneCycleTime = 0;
        U16_P_CNT = 0;              //等待下一次中断开始计数
        B_P_TestOneCycle_Mode = 0;   //初始化为计数脉冲测量模式      
    }
    else if (U16_P_OneCycleTime == D_TIME1_100MS)
    {
      if (U16_P_CNT < 2)
      {
        // 100ms内只有一次中断，说明周期>100ms,采用单周期测量模式 
        B_P_TestOneCycle_Mode = 1;
      }
      else
      {
         // 100ms内有2次或以上数量脉冲，说明周期<100ms，采用计数脉冲测量模式
         B_P_TestOneCycle_Mode = 0;   
      }
    }
     
}



void PowerIntProcess(void)
{
//功率测量
    U16_P_TotalTimes = 0;       //完成一次有效的测量，溢出寄存器清零    
    U16_P_CNT++;
    if (B_P_OVERFLOW == TRUE)
    {  
        //从溢出模式转入,开始测量     
        B_P_TestOneCycle_Mode = 0;  //初始化为计数脉冲测量模式
        U16_P_TotalTimes = 0;       //清溢出寄存器清零
        U16_P_OneCycleTime = 0;     //清测量寄存器
        U16_P_CNT = 1;              
        B_P_OVERFLOW = FALSE;       //清溢出标志位
    }
    else
    {
        if (B_P_TestOneCycle_Mode == 1)
        {
            if (U16_P_OneCycleTime >= D_TIME1_100MS)
            {
                //单周期测量模式 
                U16_P_Last_OneCycleTime = U16_P_OneCycleTime;
                B_P_Last_TestOneCycle_Mode = B_P_TestOneCycle_Mode;
                B_P_OVERFLOW = FALSE;       //溢出标志位清零
                B_P_Last_OVERFLOW = B_P_OVERFLOW;
                 //清状态参数,重新开始测试
                B_P_TestOneCycle_Mode = 0;  //初始化为计数脉冲测量模式
                U16_P_TotalTimes = 0;       //完成一次有效的测量，溢出寄存器清零
                U16_P_OneCycleTime = 0;     //清测量寄存器
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
                B_P_OVERFLOW = FALSE;       //溢出标志位清零
                B_P_Last_OVERFLOW = B_P_OVERFLOW;
                //清状态参数,重新开始测试
                B_P_TestOneCycle_Mode = 0;  //初始化为计数脉冲测量模式
                U16_P_TotalTimes = 0;       //完成一次有效的测量，溢出寄存器清零
                U16_P_OneCycleTime = 0;     //清测量寄存器
                U16_P_CNT = 1;
            }
        }
    }
    
    
//校正模式
    if (U8_CURR_WorkMode == D_CAL_START_MODE)
    {
        //记录单位时间内的用电量
        U16_E_Pluse_CNT++;
    }
    
//电能计量
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
       //单周期测量模式
       b = U16_P_Last_OneCycleTime;
       b = b*1000;  //ms转换成us 
       u32_P_Period = b;   
    }
    else
    {
      //计数脉冲测量模式
      b = U16_P_Last_OneCycleTime;
      b = b*1000;
      u32_P_Period = b/(U16_P_Last_CNT-1);  
    }
    
    U32_P_CURRENT_PLUSEWIDTH_TIME = u32_P_Period;      // 校正时取U32_P_CURRENT_PLUSEWIDTH_TIME参数作为参考值     
    a = U16_P_REF_Data * U32_P_REF_PLUSEWIDTH_TIME;        
    U16_AC_P = a/U32_P_CURRENT_PLUSEWIDTH_TIME;
    
    if (U16_AC_P == 0xffff)     //开机时U32_P_CURRENT_PLUSEWIDTH_TIME = 0，计算溢出
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

