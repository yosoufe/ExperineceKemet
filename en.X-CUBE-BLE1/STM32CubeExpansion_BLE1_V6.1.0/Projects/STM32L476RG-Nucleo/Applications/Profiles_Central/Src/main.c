/**
******************************************************************************
* @file    main.c 
* @author  AAS / CL
* @version V1.0.0
* @date    04-July-2014
* @brief   This sample code shows how to use the BlueNRG Bluetooth Low Energy 
*          Expansion Board to exchange data between two BLE devices.
*          To test this application you need:
*          - two STM32 Nucleo boards with their BlueNRG STM32 expansion boards
*          - the Java tool profileCentral.jar running on a Win/Lin/OSX PC
*          The Java tool profileCentral.jar can be found in this package at:
*          $PATH_TO_THIS_PACKAGE$\Utilities\PC_Software\ProfileCentralTool
*
*          On one STM32 Nucleo board the Heart Rate Peripheral BLE Profile must be running.
*          On the other STM32 Nucleo board this sample application for the Heart Rate Central
*          BLE Profile must be running. This board must be connected through a mini USB cable
*          to the PC where the Java tool profileCentral.jar will be launched.
*
*          The Heart Rate Peripheral BLE Profile project is part of the
*          $PATH_TO_THIS_PACKAGE$\Projects\Multi\Applications\Profiles_LowPower application.
*
*          The Heart Rate Central BLE Profile project is part of the
*          $PATH_TO_THIS_PACKAGE$\Projects\Multi\Applications\Profiles_Central application.
*
*          To set/change the BLE Peripheral to test, change the value of the macro
*          BLE_CURRENT_PROFILE_ROLES (in the "active profile" section) in file:
*          $PATH_TO_THIS_PACKAGE$\Middlewares\ST\STM32_BlueNRG\Prof_Periph\includes\host_config.h
*
*          To set/change the BLE Central Profile to test, change the value of the macro
*          BLE_CURRENT_PROFILE_ROLES (in the "active profile" section) in file:
*          $PATH_TO_THIS_PACKAGE$\Middlewares\ST\STM32_BlueNRG\Prof_Centr\includes\host_config.h
*
*          For example, if the HEART_RATE profile is set, after the connection between the
*          two boards has been established, the Profile Central tool will
*          show the information (Services and Characteristics discovered along with HR values
*          notifications) coming from the STM32 Nucleo board running the HR peripheral role.
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
#include "hci.h"

/* USER CODE BEGIN Includes */
#include "hci_tl_interface.h"
#include "uart_support.h"
#include "debug.h"
#include "bluenrg_conf.h"
/* USER CODE END Includes */

#include "timer.h"
#include "stm32xx_timerserver.h" 

#include <stdio.h>

/** @addtogroup Applications
 *  @{
 */

/** @defgroup Profiles_Central
 *  @brief Sample application for X-NUCLEO-IDB04A1/X-NUCLEO-IDB05A1 STM32 expansion boards.
 *  @{
 */

/** @defgroup MAIN
 *  @{
 */

/** @defgroup MAIN_Global_Variables
 *  @{
 */
/* Global variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
tProfileApplContext profileApplContext; /* Central Profile Application Context */

/* USER CODE END PV */
UART_HandleTypeDef UartHandle;

/**
 * @}
 */

/** @defgroup MAIN_Private_Defines
 *  @{
 */
/* Private defines -----------------------------------------------------------*/

/**
 * @}
 */

/* Private macros ------------------------------------------------------------*/

/** @defgroup MAIN_Private_Variables
 *  @{
 */
/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;  /* RTC handler declaration */

/**
 * @}
 */

/** @defgroup MAIN_Private_Function_Prototypes
 *  @{
 */
void _Error_Handler(char *, int);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void Host_Profile_Test_Application (void);
/* USER CODE END PFP */
/**
 * @}
 */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for UART interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();
  /* Enable USART1 clock */
  USARTx_CLK_ENABLE(); 
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = USARTx_TX_AF;
  
  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);
    
  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = USARTx_RX_AF;
    
  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
}

