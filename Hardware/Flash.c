/********************************************************************************
* @File         name: Flash.c
* @Author:      Andy and Left-TTC @github
* @Version:     1.10
* @Date:        2024-11-18
* @Description: the library of Flash memory-related functions
********************************************************************************/
#include "stm32f10x.h"                  // Device header
#include <string.h>
#include <stdlib.h> 
#include <stdio.h>
#include "time.h"
#include "CJSON.h"
#include "BlueTooth.h"
#include "Struct.h"
#include "Flashdefine.h"
#define PAGE_SIZE 1024                                      //the byte of erevy pages
int ifNeedRigisterSuperUser = 0;                            //0 means haven't superuser,state that can be writtern
extern char UUID;
extern time_t usingStamp;
extern int SureDeviceName;
extern char Name;
extern char Address;
extern int CanTrust;
extern char Err;

uint32_t read_Flash(uint32_t address){
	return *((__IO uint32_t *)(address));          
}
/********************************************************
* function name :Flash_Erase
* Description   :Erase the specified Flash page
* Parameter     :
* @PageAddress   specified pageaddress;Must be the start of the page
* Return        :
*                0--Error
*                1--success
**********************************************************/
int Flash_Erase(uint32_t PageAddress) {
    FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR | FLASH_FLAG_OPTERR);
	while (FLASH->SR & FLASH_SR_BSY);
	int EraseStatus = FLASH_ErasePage(PageAddress);
    FLASH_Lock();
	if(EraseStatus==4){
		return 1;
	}else{
		return 0;
	}
}
/********************************************************
* function name :Flash_Write
* Description   :write a 32_bits data to specified Flash address
* Parameter     :
* @PageAddress   specified pageaddress
* @data          32_bits data will be writen in
* Return        :
*                0--Error
*                1--success
**********************************************************/
int Flash_Write(uint32_t PageAddress, uint32_t data) { 
    FLASH_Unlock();
    int WriteOneStatus = FLASH_ProgramWord(PageAddress, data);
    FLASH_Lock();
	if(WriteOneStatus == 4){
		return 1;
	}else{
		return 0;
	}
}

SuperUser SuperUser_Flash = {0};
RentUserList Rent_Flash_Struct = {0};            //Define a RentUserList variable to store all users

