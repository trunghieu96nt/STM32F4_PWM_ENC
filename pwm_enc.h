/**
  ******************************************************************************
  * @file    pwm_enc.h
  * @author  Vu Trung Hieu
  * @version V1.0
  * @date    15-April-2017
  * @brief   This file contains all the functions prototypes for pwm_enc
  *          library.  
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PWM_ENC_H
#define __PWM_ENC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Define --------------------------------------------------------------------*/
	 
/* Initialization and Configuration functions --------------------------------*/
void PWM_ENC_Init(void);

/* Set function --------------------------------------------------------------*/
void PWM_Set_Freq(uint32_t freq);
void PWM_Set_Duty(int16_t d);

#ifdef __cplusplus
}
#endif

#endif /*__PWM_ENC_H */


/*********************************END OF FILE**********************************/
