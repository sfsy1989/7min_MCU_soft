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
    u16 IrSendLen;   //����ѧϰ���ݻ�������ⷢ�����ݵĳ���
    u16 IrSendIdx;   //����ѧϰ���ݻ�������ⷢ�����ݵ�����
	u16 IrLearnLen;
	u16 IrLearnIdx;
	u8 *pIrLearnBuf;	
	u8 *pIrSendBuf;
	u16 IrFre;              //����ѧϰ�ز�Ƶ�ʣ�Ŀǰ�̶�Ϊ38KHZ
    u32 LearningPreTimes;   //����ѧϰ��ǰ������ʱ��ֵ��
    u32 H_TimerLoadValue;   //����ѧϰ�ز�ռ�ձȣ�Ŀǰ�̶�Ϊ1:3
    u16 LearnIrDataLenMax;
	u16 LearnIrDataLenMin;
    IR_STATUS_ENUM IrStatus;       //����״̬
    u8 RemainFrame;
    u8 BigDataStartFlag;    //������������־�����ں���ѧϰ����������ķ���ͬ��
    u8 IrLearnResult;       //����ѧϰ���
    u8 IrLearninExTimerCnt; //����ѧϰ��չ���������üĴ�������1�������ѧϰ����	
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