/** 
* @brief  Init the VCOM.
* @param  None
* @return None
*/
void vcom_init(void)
{
  /*## Configure the UART peripheral ######################################*/
  /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
  /* UART1 configured as follow:
      - Word Length = 8 Bits
      - Stop Bit = One Stop bit
      - Parity = ODD parity
      - BaudRate = 115200 baud
      - Hardware flow control disabled (RTS and CTS signals) */
  UartHandle.Instance        = USARTx;
  
  UartHandle.Init.BaudRate   = 115200;
  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits   = UART_STOPBITS_1;
  UartHandle.Init.Parity     = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode       = UART_MODE_TX_RX;
  
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    /* Initialization Error */
    _Error_Handler(__FILE__, __LINE__); 
  }
}

/**
 * @brief  Main function to show how to use BlueNRG Bluetooth Low Energy 
 *         stack.
 *         To test this application you need:
 *         - two STM32 Nucleo boards with their BlueNRG STM32 expansion boards
 *         - the Java tool profileCentral.jar running on a Win/Lin/OSX PC
 *         The Java tool profileCentral.jar can be found in this package at:
 *         $PATH_TO_THIS_PACKAGE$\Utilities\PC_Software\ProfileCentralTool
 *
 *         On one STM32 Nucleo board the Heart Rate Peripheral BLE Profile must be running.
 *         On the other STM32 Nucleo board this sample application for the Heart Rate Central
 *         BLE Profile must be running. This board must be connected through a mini USB cable
 *         to the PC where the Java tool profileCentral.jar will be launched.
 *
 *         The Heart Rate Peripheral BLE Profile project is part of the
 *         $PATH_TO_THIS_PACKAGE$\Projects\Multi\Applications\Profiles_LowPower application.
 *
 *         The Heart Rate Central BLE Profile project is part of the
 *         $PATH_TO_THIS_PACKAGE$\Projects\Multi\Applications\Profiles_Central application.
 *
 *         To set/change the BLE Peripheral to test, change the value of the macro
 *         BLE_CURRENT_PROFILE_ROLES (in the "active profile" section) in file:
 *         $PATH_TO_THIS_PACKAGE$\Middlewares\ST\STM32_BlueNRG\Prof_Periph\includes\host_config.h
 *
 *         To set/change the BLE Central Profile to test, change the value of the macro
 *         BLE_CURRENT_PROFILE_ROLES (in the "active profile" section) in file:
 *         $PATH_TO_THIS_PACKAGE$\Middlewares\ST\STM32_BlueNRG\Prof_Centr\includes\host_config.h
 *
 *         For example, if the HEART_RATE profile is set, after the connection between the
 *         two boards has been established, the Profile Central tool will
 *         show the information (Services and Characteristics discovered along with HR values
 *         notifications) coming from the STM32 Nucleo board running the HR peripheral role.
 *
 * @param  None
 * @retval None
 */
int main(void)
{ 
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  
  /* STM32Cube HAL library initialization:
   *  - Configure the Flash prefetch, Flash preread and Buffer caches
   *  - Systick timer is configured by default as source of time base, but user 
   *    can eventually implement his proper time base source (a general purpose 
   *    timer for example or other time source), keeping in mind that Time base 
   *    duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
   *    handled in milliseconds basis.
   *  - Low Level Initialization
   */
  HAL_Init();
  
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  
  /* Configure the system clock */
  SystemClock_Config();
  
  /* Initialize the VCOM interface */
  vcom_init();
  
  /* Initialize the BlueNRG SPI driver */
  BNRG_SPI_Init();
  
  /* USER CODE BEGIN 2 */
  /* Initialize the Profile Application Context's Data Structures */
  BLUENRG_memset(&profileApplContext,0,sizeof(tProfileApplContext));
      
  /* Set current BlueNRG profile (HR, HT, GS, ...) */
  BNRG_Set_Current_profile();
  
  Host_Profile_Test_Application();
  /* USER CODE END 2 */  
  
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

#ifdef USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {}
}
#endif

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  PRINTF("Error_Handler\n");
  /* Blink LED2 */
  while(1)
  {
    BSP_LED_Toggle(LED2);
    HAL_Delay(100);
  }
  /* USER CODE END Error_Handler_Debug */ 
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
 
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
