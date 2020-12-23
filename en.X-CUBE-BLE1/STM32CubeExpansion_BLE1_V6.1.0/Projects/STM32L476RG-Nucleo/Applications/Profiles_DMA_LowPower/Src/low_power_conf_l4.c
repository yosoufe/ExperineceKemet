/**
******************************************************************************
* @file    low_power_conf_l4.c 
* @author  AAS / CL
* @version V1.0.0
* @date    15-July-2015
* @brief   
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
#include "main.h"
#include "low_power_conf.h"

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Configure the system for power optimization
  *
  * @note	This API configures the system to be ready for low power mode
  * 		- Unused GPIO in Analog mode
  * 		- Fast Wakeup
  * 		- Verefint disabled in low power mode
  * 		- FLASH in low power mode
  *
  * @param  None
  * @retval None
  */
void SystemPower_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /**
  * Disable FLASH in SLEEP Mode
  */
  __HAL_FLASH_SLEEP_POWERDOWN_ENABLE();

  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /**
  * Enable Ultra low power and Fast wakeup as vrefint is not used in that project
  */
  //(these APIs are not available for L4)
  //HAL_PWREx_EnableUltraLowPower();
  //HAL_PWREx_EnableFastWakeUp();

  /*
  * Select HSI as system clock source after Wake Up from Stop mode
  */
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_StopWakeUpClock_HSI);
  
  /*
  * Disable all GPIOs clock in SLEEP mode
  */
#if (JTAG_SUPPORTED == 0)
  __GPIOA_CLK_SLEEP_DISABLE();
#endif
  __GPIOB_CLK_SLEEP_DISABLE();
  __GPIOC_CLK_SLEEP_DISABLE();
  __GPIOD_CLK_SLEEP_DISABLE();
  __GPIOE_CLK_SLEEP_DISABLE();
  __GPIOF_CLK_SLEEP_DISABLE();
  __GPIOG_CLK_SLEEP_DISABLE();
  __GPIOH_CLK_SLEEP_DISABLE();
  
  /*
  * Enable GPIOs clock
  */
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOE_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();
  __GPIOH_CLK_ENABLE();

  /*
  * Configure all GPIO port pins in Analog mode with no pull
  */
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_All;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);

#if (JTAG_SUPPORTED == 1)
  GPIO_InitStructure.Pin = (GPIO_PIN_All ^ (GPIO_PIN_13 | GPIO_PIN_14));
#endif
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
  
  /*
  * Disable GPIOs clock
  */
  __GPIOA_CLK_DISABLE();
  __GPIOB_CLK_DISABLE();
  __GPIOC_CLK_DISABLE();
  __GPIOD_CLK_DISABLE();
  __GPIOE_CLK_DISABLE();
  __GPIOF_CLK_DISABLE();
  __GPIOG_CLK_DISABLE();
  __GPIOH_CLK_DISABLE();
  
  return;
}

/**
  * @brief RTC MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  * @param hrtc: RTC handle pointer
  * @note  Care must be taken when HAL_RCCEx_PeriphCLKConfig() is used to select 
  *        the RTC clock source; in this case the Backup domain will be reset in  
  *        order to modify the RTC Clock source, as consequence RTC registers (including 
  *        the backup registers) and RCC_CSR register are set to their reset values.  
  * @retval None
  */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
      
  /*##-1- Configue the RTC clock source ######################################*/
#ifdef RTC_CLOCK_SOURCE_LSE
  /* -a- Enable LSE Oscillator */
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while(1);
  }

  /* -b- Select LSE as RTC clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  { 
    while(1);
  }
#elif defined (RTC_CLOCK_SOURCE_LSI)
  /* -a- Enable LSI Oscillator */
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while(1);
  }
  /* -b- Select LSI as RTC clock source */ 
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    while(1);
  }
#else
#error Please select the RTC Clock source inside the low_power_conf.h file
#endif /*RTC_CLOCK_SOURCE_LSE*/

  /*##-2- Enable the RTC peripheral Clock ####################################*/
  /* Enable RTC Clock */
  __HAL_RCC_RTC_ENABLE();

  /*##-3- Configure the NVIC for RTC Wake up Timer  #########################*/
  HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);

}

/**
  * @brief RTC MSP De-Initialization
  *        This function freeze the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hrtc: RTC handle pointer
  * @retval None
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
  /*##-1- Reset peripherals ##################################################*/
   __HAL_RCC_RTC_DISABLE();
}

/**
 * @brief  Call procedure for configuring the system clock after wake-up
 *         from STOP
 * @param  None
 * @retval None
 */
void LPM_ExitStopMode(void)
{
#if (HCLK_80MHZ == 1)
  __HAL_RCC_PLL_ENABLE();                                     /**< Enable PLL */
  while(__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) == RESET);        /**< Wait till PLL is ready */
  __HAL_APP_RCC_SET_SYSCLK_SRC(RCC_CFGR_SW_PLL);              /**< Select PLL as system clock source */
  while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_CFGR_SWS_PLL);  /**< Wait till PLL is used as system clock source */
#endif
  
  return;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
