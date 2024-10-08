#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "string.h"
#include "AD.h"
#include "BLUETOOTH.h"
#include "CONTROLLER.h"
#include "BATTERY.h"

uint16_t ADValue = 0;
float Voltage;
volatile int check_tooth = 0;
uint8_t Noblue_dirve = 0;
int BikeLock_number = 0;
int BatteryLock_number = 2;
extern int Tooth_Flag;
extern int Site_move;
int once_load = 1;

int main(void)
{
	OLED_Init();
	Serial_Init();
	AD_Init();
	OLED_Init();
	Blue_Init();
	Battery_Init();
	Controller_Init();
	
	uint32_t whilecount = 0;
	uint32_t Batterylockcount = 0;
	int Bikelockcount = 0;
	
	while (1)
	{
//--------------------------while ++-----------------------
		if(once_load == 1 && Tooth_Flag ==0)           //when frist started using this car           
		{
			Get_BatteryLockState();                 //upstate batterylock state
			once_load = 0;
		}
		if(whilecount%10==0)
		{
			ADValue = AD_GetValue();
			Voltage = (float)ADValue / 4095 * 3.3;
		
			OLED_ShowNum(1, 9, ADValue, 4);
			OLED_ShowNum(2, 9, Voltage, 1);
			OLED_ShowNum(2, 11, (uint16_t)(Voltage * 100) % 100, 2);
		}
		
		whilecount++;                          //100 =1s
		Delay_ms(10);
		
		if (whilecount%100==0)       //a whilecount == 0.01s   1s
		{
			Blue_check();            //Determine whether Bluetooth is connected
		}
		
//----------------------BikeLock on And Bluetooth connected----------------------
//BikeLock_number£º1-on 0-off;Tooth_Flag: 1-disconnect 0-connect;
		if(BikeLock_number == 1 && Tooth_Flag == 0)          
		{
			if(Bikelockcount%10==0)        //logically redundant ,set just at once 
			{
				Controller_on();
				Bikelockcount ++;           //when Bikelockcount = 1£¬don't enter the loop
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
			if (whilecount % 1000 == 0)            
			{
				Send_CurrentRotate();
				Noblue_dirve = 0;                  //reset,prevent the next Bluetooth disconnection and lock car directly without notification
			}
			if (whilecount % 3000 == 0)            //30s
			{
				Check_move();
				if(Tooth_Flag == 1 && Site_move == 1)     //BuleTooth disconnected and haven't moved for a 2 min
				{
					check_tooth ++;
					if(check_tooth > 5)
					{
						BikeLock_number = 0;       //Bikelock but withoutnotify
					}
					else if(Tooth_Flag == 0 || Site_move == 0)  
					{
						check_tooth = 0;
					}
				}
				if(whilecount % 6000 == 0)             //every 60s check batterylock state
				{
					Get_BatteryLockState();
				}
			}
		}
		
//--------------------Bikelock on But Bluetooth is disconnected -------------

		if(BikeLock_number == 1 && Tooth_Flag == 1)        //have opened the lock but BlueTooth drop
		{
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
			if(whilecount % 500 == 0)
			{
				Noblue_dirve ++;
				if (Noblue_dirve > 5)
				{
					BikeLock_number = 0;
					Noblue_dirve = 0;
				}
			}
			if (whilecount % 1000 == 0)            
			{
				Send_CurrentRotate();
			}
		}
		
//---------------------------Bikelock off-------------------------	
		if(BikeLock_number == 0)                  //lock
		{
			if(Bikelockcount%10==1)        //logically redundant ,set just at once 
			{
				Controller_off();
				Bikelockcount = 0;
			}
			if(once_load == 0)
			{
				once_load ++;
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
				Get_BatteryLockState();
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
				Get_BatteryLockState();
				BatteryLock_number = 2;
				Batterylockcount = 0;
			}
		}
		
	}
}


