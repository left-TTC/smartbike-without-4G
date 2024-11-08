#include "stm32f10x.h"                  // Device header
#include <string.h>
#include <stdlib.h> 
#include <stdio.h>
#include "Battery.h"
#include "Flash.h"
#include <time.h>
#include "cJSON.h"
#include "AD.h"
int command_verify(const char *cmdstr,char * signaturestr,char * address,char * publicKeystr); //verify signature
#define BUFFER_SIZE3 1024
     
volatile int Tooth_Flag = 1;
extern int BikeLock_number;
extern int BatteryLock_number;
extern int once_load;
extern char UUiD;
extern char UUID;
extern char Flash_Address;
extern char Flash_store;
char Command[200];
char PubKey[512];
char Signature[512];
char Address[50];
char recievedJson[1024];
int canDOACommand = 0;
extern int ifHaveSuperUser;        //Check whether a superuser exists
time_t usingStamp;   //currently using timestamp
//----------------------------------------Init-----------------
void Blue_Init(void){//USART3
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

void Send_AT_Command(const char* command){
    while (*command) {
        BlueAT_SendData(*command++);
    }  
    //BlueAT_SendData('\r');
    //BlueAT_SendData('\n');
}
void Send_CommandStart(void){       //means command start
	Send_AT_Command("<BN");
}
void Send_CommandOver(void){        //command over
	Send_AT_Command("OR>");
}
void Send_CommandFlashCarve(void){
	Send_AT_Command("+++");
}
//----------------------------------------Send information----------------------
void Battery_openNotify(void){            //tell bluetooth that my lock'state is open
	Send_AT_Command("battery1\r\n");
}
void Battery_offNotify(void){
	Send_AT_Command("battery2\r\n");
}
void Battery_openFail(void){
	Send_AT_Command("battery3\r\n");
}
void Battery_lockFail(void){
	Send_AT_Command("battery4\r\n");
}
void NormalOperationFlag(void){
	Send_AT_Command("ready\r\n");
}
//-----------------------------------check related to BlueTooth-------------------
void Blue_check(void){
	uint8_t PIN_State = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1);          //BIT_SET.high 1         BIT_RESET.low 0	
	if(PIN_State == 1){
		Tooth_Flag = 1;
	}else {
		Tooth_Flag = 0;
	}
}
//------------------------------verify-------------------------------------------------
int Verify_Time(const char *time){
	time_t recievedTime = ConvertUint_time(time);
	if(recievedTime > usingStamp){
		usingStamp = recievedTime; //update a new time
		Update_Store_TimeStamp(time);
		return 1;
	}
	return 0;
}
int Verify_UUID(const char *id){
	if(strcmp(id, &UUID) == 0){
		return 1;
	}
	return 0;
} 
void DoToCommand(char time[20],char BikeCommand[20],char UUid[30]){
	cJSON *root = cJSON_Parse(Command);   //parse the json string by CJSON
	if (root == NULL) {
		const char* error = cJSON_GetErrorPtr();
        return;  
    }
	for(int i=13;i<23;i++){               //get now timeStamp
		time[i-13] = Command[i];
	}
	time[10]='\0';
    cJSON *command_item = cJSON_GetObjectItem(root, "command");
    if (cJSON_IsString(command_item) && command_item->valuestring != NULL) {
        strncpy(BikeCommand, command_item->valuestring, strlen(command_item->valuestring));
        BikeCommand[strlen(command_item->valuestring)] = '\0'; 
    }
    cJSON *UUID_item = cJSON_GetObjectItem(root, "UUID");
    if (cJSON_IsString(UUID_item) && UUID_item->valuestring != NULL) {
        strncpy(UUid, UUID_item->valuestring, strlen(UUID_item->valuestring));
        UUid[strlen(UUID_item->valuestring)] = '\0'; 
	}
	cJSON_Delete(root);
}
//------------------------------------IQ------------------------------------------------ 
char Get_Recieved[BUFFER_SIZE3];
void parse_ALLJSON(const char *JsonString){
	cJSON *root = cJSON_Parse(JsonString);   //parse the json string by CJSON
	if (root == NULL) {
		const char* error = cJSON_GetErrorPtr();
        return;  
    }
	cJSON *cmd_item = cJSON_GetObjectItem(root, "cmd");
    if (cJSON_IsString(cmd_item) && cmd_item->valuestring != NULL) {
        strncpy(Command, cmd_item->valuestring, strlen(cmd_item->valuestring));
        Command[strlen(cmd_item->valuestring)] = '\0'; 
    }
    cJSON *PubKey_item = cJSON_GetObjectItem(root, "PubKey");
    if (cJSON_IsString(PubKey_item) && PubKey_item->valuestring != NULL) {
        strncpy(PubKey, PubKey_item->valuestring, strlen(PubKey_item->valuestring));
        PubKey[strlen(PubKey_item->valuestring)] = '\0'; 
    }
    cJSON *signature_item = cJSON_GetObjectItem(root, "signature");
    if (cJSON_IsString(signature_item) && signature_item->valuestring != NULL) {
        strncpy(Signature, signature_item->valuestring, strlen(signature_item->valuestring));
        Signature[strlen(signature_item->valuestring)] = '\0'; 
    }
    cJSON *address_item = cJSON_GetObjectItem(root, "address");
    if (cJSON_IsString(address_item) && address_item->valuestring != NULL) {
        strncpy(Address, address_item->valuestring, strlen(address_item->valuestring));
        Address[strlen(address_item->valuestring)] = '\0'; 
    }
    cJSON_Delete(root);
}
void DoToTheseJson(void){	
	char time[20];
    char BikeCommand[20];
	char uuid[30];
	parse_ALLJSON(Get_Recieved);
	DoToCommand(time,BikeCommand,uuid);
	if(ifHaveSuperUser == 1){                 //Run only after receiving data =>superUser init
		Flash_Register(Address,time,NULL,NULL);
	}
	if(strcmp(&Flash_Address, Address) == 0){ //if the user is the superuser
		if(Verify_UUID(uuid) == 1){
			if(command_verify(Command,Signature,Address,PubKey) == 1 ){
				if(Verify_Time(time)==1){
					if(strcmp(BikeCommand, "batterylock") == 0){
						BatteryLock_number = 1;
					}
					else if(strcmp(BikeCommand, "bikelock") == 0){
						BikeLock_number = 1; 
					}
					else if(strcmp(BikeCommand, "unbikelock") == 0){
						BikeLock_number = 0;
					}
				}
			}
		}
	}
}
volatile uint16_t bufferIndex1 = 0;
uint16_t index1 = 0;
char receivedata1[BUFFER_SIZE3];
int SureDeviceName = 0;
extern int NeedClean;
void USART3_IRQHandler(void){
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET){
        char byte = USART_ReceiveData(USART3); 
        if (byte == '\n' || index1 >= BUFFER_SIZE3 - 1){
            receivedata1[index1] = '\0'; 
			if(strstr(receivedata1,"cmd")!=NULL){
				strcpy(Get_Recieved, receivedata1);      
				canDOACommand = 1; 
				GPIO_SetBits(GPIOC, GPIO_Pin_13);
			}if(strstr(receivedata1,"OK")!=NULL){
				SureDeviceName ++;
			}if(strstr(receivedata1,"lean")!= NULL){
				NeedClean = 1;
			}
            memset(receivedata1, 0, BUFFER_SIZE3); 
            index1 = 0;             
        }else{
            receivedata1[index1++] = byte; 
        }          
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);  
    }
}
void CreateSendToPhoneJson(char*SendJSON,const char*BatteyVoltage,const char*BatteryState,const char*rotata){
	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "BatteryVoltage", BatteyVoltage);
    cJSON_AddStringToObject(root, "BatteryState", BatteryState);
	cJSON_AddStringToObject(root, "Rotate", rotata);
	cJSON_AddStringToObject(root, "UUID", &UUID);
	char *json_str = cJSON_Print(root);
	if (json_str != NULL){
		snprintf(SendJSON, strlen(json_str)+2, "%s", json_str);
	}
	cJSON_Delete(root);
    free(json_str);
}
void Date_DeviceToPhone(void){
	char BatteyVoltage[20];
	char BatteryState[10];
	char rotata[10];
	char SendJSON[800];
	BatteryVoltage_get(BatteyVoltage);     //get BatteryPower
	sendBatteryLockState(BatteryState);    //BatteryState
	Send_CurrentRotate(rotata);
	CreateSendToPhoneJson(SendJSON,BatteyVoltage,BatteryState,rotata);
	Send_CommandStart();
	Send_AT_Command(SendJSON);	
	Send_CommandFlashCarve();
	Send_AT_Command(&Flash_store);
	Send_CommandOver();
}

