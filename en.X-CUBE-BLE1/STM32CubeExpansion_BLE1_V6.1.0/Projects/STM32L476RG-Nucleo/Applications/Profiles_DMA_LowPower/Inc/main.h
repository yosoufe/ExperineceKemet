/**
  ******************************************************************************
  * @file    main.h
  * @author  MCD Application Team
  * @version V1.0
  * @date    14-April-2014
  * @brief   Header for main.c module
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "cube_hal.h"

/** @addtogroup Applications
 *  @{
 */

/** @addtogroup Profiles_DMA_LowPower
 *  @{
 */

/** @addtogroup MAIN
 *  @{
 */
 
/* Exported constants --------------------------------------------------------*/
/**
 * bit mapping of event not requiring sending HCI event
 */
#define	EVENT_NOT_REQUIRING_SENDING_HCI_COMMAND	(~(uint32_t)((1<<eMAIN_HCI_THRESHOLD)-1))

/** @defgroup MAIN_Exported_Types
 *  @{
 */
/* Exported types ------------------------------------------------------------*/
/**
  * All enum after eMAIN_HCI_THRESHOLD shall not request sending HCI command
  */
typedef enum
{
  eMAIN_HCI_Process_Request_Id,
  eMAIN_Profile_Process_Request_Id,
  eMAIN_Profile_App_DeviceState_update_Id,
  eMAIN_Profile_StateMachine_update_Id,
  eMAIN_Profile_Measurement_update_Id,
  eMAIN_Profile_Event_Id,
  eMAIN_HCI_THRESHOLD  /**< Shall be in the list of enum and shall not be used by the application */
} eMAIN_Background_Task_Id_t;
/**
 * @}
 */

/** @defgroup MAIN_Exported_Defines
 *  @{
 */
/* Exported defines -----------------------------------------------------------*/
#define JTAG_SUPPORTED 0 /* if 1 keep debugger enabled while in any low power mode */

#define HCLK_80MHZ 0 /* can be set to 1 only for STM32L476xx */

#define LOW_POWER_MODE 1 /* 1 = Low Power mode ON, 0 = Low Power Mode OFF */

 /**
  * RTC cloc divider
  */
#define WUCKSEL_DIVIDER      (3) /**< Tick is  (LSI speed clock/2) */
#define RTC_ASYNCH_PRESCALER (1)
#define RTC_SYNCH_PRESCALER  (0x7FFF)
/**
 * @}
 */

/** @defgroup MAIN_Exported_Functions
 *  @{
 */
/* Exported functions ------------------------------------------------------- */
void TaskExecutionRequest(eMAIN_Background_Task_Id_t eMAIN_Background_Task_Id);
void TaskExecuted(eMAIN_Background_Task_Id_t eMAIN_Background_Task_Id);
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

 #ifdef __cplusplus
}
#endif

#endif /*__MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
