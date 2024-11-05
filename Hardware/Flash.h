#ifndef __FLASH_H
#define __FLASH_H
#include <time.h> 

void Store_Init(void);
void Store_JSON(void);
void JSON_UpstateWallet(const char *Address,const char *Time);
time_t ConvertUint_time(const char *timeStampStr);


#endif
