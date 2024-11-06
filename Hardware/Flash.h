#ifndef __FLASH_H
#define __FLASH_H
#include <time.h> 

void Store_Init(void);
void Store_JSON(void);
void Flash_Register(const char *Address,const char *Time,const char*Wechat,const char*Phone);
time_t ConvertUint_time(const char *timeStampStr);
void Update_Store_PhoneNumber(const char* NewPhone);
void Update_Store_ChatNumber(const char* NewChat);
void Update_Store_TimeStamp(const char* NewTime);
void Save_NowFlash(void);

#endif
