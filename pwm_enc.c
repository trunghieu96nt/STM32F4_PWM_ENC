/**
  ******************************************************************************
  * @file    pwm_enc.c
  * @author  Vu Trung Hieu
  * @version V1.0
  * @date    15-April-2017
  * @brief   This file provides firmware functions to use UART - DMA 
  *          with unknown number of data by using a slave timer to count 
	*          timeout on RX pin after final rising edge:           
  *           + UART4 TX(PA0) RX(PA1)
	*           + DMA1 Stream2 Channel4 for USART4 Rx
	*           + DMA1 Stream4 Channel4 for USART4 Tx
  * 
@verbatim  
 ===============================================================================
                      ##### How to use this driver #####
 ===============================================================================
   (#) Connect pin PA1(RX) to PC6(CC1 input capture TIM3)  
   (#) Call UART_DMA_Timeout_Init();   
   (#) extern uint8_t rcv_message[BUFF_SIZE];
   (#) extern bool b_UART_DMA_Timeout;           
   (#) Receive:
       (++) Wait b_UART_DMA_Timeout flag
       (++) When b_UART_DMA_Timeout flag is true, reset flag
       (++) Handle data in rcv_message
   (#) Send:
       (++) Call UART4_DMA_Send		 
@endverbatim        
  *
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "pwm_enc.h"

/* Public variables ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes UART4 (PA0, PA1) - DMA1 Stream2 Channel4 
	*         TIM3 Count up - Input capture Channel1 - Slave Mode - Compare
  * @note   ...
  * @param  None
  * @retval None
  */
void PWM_ENC_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef	TIM_OCInitStructure;
	
	/* Enable GPIOD clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	/* Enable GPIOE clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	/* Enable TIM4 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	/* Enable TIM1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
  /* -------------------------- PWM Configuration ---------------------------*/
	/* PULSE pin configuration - PD13 - TIM4_CH2*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);

#ifdef USE_DIR_EN
	/* DIR, EN pins configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
#endif

	/* TIM4 CH2 Output configuration */
	TIM_TimeBaseStructure.TIM_Period = 1000-1;// 2M/1000=2k
	TIM_TimeBaseStructure.TIM_Prescaler = 42 - 1;// 84M/42=2M
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

	TIM_Cmd(TIM4, ENABLE);
	
	/* -------------------------- ENC Configuration ---------------------------*/
	/* TIM1 channel1,2 configuration */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_TIM1);

	/* Initialise encoder interface */
	TIM_EncoderInterfaceConfig(TIM1, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	
	/* TIM enable counter */
	TIM_SetCounter(TIM1, 0);
	TIM_Cmd(TIM1, ENABLE);
}

/**
  * @brief  Setup pwm frequency - unit: Hz
  * @note   ...
  * @param  Frequency
  * @retval None
  */
void PWM_Set_Freq(uint32_t freq)
{
	uint32_t period;
	/* SystemCoreClock/2 is Timer Clock when APB PreScaler is not equal 1*/
	period = (SystemCoreClock / 2) / (freq*(TIM_GetPrescaler(TIM4) + 1)) - 1;	
	if (period > 0xFFFF)
		period = 0xFFFF;
	
	TIM_SetAutoreload(TIM4, period);
}

/**
  * @brief  Output pwm frequency - unit: 0.1%
  * @note   ...
  * @param  duty cycle
  * @retval None
  */
void PWM_Set_Duty(int16_t d)
{
	if (d<-1000)
		d = -1000;
	else if (d>1000)
		d = 1000;

#ifdef USE_DIR_EN
	if (d == 0){
		GPIO_SetBits(GPIOD,GPIO_Pin_10);  	// disable pwm
	}	
	else if (d > 0)	{
		GPIO_ResetBits(GPIOD,GPIO_Pin_10);	// enabe pwm
		GPIO_SetBits(GPIOB,GPIO_Pin_15); 		// dir = 1
	}
	else 	{
		GPIO_ResetBits(GPIOD,GPIO_Pin_10);	// enabe pwm
		GPIO_ResetBits(GPIOB,GPIO_Pin_15); 		// dir = 0
		d = -d;
	}
#elseif
	
#endif
	d = (TIM4->ARR+1)*d/1000;
	TIM_SetCompare2(TIM4, (uint32_t)d);
}

/*********************************END OF FILE**********************************/
