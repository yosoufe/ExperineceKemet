/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : hw_config.h
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : Hardware Configuration & Setup
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "usb_type.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define PRINT_DATA_SIZE     2048
#define READ_DATA_SIZE      1024

/* Constants for user timer */
#define USER_TIMER_PRESCALER    64000-1 //TBR
/* Timeout in milliseconds. */
#define USER_TIMER_PERIOD_MSEC   200
#define USER_TIMER_PERIOD       USER_TIMER_PERIOD_MSEC*(SYSCLK_FREQ/(USER_TIMER_PRESCALER+1))/1000

#define USER_TIMER_PRESCALER_SLEEP  (USER_TIMER_PRESCALER+1)/(SYSCLK_FREQ/SYSCLK_FREQ_SLEEP) - 1

/* Exported functions ------------------------------------------------------- */
void RCC_Configuration(void);
void Init_GPIOs(void);
void Set_System(void);
void Set_USBClock(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config (FunctionalState NewState);
void Handle_USBAsynchXfer (void);
void Get_SerialNum(void);
void USB_Send_Data(uint8_t byte);
void USB_Init(void);

/* External variables --------------------------------------------------------*/

#endif  /*__HW_CONFIG_H*/
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
