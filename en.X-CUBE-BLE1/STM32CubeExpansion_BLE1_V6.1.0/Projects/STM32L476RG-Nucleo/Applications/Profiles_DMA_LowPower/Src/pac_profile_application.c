/**
******************************************************************************
* @file    pac_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    13-October-2014
* @brief   This file implements the main function executing the Phone Alert
*          Client profile.
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
#include <bluenrg_types.h>
#include <bluenrg_def.h>
#include <debug.h>
#include <ble_list.h>
#include <hci_const.h>
#include <ble_profile.h>
#include <ble_events.h>
#include <timer.h>
#include "profile_application.h"
#include <phone_alert_client.h>

#include "hci_tl_interface.h"

#if (BLE_CURRENT_PROFILE_ROLES & PHONE_ALERT_CLIENT)
/*******************************************************************************
 * Macros
*******************************************************************************/
#define PAC_TIMER_VALUE	 1000000 /**< Value in us */

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
int PAC_Start_Timer = 0;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */

/******************************************************************************
 * Function Declarations
******************************************************************************/
void PACProfileGetAddress(uint8_t* addr);
void Profile_Appl_Evt_Notify(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);
void Init_Profile(void);
void Advertize(void);
static void PAC_Application_Process(void);
static void PAC_Process_Notification(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/

/**
 * @brief  Initialize the Application Context for using the Phone Alert Client
 *         profile methods
 * @param  The profile application context
 * @retval None
 */
void PAC_Appl_Context_Init(void)
{
  if (profileApplContext.profileGetAddressFunc == NULL)
  {
    profileApplContext.profileGetAddressFunc = PACProfileGetAddress;
  }
  
  if (profileApplContext.profileApplNotifyFunc == NULL)
  {
    profileApplContext.profileApplNotifyFunc = Profile_Appl_Evt_Notify;
  }
  
  if (profileApplContext.initProfileFunc == NULL)
  {
    profileApplContext.initProfileFunc = Init_Profile;
  }
  
  if (profileApplContext.advertizeFunc == NULL)
  {
    profileApplContext.advertizeFunc = Advertize;
  }

  if (profileApplContext.profileStateMachineFunc == NULL)
  {
    profileApplContext.profileStateMachineFunc = PACProfile_StateMachine;
  }
  
  if (profileApplContext.profileApplicationProcessFunc == NULL)
  {
    profileApplContext.profileApplicationProcessFunc = PAC_Application_Process;
  }
  
  if (profileApplContext.profileProcessMeasurementFunc == NULL)
  {
    profileApplContext.profileProcessMeasurementFunc = PAC_Process_Notification;
  }
}

/**
 * @brief This function sets PAC specific address
 * @param The profile address
 * @retval None
 */
void PACProfileGetAddress(uint8_t* addr)
{
  uint8_t PROFILE_ADDR[] = {0x12, 0x34, 0x09, 0xE1, 0x80, 0x02};
  
  BLUENRG_memcpy(addr, PROFILE_ADDR, sizeof(PROFILE_ADDR));
}

/**
 * @brief  Profile Application Event Notify
 * @param  Event
 * @param  Event length
 * @param  Event data
 * @retval None
 */
void Profile_Appl_Evt_Notify (tNotificationEvent event,uint8_t evtLen,uint8_t* evtData)
{  
  switch(event)
  {
  case EVT_PAC_INITIALIZED:
    {
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
      APPL_MESG_DBG(profiledbgfile,"EVT_PAC_INITIALIZED\n");
    }
    break;
  case EVT_PAC_DISCOVERY_CMPLT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_PAC_DISCOVERY_CMPLT %x\n",evtData[0]);
    }
    break;
  case EVT_PAC_ALERT_STATUS:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_PAC_ALERT_STATUS %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_PHONE_READ_ALERT_STATUS);
    }
    break;
  case EVT_PAC_RINGER_SETTING:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_PAC_RINGER_SETTING %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_PHONE_READ_RINGER_SETTING);
    }
    break;
  }
  
}/* end Profile_Appl_Evt_Notify() */


/**
 * @brief This function initializes the Phone Alert Client Profile by calling the
 *        function PAC_Init().
 * @param None
 * @retval None
 */
void Init_Profile(void)
{
  PRINTF ("Test Application: Initializing Phone Alert Client profile\n");
  if (PAC_Init(BLE_Profiles_Evt_Notify_Cb) == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Initialized Phone Alert Client Profile \n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Failed Phone Alert Client Profile \n" );
  }
  
}/* end Init_Profile() */


/**
 * @brief  This function enables advertising.
 * @param  None
 * @retval None
 */
void Advertize(void)
{
  APPL_MESG_DBG(profiledbgfile,"Test Application: advertize\n");
  PAC_Advertize();
}/* end Advertize() */


/**
 * @brief  Application specific processing
 * @param  None
 * @retval None
 */
static void PAC_Application_Process(void)
{
  switch(BLE_Profile_Read_DeviceState())
  { 
  case APPL_INIT_DONE:
    {
      if(PAC_Start_Timer == 1)
      {
        /**
    	 * Make sure to stop the timer only when it has been started at least once
    	 * The Timer ID must be existing to call TIMER_Stop();
    	 */
    	TIMER_Stop(profileApplContext.profileTimer_Id);
    	PAC_Start_Timer = 0;
      }
      Advertize();
      BLE_Profile_Write_DeviceState(APPL_ADVERTISING);
    }
    break;
  case APPL_TERMINATE_LINK:
    {
      /* on disconnection complete, the state will be changed to INIT_DONE
      */ 
      BLE_Profile_Disconnect();
      BLE_Profile_Write_DeviceState(APPL_UNINITIALIZED);
    }
    break;
  case APPL_CONNECTED:
    {
      if (PAC_Start_Timer == 0)
      {
        TIMER_Start(profileApplContext.profileTimer_Id, (uint16_t)(PAC_TIMER_VALUE/TIMERSERVER_TICK_VALUE));
        PAC_Start_Timer = 1;
      }
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  case APPL_PHONE_READ_ALERT_STATUS:		 
    {
      PAC_Read_AlertStatus();
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  case APPL_PHONE_READ_RINGER_SETTING:	
    {
      PAC_Read_RingerSetting();
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }	
    break;
  case APPL_DISABLE_ALERT_STATUS_NOTIF:	
    {
      PAC_Disable_ALert_Status_Notification();
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  case APPL_DISABLE_RINGER_SETTING_NOTIF:
    {
      PAC_Disable_Ringer_Status_Notification();
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;	
  case APPL_CONFIGURE_RINGER_SILENCE:
    {
      PAC_Configure_Ringer(0x01);
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  case APPL_CONFIGURE_RINGER_MUTE:
    {
      PAC_Configure_Ringer(0x02);
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  case APPL_CONFIGURE_RINGER_CANCEL_SILENCE: 
    {
      PAC_Configure_Ringer(0x03);
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;    
  default:    
    break;
  }
  return;
}

static void PAC_Process_Notification(void)
{
  return;
}

#endif  /* (BLE_CURRENT_PROFILE_ROLES & PHONE_ALERT_CLIENT) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
