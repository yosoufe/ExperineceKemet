/**
  ******************************************************************************
  * @file    uart_support.h 
  * @author  AAS / CL
  * @version V1.0.0
  * @date    18-May-2015
  * @brief   Header for uart_support.c module
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_SUPPORT_H
#define __UART_SUPPORT_H

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_nucleo.h"

/* Exported types ------------------------------------------------------------*/
typedef void (* INIT_DEVICE_TYPE)(void);
typedef void (* DEVICE_SET_SECURITY_TYPE)(void);
typedef uint8_t (* DEVICE_DISCOVERY_PROCEDURE_TYPE)(void);
typedef void (* DEVICE_DISCONNECTION_TYPE)(void);
typedef void (* CP_FUNCTION_TYPE)(uint8_t*, uint8_t);

/**
* @brief Profile Application Context
*/
typedef struct _tProfileApplContext
{  
  /**
  * pointer to the specific central device init function
  */
  INIT_DEVICE_TYPE initDeviceFunc;

  /**
  * pointer to the specific central device function to set security
  */
  DEVICE_SET_SECURITY_TYPE deviceSetSecurityFunc;
  
  /**
  * pointer to the specific central disconnection function
  */
  DEVICE_DISCONNECTION_TYPE deviceDisconnectionFunc;
  
  /**
  * pointer to the specific central discovery procedure function
  */
  DEVICE_DISCOVERY_PROCEDURE_TYPE deviceDiscoveryProcedureFunc;
  
  /**
  * pointer to the specific central control point function
  */
  CP_FUNCTION_TYPE cpFunc;
  
  uint8_t deviceState;
  
  BOOL deviceMasterInited;
  
  BOOL startDeviceConn;
  
  uint8_t peerAddr[6];
  
} tProfileApplContext;

/* Exported constants --------------------------------------------------------*/
/** 
 * @brief User can use this section to tailor USARTx/UARTx instance used and 
 *        associated resources.
 */

#ifdef USE_STM32F4XX_NUCLEO
  /* Definition for USARTx clock resources */
  #define USARTx                           USART2
  #define USARTx_CLK_ENABLE()              __USART2_CLK_ENABLE();
  #define USARTx_RX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
  #define USARTx_TX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE() 
  #define USARTx_FORCE_RESET()             __USART2_FORCE_RESET()
  #define USARTx_RELEASE_RESET()           __USART2_RELEASE_RESET()
  /* Definition for USARTx Pins */
  #define USARTx_TX_PIN                    GPIO_PIN_2
  #define USARTx_TX_GPIO_PORT              GPIOA  
  #define USARTx_TX_AF                     GPIO_AF7_USART2
  #define USARTx_RX_PIN                    GPIO_PIN_3
  #define USARTx_RX_GPIO_PORT              GPIOA 
  #define USARTx_RX_AF                     GPIO_AF7_USART2
  /* Definition for USARTx's NVIC */
  #define USARTx_IRQn                      USART2_IRQn
  #define USARTx_IRQHandler                USART2_IRQHandler
#endif /* USE_STM32F4XX_NUCLEO */

#ifdef USE_STM32L0XX_NUCLEO
  /* Definition for USARTx clock resources */
  #define USARTx                           USART2
  #define USARTx_CLK_ENABLE()              __USART2_CLK_ENABLE();
  #define USARTx_RX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
  #define USARTx_TX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE() 
  #define USARTx_FORCE_RESET()             __USART2_FORCE_RESET()
  #define USARTx_RELEASE_RESET()           __USART2_RELEASE_RESET()
  /* Definition for USARTx Pins */
  #define USARTx_TX_PIN                    GPIO_PIN_2
  #define USARTx_TX_GPIO_PORT              GPIOA  
  #define USARTx_TX_AF                     GPIO_AF4_USART2
  #define USARTx_RX_PIN                    GPIO_PIN_3
  #define USARTx_RX_GPIO_PORT              GPIOA 
  #define USARTx_RX_AF                     GPIO_AF4_USART2
  /* Definition for USARTx's NVIC */
  #define USARTx_IRQn                      USART2_IRQn
  #define USARTx_IRQHandler                USART2_IRQHandler
#endif /* USE_STM32L0XX_NUCLEO */

