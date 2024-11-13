#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

void Blue_Init(void);
void Blue_Forbid(void);
void Blue_check(void);
extern void USART3_IRQHandler(void);
void Battery_openNotify(void);
void Battery_offNotify(void);
void Battery_openFail(void);
void Battery_lockFail(void);
void NormalOperationFlag(void);
void Send_AT_Command(const char* command);
void DoToTheseJson(void);
void Date_DeviceToPhone(void);
void TimeClock_Init(void);
void ResetUser(void);

#endif
