#include "stm32f10x.h"                  // Device header
#include <string.h>
#include <stdlib.h> 
#include <stdio.h>
#include "Battery.h"
#define BUFFER_SIZE3 1024
     
volatile int Tooth_Flag = 1;
extern int BikeLock_number;
extern int BatteryLock_number;
extern int once_load;
extern char UUID;
char Command[100];
char PubKey[100];
char Signature[100];
char Address[100];
int canDOACommand = 0;


//----------------------------------------Init-----------------
void Blue_Init(void)//USART3
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); 
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;       
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;         
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;          //used  to control the device other than the central control
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;     
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure); 
	
	USART_Cmd(USART3, ENABLE);
}
//----------------------------------send function-----------------------
void BlueAT_SendData(uint8_t data)         
{
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    USART_SendData(USART3, data);
}

void Send_AT_Command(const char* command)    
{
    while (*command) {
        BlueAT_SendData(*command++);
    }
  
    BlueAT_SendData('\r');
    BlueAT_SendData('\n');
}
//----------------------------------------Send information----------------------
void Battery_openNotify(void)            //tell bluetooth that my lock'state is open
{
	Send_AT_Command("battery1");
}

void Battery_offNotify(void)
{
	Send_AT_Command("battery2");
}

void Battery_openFail(void)
{
	Send_AT_Command("battery3");
}

void Battery_lockFail(void)
{
	Send_AT_Command("battery4");
}

void NormalOperationFlag(void)
{
	Send_AT_Command("ready");
}
//-----------------------------------check related to BlueTooth-------------------
void Blue_check(void)
{
	uint8_t PIN_State = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1);          //BIT_SET.high 1         BIT_RESET.low 0
	
	if(PIN_State == 1)
	{
		Tooth_Flag = 1;
	}
	else 
	{
		Tooth_Flag = 0;
	}
}
//------------------------------verify-------------------------------------------------
int Verify_Time(char *time)
{
	return 1;
}
int Command_verify(char * command, char * signature ,char * pubkey,char * address)
{
	return 1;
}
int Verify_UUID(const char *id)
{
	if(strcmp(id, &UUID) == 0)
	{
		return 1;
	}
	return 0;
}

void DoToCommand(char time[20],char BikeCommand[20],char UUid[30])
{
	//{\"TimeStamp\":1730548906,\"command\":\"batterylock\",\"UUID\":\"066EFF505657874887184322\"}
	//0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
	//0         1         2         3         4         5         6         7         8         9
	for(int i=15;i<25;i++){
		time[i-15]=Command[i];
	}
	time[10]='\0';
	int k=0;
	for(int i=40;Command[i] != '\0';i++,k++){
		if(Command[i] == '\\'){
			break;
		}
		BikeCommand[k]=Command[i];
	}
	BikeCommand[k] = '\0';
	for(int i= 54;Command[i] != '\0';i++){
		if(Command[i+k] == '\\'){
			break;
		}
		UUid[i-54]=Command[i+k];
	}
	UUid[24]='\0';
}
void DoToTheseJson(void)
{	
	char time[20];
    char BikeCommand[20];
	char uuid[30];
	DoToCommand(time,BikeCommand,uuid);
	if(Verify_UUID(uuid) == 1){
		if(Verify_Time(time) == 1){
			if(Command_verify(Command,Signature,PubKey,Address) == 1 )
			{
				if(strcmp(BikeCommand, "batterylock") == 0)
				{
					BatteryLock_number = 1;
				}
				else if(strcmp(BikeCommand, "bikelock") == 0)
				{
					BikeLock_number = 1; 
				}
				else if(strcmp(BikeCommand, "unbikelock") == 0)
				{
					BikeLock_number = 0;
					once_load = 1;
				}
			}
		}
	}
}
//------------------------------------IQ------------------------------------------------ 
void parseData(char *data) {
    char *cmdStart = strstr(data, "\"cmd\":\"");
    char *pubKeyStart = strstr(data, "\"PubKey\":\"");
    char *signatureStart = strstr(data, "\"signature\":\"");
    char *addressStart = strstr(data, "\"address\":\"");

    if (cmdStart) {
        char *cmdEnd = strstr(cmdStart, "\"}");
        if (cmdEnd) {
            size_t length = cmdEnd - cmdStart - 7; 
            strncpy(Command, cmdStart + 7, length); 
            Command[length] = '\0'; 
        }
    }

    if (pubKeyStart) {
        sscanf(pubKeyStart, "\"PubKey\":\"%[^\"]\"", PubKey);
    }
    if (signatureStart) {
        sscanf(signatureStart, "\"signature\":\"%[^\"]\"", Signature);
    }
    if (addressStart) {
        sscanf(addressStart, "\"address\":\"%[^\"]\"", Address);
    }
}

volatile uint16_t bufferIndex1 = 0;
uint16_t index1 = 0;
char receivedata1[BUFFER_SIZE3];
//         0xFFE1: Write Without Response APP --> UART?           0xFFE2: Notify  UART --> APP?
//   0x01 LockBike    0x02 Unlockbike  0x03 batterylock  0x04  batteryUnlock
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  
    {
        char byte = USART_ReceiveData(USART3); 

        if (byte == '\n' || index1 >= BUFFER_SIZE3 - 1)
        {
            receivedata1[index1] = '\0'; 
            
            parseData(receivedata1);
            
            memset(receivedata1, 0, BUFFER_SIZE3); 
            index1 = 0; 
            canDOACommand = 1; 
        }
        else
        {
            receivedata1[index1++] = byte; 
        }
        GPIO_SetBits(GPIOC, GPIO_Pin_13);  
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);  
    }
}

