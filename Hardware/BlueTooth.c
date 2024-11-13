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
char PubKey[150];
char Signature[150];
char Address[50];
char recievedJson[1024];
int canDOACommand = 0;
extern int ifHaveSuperUser;        //Check whether a superuser exists
time_t usingStamp;   //currently using timestamp
char Err[20];
int needUpUsingTime = 0;           //reset =>0=>need updata time;up=>1=>need't
int CanTrust = 0;
extern int isRent ;
extern int isSuper;
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
void TimeClock_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = 9999; 
    TIM_TimeBaseStructure.TIM_Prescaler = 7199; 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_Cmd(TIM2, ENABLE);
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
void ResetUser(void){
	isSuper = 0;
	isRent = 0;
	CanTrust = 0;
}
//------------------------------verify-------------------------------------------------
int Verify_Time(const char *time){
	time_t recievedTime = ConvertUint_time(time);
	if(((recievedTime +20) > usingStamp )&& needUpUsingTime == 1){
		return 1;
	}if(needUpUsingTime == 0 && (isSuper == 1 || CanTrust == 1)){       //upDateTime
		needUpUsingTime =1;                          //means have already update the time
		usingStamp = recievedTime;                   //superUser's time update the user's time
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
	cJSON *timestamp_item = cJSON_GetObjectItem(root, "TimeStamp");
    if (cJSON_IsNumber(timestamp_item)) {
        snprintf(time, 20, "%010ld", (long)timestamp_item->valueint);  // ??10????
    }
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
    char BikeCommand[100];
	char uuid[30];
	parse_ALLJSON(Get_Recieved);
	DoToCommand(time,BikeCommand,uuid);
	if(isRent == 0 && isSuper == 0){
		VerifyIf_Superser();
		VerifyIf_RentUser();
	}
	if(ifHaveSuperUser == 1){                 //Run only after receiving data =>superUser init
		Flash_Register(Address,time,NULL,NULL);
	}
	if(isSuper == 1){ //if the user is the superuser
		if(Verify_UUID(uuid) == 1){
			if(command_verify(Command,Signature,Address,PubKey) == 1 ){
				if(Verify_Time(time)==1){
					if(strcmp(BikeCommand, "batterylock") == 0){
						BatteryLock_number = 1;
					}else if(strcmp(BikeCommand, "bikelock") == 0){
						BikeLock_number = 1; 
					}else if(strcmp(BikeCommand, "unbikelock") == 0){
						BikeLock_number = 0;
					}else if(strstr(BikeCommand,"RentAdd")!= NULL){
						Flash_AddRentUser(BikeCommand);
					}else if(strstr(BikeCommand,"SuChange")!=NULL){ //every time when lock the car
						ChangeSuperUser(BikeCommand);
						isSuper = 0;
					}else if(strstr(BikeCommand,"addPAC")!=NULL){
						AddPhoneAndChat(BikeCommand);
					}
				}else{strcpy(Err, "TimeErr");}//illegal ERR
			}else{strcpy(Err, "SignErr");}//illegal siganature
		}else{strcpy(Err, "IDErr");}//recieve ERR
	}else if(isRent == 0){strcpy(Err, "UserErr");}//means user's address err}
	if(isRent == 1){
		if(Verify_UUID(uuid) == 1){
			if(command_verify(Command,Signature,Address,PubKey) == 1 ){
				if(Verify_Time(time)==1){
					if(strcmp(BikeCommand, "batterylock") == 0){
						BatteryLock_number = 1;
					}else if(strcmp(BikeCommand, "bikelock") == 0){
						BikeLock_number = 1; 
					}else if(strcmp(BikeCommand, "unbikelock"  ) == 0){
						BikeLock_number = 0;
					}
				}else{strcpy(Err, "TimeErr");}//illegal ERR
			}else{strcpy(Err, "SignErr");}//illegal siganature
		}else{strcpy(Err, "IDErr");}//recieve ERR
	}else if(isSuper==0){strcpy(Err, "UserErr");}
}
volatile uint16_t bufferIndex1 = 0;
uint16_t index1 = 0;
char receivedata1[BUFFER_SIZE3];
int SureDeviceName = 0;
extern int NeedClean;
void USART3_IRQHandler(void){
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET){
        char byte = USART_ReceiveData(USART3); 
        if (byte == '\n' || index1 >= BUFFER_SIZE3 - 1 || byte == '>'){
			if(byte == '>'){
				receivedata1[index1]=byte;
				receivedata1[index1+1] = '\0';
			}else{
				receivedata1[index1] = '\0'; 
			}
			if(strstr(receivedata1,"OK")!=NULL){
				SureDeviceName ++;
			}else if(strstr(receivedata1,"lean")!= NULL){
				NeedClean = 1;
			}else if(strstr(receivedata1, "<{") != NULL && strstr(receivedata1, "}>") != NULL){
				char *start = strchr(receivedata1, '<');   //find the start
				char *end = strchr(receivedata1, '>');     //find the end
				if (start != NULL && end != NULL && end > start) {
					int length = end - start - 1;          //get json string lenth      
					strncpy(Get_Recieved, start + 1, length);  //move it into the char 
					Get_Recieved[length] = '\0';
					canDOACommand = 1;                     //flag =>1
					GPIO_SetBits(GPIOC, GPIO_Pin_13);
				}
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
	cJSON_AddStringToObject(root, "V", "V1.0");
	cJSON_AddStringToObject(root, "BatteryVoltage", BatteyVoltage);
    cJSON_AddStringToObject(root, "BatteryState", BatteryState);
	cJSON_AddStringToObject(root, "Rotate", rotata);
	cJSON_AddStringToObject(root, "UUID", &UUID);
	if(strlen(Err)!=0){
		cJSON_AddStringToObject(root, "ERR", Err);
		memset(Err, 0, sizeof(Err));
	}else{
		cJSON_AddStringToObject(root, "ERR", "NULL");
	}
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
	if(strlen(&Flash_store) != 0){
		Send_AT_Command(&Flash_store);
	}else{
		Send_AT_Command("{\"user\":\"NULL\"}");   //send command means NO superUser
	}
	Send_CommandOver();
}
//-------------------clock IQ--------------------
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        usingStamp++;  
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update); 
    }
}


