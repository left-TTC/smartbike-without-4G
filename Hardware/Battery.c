#include "stm32f10x.h"                  // Device header
#include <string.h>
#include "BLUETOOTH.h"

extern int BatteryLock_number;

void Battery_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_4;        //used  to control the battery lock
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;                  //used  to detect battery lock feedback
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

//-----------------------------Battery Lock command---------------------------
void BatteryLock_on(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}

void BatteryLock_off(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_6);
}

void BatteryLock_Reset(void)        //need wait for unlocking at intervals of 1s before reseting(main)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	GPIO_ResetBits(GPIOA,GPIO_Pin_6);
}

void Get_BatteryLockState(void)
{
	uint8_t PIN_State = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
	
	if(PIN_State == 1)                      //high,means batterylock open
	{
		if(BatteryLock_number ==0)               //it means have send lockoff command,but lock is still open
		{
			Battery_lockFail();
		}
		else
		{
			Battery_openNotify();            //send a open signal to app
		}
	}
	else
	{
		if(BatteryLock_number ==1)               //it means have send lockopen command,but lock is still off
		{
			Battery_openFail();
		}
		else
		{
			Battery_offNotify();            //send a open signal to app
		}	
	}
}
//-----------------------------


