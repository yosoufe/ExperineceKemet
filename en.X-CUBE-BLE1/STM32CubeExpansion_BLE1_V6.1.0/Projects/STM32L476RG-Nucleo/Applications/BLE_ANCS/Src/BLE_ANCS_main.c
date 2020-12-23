/******************** (C) COPYRIGHT 2016 STMicroelectronics ********************
* File Name          : BLE_ANCS_main.c
* Author             : AMG RF Application Team
* Version            : V1.0.0
* Date               : 14-July-2016
* Description        : BlueNRG-MS ANCS main
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.2q
*******************************************************************************/
/**
 * @file BLE_ANCS_main.c
 * @brief This is a BLE ANCS demo that shows how to configure a BlueNRG-MS device 
 * in order to works like a notification consumer device.
 * 
 * <!-- Copyright 2016 by STMicroelectronics.  All rights reserved.       *80*-->

* \section IAR_project IAR project
  To use the project with IAR Embedded Workbench for ARM, please follow the instructions below:
  -# Open the Embedded Workbench for ARM and select File->Open->Workspace menu. 
  -# Open the IAR project
     <tt> ...\\Projects\\Projects_STD_Library\\BLE_ANCS\\EWARM_BlueNRG-MS\\BLE_ANCS.eww</tt> or
     <tt> ...\\Projects\\Projects_Cube\\BLE_ANCS\\EWARM_BlueNRG-MS\\BLE_ANCS.eww </tt>
  -# Select desired configuration to build
  -# Select Project->Rebuild All. This will recompile and link the entire application
  -# To download the binary image, please connect STLink to JTAG connector in your board (if available).
  -# Select Project->Download and Debug to download the related binary image.
  -# Alternatively, open the BlueNRG1 GUI, put the board in bootloader mode and download the built binary image.

* \subsection IAR_project_configurations IAR project configurations

  - \c Release - Notification consumer device configuration (BlueNRG-MS Kits platforms).
  - \c Release_Nucleo - Notification consumer device configuration (UCLEO-L152RE + with X-NUCLEO-IDB05A1 (BlueNRG-MS) platform).

* \section Prebuilt_images Prebuilt images
  - None
 
* \section Jumper_settings Jumper settings
@table
------------------------------------------------------
| Jumper name       |  Description                   | 
------------------------------------------------------
| JP1, if available | USB or Battery supply position | 

@endtable 

* \section Board_supported Boards supported
@table
| Board name (Order Code)             | Description                           | 
-------------------------------------------------------------------------------
| STEVAL-IDB005V1                     | BlueNRG-MS Development Platform       | 
| STEVAL-IDB005V1D                    | BlueNRG-MS Daughter Board             |  
| STEVAL-IDB006V1                     | BlueNRG-MS  USB Dongle                | 
| NUCLEO-L152RE + X-NUCLEO-IDB05A1    | STM32L152 Nucleo + BlueNRG-MS         | 
@endtable

* \section Serial_IO Serial I/O
 - Not Applicable;

* \section LEDs_description LEDs description
@table                    
| LED name         | STEVAL-IDB005V1/5V1D | STEVAL-IDB006V1     | NUCLEO-L152RE + X-NUCLEO-IDB05A1 |
----------------------------------------------------------------------------------------------------
| D1               | Configuration led    | NA                  | NA                               |
| D2               | Not used             | Configuration led   | Configuration/Connection led     |
| D3               | Connection led       | Connection led      | NA                               |
| D4               | Not used             | NA                  | NA                               |
| D5               | Not used             | NA                  | NA                               |
@endtable
 - NA : Not Applicable;

* \section Buttons_description Buttons description
@table                
| BUTTON name      | STEVAL-IDB005V1/5V1D | STEVAL-IDB006V1  | NUCLEO-L152RE + X-NUCLEO-IDB05A1 |    
-------------------------------------------------------------------------------------------------
| RESET            | X                   | NA                | X                                |
| Push Button      | Not used            | NA                | Not used                         | 
| Jostick Sel      | Not used            | NA                | X                                | 
| SW1              | NA                  | Not used          | X                                |
| SW2              | NA                  | Not used          | X                                |
@endtable
 - NA : Not Applicable;

* \section DFU_Activation  DFU activation
BlueNRG-MS boards are preprogrammed with a DFU application which allows to upload the 
STM32L micro with a selected binary image through USB. Follow list of actions 
for activating DFU on each supported platforms
@table
-------------------------------------------------------------------------------------------------
| Board  name          | Event                                             | Note               |
-------------------------------------------------------------------------------------------------
| STEVAL-IDB005V1      | Press RESET and Push Button. Release RESET button | LED D2 is toggling |  
| STEVAL-IDB006V1      | Press SW1 button and plug USB dongle on a PC port | LED D3 is toggling |  
@endtable
- Note: No DFU support on NUCLEO-L152RE with X-NUCLEO-IDB05A1 (BlueNRG-MS).

* \section Usage Usage

The ANCS demo configures a BlueNRG-MS device as a Notification Consumer. The purpose of the ANCS profile, BLE Notification Consumer role is to give Bluetooth accessories a simple and convenient way to access many kinds of notifications that are generated on a Notification Provider.
The BLE_ANCS demo, after the reset, puts the BlueNRG-MS in advertising with device name "ANCSdemo", and it sets the BlueNRG-MS authentication requirements for enabling bonding.
When the device is connected and bonded with a Notification Provider, the demo configures the BlueNRG-MS Notification Consumer device to discover the service and the characteristics of the Notification Provider. When the setup phase is completed, the BlueNRG-MS device is configured as a Notification Consumer and is able to receive every notification sent from the Notification Provider.

**/

/** @addtogroup Applications
 *  @{
 */

/** @addtogroup BLE_ANCS
 *  @{
 */
 
/** @defgroup BLE_ANCS_MAIN
 * @{
 */
 
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "cube_hal.h"

/* USER CODE BEGIN Includes */
#include "hci_tl.h"
#include "user_timer.h"
#include "demo_application.h"
#include "uart_support.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef    TimHandle;
/* Private functions ---------------------------------------------------------*/
void _Error_Handler(char *, int);   
   
UART_HandleTypeDef UartHandle;

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

int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t ret;
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
  /* Configure LED2 */
  BSP_LED_Init(LED2);

  /* Configure the User Button in GPIO Mode */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
  /* USER CODE END Init */
  
  /* Configure the system clock */
  SystemClock_Config();
   
  /* Initialize the VCOM interface */
  vcom_init();

  /* Initialize the BlueNRG SPI driver */
  BNRG_SPI_Init();
  
  /* Initialize the BlueNRG HCI */
  HCI_Init();

  /* Reset BlueNRG hardware */
  BlueNRG_RST();

  /* USER CODE BEGIN 2 */
  /* Init Application Timer */
  Init_User_Timer();
  
  /* Application Init */
  ret = application_init();
  if (ret != BLE_STATUS_SUCCESS) {
    PRINTF("Error in application_init() 0x%02x\n", ret);
  }

  /* Start Advertising */
  ret = application_ADV_Start();
  if (ret != BLE_STATUS_SUCCESS) {
    PRINTF("Error in application_ADV_Start() 0x%02x\n", ret);
  }
  /* USER CODE END 2 */
  
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(1) 
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    HCI_Process();
   
    /* Application Tick */
    APP_Tick();
  }
  /* USER CODE END 3 */
  
}

#ifdef  USE_FULL_ASSERT

/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {
  }
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
  //printf("Error_Handler\n");
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
 
/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
