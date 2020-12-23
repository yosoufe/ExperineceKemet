/**
******************************************************************************
* @file    uart_support.c 
* @author  AAS / CL
* @version V1.0.0
* @date    18-May-2015
* @brief   This file implements the generic function for UART communication.
******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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

/*******************************************************************************
 * Include Files
*******************************************************************************/
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "bluenrg_types.h"
#include "hci.h"
#include "bluenrg_conf.h"

#include "hci_tl_interface.h"
#include "uart_support.h"
#include "main.h"

/** @addtogroup Applications
 *  @{
 */

/** @defgroup SampleAppThT
 *  @{
 */
 
/** @defgroup UART_SUPPORT 
 * @{
 */
 
/*******************************************************************************
 * Macros
*******************************************************************************/

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/

/** @defgroup UART_SUPPORT_Exported_Variables
 *  @{
 */
/******************************************************************************
 * Global Variable Declarations
******************************************************************************/

extern UART_HandleTypeDef UartHandle;
uint8_t uart_header[UARTHEADERSIZE];
uint8_t aRxBuffer[RXBUFFERSIZE];

void *profiledbgfile;

/**
 * @}
 */
 
/******************************************************************************
 * Function Declarations
******************************************************************************/

/** @defgroup UART_SUPPORT_Functions
 *  @{
 */
/******************************************************************************
 * Function Definitions 
******************************************************************************/
/**
 * @brief
 * @param
 */
void PRINT_MESG_UART(const char * format, ... )
{
  va_list ap;
  uint8_t buffer [128];
  int n;
  
  va_start(ap, format);
  n = vsnprintf ((char*)buffer, 128, format, ap);
  va_end(ap);
  
  //notify_uart(buffer, n);
  if(HAL_UART_Transmit(&UartHandle, (uint8_t*)buffer, n, 300) != HAL_OK) {
    Error_Handler();
  }
}


void notify_uart(uint8_t* payloadString, int n)
{
  if(HAL_UART_Transmit(&UartHandle, (uint8_t*)payloadString, n, 300) != HAL_OK) {
    Error_Handler();
  }
}

static void wait(void)
{
  volatile uint32_t i;
  for (i=0; i<200000; i++) __NOP();
}

static void blink_led(void)
{
  BSP_LED_Toggle(LED2);
  wait();
  BSP_LED_Toggle(LED2); 
  wait();
}

/**
  * @brief  UART error callbacks
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @param  UART handle pointer  
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
  blink_led();
  //PRINTF("HAL_UART_ErrorCallback\n");
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
 
 /**
 * @}
 */
 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
