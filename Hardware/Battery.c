/********************************************************************************
* @File         name: Battery.c
* @Author:      Andy and Left-TTC @github
* @Version:     1.01
* @Date:        2024-11-16
* @Description: A library of processes related to Bluetooth
********************************************************************************/
#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <string.h>
#include "BLUETOOTH.h"
#include "Flash.h"
#include "Delay.h"
#include "Flashdefine.h"
extern int BatteryLock_number;
uint8_t LastlyPinState ; 
char UUID[30];
char Name[12];
int needToSendUnusual = 0;
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
/********************************************************
* Function name :changeDeviceName
* Description   :Query and change the Bike name
* Parameter     :None
* Return        :None
**********************************************************/
void changeDeviceName(void){
	uint32_t ifNeedChangeName = read_Flash(SuperUserFlash_IfRecordName_Record);
	if(ifNeedChangeName != 0xAAAA){          
		char DEVICEid[7];
		strncpy(DEVICEid, UUID + strlen(UUID) - 6, 6);
		DEVICEid[6]='\0';
		char DEVICENAME[30];
		sprintf(DEVICENAME,"AT+LENABIKE_%s\r\n",DEVICEid);;
		Delay_ms(1000);
		Send_AT_Command("AT+ENAT\r\n");
		Delay_ms(1000);
		Send_AT_Command(DEVICENAME);
		Delay_ms(1000);
		Send_AT_Command("AT+REST\r\n");         //used to change name
		sprintf(Name,"BIKE_%s", DEVICEid);
		if(SureDeviceName == 2){
			Update_Struct_DeviceName(Name);
			Flash_Write(SuperUserFlash_IfRecordName_Record,0x01);
		}
	}
}
/********************************************************
* Function name :GetUniqueID
* Description   :Gets the CPU number of Stm32
* Parameter     :None
* Return        :None
**********************************************************/
void GetUniqueID(void){
	uint32_t UID[3];
	UID[0] = *(__IO uint32_t*)(0x1FFFF7E8); // UID[0]
    UID[1] = *(__IO uint32_t*)(0x1FFFF7EC); // UID[1]
    UID[2] = *(__IO uint32_t*)(0x1FFFF7F0); // UID[2]	
	sprintf(UUID, "%08X%08X%08X", UID[0], UID[1], UID[2]);
}
/********************************************************
* Function name :Check_Now_Battery_Lock_State
* Description   :Detects the current cabin lock open status
* Parameter     :
* @BatteryState  Will be assigned a value: battery1 => unlock ; battery2 => lock
* Return        :None
**********************************************************/
void Check_Now_Battery_Lock_State(char *BatteryState){                  //used to tell device the lock state when start driving and give LastlyPinState a state
	uint8_t PinState = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);         //batteryLockState	
	if(PinState == 1){                                                  //means lock has been opened
		sprintf(BatteryState,"battery1");
	}
	else if(PinState == 0){
		sprintf(BatteryState,"battery2");
	}
}
/********************************************************
* Function name :Get_BatteryLockState
* Description   :Detects and performs automatic cockpit lock closing
* Parameter     :None
* Note          :
* @BatteryLock_number    0--The lockout operation will be performed in the main program
*						 1--The unlocking operation will be performed in the main program
*						 2--Initial state, no operation will be performed
* Return        :None
**********************************************************/
void Get_BatteryLockState(void){
	uint8_t PIN_State = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
	if (LastlyPinState == 1 && PIN_State == 0){
		BatteryLock_number = 0;                  
	}	
	LastlyPinState = PIN_State;
}
/********************************************************
* Function name :checkBatteryCommand
* Description   :Check the execution of the received cockpit lock command
* Parameter     :
* @BatteryState  Will be assigned a value: battery1 => unlock ; battery2 => lock ; battery3 => openFail ; battery4 => LockFali 
* Note          :
* @needToSendUnusual	Normally, the execution status is not checked,we use this Flag to send Unusual status like battery3 and battery4		 
* Return        :None
**********************************************************/
void checkBatteryCommand(char* BatteryState){
	int PIN = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);	
	if(BatteryLock_number == 1){              //means in open command	
		if(PIN == 1){                         //means is's on now 
			sprintf(BatteryState,"battery1");
		}else{
			sprintf(BatteryState,"battery3"); //open fail
		}
	}else if (BatteryLock_number == 0){       //means in off command
		if(PIN == 0){                         //means is's off now 	
			sprintf(BatteryState,"battery2");
		}else{
			sprintf(BatteryState,"battery4"); //lock fail
		}
	}
	needToSendUnusual = 1;
}



