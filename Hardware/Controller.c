/********************************************************************************
* @File         name: Controller.c
* @Author:      Andy and Left-TTC @github
* @Version:     1.01
* @Date:        2024-11-16
* @Description: functions about Central control protocol
********************************************************************************/
#include "stm32f10x.h"                    // Device header
#include "SERIAL.h"
void Controller_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;          //used  to control the device other than the central control
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
}
//-------------------------------Command---------------------------
void Controller_on(void)                  //Power on
{
	GPIO_SetBits(GPIOA,GPIO_Pin_7);
}
void Controller_off(void)                 //Cut off power
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_7);
}
void unLockBikeCommand1(void){
	//AA 55 10 11 00 21
	//AA 55 10 17 01 A5 CD
	//AA 55 10 19 01 00 2A
	uint8_t d[10];
	d[0]=0xAA;
	d[1]=0x55;
	d[2]=0x10;
	d[3]=0x11;
	d[4]=0x00;
	d[5]=0x21;
	Serial_SendArray(d,6);
}	
void unLockBikeCommand2(void){
    uint8_t d[10];
	d[0]=0xAA;
	d[1]=0x55;
	d[2]=0x10;
	d[3]=0x17;
	d[4]=0x01;
	d[5]=0xA5;
	d[6]=0xCD;
	Serial_SendArray(d,7);
}
void unLockBikeCommand3(void){
	uint8_t d[10];
	d[0]=0xAA;
	d[1]=0x55;
	d[2]=0x10;
	d[3]=0x19;
	d[4]=0x01;
	d[5]=0x00;
	d[6]=0x2A;
	Serial_SendArray(d,7);
}
