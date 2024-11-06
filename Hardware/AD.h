#ifndef __AD_H
#define __AD_H

void AD_Init(void);
uint16_t AD1_GetValue(void);
extern void ADC1_2_IRQHandler(void);
void Send_CurrentRotate(char *Rotate);
void Check_move(void);
void BatteryVoltage_get(char *BatteryPower);

#endif
