#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <string.h>
#include "BLUETOOTH.h"
#include "Flash.h"
#include "Delay.h"
extern int BatteryLock_number;
uint8_t LastlyPinState ; 
char UUID[30];
char UUiD[30];
char Name[12];
extern int SureDeviceName;
void Battery_Init(void){
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
void BatteryLock_on(void){
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}
void BatteryLock_off(void){
	GPIO_SetBits(GPIOA,GPIO_Pin_6);
}
void BatteryLock_Reset(void){        //need wait for unlocking at intervals of 1s before reseting(main)
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	GPIO_ResetBits(GPIOA,GPIO_Pin_6);
}
void changeDeviceName(void){
	uint32_t ifNeedChangeName = read_Flash(0x0800F80C);
	if(ifNeedChangeName != 0x01){
		char DEVICEid[7];
		strncpy(DEVICEid, UUID + strlen(UUID) - 6, 6);
		DEVICEid[6]='\0';
		char DEVICENAME[30];
		sprintf(DEVICENAME,"AT+LENABIKE_%s\r\n",DEVICEid);;
		Send_AT_Command("AT+ENAT\r\n");
		Delay_ms(300);
		Send_AT_Command(DEVICENAME);
		Delay_ms(300);
		Send_AT_Command("AT+REST\r\n");         //used to change name
		if(SureDeviceName == 2){
			sprintf(Name,"BIKE_%s", DEVICEid);
		}
	}
}
void GetUniqueID(void){
	uint32_t UID[3];
	UID[0] = *(__IO uint32_t*)(0x1FFFF7E8); // UID[0]
    UID[1] = *(__IO uint32_t*)(0x1FFFF7EC); // UID[1]
    UID[2] = *(__IO uint32_t*)(0x1FFFF7F0); // UID[2]	
	sprintf(UUID, "%08X%08X%08X", UID[0], UID[1], UID[2]);
	sprintf(UUiD, "U:%08X%08X%08X", UID[0], UID[1], UID[2]);
}
void sendBatteryLockState(char *BatteryState){    //used to tell device the lock state when start driving and give LastlyPinState a state
	uint8_t PinState = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5); //batteryLockState	
	if(PinState == 1){  //means lock has been opened
		sprintf(BatteryState,"battery1");
	}
	else if(PinState == 0){
		sprintf(BatteryState,"battery2");
	}
}
void Get_BatteryLockState(void){
	uint8_t PIN_State = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
	if (LastlyPinState == 1 && PIN_State == 0){//automatically lock when find the physical lock status
		BatteryLock_number = 0;                     //lock
	}	
	LastlyPinState = PIN_State;
}
void checkBatteryCommand(void){
	int PIN = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);	
	if(BatteryLock_number == 1){        //means in open command	
		if(PIN == 1){                   //means is's on now 
			Battery_openNotify();
		}
		else{
			Battery_openFail();
		}
	}else if (BatteryLock_number == 0){ //means in off command
		if(PIN == 0){                   //means is's off now 	
			Battery_offNotify();
		}
		else{
			Battery_lockFail();
		}
	}	
}



