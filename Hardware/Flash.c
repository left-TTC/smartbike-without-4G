#include "stm32f10x.h"                  // Device header
#define StorePage 0x0800F800            //use penultimate page as the store
#define PAGE_SIZE 2048                  //the byte of erevy pages
#include <string.h>
#include <stdlib.h> 
#include <stdio.h>
#include "time.h"
#include "CJSON.h"

char Flash_store[1024];   //as the buffer
char SuperUserAddress[50];
int ifHaveSuperUser = 1;     //0 means haven't superuser,state that can be writtern
char Flash_Address[50];    //from bluetooth.c 
char Flash_TimeStamp[30];
char Flash_UserChat[30];
char Flash_UserPhone[20];
extern time_t usingStamp;
//:0x0800F400 save the userFlag 0x0800F404 save how many pages the data use
//:0x0800F400:0xAAAA--have superuser,can't change wallet address;0x0800F404 0x01means 1 pages 0x02 means 2 
uint32_t read_Flash(uint32_t address){
	return *((__IO uint32_t *)(address));          //data was saved by string,accurately in place
}
void Flash_Erase(uint32_t PageAddress) {
    FLASH_Unlock();
    FLASH_ErasePage(PageAddress);
    FLASH_Lock();
}
void Flash_Write(uint32_t PageAddress, uint32_t data) { 
    FLASH_Unlock();
    FLASH_ProgramWord(PageAddress, data);
    FLASH_Lock();
}
void Flash_WriteString(uint32_t PageAddress,const char *str){    //save string
	FLASH_Unlock();
	while(*str){
		FLASH_ProgramWord(PageAddress,(uint32_t)(*str));
		PageAddress +=4;
		str++;
	}
	FLASH_Lock();
}
//-------Walletaddress needn't update,but time need,chat and phoneNumber may need--------
void Update_Store_TimeStamp(const char* NewTime){             //update the timeStamp in FlashStore
	cJSON *root = cJSON_Parse(Flash_store);
    if (root == NULL) {
        return;
    }
	cJSON *TimeStamp_item = cJSON_GetObjectItem(root, "TimeStamp");
    if (cJSON_IsString(TimeStamp_item) && TimeStamp_item->valuestring != NULL) {
        cJSON_ReplaceItemInObject(root, "TimeStamp", cJSON_CreateString(NewTime));
    }
	char *json_str = cJSON_Print(root);           //change it to string
	if (json_str != NULL){
		snprintf(Flash_store, sizeof(Flash_store), "%s", json_str);
	}
	free(json_str);
    cJSON_Delete(root);
}
void Update_Store_ChatNumber(const char* NewChat){
	cJSON *root = cJSON_Parse(Flash_store);
    if (root == NULL) {
        return;
    }
	cJSON *UserWechat_item = cJSON_GetObjectItem(root, "UserWechat");
    if (cJSON_IsString(UserWechat_item) && UserWechat_item->valuestring != NULL) {
        cJSON_ReplaceItemInObject(root, "UserWechat", cJSON_CreateString(NewChat));
    }
	char *json_str = cJSON_Print(root);           //change it to string
	if (json_str != NULL){
		snprintf(Flash_store, sizeof(Flash_store), "%s", json_str);
	}
	free(json_str);
    cJSON_Delete(root);
}
void Update_Store_PhoneNumber(const char* NewNum){
	cJSON *root = cJSON_Parse(Flash_store);
    if (root == NULL) {
        return;
    }
	cJSON *UserPhone_item = cJSON_GetObjectItem(root, "UserPhone");
    if (cJSON_IsString(UserPhone_item) && UserPhone_item->valuestring != NULL) {
        cJSON_ReplaceItemInObject(root, "UserPhone", cJSON_CreateString(NewNum));
    }
	char *json_str = cJSON_Print(root);           //change it to string
	if (json_str != NULL){
		snprintf(Flash_store, sizeof(Flash_store), "%s", json_str);
	}
	free(json_str);
    cJSON_Delete(root);
}
void parse_FLASHJSON(void){
	cJSON *root = cJSON_Parse(Flash_store);
	if (root == NULL) {
		const char* error = cJSON_GetErrorPtr();
		int a = 1+1;
        return;  
    }
	cJSON *WalletAddress_item = cJSON_GetObjectItem(root, "WalletAddress");
    if (cJSON_IsString(WalletAddress_item) && WalletAddress_item->valuestring != NULL) {
        strncpy(Flash_Address, WalletAddress_item->valuestring, strlen(WalletAddress_item->valuestring));
        Flash_Address[strlen(WalletAddress_item->valuestring)] = '\0'; 
    }
	cJSON *LastTimeStamp_item = cJSON_GetObjectItem(root, "TimeStamp");
	if (cJSON_IsString(LastTimeStamp_item) && LastTimeStamp_item->valuestring != NULL) {
        strncpy(Flash_TimeStamp, LastTimeStamp_item->valuestring, strlen(LastTimeStamp_item->valuestring));
        Flash_TimeStamp[strlen(LastTimeStamp_item->valuestring)] = '\0'; 
    }
	cJSON *UserWechat_item = cJSON_GetObjectItem(root,"UserWechat");
	if (cJSON_IsString(UserWechat_item) && UserWechat_item->valuestring != NULL) {
        strncpy(Flash_UserChat, UserWechat_item->valuestring, strlen(UserWechat_item->valuestring));
        Flash_UserChat[strlen(UserWechat_item->valuestring)] = '\0'; 
    }
	cJSON *UserPhone_item = cJSON_GetObjectItem(root,"UserPhone");
	if (cJSON_IsString(UserPhone_item) && UserPhone_item->valuestring != NULL) {
        strncpy(Flash_UserPhone, UserPhone_item->valuestring, strlen(UserPhone_item->valuestring));
        Flash_UserPhone[strlen(UserPhone_item->valuestring)] = '\0'; 
    }
	cJSON_Delete(root);
}
void Save_NowFlash(void){        //used to save json ---the json is a string
	uint32_t HowManyPages = read_Flash(0x0800F404);      //0x01 0x02 
	if(HowManyPages == 0x01 || HowManyPages == 0x02){
		for (int i=0;i<HowManyPages;i++){
			Flash_Erase(0x0800F800 + 0x0400*i);          //Erase all pages
		}
	}else{
		Flash_Erase(0x0800F800);
	}
	if(strlen(Flash_store)>510){
		Flash_Write(0x0800F804,0x02);
	}else{
		Flash_Write(0x0800F804,0x01);
	}
	Flash_Write(0x0800F800,0xAAAA);
	uint32_t startAddress = StorePage + 4 * 2;
	Flash_WriteString(startAddress, Flash_store);         //skip the flag and pageSave,start from 0x0800F408
	Flash_WriteString(startAddress + (strlen(Flash_store) + 1) * 4, "\0");
}
void Read_FLASH(void) {                        //remove Flash data to the Flash_Store
	uint32_t HowManyPages = read_Flash(0x0800F804);//0x01 0x02 
    uint32_t addr = StorePage+8;               //skip the flag and pageSave
    for (int i = 0; i < PAGE_SIZE*HowManyPages/4; i++) {
        Flash_store[i] = (char)read_Flash(addr); 
        if (Flash_store[i] == '\0') break; 
        addr += 4; 
    }
}
time_t ConvertUint_time(const char *timeStampStr){    //get lastly used timestamp
	long int timestampLong = strtol(timeStampStr, NULL, 10);
    time_t convertedTime = (time_t)timestampLong; 
	return convertedTime;
}
void Store_Init(void){
	uint32_t ifFristFlag = read_Flash(StorePage);     //read the frist byte 
	if(ifFristFlag != 0xAAAA){     //means the store is used fristly
		ifHaveSuperUser =1;        //can be written means no data
	}
	else{
		Read_FLASH();          //If there is data, migrated to SRAM 
		parse_FLASHJSON();    //now we get the superuser address and lastly timestamp and commonuser jurisdiction
		usingStamp = ConvertUint_time(Flash_TimeStamp);
	}
}
void Create_FlashStoreJson(void){                    //when the device is fristly used,we create a FlashStore
	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "WalletAddress", Flash_Address);
    cJSON_AddStringToObject(root, "TimeStamp", Flash_TimeStamp);
    cJSON_AddStringToObject(root, "UserWechat", Flash_UserChat);
    cJSON_AddStringToObject(root, "UserPhone", Flash_UserPhone);
	char *json_str = cJSON_Print(root);           //change it to string
	if (json_str != NULL){
		snprintf(Flash_store, sizeof(Flash_store), "%s", json_str);
	}
	cJSON_Delete(root);
    free(json_str);
}
void Flash_Register(const char *Address,const char *Time,const char*Wechat,const char*Phone){//main.c need to register superUser =>fristly Init
	if(strlen(Address)==42){                                  //Ensure walletaddress is received
		strncpy(Flash_Address, Address, strlen(Address));
		Flash_Address[sizeof(Flash_Address) - 1] = '\0';      //now FlashAddress is wuperUserAddress
		strncpy(Flash_TimeStamp, Time, strlen(Time));
		Flash_TimeStamp[sizeof(Flash_TimeStamp) - 1] = '\0';  //when init ,no frist timestamp 
		strncpy(Flash_UserChat,Wechat,strlen(Wechat));
		Flash_UserChat[sizeof(Flash_UserChat) - 1]='\0';
		strncpy(Flash_UserPhone,Wechat,strlen(Phone));
		Flash_UserPhone[sizeof(Phone) - 1]='\0';
		Create_FlashStoreJson();                              
		Save_NowFlash();                                      //save once immediately
		ifHaveSuperUser = 0;
	}
}

