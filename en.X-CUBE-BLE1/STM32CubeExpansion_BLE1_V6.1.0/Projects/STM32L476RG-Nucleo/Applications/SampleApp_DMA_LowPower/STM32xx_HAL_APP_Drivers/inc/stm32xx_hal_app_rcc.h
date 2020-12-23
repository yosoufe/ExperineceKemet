/**
  ******************************************************************************
  * @file    stm32xx_hal_app_rcc.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    01-October-2014
  * @brief   Header for stm32xx_hal_app_rcc.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32XX_HAL_APP_RCC_H
#define __STM32XX_HAL_APP_RCC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#ifdef USE_STM32F4XX_NUCLEO
  #include "stm32f4xx_hal.h"
#endif /* USE_STM32F4XX_NUCLEO */

#ifdef USE_STM32L0XX_NUCLEO
  #include "stm32l0xx_hal.h"
#endif /* USE_STM32L0XX_NUCLEO */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported defines --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
 /**
   * @brief  Set the System Clock Source
   *
   * @param  __SOURCE__: System clock source
   *
   * @retval None
   */
 #define __HAL_APP_RCC_SET_SYSCLK_SRC(__SOURCE__)   (RCC->CFGR |= (__SOURCE__))

/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /*__STM32L0XX_HAL_APP_RCC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