#ifdef USE_STM32L4XX_NUCLEO  
  /* Definition for USARTx clock resources */
  #define USARTx                           USART2
  #define USARTx_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE()
  #define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
  #define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
  #define USARTx_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
  #define USARTx_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()
  /* Definition for USARTx Pins */
  #define USARTx_TX_PIN                    GPIO_PIN_2
  #define USARTx_TX_GPIO_PORT              GPIOA  
  #define USARTx_TX_AF                     GPIO_AF7_USART2
  #define USARTx_RX_PIN                    GPIO_PIN_3
  #define USARTx_RX_GPIO_PORT              GPIOA 
  #define USARTx_RX_AF                     GPIO_AF7_USART2
  /* Definition for USARTx's NVIC */
  #define USARTx_IRQn                      USART2_IRQn
  #define USARTx_IRQHandler                USART2_IRQHandler
#endif /* USE_STM32L4XX_NUCLEO */

/* Size of Transmission buffer */
#define TXSTARTMESSAGESIZE                 (COUNTOF(aTxStartMessage) - 1)
#define TXENDMESSAGESIZE                   (COUNTOF(aTxEndMessage) - 1)

/* Size of Reception buffer */
#define UARTHEADERSIZE 4
#define RXBUFFERSIZE 255

/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

/* application states */
#define APPL_UNINITIALIZED          (0x00)
#define APPL_INIT_DONE              (0x01)
#define APPL_CONNECTED              (0x02)
#define APPL_SERVICES_DISCOVERED    (0x03)
#define APPL_CHARS_DISCOVERED       (0x04)
#define APPL_GOT_CHAR_DESC          (0x05)
#define APPL_NOTIFICATION_ENABLED   (0x06)
#define APPL_INDICATION_ENABLED     (0x07)
#define APPL_VALUE_READ             (0x08)

#define UNKNOWN_COMMAND      0x01

#define RESP_VENDOR_CODE_OFFSET 1
#define RESP_LEN_OFFSET_LSB     2
#define RESP_LEN_OFFSET_MSB     3
#define RESP_CMDCODE_OFFSET     4
#define RESP_STRING_LEN_OFFSET  5
#define RESP_STATUS_OFFSET      6

#define PKT_TYPE_OFFSET         0
#define PKT_CMDCODE_OFFSET      1

#define RESPONSE                     1
#define PROFILE_CENTRAL_HEADER_SIZE  7

#define PROFILE_CENTRAL "Central Profile App"

/* Commands */
#define APP_TYPE                0x01
#define PROFILE_CENTRAL_OPEN    0x06
#define PROFILE_CENTRAL_NOTIFY  0x07
#define SCAN_RES                0x08
#define DEVICE_CONNECTION       0x09
#define CONNECTION_STATUS       0x0A
#define CLOSE_CONNECTION        0x0B
#define DISCOVERY_TIMEOUT       0x0C
#define SCAN_REQ                0x0D
#define SERVICE_UUID            0x11
#define CP_COMMAND              0x12
#define VENDOR_PKT              0xFF

#define PACK_2_BYTE_PARAMETER(ptr, param)  do{\
                *((uint8_t *)ptr) = (uint8_t)(param);   \
                *((uint8_t *)ptr+1) = (uint8_t)(param)>>8; \
                }while(0)

#define PACK_2_BYTE_PARAM16(ptr, param)  do{\
                *((uint8_t *)ptr) = (param)>>8;   \
                *((uint8_t *)ptr+1) = (param); \
                }while(0)
                  
/* Exported functions ------------------------------------------------------- */
void BNRG_Set_Current_profile(void);
void handle_uart_cmd(uint8_t* message, uint8_t msg_len, const char* payloadString);
void notify_uart(uint8_t* payloadString, int n, uint8_t cmdCode);

void HR_Appl_Context_Init(void);
void HT_Appl_Context_Init(void);
void GC_Appl_Context_Init(void);
void BP_Appl_Context_Init(void);
void TC_Appl_Context_Init(void);
void FML_Appl_Context_Init(void);
void ANC_Appl_Context_Init(void);

extern UART_HandleTypeDef UartHandle;
extern uint8_t uart_header[UARTHEADERSIZE];

extern void *profiledbgfile;

#endif /* __UART_SUPPORT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
