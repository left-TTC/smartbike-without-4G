#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <string.h>
#include "BLUETOOTH.h"

extern int BatteryLock_number;
uint8_t LastlyPinState ; 
char UUID[30];

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
void  GetUniqueID(void)
{
	uint32_t UID[3];
	UID[0] = *(__IO uint32_t*)(0x1FFFF7E8); // UID[0]
    UID[1] = *(__IO uint32_t*)(0x1FFFF7EC); // UID[1]
    UID[2] = *(__IO uint32_t*)(0x1FFFF7F0); // UID[2]
	
	sprintf(UUID, "%08X%08X%08X", UID[0], UID[1], UID[2]);
}
void GetStateWhenopen(void)         //used to tell device the lock state when start driving and give LastlyPinState a state
{
	LastlyPinState = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5); //batteryLockState	
	if(LastlyPinState == 1)  //means lock has been opened
	{
		Battery_openNotify();
	}
	else if(LastlyPinState == 0)
	{
		Battery_offNotify();
	}
	GetUniqueID();
	char SendUUID[35];
	snprintf(SendUUID, sizeof(SendUUID), "U:%s", UUID);
	Send_AT_Command(SendUUID);
}

void Get_BatteryLockState(void)
{
	uint8_t PIN_State = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
	
	if (PIN_State == LastlyPinState)              // BatteryLock_number == 2
	{
		//do nothing
	}
	else if (LastlyPinState == 1 && PIN_State == 0) //automatically lock when find the physical lock status
	{
		BatteryLock_number = 0; //lock
	}
	
	LastlyPinState = PIN_State;
}

void checkBatteryCommand(void)
{
	int PIN = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
	
	if(BatteryLock_number == 1)         //means in onlock command
	{
		if(PIN == 1)     //means is's on now 
		{
			Battery_openNotify();
		}
		else
		{
			Battery_openFail();
		}
	}
	else if (BatteryLock_number == 0)
	{
		if(PIN == 0)     //means is's off now 
		{
			Battery_offNotify();
		}
		else
		{
			Battery_lockFail();
		}
	}
	
	
}
//-----------------------------


