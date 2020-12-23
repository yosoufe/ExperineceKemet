/**
******************************************************************************
* @file    anc_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    13-October-2014
* @brief   This file implements the main function executing the Alert
*          Notification Client profile.
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
#include <hci_const.h>
#include <ble_profile.h>
#include <ble_events.h>
#include <timer.h>
#include "stm32xx_timerserver.h"
#include "profile_application.h"
#include <alertNotification_Client.h>

#include "hci_tl_interface.h"

#if (BLE_CURRENT_PROFILE_ROLES & ALERT_NOTIFICATION_CLIENT)
/*******************************************************************************
 * Macros
*******************************************************************************/
#define ANC_TIMER_VALUE	 1000000 /**< Value in us */

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
int ANC_Start_Timer = 0;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */

/******************************************************************************
 * Function Declarations
******************************************************************************/
void ANCProfileGetAddress(uint8_t* addr);
void Profile_Appl_Evt_Notify(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);
void Init_Profile(void);
void Advertize(void);
static int ANC_Application_Process(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/

/**
 * @brief  Initialize the Application Context for using the Alert Notification
 *         Client profile methods
 * @param  The profile application context
 * @retval None
 */
void ANC_Appl_Context_Init(void)
{
  if (profileApplContext.profileGetAddressFunc == NULL)
  {
    profileApplContext.profileGetAddressFunc = ANCProfileGetAddress;
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
    profileApplContext.profileStateMachineFunc = ANCProfile_StateMachine;
  }
  
  if (profileApplContext.profileApplicationProcessFunc == NULL)
  {
    profileApplContext.profileApplicationProcessFunc = ANC_Application_Process;
  }

}

/**
 * @brief This function sets Alert Notification Client specific address
 * @param The profile address
 * @retval None
 */
void ANCProfileGetAddress(uint8_t* addr)
{
  uint8_t PROFILE_ADDR[] = {0x12, 0x34, 0x08, 0xE1, 0x80, 0x02};
  
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
  case EVT_ANC_INITIALIZED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_ANC_INITIALIZED %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
    }
    break;
  case EVT_ANC_DISCOVERY_CMPLT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_ANC_DISCOVERY_CMPLT %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_CONNECTED);
    }
    break;
  case EVT_ANC_NEW_ALERT_RECEIVED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_ANC_NEW_ALERT_RECEIVED %x %x %x\n",evtLen,evtData[0],evtData[1]);
    }
    break;
  case EVT_ANC_UNREAD_ALERT_STATUS_RECEIVED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_ANC_UNREAD_ALERT_STATUS_RECEIVED %x %x %x\n",evtLen,evtData[0],evtData[1]);
    }
    break;
  }
  
}/* end Profile_Appl_Evt_Notify() */

/**
 * @brief  This function initializes the Alert Notification Client Profile by calling the
 *         function ANC_Client_Init().
 * @param  None
 * @retval None
 */
void Init_Profile(void)
{
  PRINTF ("Test Application: Initializing Alert Notification Client profile\n");
  if (ANC_Client_Init(BLE_Profiles_Evt_Notify_Cb) == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Initialized Alert Notification Client Profile \n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Failed Alert Notification Client Profile \n" );
  }
  
}/* end Init_Profile() */


/**
 * @brief This function enables advertising.
 * @param None
 * @retval None
 */
void Advertize(void)
{
  APPL_MESG_DBG(profiledbgfile,"Test Application: advertize\n");
  ANC_Advertize(0x00);
}/* end Advertize() */


/**
 * @brief  Application specific processing
 * @param  None
 * @retval int
 */
static int ANC_Application_Process(void)
{
  int ret_val = 1;

  switch(BLE_Profile_Read_DeviceState())
  {
  case APPL_UNINITIALIZED:
    break;
  case APPL_INIT_DONE:
    {    
      if(ANC_Start_Timer == 1)
      {
        /**
    	 * Make sure to stop the timer only when it has been started at least once
    	 * The Timer ID must be existing to call TIMER_Stop();
    	 */
    	TIMER_Stop(profileApplContext.profileTimer_Id);
    	ANC_Start_Timer = 0;
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
      if (ANC_Start_Timer == 0)
      {
        TIMER_Start(profileApplContext.profileTimer_Id, (uint16_t)(ANC_TIMER_VALUE/TIMERSERVER_TICK_VALUE));
        ANC_Start_Timer = 1;
      }
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  case APPL_EN_DIS_NEW_ALERT_NOTIF:
    {
      ANC_Enable_Disable_New_Alert_Notification(0x0001);
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }			 	 
    break;
  case APPL_EN_DIS_UNREAD_ALERT_STATUS_NOTIF: 
    {
      ANC_Enable_Disable_Unread_Alert_Status_Notification(0x0001);
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  case APPL_WRITE_CTRL_POINT:
    {
      ANC_Write_Control_Point(ENABLE_NEW_ALERT_NOTIFICATION, CATEGORY_ID_EMAIL);
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  default:
    ret_val = 0;
    break;
  }
  return ret_val;
}

#endif  /* (BLE_CURRENT_PROFILE_ROLES & ALERT_NOTIFICATION_CLIENT) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
