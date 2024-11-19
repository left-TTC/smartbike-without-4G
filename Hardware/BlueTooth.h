/********************************************************************************
* @File         name: BlueTooth.h
* @Author:      Andy and Left-TTC @github
* @Version:     1.01
* @Date:        2024-11-16
* @Description: A library of processes related to Bluetooth
********************************************************************************/
#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

void Blue_Init(void);
void Blue_Forbid(void);
void Blue_check(void);
extern void USART3_IRQHandler(void);
void Send_AT_Command(const char* command);
void DoToTheseJson(void);
void Date_DeviceToPhone(void);
void TimeClock_Init(void);

#endif
