#ifndef __USER_IR_H__
#define __USER_IR_H__
#include "user_type.h"

#define IR_BUF_SIZE             1280

typedef enum
{
    IR_SEDN_START = 0,
    IR_SENDING,
    IR_LEARN_START,
    IR_LEARNING,
    IR_SENDING_END,
    IR_LEARNING_END,
    IR_FORCE_LEARN,
    IR_FORCE_LEARNING,
    IR_FORCE_LEARN_END
}IR_STATUS_ENUM;


typedef struct
{
    u16 IrSendLen;   //红外学习数据或请求红外发送数据的长度
    u16 IrSendIdx;   //红外学习数据或请求红外发送数据的索引
	u16 IrLearnLen;
	u16 IrLearnIdx;
	u8 *pIrLearnBuf;	
	u8 *pIrSendBuf;
	u16 IrFre;              //红外学习载波频率，目前固定为38KHZ
    u32 LearningPreTimes;   //红外学习先前读到的时间值。
    u32 H_TimerLoadValue;   //红外学习载波占空比，目前固定为1:3
    u16 LearnIrDataLenMax;
	u16 LearnIrDataLenMin;
    IR_STATUS_ENUM IrStatus;       //工作状态
    u8 RemainFrame;
    u8 BigDataStartFlag;    //大数据启动标志，用于红外学习，红外命令的发送同步
    u8 IrLearnResult;       //红外学习结果
    u8 IrLearninExTimerCnt; //红外学习扩展计数器，该寄存器大于1代表红外学习结束	
}IR_STRUCT;


void Ir_Data_Init(void);

void IrCapture(void);

void Ir_Test_Learn_End(void);

void Ir_Event_Poll(void);

void Ir_Learn_Init(void);
void Ir_Learn_Enable(void);
void Ir_Learn_Disable(void);

void Ir_Send_Enable(void);
void Ir_Send_Disable(void);



#endif
