/**
******************************************************************************
* @file    profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    22-September-2014
* @brief   This file implements the generic function executing the BLE profile.
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

/*******************************************************************************
 * Include Files
*******************************************************************************/
#include <host_config.h>
#if (PLATFORM_WINDOWS == 1)
#include <Windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "bluenrg_types.h"
#include "bluenrg_def.h"
#include <debug.h>
#include <ble_list.h>
#include <hci.h>
#include <ble_events.h>
#include <timer.h>
#include <hci_const.h>

#include "profile_application.h"
#include "low_power_conf.h"

#include "bluenrg_aci.h"
#include "hci_tl_interface.h"
#include "bluenrg_utils.h"
#include "main.h"
#include "stm32xx_hal_app_rcc.h"

/** @addtogroup Applications
 *  @{
 */

/** @addtogroup Profiles_LowPower
 *  @{
 */

/** @defgroup PROFILE_APPLICATION
 *  @{
 */
 
/*******************************************************************************
 * Macros
*******************************************************************************/

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/

/** @defgroup PROFILE_APPLICATION_Exported_Variables
 *  @{
 */
/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
tSecurityParams Security_Param;
extern tProfileApplContext profileApplContext; /* Profile Application Context */
extern tBLEProfileGlobalContext gBLEProfileContext;

//extern tCentralDevice centralDevice;

void *profiledbgfile;

uint8_t connection_closed = 0; //add here disconnection bug details
uint8_t deviceState = APPL_UNINITIALIZED;
/**
 * @}
 */
 
/******************************************************************************
 * Function Declarations
******************************************************************************/

/** @defgroup PROFILE_APPLICATION_Exported_Functions
 *  @{
 */
/******************************************************************************
 * Function Definitions 
******************************************************************************/
/**
 * @brief
 * @param
 */
void Profile_Notify_StateMachine_Update(void)
{
  TaskExecutionRequest(eMAIN_Profile_StateMachine_update_Id);
    
  return;
}

/**
 * @brief
 * @param writedevicestate
 */
void BLE_Profile_Write_DeviceState(uint8_t writedevicestate)
{
  deviceState = writedevicestate;
  
  TaskExecutionRequest(eMAIN_Profile_App_DeviceState_update_Id);
  
  return;
}

/**
 * @brief
 * @return	devicestate
 */
uint8_t BLE_Profile_Read_DeviceState(void)
{
  return (deviceState);
}

/**
 * @brief  Event notify callback 
 * @param  Event
 * @param  Event length
 * @param  Event data
 * @retval None
 */
void BLE_Profiles_Evt_Notify_Cb(tNotificationEvent event, uint8_t evtLen, uint8_t* evtData)
{  
  switch(event)
  {
  case EVT_MP_BLUE_INITIALIZED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_BLUE_INITIALIZED\n");
    }
    break;
  case EVT_MP_ADVERTISING_TIMEOUT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_ADVERTISING_TIMEOUT\n");
    }
    break;
  case EVT_MP_CONNECTION_COMPLETE:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_CONNECTION_COMPLETE\n");
      BLE_Profile_Write_DeviceState(APPL_CONNECTED);//uncommented for the time test case csp_bv_02_c(cts).

      //FIXME: L2CAP_CONN_UPD_REQ to be moved to its proper place
//#ifdef SET_L2CAP_CONN_UPD_REQ_COMMAND
//      uint16_t connection_handle = gBLEProfileContext.connectionHandle;
//      uint16_t interval_min = L2CAP_INTERVAL_MIN;
//      uint16_t interval_max = L2CAP_INTERVAL_MAX;
//      uint16_t slave_latency = L2CAP_SLAVE_LATENCY;
//      uint16_t timeout_multiplier = L2CAP_TIMEOUT_MULTIPLIER;
//      
//      aci_l2cap_connection_parameter_update_request(connection_handle,
//                                                    interval_min, interval_max,
//                                                    slave_latency, timeout_multiplier);
//#endif
      
    }
    break;
  case EVT_MP_DISCONNECTION_COMPLETE:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_DISCONNECTION_COMPLETE %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
    }
    break;
  case EVT_MP_PASSKEY_REQUEST:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_PASSKEY_REQUEST\n");
      BLE_Profile_Send_Pass_Key(111111);
    }
    break;
  case EVT_MP_PAIRING_COMPLETE:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_PAIRING_COMPLETE\n");
      BLE_Profile_Write_DeviceState(APPL_CONNECTED);
    }
    break;
  }/* end switch() */
  
  profileApplContext.profileApplNotifyFunc(event, evtLen, evtData);
  
}/* end BLE_Profiles_Evt_Notify_Cb() */


/**
 * @brief  Initialization common to all profiles
 * @param  None
 * @retval None
 */
