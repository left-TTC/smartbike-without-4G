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

volatile int check_tooth = 0;
int BikeLock_number = 0;
int BatteryLock_number = 2;
extern int Tooth_Flag;
extern int Site_move;
int once_load = 1;
int NoMoveFlag = 0;

int main(void)
 {
 
 	OLED_Init();
	Serial_Init();
	AD_Init();
	OLED_Init();
	Blue_Init();
	Battery_Init();
	Controller_Init();
	beep_Init();
	
	uint32_t whilecount = 0;
	uint32_t Batterylockcount = 0;
	int Bikelockcount = 0;
	uint16_t ad;
	
	while (1)
	{
//--------------------------while ++-----------------------
		if(once_load == 1 && Tooth_Flag ==0)           //when frist started using this car           
		{
			once_load = 0;
			BatteryVoltage_get();                      //get batterysource
			GetStateWhenopen();
		}
		
		if(once_load == 0 && Tooth_Flag ==1)           //when frist started using this car           
		{
			once_load = 1;
		}
		
		whilecount++;                          //100 =1s
		Delay_ms(10);
		
		if (whilecount%100==0)       //a whilecount == 0.01s   1s
		{
			Blue_check();            //Determine whether Bluetooth is connected
			ad = AD1_GetValue();
			OLED_ShowNum(1,1,ad,4);
		}
		if (whilecount%10==0)       //a whilecount == 0.01s   1s
		{
			ad = AD1_GetValue();
			OLED_ShowNum(1,1,ad,4);
		}
		
//----------------------BikeLock on And Bluetooth connected----------------------
//BikeLock_number��1-on 0-off;Tooth_Flag: 1-disconnect 0-connect;
		if(BikeLock_number == 1 && Tooth_Flag == 0)          
		{
			if(Bikelockcount%10==0)        //logically redundant ,set just at once 
			{
				Controller_on();
				Bikelockcount ++;           //when Bikelockcount = 1��don't enter the loop
				beep_unlock();
				NormalOperationFlag();
			}
 			if(whilecount%100==0)         
			{
				unLockBikeCommand1();
			}			
			if(whilecount%100==20)
			{
				unLockBikeCommand2();
			}	
			if(whilecount%100==40)
			{
				unLockBikeCommand3();
			}
			if (whilecount % 500 == 0)           
			{
				NoMoveFlag = 0;
			}
			if(whilecount % 50 == 0)             //every 0.5s update the state of rotate
			{
				Send_CurrentRotate();
			}
			if(whilecount %100 == 20)             //every 1s get battery and batterylock state
			{
				Get_BatteryLockState();
				BatteryVoltage_get();
			}
			if(NoMoveFlag > 0 )
			{
				NoMoveFlag = 0;
			}
		}
				
//---------------------------Bikelock off-------------------------	
		if(BikeLock_number == 0 && Tooth_Flag == 0)                  //lock
		{
			if(Bikelockcount%10==1)        //logically redundant ,set just at once 
			{
				Controller_off();
				Bikelockcount = 0;
				beep_lock();
			}
			if(whilecount %100 == 0)      //every 1s get batterysource
			{
				BatteryVoltage_get();
				Get_BatteryLockState();
			}
		}
//------------------------forget lock the car-------------------------
		if(BikeLock_number == 1 && Tooth_Flag == 1)   //user is far away from the device or bluetooth disconnected
		{
			if(whilecount %1000 == 0)
			{
				Check_move();
			}
			if(Site_move == 1 && whilecount %800 == 0)
			{
				NoMoveFlag ++;
				if(NoMoveFlag > 3)
				{
					BikeLock_number = 0;
					NoMoveFlag = 0;
				}
			}
			if(Site_move == 0 && whilecount %800 == 0)  //means device is still moving but bulutooth disconnect
			{
				                                //remind user to connect bluetooth/beep
				NoMoveFlag = 0;
			}	
		}
//-----------------------used to lock the car without bluetooth-------
		if(BikeLock_number == 0 && Tooth_Flag == 1)
		{
			if(Bikelockcount%10==1)        //logically redundant ,set just at once 
			{
				Controller_off();
				Bikelockcount = 0;
				beep_lock();
			}
		}
//-------------------------Battery command----------------------------
		//batterylock_number:1-on 0-off 2-wait (the number dosen't mean state but action
		if(BatteryLock_number == 1)             //let Batterylock on
		{
			Batterylockcount ++;
			if(Batterylockcount%101==1)         //when Batterylockcount =1,102
			{
				BatteryLock_on();
			}
			if(Batterylockcount%101==0)         //delay ~1s
			{
				BatteryLock_Reset();
				checkBatteryCommand();
				BatteryLock_number = 2;
				Batterylockcount = 0;            //Batterylockcount(max) == 101
			}
		}
		
		if(BatteryLock_number == 0)             //let Batterylock off
		{
			Batterylockcount ++;
			if(Batterylockcount%101==1) 
			{
				BatteryLock_off();
			}
			if(Batterylockcount%101==0)           //delay ~1s
			{
				BatteryLock_Reset();
				checkBatteryCommand();
				BatteryLock_number = 2;
				Batterylockcount = 0;
			}
		}
		
	}
}


