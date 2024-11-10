#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "string.h"
#include "AD.h"
#include "BLUETOOTH.h"
#include "CONTROLLER.h"
#include "BATTERY.h"
#include "beep.h"
#include "Flash.h"
int command_verify(const char *cmdstr,char * signaturestr,char * address,char * publicKeystr);

volatile int check_tooth = 0;
int BikeLock_number = 0;
int BatteryLock_number = 2;
extern int Tooth_Flag;
extern int Site_move;
extern int canDOACommand;
extern int ifHaveSuperUser;        //Check whether a superuser exists
extern char Flash_store;
extern char UUiD;
int NoMoveFlag = 0;
int NeedClean = 0;

void Flash_Clean(void){
	Flash_Erase(0x0800F800);
	Flash_Erase(0x0800FC00);
	for (uint32_t i = 0; i < 2048 * 2; i += 4) {
        Flash_Write(0x0800F800 + i, 0x00000000);  
    }
}
int main(void){	
	AD_Init();     
	Blue_Init();
	beep_Init();
	Store_Init();     //get Flash
	GetUniqueID();
	Serial_Init();
	Battery_Init();
	Controller_Init();
	//changeDeviceName();  //change devicename
	int Bikelockcount = 0;
	uint32_t whilecount = 0;
	uint32_t Batterylockcount = 0;	
	while (1){	
		if(canDOACommand == 1){ //from bluetoothIQ,means need to processe the received data 
			DoToTheseJson();
			canDOACommand = 0;
		}
		if(NeedClean == 1){
			Flash_Clean();
		}
		whilecount++;                          //100 =1s
		Delay_ms(10);		
		if (whilecount%101==0){      //a whilecount == 0.01s   1s
			Blue_check();            //Determine whether Bluetooth is connected -> parameter:Tooth_Flag
			if(Tooth_Flag ==0){
				Date_DeviceToPhone();   //
			}
			Get_BatteryLockState();  //every 1s check need to close the lock
		}
//----------------------BikeLock on And Bluetooth connected----------------------
//BikeLock_number£º1-on 0-off;Tooth_Flag: 1-disconnect 0-connect;
		if(BikeLock_number == 1){
			if(Bikelockcount%10==0){        //logically redundant ,set just at once 	
				Controller_on();            //supply electricity
				Bikelockcount ++;           //when Bikelockcount = 1£¬don't enter the loop
				beep_unlock();
				NormalOperationFlag();
			}
 			if(whilecount%100==0){
				unLockBikeCommand1();
			}			
			if(whilecount%100==20){
				unLockBikeCommand2();
			}	
			if(whilecount%100==40){
				unLockBikeCommand3();
			}
			if(Tooth_Flag==0){
				if(NoMoveFlag > 0 ){        //reset the unmoved flag bit
					NoMoveFlag = 0;
				}
			}
			if(Tooth_Flag==1){
				if(whilecount %1000 == 0){
				Check_move();
				}
				if(Site_move == 1 && whilecount %800 == 0){
					NoMoveFlag ++;
					if(NoMoveFlag > 3){
						BikeLock_number = 0;
						NoMoveFlag = 0;
					}
				}
				if(Site_move == 0 && whilecount %800 == 0){  //means device is still moving but bulutooth disconnect
					//remind user to connect bluetooth/beep
					NoMoveFlag = 0;
				}
			}
		}				
//---------------------------Bikelock off-------------------------	
		if(BikeLock_number == 0){                  //lock
			if(Bikelockcount%10==1){        //logically redundant ,set just at once 
				Controller_off();
				Bikelockcount = 0;
				beep_lock();
				Save_NowFlash();            //stop drive ->save the data in Flash_Store now
			}
		}
//-------------------------Battery command----------------------------
		//batterylock_number:1-on 0-off 2-wait (the number dosen't mean state but action
		if(BatteryLock_number == 1){             //let Batterylock on
			Batterylockcount ++;
			if(Batterylockcount%101==1){         //when Batterylockcount =1,102
				BatteryLock_on();
			}
			if(Batterylockcount%101==0){         //delay ~1s
				BatteryLock_Reset();
				checkBatteryCommand();
				BatteryLock_number = 2;
				Batterylockcount = 0;            //Batterylockcount(max) == 101
			}
		}		
		if(BatteryLock_number == 0){             //let Batterylock off
			Batterylockcount ++;
			if(Batterylockcount%101==1){
				BatteryLock_off();
			}
			if(Batterylockcount%101==0){           //delay ~1s
				BatteryLock_Reset();
				checkBatteryCommand();
				BatteryLock_number = 2;
				Batterylockcount = 0;
			}
		}		
	}
}