void BNRG_Profiles_Init(void)
{
  int indx;
  uint8_t  hwVersion;
  uint16_t fwVersion;
  uint8_t addr[BDADDR_SIZE] = {NULL};
 
  HCI_Init();
  
  /* Reset BlueNRG hardware */
  BlueNRG_RST();
  
  profileApplContext.profileGetAddressFunc(addr);
  
  /* get the BlueNRG HW and FW versions */
  getBlueNRGVersion(&hwVersion, &fwVersion);

  /* 
   * Reset BlueNRG again otherwise we won't
   * be able to change its MAC address.
   * aci_hal_write_config_data() must be the first
   * command after reset otherwise it will fail.
   */
  BlueNRG_RST();
  
  PRINTF("HWver %d, FWver %d", hwVersion, fwVersion);
  
  if (hwVersion > 0x30) { /* X-NUCLEO-IDB05A1 expansion board is used */
    /*
     * Change the MAC address to avoid issues with Android cache:
     * if different boards have the same MAC address, Android
     * applications do not properly work unless you restart Bluetooth
     * on tablet/phone
     */
    addr[5] = 0x03;
  }
  
  if(addr != NULL) {
    aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                              CONFIG_DATA_PUBADDR_LEN,
                              addr);
  }

  /**
   * Set TX Power to -2dBm.
   * This avoids undesired disconnection due to instability on 32KHz
   * internal oscillator for high transmission power.
   */
  aci_hal_set_tx_power_level(1,4);

/*
#if (BLE_CURRENT_PROFILE_ROLES & HID_DEVICE)
  aci_gap_clear_security_database();
#endif
*/

  PRINTF ("Profile Application: Starting BLE Profile\n");
  
  /* initialize the security parameters */
  BLUENRG_memset((void *)&Security_Param, 0, sizeof(Security_Param));
  Security_Param.ioCapability = SPI_HOST_MAIN_IO_CAPABILITY_NO_INPUT_NO_OUTPUT;//SPI_HOST_MAIN_IO_CAPABILITY_DISPLAY_ONLY;
  Security_Param.mitm_mode = SPI_HOST_MITM_PROTECTION_NOT_REQUIRED;//SPI_HOST_MITM_PROTECTION_REQUIRED;
  Security_Param.bonding_mode = SPI_HOST_BONDING;
  Security_Param.OOB_Data_Present = SPI_HOST_OOB_AUTH_DATA_ABSENT;
  Security_Param.Use_Fixed_Pin = SPI_HOST_USE_FIXED_PIN_FOR_PAIRING;
  Security_Param.encryptionKeySizeMin = 8;
  Security_Param.encryptionKeySizeMax = 16;
  Security_Param.Fixed_Pin = 111111;
  
  Security_Param.initiateSecurity = NO_SECURITY_REQUIRED;
  
  for (indx=0;indx<16;indx++){
    Security_Param.OOB_Data[indx] = (uint8_t)indx;
  }
  
  /* initialize profile */
  BLE_Profile_Init(&Security_Param, BLE_Profiles_Evt_Notify_Cb);
  
  PRINTF ("Profile Application: BLE main profile initialized. \n" );
  
}

/**
 * @brief  Set current BlueNRG profile
 * @param  None
 * @retval None
 */
void BNRG_Set_Current_profile(void)
{
#if (BLE_CURRENT_PROFILE_ROLES & HEART_RATE_SENSOR)
  HR_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & HEALTH_THERMOMETER)
  HT_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & GLUCOSE_SENSOR)
  GL_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & BLOOD_PRESSURE_SENSOR)
  BP_Appl_Context_Init();
#endif
  
#if (BLE_CURRENT_PROFILE_ROLES & FIND_ME_TARGET)
  FMT_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & FIND_ME_LOCATOR)
  FML_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & ALERT_NOTIFICATION_SERVER)
  ANS_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & ALERT_NOTIFICATION_CLIENT)
  ANC_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & PHONE_ALERT_CLIENT)
  PAC_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & TIME_SERVER)
  TS_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & TIME_CLIENT)
  TC_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & PROXIMITY_REPORTER)
  PR_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & PROXIMITY_MONITOR)
  PM_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & HID_DEVICE)
  HID_Appl_Context_Init();
#endif
}

/**
  * @brief  Request action to the scheduler in background
  *
  * @note
  * @retval None
  */
__weak void TaskExecutionRequest(eMAIN_Backround_Task_Id_t eMAIN_Backround_Task_Id) 
{
  /* If needed, to be implemented in the user files */
  return;
}

/**
  * @brief  Notify the action in background has been completed
  *
  * @note
  * @param  eMAIN_Backround_Task_Id: Id of the request
  * @retval None
  */
__weak void TaskExecuted(eMAIN_Backround_Task_Id_t eMAIN_Backround_Task_Id)
{
  /* If needed, to be implemented in the user files */
  return;
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
