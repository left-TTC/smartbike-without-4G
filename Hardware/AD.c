#include "stm32f10x.h"                  // Device header
#include <math.h>
#include <stdio.h> 
#include <string.h>
#include "BLUETOOTH.h"

uint16_t ADValue = 0;
volatile uint32_t Rotate_Counter = 0;
int Countstate = 0;      //means can be counted
int Site_move = 0;
uint16_t CheckHelp = 0;
float BatteryVoltage = 0;  //store power supply voltage

void AD_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; // PA0
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; // PB1
    GPIO_Init(GPIOB, &GPIO_InitStructure);
        
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5); // PA0

    ADC_InitStructure.ADC_NbrOfChannel = 1;        
    ADC_Init(ADC2, &ADC_InitStructure);
    
    ADC_RegularChannelConfig(ADC2, ADC_Channel_9, 1, ADC_SampleTime_239Cycles5); // PB1
    
    ADC_Cmd(ADC1, ENABLE);
    ADC_Cmd(ADC2, ENABLE); 
    
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) == SET);
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) == SET);
    
    ADC_ResetCalibration(ADC2);
    while (ADC_GetResetCalibrationStatus(ADC2) == SET);
    ADC_StartCalibration(ADC2);
    while (ADC_GetCalibrationStatus(ADC2) == SET);
    
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);

    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE); 
}

//------------------------------------------------------------

uint16_t AD1_GetValue(void)
{
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC1);
}

uint16_t AD2_GetValue(void)
{
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);
	while (ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC2);
}

void BatteryVoltage_get(void)
{	
	uint16_t PB1_ADCValue = AD2_GetValue();
	BatteryVoltage = (PB1_ADCValue * 3.3 / 4096) * 32;
	
	char BatteryV[10] = "BV:";
	sprintf(BatteryV, "BV:%.1f", BatteryVoltage);
	
	Send_AT_Command(BatteryV);
}

//-----------------------mileage calculation------------------
void Send_CurrentRotate(void)
{
	char buffer[15];
	char a[20] = "R:";
	sprintf(buffer, "%u", Rotate_Counter);
	strcat(a,buffer);
	
	Send_AT_Command(a);
}

void Check_move(void)
{
	if(CheckHelp == Rotate_Counter)
	{
		Site_move = 1;             //means no move
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
	if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET)
    {
		uint16_t this_ADvalue = AD1_GetValue();		
		
		if(this_ADvalue > 1395 && Countstate == 0 && this_ADvalue > (ADValue + 139))
		{
			Rotate_Counter ++;
			Countstate = 1;
		}
		else if(Countstate == 1 && this_ADvalue < (ADValue - 139))
		{
			Countstate =0;
		}
			
		ADValue = this_ADvalue;
			
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
	}
}

