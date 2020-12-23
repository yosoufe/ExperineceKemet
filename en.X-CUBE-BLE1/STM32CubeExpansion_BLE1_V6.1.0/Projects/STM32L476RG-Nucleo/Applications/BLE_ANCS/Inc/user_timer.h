/**
  ******************************************************************************
  * @file    user_timer.h
  * @author  AMS VMA Application Team
  * @version V1.0
  * @date    24-June-2015
  * @brief   Header for user_timer.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USER_TIMER_H
#define __USER_TIMER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "ble_clock.h"
#include "hci_tl_interface.h"


/* Definition for TIMx: TIM2/3 */
#ifdef STM32L053xx
#define TIMx                           TIM2
#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM2_CLK_ENABLE()
#else
#define TIMx                           TIM3
#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM3_CLK_ENABLE()
#endif /* STM32L053xx */

/* Definition for TIMx's NVIC */
#ifdef STM32L053xx
#define TIMx_IRQn                      TIM2_IRQn
#define TIMx_IRQHandler                TIM2_IRQHandler
#else
#define TIMx_IRQn                      TIM3_IRQn
#define TIMx_IRQHandler                TIM3_IRQHandler
#endif /* STM32L053xx */

#define USER_TIMER_PRESCALER    (64000-1)
/* Timeout in milliseconds. */
#define USER_TIMER_PERIOD_MSEC   200
#define USER_TIMER_PERIOD       (USER_TIMER_PERIOD_MSEC*(SYSCLK_FREQ/(USER_TIMER_PRESCALER+1))/1000)

#define USER_TIMER_PRESCALER_SLEEP  ((USER_TIMER_PRESCALER+1)/(SYSCLK_FREQ/SYSCLK_FREQ_SLEEP) - 1)

extern TIM_HandleTypeDef    TimHandle;
extern tClockTime user_timer_expired; 


void Init_User_Timer(void);
void Start_User_Timer(void);
void Stop_User_Timer(void);
void User_Timer_Enter_Sleep(void);
void User_Timer_Exit_Sleep(void);

 
#ifdef __cplusplus
}
#endif

#endif /*__USER_TIMER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
