/********************************************************************************
* @File         name: Fault.c
* @Author:      Andy and Left-TTC @github
* @Version:     1.01
* @Date:        2024-11-16
* @Description: Handling when errors occur
********************************************************************************/
#include "stm32f10x.h"                  // Device header
void IWDG_Init(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  
    IWDG_SetPrescaler(IWDG_Prescaler_64);  
    IWDG_SetReload(0xFFF);
	IWDG_Enable();
}
void IWDG_Feed(void){
	IWDG_ReloadCounter();
}

