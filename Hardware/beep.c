#include "stm32f10x.h"                  // Device header
#include "string.h"

void beep_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
}
void beep_on(void)                  //Power on
{
	GPIO_SetBits(GPIOB,GPIO_Pin_0);
}

void beep_off(void)                 //Cut off power
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_0);
}

void beep(int freq,int l){
	for(int ii=0;ii<l;ii++){
	beep_on();
   
  for(int i=0;i<freq;i++){}
		
		beep_off();
   
  for(int i=0;i<freq;i++){}
	}
}

void beep_unlock(){
   for(int i=0;i<200;i++){
    beep(2500*2,1);
	 }
   for(int i=0;i<200;i++){
    beep(2227*2,1);
	 }
   for(int i=0;i<200;i++){
    beep(1984*2,1);
	 }
}

void beep_lock(){
    for(int i=0;i<200;i++){
    beep(1984*2,1);
   
	 }
   for(int i=0;i<200;i++){
    beep(2227*2,1);
	 }
   for(int i=0;i<200;i++){
    beep(2500*2,1);
	 }
}
