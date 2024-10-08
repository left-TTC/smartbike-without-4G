#include "stm32f10x.h"                  // Device header
#include <string.h>
#include <stdlib.h> 
#include <stdio.h>
#include "Battery.h"

#define BUFFER_SIZE3 32
     
volatile int Tooth_Flag = 1;
extern int BikeLock_number;
extern int BatteryLock_number;
extern int once_load;

//----------------------------------------Init-----------------
void Blue_Init(void)//USART3
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); 
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;       
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;         
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;          //used  to control the device other than the central control
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;     
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure); 
	
	USART_Cmd(USART3, ENABLE);
}
//----------------------------------send function-----------------------
void BlueAT_SendData(uint8_t data)         
{
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    USART_SendData(USART3, data);
}

void Send_AT_Command(const char* command)    
{
    while (*command) {
        BlueAT_SendData(*command++);
    }
  
    BlueAT_SendData('\r');
    BlueAT_SendData('\n');
}
//----------------------------------------Send information----------------------
void Battery_openNotify(void)
{
	Send_AT_Command("battery1");
}

void Battery_offNotify(void)
{
	Send_AT_Command("battery2");
}

void Battery_openFail(void)
{
	Send_AT_Command("battery3");
}

void Battery_lockFail(void)
{
	Send_AT_Command("battery4");
}

void NormalOperationFlag(void)
{
	Send_AT_Command("ready");
}
//-----------------------------------check related to BlueTooth-------------------
void Blue_check(void)
{
	uint8_t PIN_State = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1);          //BIT_SET.high 1         BIT_RESET.low 0
	
	if(PIN_State == 1)
	{
		//strcpy(fault_string,"NO Tooth ");
		Tooth_Flag = 1;
	}
	else 
	{
		Tooth_Flag = 0;
		//strcpy(fault_string,"Tooth OK ");
	}
}

//------------------------------------IQ------------------------------------------------  
volatile uint16_t bufferIndex1 = 0;
uint8_t index1 = 0;
char receivedata1[BUFFER_SIZE3];
//         0xFFE1: Write Without Response APP --> UART?           0xFFE2: Notify  UART --> APP?
//   0x01 LockBike    0x02 Unlockbike  0x03 batterylock  0x04  batteryUnlock

void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  
    {
        char byte = USART_ReceiveData(USART3); 

        if (byte == '$' || index1 >= BUFFER_SIZE3 - 1)
        {
            receivedata1[index1] = '\0';  
			
			if (strstr(receivedata1, "unbikelock") != NULL)
            {
                BikeLock_number = 0;
				once_load = 1;
            }
            else if (strstr(receivedata1, "bikelock") != NULL)
            {
                 BikeLock_number = 1; 
			}
			else if (strstr(receivedata1, "unbatterylock") != NULL)
            {
                BatteryLock_number = 0; 
            }				
            else if (strstr(receivedata1, "batterylock") != NULL)
            {
                BatteryLock_number = 1; 
            }
            else
            {
                Send_AT_Command("unknown command");
            }

			memset(receivedata1,0,BUFFER_SIZE3);
            index1 = 0;  
        }
        else
        {
            receivedata1[index1++] = byte;  
        }

        GPIO_SetBits(GPIOC, GPIO_Pin_13);  

        USART_ClearITPendingBit(USART3, USART_IT_RXNE);  
    }
}

