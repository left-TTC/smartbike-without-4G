#ifndef __BATTERY_H
#define __BATTERY_H

void BatteryLock_on(void);
void BatteryLock_off(void);
void BatteryLock_Reset(void);
void Battery_Init(void);
void Get_BatteryLockState(void);
void checkBatteryCommand(void);
void GetStateWhenopen(void) ;


#endif
