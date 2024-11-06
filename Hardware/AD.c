#include "stm32f10x.h"                  // Device header
#include <math.h>
#include <stdio.h> 
#include <string.h>
#include "BLUETOOTH.h"

uint16_t ADValue = 0;
volatile uint32_t Rotate_Counter = 0;
int Site_move = 0;
uint16_t CheckHelp = 0;
float BatteryVoltage = 0;  //store power supply voltage
int if_up = 0;      //==0 means it's up 


void AD_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);     //rotation analog input
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    //for pa0
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
    
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
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_7Cycles5); // PA0
	ADC_AnalogWatchdogThresholdsConfig(ADC1, 1395, 100);
	ADC_AnalogWatchdogSingleChannelConfig(ADC1, ADC_Channel_0);
	ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_SingleRegEnable);
	ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);
      
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
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE); 
}
//------------------------------------------------------------
uint16_t AD1_GetValue(void){
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC1);
}

uint16_t AD2_GetValue(void){
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);
	while (ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC2);
}
void BatteryVoltage_get(char *BatteryPower){	
	uint16_t PB1_ADCValue = AD2_GetValue();
	BatteryVoltage = (PB1_ADCValue * 3.3 / 4096) * 32;	
	sprintf(BatteryPower, "%.1f", BatteryVoltage);
} 
//-----------------------mileage calculation------------------
void Send_CurrentRotate(char *Rotate){
	sprintf(Rotate, "%u", Rotate_Counter);
}

void Check_move(void){
	if(CheckHelp == Rotate_Counter){
		Site_move = 1;             //means no move
	}
	else{
		Site_move = 0;
	}
	CheckHelp = Rotate_Counter;
}

//------------------------------------------------------------
void ADC1_2_IRQHandler(void){
	if (ADC_GetFlagStatus(ADC1, ADC_FLAG_AWD) == 1) {
		uint16_t AD_value = AD1_GetValue();
		
		if (AD_value > 1395)               //judge whether the ad > 1395
		{
			if(if_up == 0)                 //means it's fristly count
			{
				Rotate_Counter ++;
				if_up = 1;
			}
		}
		else
		{
			if_up = 0;
		}
        ADC_ClearFlag(ADC1, ADC_FLAG_AWD); 
    }
}

