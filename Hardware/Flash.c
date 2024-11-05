#include "stm32f10x.h"                  // Device header
#define StorePage 0x0800F800            //use penultimate page as the store
#define PAGE_SIZE 2048                  //the byte of erevy pages
#include <string.h>
#include <stdlib.h> 
#include <stdio.h>
#include "time.h"

char Flash_store[1024];   //as the buffer
char SuperUserAddress[50];
int ifHaveSuperUser = 0;     //0 means haven't superuser,state that can be writtern
char Flash_Address[50];    //from bluetooth.c 
char Flash_TimeStamp[30];
char Flash_commonUser[100];
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
void Store_JSON(void){        //used to save json ---the json is a string
	//0x0800F800--if Init?      0x0800F804--how many pages there is  1 or 2 
	uint32_t HowManyPages = read_Flash(0x0800F404);//0x01 0x02 
	if(HowManyPages == 0x01 || HowManyPages == 0x02){
		for (int i=0;i<HowManyPages;i++){
			Flash_Erase(0x0800F800 + 0x0400*i);          //Erase all pages
		}
	}else{
		Flash_Erase(0x0800F800);
	}
	snprintf(Flash_store, sizeof(Flash_store), 
                 "{\"address\": \"%s\", \"timeStamp\": \"%s\", \"commonUser\": \"%s\"}", 
                 Flash_Address, Flash_TimeStamp, Flash_commonUser);
	if(strlen(Flash_store)>510){
		Flash_Write(0x0800F804,0x02);
	}else{
		Flash_Write(0x0800F804,0x01);
	}
	Flash_Write(0x0800F800,0xAAAA);
	uint32_t startAddress = StorePage + 4 * 2;
	Flash_WriteString(startAddress, Flash_store);  //skip the flag and pageSave,start from 0x0800F408
	Flash_WriteString(startAddress + (strlen(Flash_store) + 1) * 4, "\0");
}
void Read_JSON(void) {  //update the SRAM
	uint32_t HowManyPages = read_Flash(0x0800F804);//0x01 0x02 0x03
    uint32_t addr = StorePage+8;//skip the flag and pageSave
    for (int i = 0; i < PAGE_SIZE*HowManyPages/4; i++) {
        Flash_store[i] = (char)read_Flash(addr); 
        if (Flash_store[i] == '\0') break; 
        addr += 4; 
    }
}
void Distribute_Store(void){     //get data from Flash
	sscanf(Flash_store, "{\"address\": \"%[^\"]\", \"timeStamp\": \"%[^\"]\", \"commonUser\": \"%[^\"]\"}", 
           Flash_Address, Flash_TimeStamp, Flash_commonUser);
}
time_t ConvertUint_time(const char *timeStampStr){      //get lastly used timestamp
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
		Read_JSON();    //If there is data, migrated to SRAM 
		Distribute_Store();   //now we get the superuser address and lastly timestamp and commonuser jurisdiction
		usingStamp = ConvertUint_time(Flash_TimeStamp);
	}
}
void JSON_UpstateWallet(const char *Address,const char *Time){        //main.c need to register superUser =>fristly Init
	//0xF12460f0b55A17eD18963F6815cE588237a80619
	//123456789012345678901234567890123456789012
	if(strlen(Address)==42){          //Ensure walletaddress is received
		strncpy(Flash_Address, Address, strlen(Address));
		Flash_Address[sizeof(Flash_Address) - 1] = '\0';   //now FlashAddress is wuperUserAddress
		strncpy(Flash_TimeStamp, Time, strlen(Time));
		Flash_TimeStamp[sizeof(Flash_TimeStamp) - 1] = '\0';    //when init ,no frist timestamp 
	}
	Store_JSON();//save once immediately
}

