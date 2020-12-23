/**
******************************************************************************
* @file    profile_application.h 
* @author  AAS / CL
* @version V1.0.0
* @date    22-September-2014
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PROFILE_APPLICATION_H
#define __PROFILE_APPLICATION_H

#include "ble_profile.h"

/** @addtogroup Applications
 *  @{
 */

/** @addtogroup Profiles_LowPower
 *  @{
 */

/** @addtogroup PROFILE_APPLICATION
 *  @{
 */

/** @addtogroup PROFILE_APPLICATION_Exported_Macros
 *  @{
 */
/*******************************************************************************
 * Macros
*******************************************************************************/
/* IO capabilities */
#define SPI_HOST_MAIN_IO_CAPABILITY_DISPLAY_ONLY        (0x00)
#define SPI_HOST_MAIN_IO_CAPABILITY_DISPLAY_YES_NO      (0x01)
#define SPI_HOST_MAIN_IO_CAPABILITY_KEYBOARD_ONLY       (0x02)
#define SPI_HOST_MAIN_IO_CAPABILITY_NO_INPUT_NO_OUTPUT  (0x03)
#define SPI_HOST_MAIN_IO_CAPABILITY_KEYBOARD_DISPLAY    (0x04)

/* MITM modes */
#define SPI_HOST_MITM_PROTECTION_NOT_REQUIRED           (0x00)
#define SPI_HOST_MITM_PROTECTION_REQUIRED               (0x01)

/* Bonding */
#define SPI_HOST_BONDING                                (0x01)
#define SPI_HOST_NO_BONDING                             (0x00)

/* OOB data */
#define SPI_HOST_OOB_AUTH_DATA_ABSENT                   (0x00)
#define SPI_HOST_OOB_AUTH_DATA_PRESENT                  (0x01)

/* PIN */
#define SPI_HOST_USE_FIXED_PIN_FOR_PAIRING              (0x00)
#define SPI_HOST_DONOT_USE_FIXED_PIN_FOR_PAIRING        (0x01)

/* application states */
#define APPL_UNINITIALIZED                              (0x00)
#define APPL_INIT_DONE                                  (0x01)
#define APPL_ADVERTISING                                (0x02)
#define APPL_CONNECTED                                  (0x03)
#define APPL_READ                                       (0x04)
#define APPL_WAIT                                       (0x05)
#define APPL_START_TIMER                                (0x06)
#define APPL_UPDATE                                     (0x07)
#define APPL_SET_TX_PWR                                 (0x08)

#define APPL_TERMINATE_LINK                             (0x09)

#define APPL_SET_BODY_SENSOR_LOCATION                   (0x0a)
#define APPL_UPDATE_MEASUREMENT_INTERVAL                (0x0b)
#define APPL_FMT_ALERT_LEVEL                            (0x0c)
#define APPL_LOCATOR_ALERT_TARGET                       (0x0d)

#define APPL_EN_DIS_NEW_ALERT_NOTIF                     (0x0e)
#define APPL_EN_DIS_UNREAD_ALERT_STATUS_NOTIF           (0x0f)
#define APPL_WRITE_CTRL_POINT                           (0x10)

#define APPL_PHONE_READ_ALERT_STATUS                    (0x11)
#define APPL_PHONE_READ_RINGER_SETTING                  (0x12)
#define APPL_DISABLE_ALERT_STATUS_NOTIF                 (0x13)
#define APPL_DISABLE_RINGER_SETTING_NOTIF               (0x14)
#define APPL_CONFIGURE_RINGER_SILENCE                   (0x15)
#define APPL_CONFIGURE_RINGER_MUTE                      (0x16)
#define APPL_CONFIGURE_RINGER_CANCEL_SILENCE            (0x17)

#define APPL_GET_REF_TIME_UPDATE                        (0x18)
#define APPL_CANCEL_REF_TIME_UPDATE                     (0x19)
#define APPL_TIME_UPDATE_NOTIFICATION                   (0x1a)
#define APPL_GET_SERV_TIME_UPDATE_STATE                 (0x1b)
#define APPL_READ_NEXT_DST_CHANGE_TIME                  (0x1c)
#define APPL_READ_LOCAL_TIME_INFORM                     (0x1d)
#define APPL_READ_CURRENT_TIME                          (0x1e)
#define APPL_GET_TIME_ACCU_INFO_SERVER                  (0x1f)

