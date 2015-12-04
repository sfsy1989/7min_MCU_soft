#ifndef __UART_H__
#define __UART_H__

#include "stm32f0xx.h"
#include <stdio.h>


void USART_Configuration(void);
void UART_send_byte(uint8_t byte);
void UART_Send(uint8_t *Buffer, uint32_t Length);


#endif


