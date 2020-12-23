/**
  ******************************************************************************
  * @file    bluenrg_interface.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    04-July-2014
  * @brief   This file provides code for the BlueNRG Expansion Board driver
  *          based on STM32Cube HAL for STM32 Nucleo boards.
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
#include "hci_tl_interface.h"

#include "debug.h"
#include "bluenrg_def.h"
#include "hci_tl.h"
#include "stm32_bluenrg_ble.h"
#include "gp_timer.h"
#include "stm32_bluenrg_ble.h"
#include "low_power_conf.h"
#include "stm32xx_lpm.h"

extern SPI_HandleTypeDef SpiHandle;

volatile uint8_t send_measurement = 0;

/**
  * @brief  RTC Wake Up callback
  * @param  RTC handle pointer
  * @retval None
  */
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    uint32_t uwPRIMASK_Bit = __get_PRIMASK();	/**< backup PRIMASK bit */;
  
    /* Clear Wake Up Flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    __disable_irq();			/**< Disable all interrupts by setting PRIMASK bit on Cortex*/
    send_measurement++;
    __set_PRIMASK(uwPRIMASK_Bit);	/**< Restore PRIMASK bit*/
    LPM_Mode_Request(eLPM_MAIN_LOOP_PROCESSES, eLPM_Mode_RUN);
}

/**
 * @brief  EXTI line detection callback.
 * @param  uint16_t Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Clear Wake Up Flag */
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  
  HCI_Isr();
  
  LPM_Mode_Request(eLPM_MAIN_LOOP_PROCESSES, eLPM_Mode_RUN);  
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
