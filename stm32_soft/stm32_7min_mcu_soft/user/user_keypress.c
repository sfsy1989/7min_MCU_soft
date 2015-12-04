#include "user_keypress.h"
#include "user_type.h"
#include "stm32f0xx_gpio.h"

extern u8 Int_Event;
u8 keyStatus2[2] = {0};


void Process_KeyPress_Event(void)
{
	static u8 count = 0;
	if((GPIO_ReadInputData(GPIOA)&GPIO_Pin_3)==0){
		keyStatus2[0] = 1;
	}
	if((GPIO_ReadInputData(GPIOA)&GPIO_Pin_7)==0)	{
		keyStatus2[1] = 1;
	}	

	if((GPIO_ReadOutputData(GPIOA)&GPIO_Pin_1) == 0){
		count++;
		if(count == 20){
			GPIO_SetBits(GPIOA,GPIO_Pin_1);
			count = 0;
		}
	}

	if(keyStatus2[0] == 1){
		if((GPIO_ReadInputData(GPIOA)&GPIO_Pin_3)!=0){
			keyStatus2[0] = 0;
			Int_Event = 0x02;
			Send_Int_Event();
			if((GPIO_ReadOutputData(GPIOA)&GPIO_Pin_5) == 0){
				GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_SET);
			}else{
				GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_RESET);	
			}
		}	
	}
	if(keyStatus2[1] == 1){
		if((GPIO_ReadInputData(GPIOA)&GPIO_Pin_7)!=0){
			keyStatus2[1] = 0;
			Int_Event = 0x02;
			Send_Int_Event();
			if((GPIO_ReadOutputData(GPIOA)&GPIO_Pin_6) == 0){
				GPIO_WriteBit(GPIOA,GPIO_Pin_6,Bit_SET);
			}else{
				GPIO_WriteBit(GPIOA,GPIO_Pin_6,Bit_RESET);	
			}	
		}	
	}
	
}


void Send_Int_Event(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
}

