#include "stm32f10x.h"                  // Device header

void Flash_Unlock(void){
	FLASH_Unlock();          //Unlocks the FLASH Program Erase Controller
}

