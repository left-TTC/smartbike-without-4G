#include "stm32f10x.h"                  // Device header
#define M_PI 3.1415926535
#define DeadZone 50 
#include <math.h>
#include <stdio.h> 
#include <string.h>
#include "BLUETOOTH.h"

extern uint16_t ADValue;
volatile uint32_t Rotate_Counter = 0;
volatile int Countstate = 0;//means can be counted
int Site_move = 0;
uint16_t CheckHelp = 0;

void AD_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
	
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	ADC_Cmd(ADC1, ENABLE);
	
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
	
	NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);

    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE); 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
}

//------------------------------------------------------------

uint16_t AD_GetValue(void)
{
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC1);
}

//-----------------------mileage calculation------------------
void Send_CurrentRotate(void)
{
	char buffer[15];
	char a[20] = "Mileage:";
	sprintf(buffer, "%u", Rotate_Counter);
	strcat(a,buffer);
	
	Send_AT_Command(a);
}

void Check_move(void)
{
	if(CheckHelp == Rotate_Counter)
	{
		Site_move = 1;
	}
	else 
	{
		Site_move = 0;
	}
	CheckHelp = Rotate_Counter;
}

//------------------------------------------------------------

void ADC1_2_IRQHandler(void)
 {
	uint16_t now_advalue = AD_GetValue();
	if(Countstate == 0 && now_advalue > (ADValue+DeadZone))// means voltage is rising
	{
		if(now_advalue > 1365)
		{
			Rotate_Counter++;
			Countstate = 1;
		}
	}
	else if(Countstate == 1 && now_advalue < (ADValue - DeadZone))
	{
		Countstate = 0;
	}
	
	ADValue = now_advalue;   //upstate the ADvalue
}

