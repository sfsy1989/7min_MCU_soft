#include "uart.h"
#include <stdarg.h>
#include <stdio.h>
#include"stm32f0xx_gpio.h"
#include"stm32f0xx_usart.h"
#include"stm32f0xx_misc.h"
#include"stm32f0xx_rcc.h"



void USART_Configuration(void)//���ڳ�ʼ������
{  

	 GPIO_InitTypeDef GPIO_InitStructure;
	 USART_InitTypeDef USART_InitStructure;

   /* Enable GPIO clock */
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
   /* Enable USART clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 	
	     /*
        *  USART1_TX -> PA9 , USART1_RX ->        PA10
        */                                       	
   /* Connect PA9 to USART1_Tx */
   GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
   /* Connect PA10 to USART1_Rx */
   GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);   
   /* Configure USART Tx as alternate function push-pull */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);     
   /* Configure USART Rx as alternate function push-pull */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
	 
   USART_InitStructure.USART_BaudRate = 9600;//���ô��ڲ�����
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;//��������λ
   USART_InitStructure.USART_StopBits = USART_StopBits_1;//����ֹͣλ
   USART_InitStructure.USART_Parity = USART_Parity_No;//����Ч��λ
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//����������
   USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;//���ù���ģʽ 
   /* USART configuration */
   USART_Init(USART1,&USART_InitStructure);//������ṹ��
//   USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�	 
   /* Enable USART */
   USART_Cmd(USART1, ENABLE);//ʹ�ܴ���1

}

void UART_send_byte(uint8_t byte) //����1�ֽ�����
{
 	while(!((USART1->ISR)&(1<<7)));
 	USART1->TDR=byte;	
}		

void UART_Send(uint8_t *Buffer, uint32_t Length)
{
	while(Length != 0)
	{
		while(!((USART1->ISR)&(1<<7)));//�ȴ�������
		USART1->TDR= *Buffer;
		Buffer++;
		Length--;
	}
	UART_send_byte('\r');
	UART_send_byte('\n');
}