#ifdef SET_L2CAP_CONN_UPD_REQ_COMMAND /* TBR */
        
/**
* @brief Connection period, arg in msec
*/
#define CONN_P(x) ((int)((x)/1.25f))

/* HRM L2CAP Connection Update request parameters used for test */
#define L2CAP_INTERVAL_MIN        CONN_P(1000) //100
#define L2CAP_INTERVAL_MAX        CONN_P(1000) //250
#define L2CAP_SLAVE_LATENCY       0x0000
#define L2CAP_TIMEOUT_MULTIPLIER  800          //0x258
                          
#endif /* SET_L2CAP_CONN_UPD_REQ_COMMAND */

#define BDADDR_SIZE 6
/**
 * @}
 */

/** @addtogroup PROFILE_APPLICATION_Exported_Types
 *  @{
 */
/******************************************************************************
 * Types
******************************************************************************/
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
  eMAIN_HCI_THRESHOLD,            /**< Shall be in the list of enum and shall not be used by the application */
} eMAIN_Backround_Task_Id_t;
/**
 * @}
 */

/** @addtogroup PROFILE_APPLICATION_Exported_Functions
 *  @{
 */
/******************************************************************************
 * Function Declarations
******************************************************************************/
void BNRG_Profiles_Init(void);
void BNRG_Set_Current_profile(void);
void BLE_Profiles_Evt_Notify_Cb(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);

void HR_Appl_Context_Init(void);
void HT_Appl_Context_Init(void);
void GL_Appl_Context_Init(void);
void BP_Appl_Context_Init(void);
void FMT_Appl_Context_Init(void);
void FML_Appl_Context_Init(void);
void ANS_Appl_Context_Init(void);
void ANC_Appl_Context_Init(void);
void PAC_Appl_Context_Init(void);
void TS_Appl_Context_Init(void);
void TC_Appl_Context_Init(void);
void PR_Appl_Context_Init(void);
void PM_Appl_Context_Init(void);
void HID_Appl_Context_Init(void);

void BLE_Profile_Write_DeviceState(uint8_t writedevicestate);
uint8_t BLE_Profile_Read_DeviceState(void);

void TaskExecutionRequest(eMAIN_Backround_Task_Id_t eMAIN_Backround_Task_Id);
void TaskExecuted(eMAIN_Backround_Task_Id_t eMAIN_Backround_Task_Id);

/**
 * @}
 */

/** @addtogroup PROFILE_APPLICATION_Exported_Types
 *  @{
 */
/******************************************************************************
* Type definitions
******************************************************************************/
typedef void       (* GET_PROFILE_ADDR_TYPE)(uint8_t*);
typedef void       (* INIT_PROFILE_TYPE)(void);
typedef void       (* ADVERTIZE_TYPE)(void);
typedef tBleStatus (* PROFILE_STATE_MACHINE_TYPE)(void);
typedef int        (* PROFILE_APPLICATION_PROCESS_TYPE)(void);
typedef uint8_t    PROFILE_TIMER_TYPE;

/**
* @brief Profile Application Context
*/
typedef struct _tProfileApplContext
{  
  /**
  * pointer to the function called by the callback returning
  * the address specific to that profile
  */
  GET_PROFILE_ADDR_TYPE profileGetAddressFunc;

  /**
  * pointer to the function called by the callback notifying the application with
  * the profile specific events
  */
  BLE_CALLBACK_FUNCTION_TYPE profileApplNotifyFunc;
  
  /**
  * pointer to the specific profile init function
  */
  INIT_PROFILE_TYPE initProfileFunc;
  
  /**
  * pointer to specific profile function enablig advertising
  */
  ADVERTIZE_TYPE advertizeFunc;
  
  /**
  * pointer to specific profile state machine function
  */
  PROFILE_STATE_MACHINE_TYPE profileStateMachineFunc;
  
  /**
  * pointer to specific profile application process function
  */
  PROFILE_APPLICATION_PROCESS_TYPE profileApplicationProcessFunc;
  
  /**
  * Id of the timer of a profile
  */
  PROFILE_TIMER_TYPE profileTimer_Id;
    
} tProfileApplContext;
/**
 * @}
 */
 
/******************************************************************************
 * Global Variable Declarations
******************************************************************************/

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
 
#endif // __PROFILE_APPLICATION_H

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
