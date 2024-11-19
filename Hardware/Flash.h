#ifndef __FLASH_H
#define __FLASH_H
#include <time.h> 

void All_Struct_Init(void);
int Flash_Struct_Register(const char *Address);
time_t ConvertUint_time(const char *timeStampStr);
void Update_Struct_PhoneNumber(const char* NewPhone);
void Update_Struct_ChatNumber(const char* NewChat);
void Update_Struct_TimeStamp(const char* NewTime);
void Update_Struct_DeviceName(const char* NewName);
void Save_NowFlashStruct(void);
uint32_t read_Flash(uint32_t address);
void Flash_Write(uint32_t PageAddress, uint32_t data);
int Flash_Erase(uint32_t PageAddress);
int VerifyIf_RentUser(void);
int VerifyIf_Superser(void);
int Flash_AddRentUser(const char*Bikecommand);
void cleanIllegalUser(void);
int ChangeSuperUser(const char* BikeCommand);
int AddPhoneAndChat(const char*BikeCommand);


#endif