void Update_Struct_WalletAddress(const char* wallerAddress){           
	snprintf(SuperUser_Flash.WalletAddress,sizeof(SuperUser_Flash.WalletAddress),"%s",wallerAddress);
}
void Update_Struct_ChatNumber(const char* NewChat){
	snprintf(SuperUser_Flash.User_Wechat,sizeof(SuperUser_Flash.User_Wechat),"%s",NewChat);
}
void Update_Struct_PhoneNumber(const char* NewNum){
	snprintf(SuperUser_Flash.User_PhoneNumber,sizeof(SuperUser_Flash.User_PhoneNumber),"%s",NewNum);
}
void Update_Struct_DeviceName(const char* NewName){
	snprintf(SuperUser_Flash.Bike_Name,sizeof(SuperUser_Flash.Bike_Name),"%s",NewName);
}
/********************************************************
* function name :Flash_Super_WriteStruct/Flash_Rent_WriteStruct
* Description   :Write a Struct to Flash
* Parameter     :
* @PageAddress   stary Address
* @user          A structure of type RentUser/SuperUser
* @struct_size   size of struct
* Return        :
*                0--A write error occurred
*                1--success
**********************************************************/
int Flash_Super_WriteStruct(uint32_t PageAddress,SuperUser *user,size_t struct_size){   
	FLASH_Unlock();
	uint8_t *data = (uint8_t *)user;
	for(size_t i = 0; i < struct_size; i+= 4){
		uint32_t Word_Block = 0;
		for (size_t j = 0; j < 4; ++j) {
            if (i + j < struct_size) {
                Word_Block |= data[i + j] << (j * 8);
            }
        }
		if(FLASH_ProgramWord(PageAddress, Word_Block)!=4){
			FLASH_Lock();
			return 0;
		}
        PageAddress += 4;
	}
	FLASH_Lock();
	return 1;
}
int Flash_Rent_WriteStruct(uint32_t PageAddress,RentUserList *user,size_t struct_size){   
	FLASH_Unlock();
	uint8_t *data = (uint8_t *)user;
	for(size_t i = 0; i < struct_size; i+=4){
		uint32_t Word_Block = 0;
		for (size_t j = 0; j < 4; ++j) {
            if (i + j < struct_size) {
                Word_Block |= data[i + j] << (j * 8);
            }
        }
		if(FLASH_ProgramWord(PageAddress, Word_Block)!=4){
			FLASH_Lock();
			return 0;
		}
        PageAddress += 4;
	}
	FLASH_Lock();
	return 1;
}
/********************************************************
* Function name 	:Save_NowFlashStruct
* Description   	:Store the current data of the superUser structure
* Parameter     	:None
* Return        	:
*					 0--fail    
*					 1--success
**********************************************************/
int Save_NowFlashStruct(void){        
	if(Flash_Erase(SuperUserFlash_page_Start)==1){                              //1 => Erase successfully
		uint32_t Flash_Struct_Size = sizeof(SuperUser_Flash);
		if(Flash_Struct_Size > (PAGE_SIZE - 50)){
			return 0;
		}
		if(Flash_Write(SuperUserFlash_Strlen_Record,Flash_Struct_Size)!=1){     //1 => write successsfully
			return 0;
		}
		if(SureDeviceName==2 || strlen(&Name)> 0 ){          
			if(Flash_Write(SuperUserFlash_IfRecordName_Record,0xAAAA)!=1){        //1 => write successsfully
				return 0;
			}
			if(strstr(SuperUser_Flash.Bike_Name,"BIKE_")==NULL){
				Update_Struct_DeviceName(&Name);
			}
		}
		if(Flash_Write(SuperUserFlash_page_Start,0xAAAA)!=1){                   //1 => write successsfully
			return 0;
		}
		if(Flash_Super_WriteStruct(SuperUserFlash_Start_Write_Address,&SuperUser_Flash,Flash_Struct_Size)!=1){
			return 0;
		}
		return 1;
	}else{
		return 0;
	}
}
/********************************************************
* Function name 	:Read_FLASH_Super_Struct
* Description   	:Read the superuser struct in flash
* Parameter     	:None
* Return        	:
* 					 0--Flash's lenth is illegal
*                    1--success
**********************************************************/
int Read_FLASH_Super_Struct(void) {                        //remove Flash data to the Flash_Store
	uint32_t Flash_Struct_Size = read_Flash(SuperUserFlash_Strlen_Record); 
	if (Flash_Struct_Size > (PAGE_SIZE - 50)){
		return 0;
	}
	uint8_t *data = (uint8_t *)&SuperUser_Flash;
    uint32_t addr = SuperUserFlash_Start_Write_Address;               //skip the flag and pageSave
    for (size_t i = 0; i < Flash_Struct_Size; i += 4) {
        uint32_t word = *(uint32_t *)(addr + i);
        for (size_t j = 0; j < 4; ++j) {
            if (i + j < Flash_Struct_Size) {
                data[i + j] = (word >> (j * 8)) & 0xFF;
            }
        }
    }return 1;
}
/********************************************************
* Function name 	:Save_RentUser_Struct
* Description   	:Store the current data of the RentList structure
* Parameter     	:None
* Return        	:
*					 0--fail   
*					 1--success
**********************************************************/
int Save_RentUser_Struct(void){
	if(Flash_Erase(RentUserFlash_page_Start)==1 && Flash_Erase(RentUserFlash_Second_page_Start)==1){
		uint32_t Rent_Struct_Size = sizeof(Rent_Flash_Struct);
		if(Rent_Struct_Size > (2*PAGE_SIZE - 60)){
			return 0;
		}
		if(Flash_Write(RentUserFlash_page_Start,Rent_Struct_Size)!=1){
			return 0;
		}
		if(Flash_Rent_WriteStruct(RentUserFlash_Start_Write_Address,&Rent_Flash_Struct,Rent_Struct_Size)!= 1){
			return 0;
		}
		return 1;
	}else{
		return 0;
	}
}
/********************************************************
* Function name 	:Read_FLASH_Rent_Struct
* Description   	:Read the Rentuser struct in flash
* Parameter     	:None
* Note              :None
* Return        	:
*                    0--The read length is not reasonable
*                    1--success
**********************************************************/
int Read_FLASH_Rent_Struct(void) {                                         //remove Flash data to the Flash_Store
	uint32_t RentStruct_length = read_Flash(RentUserFlash_page_Start);    
	if(RentStruct_length > (2*PAGE_SIZE - 60)){
		return 0;
	}
    uint8_t *data = (uint8_t *)&Rent_Flash_Struct;
    uint32_t addr = RentUserFlash_Start_Write_Address;               
    for (size_t i = 0; i < RentStruct_length; i += 4) {
        uint32_t word = *(uint32_t *)(addr + i);
        for (size_t j = 0; j < 4; ++j) {
            if (i + j < RentStruct_length) {
                data[i + j] = (word >> (j * 8)) & 0xFF;
            }
        }
	}
	return 1;
}
/********************************************************
* Function name 	:ConvertUint_time
* Description   	:Converts the character type to timestamp
* Parameter     	:
* @timeStampStr      Timestamp of character type
* Return        	:
* @convertedTime     hexadecimal timestamp             
**********************************************************/
time_t ConvertUint_time(const char *timeStampStr){       
	long int timestampLong = strtol(timeStampStr, NULL, 10);
    time_t convertedTime = (time_t)timestampLong; 
	return convertedTime;
}
/********************************************************
* Function name 	:All_Struct_Init
* Description   	:used to read data or prepare conditions for creating data
* Parameter     	:None
* Return        	:None            
**********************************************************/
void All_Struct_Init(void){
	uint32_t ifFristFlag = read_Flash(SuperUserFlash_page_Start);       //read the frist byte 
	if(ifFristFlag != 0xAAAA){                                          //Read flag bit
		ifNeedRigisterSuperUser =1;                                     //can be written means no data
	}
	else{
		Read_FLASH_Super_Struct();          //If there is data, migrated to SRAM 
		Read_FLASH_Rent_Struct();
	}
}
int IfNeedSendRentAbout = 0;
int CanRentOpenBattery = 0;       //means rent user can't open battery lock=>0
char RentToTime[11];
/********************************************************
* Function name 	:VerifyIf_RentUser
* Description   	:Used to verify wheather the current address is RentUser
* Parameter     	:None
* @Address           Global variable, the wallet address received this time
* Return        	:
* 					 0--The user does not exist     
* 					 1--valid User   
**********************************************************/
int VerifyIf_RentUser(void){             
	for(int i= 0;i<Rent_Flash_Struct.userCount;i++){
		RentUser* NowCheckingUser = &Rent_Flash_Struct.users[i]; 
		if(strncmp(NowCheckingUser->RentAddress,&Address,sizeof(Address))==0){
			if(strcmp(NowCheckingUser->CanUserOpenBatteryLock ,"Y")==0){
				CanRentOpenBattery = 1;
			}
			if(strcmp(NowCheckingUser->IfTrustCompletely ,"Y")==0){
				CanTrust = 1;
			}
			return 1;
		}
	}
	return 0;
}
/********************************************************
* Function name 	:VerifyIf_RentUser
* Description   	:Used to verify wheather the current address is SuperUser
* Parameter     	:None
* Return        	:
* 					 0--The user does not exist     
* 					 1--valid User   
**********************************************************/
int VerifyIf_Superser(void){
	if(strcmp(SuperUser_Flash.WalletAddress, &Address)==0){
		IfNeedSendRentAbout = 0;
		return 1;
	}else{
		IfNeedSendRentAbout = 1;
		return 0;
}
}
/********************************************************
* Function name :checkUserValid
* Description   :Read the usable time of the RentUser stored in the flash to determine whether it is valid
* Parameter     :
* @CheckingTime  Timestamp in structure
* Return        :
*				 0--illegal => Note the data should be deleted
*				 1--llegal  => Note the data should be saved
**********************************************************/
int checkUserValid(const char*CheckingTime){
	time_t checkING_time = ConvertUint_time(CheckingTime);
	if(checkING_time > usingStamp){
		return 1;
	}else{
		return 0;
	}
}
/********************************************************
* Function name :cleanIllegalUser
* Description   :Clear the rent data that has expired in the flash
* Parameter     :None
* Return        :None
**********************************************************/
void cleanIllegalUser(void){
	int totalUsers = Rent_Flash_Struct.userCount;
	int validCount  = 0;
	for(int k = 0;k < totalUsers;k ++){
		RentUser* CheckingUser = &Rent_Flash_Struct.users[k]; 
		char Checking_time[11];strcpy(Checking_time, CheckingUser->TimeStamp);
		if(checkUserValid(Checking_time) == 1){
			Rent_Flash_Struct.users[validCount] = *CheckingUser;
			validCount ++;
		}
	}
	Rent_Flash_Struct.userCount = validCount;
	if(validCount != totalUsers){
		Save_RentUser_Struct();                        //No need to check, can't ride though it's not be deleted
	}
}
/********************************************************
* Function name :DealRentCommand
* Description   :Process the received Rentcommand
* Parameter     :
* @BikeCommand   complete processed command      example:RentAdd+RentAddress+0/1+TimeStamp
* @add           Store (0x)add
* @open          Store unlock permissions Y/N
* @time          Storage time available
* @trust         Storage if be trusted
* Return        :None
**********************************************************/
void DealRentCommand(const char* BikeCommand,char *add,char* open,char *time,char* trust){
	char Info[100];
	const char* keyword = "RentAdd";
    const char* position = strstr(BikeCommand, keyword);
	if(position != NULL){
		const char* dataStart = position + strlen(keyword);
		strncpy(Info, dataStart, strlen(dataStart) + 1);
	}
	strncpy(add, &Info[2], 40);add[40] = '\0';
	strncpy(time, &Info[43], 10);time[10] = '\0';
	if(Info[42] == '1'){
		strcpy(open, "Y");
	}else{
		strcpy(open, "N");
	}if(Info[53] == '$'){
		strcpy(trust, "Y");
	}else{
		strcpy(trust, "N");
	}
}
/********************************************************
* Function name :CheckIfuserExist
* Description   :Add a Rent User when Recieve a command containing the RentAdd field
* Parameter     :
* @Addresss   	 The user address to check
* Return        :
* 				 -1--new
*				 i --Index of duplicate addresses
**********************************************************/
int CheckIfuserExist(const char *RentingAddress){
	for(int i= 0;i<Rent_Flash_Struct.userCount;i++){
		RentUser* NowCheckingUser = &Rent_Flash_Struct.users[i]; 
		if(strncmp(NowCheckingUser->RentAddress,RentingAddress,40)==0){
			return i;
		}
	}
	return -1;
}
/********************************************************
* Function name :UpdateUser
* Description   :used to add or modify the specified index Rentuser 
* Parameter     :
* @index
* @Addresss   	 
* @CanUserOpenBatteryLock   	 
* @TimeStamp   	 
* @IfTrustCompletely   	 
* Return        :None
**********************************************************/
void UpdateUser(int index, const char* Addresss,const char* CanUserOpenBatteryLock, const char* TimeStamp,const char* IfTrustCompletely) {
    snprintf(Rent_Flash_Struct.users[index].RentAddress, sizeof(Rent_Flash_Struct.users[index].RentAddress), "%s", Addresss);
    snprintf(Rent_Flash_Struct.users[index].CanUserOpenBatteryLock,sizeof(Rent_Flash_Struct.users[index].CanUserOpenBatteryLock),"%s",CanUserOpenBatteryLock);
    snprintf(Rent_Flash_Struct.users[index].TimeStamp, sizeof(Rent_Flash_Struct.users[index].TimeStamp), "%s", TimeStamp);
    snprintf(Rent_Flash_Struct.users[index].IfTrustCompletely, sizeof(Rent_Flash_Struct.users[index].IfTrustCompletely), "%s", IfTrustCompletely);
}
/********************************************************
* Function name :Flash_AddRentUser
* Description   :Add a Rent User when Recieve a command containing the RentAdd field
* Parameter     :
* @BikeCommand   complete processed command      example:RentAdd+RentAddress+0/1+TimeStamp
* Return        :
* 				 0--Update existing user successfully
* 				 1--Add new user successfully
* 				 2--Insufficient space
**********************************************************/
int Flash_AddRentUser(const char*BikeCommand){
	if(Rent_Flash_Struct.userCount >= Max_RentUser_Count){
		return 2;
	}
	char RentingAddress[41];                                       //42-bit address(Omit 0x)
	char CanUserOpenBatteryLock[2];
	char TimeStamp[11];
	char IfTrustCompletely[2];
	DealRentCommand(BikeCommand,RentingAddress,CanUserOpenBatteryLock,TimeStamp,IfTrustCompletely);	
	int IfCheckPass = CheckIfuserExist(RentingAddress);
	if(IfCheckPass == -1){
		UpdateUser(Rent_Flash_Struct.userCount, RentingAddress, CanUserOpenBatteryLock, TimeStamp, IfTrustCompletely);
		Rent_Flash_Struct.userCount++;
		if(Save_RentUser_Struct()==1){
			return 1;
		}else{
			return 0;
		}
	}else{                                                 //need update
		UpdateUser(IfCheckPass, RentingAddress, CanUserOpenBatteryLock, TimeStamp, IfTrustCompletely);
		Save_RentUser_Struct();
		return 2;
	}
}
/********************************************************
* Function name :Flash_Struct_Register
* Description   :Registered superuser Struct
* Parameter     :
* @Address       The address of the currently received data
* @Time          The timestamp of the currently received data
* Return        :
*				 0--fail
* 				 1--success
**********************************************************/
int Flash_Struct_Register(const char *Address){     //main.c need to register superUser =>fristly Init
	if(strlen(Address)==40){                                  //Ensure walletaddress is received
		snprintf(SuperUser_Flash.WalletAddress,sizeof(SuperUser_Flash.WalletAddress),"%s",Address);
		if(Save_NowFlashStruct()==1){
			return 1;
		}else{
			return 0;
		}
	}else{
		return 0;
	}
}
/********************************************************
* Function name :ChangeSuperUser
* Description   :Change the address of the super user
* Parameter     :
* @BikeCommand   command include "SuChange"
* Return        :
*				 0--fail
* 				 1--success
**********************************************************/
int ChangeSuperUser(const char* BikeCommand){
	if(strlen(BikeCommand)==50){                                       //lenth llegal
		char newSuperAdd[41];
		strncpy(newSuperAdd,BikeCommand+10,sizeof(newSuperAdd)-1);
		newSuperAdd[sizeof(newSuperAdd)-1]='\0';                       //get new walletaddress
		Update_Struct_WalletAddress(newSuperAdd);
		if(Save_NowFlashStruct()==1){
			return 1;	
		}else{
			return 0;
		}
	}else{
		return 0;
	}
}
/********************************************************
* Function name :Parse_PhoneAndChat
* Description   :Parse the data used to modify the mobile phone number and micro signal function
* Parameter     :
* @Info          json string containing mobile phone number and wechat ID
* @Phone         Save the resolved phone number
* @Chat          Save the resolved Wechat Info
* Return        :
*				 0--incorrect format
* 				 1--success
**********************************************************/
int Parse_PhoneAndChat(const char*Info,char Phone[15],char Chat [20]){
	cJSON *root = cJSON_Parse(Info);
	if (root == NULL) {
        return 0;  
    }
	cJSON *Phone_item = cJSON_GetObjectItem(root, "Phone");
	if (cJSON_IsString(Phone_item) && Phone_item->valuestring != NULL) {
        strncpy(Phone, Phone_item->valuestring, strlen(Phone_item->valuestring));
        Phone[strlen(Phone_item->valuestring)] = '\0'; 
    }else{
		cJSON_Delete(root);
		return 0;
	}
	cJSON *Chat_item = cJSON_GetObjectItem(root, "Wechat");
	if (cJSON_IsString(Chat_item) && Chat_item->valuestring != NULL) {
        strncpy(Chat, Chat_item->valuestring, strlen(Chat_item->valuestring));
        Chat[strlen(Chat_item->valuestring)] = '\0'; 
    }else{
		cJSON_Delete(root);
		return 0;
	}
	cJSON_Delete(root);
	return 1;
}
/********************************************************
* Function name :AddPhoneAndChat
* Description   :Store the parsed data
* Parameter     :
* @BikeCommand   command include "addPAC"
* Return        :
*				 0--fail
* 				 1--success
**********************************************************/
int AddPhoneAndChat(const char*BikeCommand){           //get a json with phoneNumber and chatnumber
	char INfo[100];
	strncpy(INfo,BikeCommand+6,sizeof(INfo)-1);
	INfo[sizeof(INfo)-1]='\0'; 
	char phone[15];char chat[20];
	if(Parse_PhoneAndChat(INfo,phone,chat)==1){
		Update_Struct_ChatNumber(chat);
		Update_Struct_PhoneNumber(phone);
		if(Save_NowFlashStruct()==1){
			return 1;
		}else{
			return 0;
		}
	}else{
		return 0;
	}
}

