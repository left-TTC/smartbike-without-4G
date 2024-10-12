#ifndef __AD_H
#define __AD_H

void AD_Init(void);
uint16_t AD_GetValue(void);
//extern void ADC_IRQHandler(void);
void Send_CurrentRotate(void);
void Check_move(void);

#endif
