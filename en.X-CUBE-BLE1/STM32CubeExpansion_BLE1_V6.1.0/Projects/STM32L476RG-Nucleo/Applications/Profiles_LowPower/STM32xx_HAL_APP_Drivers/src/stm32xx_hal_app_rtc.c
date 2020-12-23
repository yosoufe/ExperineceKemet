/**
  ******************************************************************************
  * @file    stm32xx_hal_app_rtc.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    01-October-2014
  * @brief   HAL specific implementation for the application
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32xx_hal_app_rtc.h"

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Set the Wakeup clock source
  *
  * @param  hrtc : RTC Handle
  *
  * @param  RTC_Wucksel_Value : The Wakeup clock selection source
  *
  * @retval None
  */
void HAL_APP_RTC_Set_Wucksel(RTC_HandleTypeDef *localhrtc, uint32_t RTC_Wucksel_Value)
{
  /**
  * This is a temporary register to avoid any unwanted transient state when performing read/modify/write
  * operation on the destination register
  */
  uint32_t uwTempRegister;
  
  uwTempRegister = localhrtc->Instance->CR;
  uwTempRegister &= (uint32_t)~RTC_CR_WUCKSEL;
  uwTempRegister |= RTC_Wucksel_Value;
  localhrtc->Instance->CR = uwTempRegister;
  
  return;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
