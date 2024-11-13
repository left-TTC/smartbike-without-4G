#include "stm32f10x.h"                  // Device header
#define StorePage 0x0800F000            //use penultimate page as the store
#define PAGE_SIZE 2048                  //the byte of erevy pages
#include <string.h>
#include <stdlib.h> 
#include <stdio.h>
#include "time.h"
#include "CJSON.h"
#include "BlueTooth.h"

char Flash_store[1024];   //as the buffer
char SuperUserAddress[50];
int ifHaveSuperUser = 0;     //0 means haven't superuser,state that can be writtern
char Flash_Address[50];    //from bluetooth.c 
char Flash_RentAddress[1024]; //second address
char Flash_TimeStamp[30];
char Flahs_RentTo[30];     //a time stamp means rent user can use it before that time
char Flash_UserChat[30];
char Flash_UserPhone[20];
extern char UUID;
extern time_t usingStamp;
extern int SureDeviceName;
extern char Name;
extern char Address;
extern int CanTrust;
//:0x08char DeviceName2[12]00F400 save the userFlag 0x0800F404 save how many pages the data use
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
void Update_Store_WalletAddress(const char* wallerAddress){             //update the timeStamp in FlashStore
	cJSON *root = cJSON_Parse(Flash_store);
    if (root == NULL) {
        return;
    }
	cJSON *WalletAddress_item = cJSON_GetObjectItem(root, "WalletAddress");
    if (cJSON_IsString(WalletAddress_item) && WalletAddress_item->valuestring != NULL) {
        cJSON_ReplaceItemInObject(root, "WalletAddress", cJSON_CreateString(wallerAddress));
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
void Update_Store_DeviceName(const char* NewName){
	cJSON *root = cJSON_Parse(Flash_store);
    if (root == NULL) {
        return;
    }
	cJSON *Name_item = cJSON_GetObjectItem(root, "Name");
    if (cJSON_IsString(Name_item) && Name_item->valuestring != NULL) {
        cJSON_ReplaceItemInObject(root, "Name", cJSON_CreateString(NewName));
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
	uint32_t HowManyPages = read_Flash(0x0800F004);      //0x01 0x02   
	if(HowManyPages == 0x01 || HowManyPages == 0x02){
		for (int i=0;i<HowManyPages;i++){
			Flash_Erase(0x0800F000 + 0x0400*i);          //Erase all pages
		}
	}else{
		Flash_Erase(0x0800F000);
		Flash_Erase(0x0800F400);
	}
	uint32_t Flash_Size = strlen(Flash_store);
	if(Flash_Size > 509){
		Flash_Write(0x0800F004,0x02);                  //page Flag
		Flash_Write(0x0800F008,Flash_Size);              //Strlen Flag
	}else{
		Flash_Write(0x0800F004,0x01);                  //page Flag
		Flash_Write(0x0800F008,Flash_Size);              //Strlen Flag
	}
	if(SureDeviceName==2){                               //means have already changed the deviceName
		Flash_Write(0x0800F00C,0x01);
		if(strstr(Flash_store,"BIKE_")==NULL){
			Update_Store_DeviceName(&Name);
		}
	}
	Flash_Write(0x0800F000,0xAAAA);
	uint32_t startAddress = StorePage + 4 * 4;
	Flash_WriteString(startAddress, Flash_store);         //skip the flag and pageSave,start from 0x0800F408
	Flash_WriteString(startAddress + (Flash_Size + 1) * 4, "\0");
}
void Read_FLASH(void) {                        //remove Flash data to the Flash_Store
	uint32_t FlashStrlen = read_Flash(0x0800F008); 
    uint32_t addr = StorePage+16;               //skip the flag and pageSave
    for (int i = 0; i < FlashStrlen; i++) {
        Flash_store[i] = (char)read_Flash(addr); 
        if (Flash_store[i] == '\0') break; 
        addr += 4; 
    }
}
void Save_RentAddress(void){
	Flash_Erase(0x0800F800);
	Flash_Erase(0x0800FC00);
	uint32_t InfoSize = strlen(Flash_RentAddress);
	Flash_Write(0x0800F800,InfoSize);
	Flash_WriteString(0x0800F804, Flash_RentAddress);
	Flash_WriteString(0x0800F804 + (strlen(Flash_RentAddress)+1)*4,"\0");
}
void Read_RentFLASH(void) {                        //remove Flash data to the Flash_Store
	uint32_t RentStrlen = read_Flash(0x0800F800); 
    uint32_t addr = 0x0800F804;               //skip the flag and pageSave
    for (int i = 0; i < RentStrlen; i++) {
        Flash_RentAddress[i] = (char)read_Flash(addr); 
        if (Flash_RentAddress[i] == '\0') break; 
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
		Read_RentFLASH();
		usingStamp = ConvertUint_time(Flash_TimeStamp);
	}
}
void Create_FlashStoreJson(void){                    //when the device is fristly used,we create a FlashStore
	cJSON *root = cJSON_CreateObject();
	char Flash_Name[12];
	strcpy(Flash_Name,&Name);
	cJSON_AddStringToObject(root, "Name", Flash_Name);
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
int isRent = 0;      //means rent user
int isSuper = 0;     //means super user
void VerifyIf_RentUser(void){
	const char* startPosition = strstr(Flash_RentAddress,&Address);
	if(startPosition!=NULL && strlen(Flash_RentAddress) > 0){   //means it is a rent user
		isRent = 1;
		if (*(startPosition + 53) == '$'){
			CanTrust = 1;
		}
	}
}
void VerifyIf_Superser(void){
	if(strcmp(Flash_Address, &Address)==0){
		isSuper = 1;
	}
}
int checkUserValid(const char*UserInfo){
	//0xF12460f0b55A17eD18963F6815cE588237a80619 0 1730811970
	//012345678901234567890123456789012345678901 2 3456789012
	//0         1         2         3         4           5
	char canUseToTime[20];
	if (strlen(UserInfo) > 43) {
        strncpy(canUseToTime, UserInfo + 43, sizeof(canUseToTime) - 1);
		canUseToTime[sizeof(canUseToTime) - 1] = '\0';       //get the user time
	}
	if(ConvertUint_time(canUseToTime)>usingStamp){   //means he can still use
		return 1;
	}else{
		return 0;
	}
}
void cleanIllegalUser(void){
	cJSON *root = cJSON_Parse(Flash_RentAddress);
	if (root == NULL){
		return;
	} 
	int count = cJSON_GetObjectItem(root, "count")->valueint;   //get nowly rentuser
    int validUserCount = 0;
	cJSON *newRoot = cJSON_CreateObject();         //to save new rent user array
	cJSON *newUsers = cJSON_CreateObject();
	for (int i = 1; i <= count; ++i){
		char userKey[20];
        snprintf(userKey, sizeof(userKey), "user%d", i); //use same name
		cJSON *userItem = cJSON_GetObjectItem(root, userKey);  //get useri's INFO	
		if (userItem != NULL && checkUserValid(userItem->valuestring)==1){
			validUserCount++;
            char newUserKey[20];
            snprintf(newUserKey, sizeof(newUserKey), "user%d", validUserCount);
            cJSON_AddStringToObject(newUsers, newUserKey, userItem->valuestring);
		}
	}
	cJSON_AddNumberToObject(newRoot, "count", validUserCount);
	cJSON_AddItemToObject(newRoot, "users", newUsers);
	char *updatedJsonStr = cJSON_Print(newRoot);
	snprintf(Flash_RentAddress, strlen(updatedJsonStr) + 1, "%s", updatedJsonStr);
	free(updatedJsonStr);
	cJSON_Delete(root);
    cJSON_Delete(newRoot);
	Save_RentAddress();         //check and upstate rent user list
}
void DealRentCommand(const char* BikeCommand,char Info[50]){
	const char* keyword = "RentAdd";
    const char* position = strstr(BikeCommand, keyword);
	if(position != NULL){
		const char* dataStart = position + strlen(keyword);
		strncpy(Info, dataStart, strlen(dataStart) + 1);
	}
}
void Flash_AddRentUser(const char*BikeCommand){
	char Info[100];
	DealRentCommand(BikeCommand,Info);
	cJSON *root = cJSON_Parse(Flash_RentAddress);
	if (root == NULL) {
        root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "count", 0);  //init the user to 0
    }
	int count = cJSON_GetObjectItem(root, "count")->valueint; //get nowly user number
	count += 1;
	cJSON_ReplaceItemInObject(root, "count", cJSON_CreateNumber(count)); //update the number
	char userKey[20];
	snprintf(userKey, sizeof(userKey), "user%d", count);   //give every user a count
	cJSON_AddStringToObject(root, userKey, Info);
	char *json_str = cJSON_Print(root);
	if (json_str != NULL){
		snprintf(Flash_RentAddress, sizeof(Flash_RentAddress), "%s", json_str);
	}cJSON_Delete(root);
	free(json_str);
	Save_RentAddress();
}
void Flash_Register(const char *Address,const char *Time){//main.c need to register superUser =>fristly Init
	if(strlen(Address)==42){                                  //Ensure walletaddress is received
		strncpy(Flash_Address, Address, strlen(Address));
		Flash_Address[sizeof(Flash_Address) - 1] = '\0';      //now FlashAddress is wuperUserAddress
		strncpy(Flash_TimeStamp, Time, strlen(Time));
		Flash_TimeStamp[sizeof(Flash_TimeStamp) - 1] = '\0';  //when init ,no frist timestamp 
		Create_FlashStoreJson();                              
		Save_NowFlash();                                      //save once immediately
		ifHaveSuperUser = 0;
	}
}
void ChangeSuperUser(const char* BikeCommand){
	if(strlen(BikeCommand)==50){       //lenth llegal
		char newSuperAdd[50];
		strncpy(newSuperAdd,BikeCommand+8,sizeof(newSuperAdd)-1);
		newSuperAdd[sizeof(newSuperAdd)-1]='\0';        //get new walletaddress
		Update_Store_WalletAddress(newSuperAdd);
		Save_NowFlash();
		parse_FLASHJSON();
	}
}
void Parse_PhoneAndChat(const char*Info,char Phone[12],char Chat [20]){
	cJSON *root = cJSON_Parse(Info);
	if (root == NULL) {
        return;  
    }
	for(int i=0;i<11;i++){
		Phone[i]=Info[i+9];
	}
	Phone[11] = '\0';
	cJSON *Chat_item = cJSON_GetObjectItem(root, "Wechat");
	if (cJSON_IsString(Chat_item) && Chat_item->valuestring != NULL) {
        strncpy(Chat, Chat_item->valuestring, strlen(Chat_item->valuestring));
        Chat[strlen(Chat_item->valuestring)] = '\0'; 
    }
	cJSON_Delete(root);
}
void AddPhoneAndChat(const char*BikeCommand){           //get a json with phoneNumber and chatnumber
	char INfo[100];
	strncpy(INfo,BikeCommand+6,sizeof(INfo)-1);
	INfo[sizeof(INfo)-1]='\0'; 
	char phone[12];char chat[20];
	Parse_PhoneAndChat(INfo,phone,chat);
	Update_Store_ChatNumber(chat);
	Update_Store_PhoneNumber(phone);
	Save_NowFlash();
}

