/**
******************************************************************************
* @file    tc_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    15-October-2014
* @brief   This file implements the main function executing the Time Client
*          profile.
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
#include "low_power_conf.h"
#include "stm32xx_timerserver.h"
#include "profile_application.h"
#include <time_client.h>

#include "hci_tl_interface.h"

#if (BLE_CURRENT_PROFILE_ROLES & TIME_CLIENT)
/*******************************************************************************
 * Macros
*******************************************************************************/
#define TC_TIMER_VALUE	1000000 /**< Value in us */

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
int TC_Start_Timer = 0;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */

/******************************************************************************
 * Function Declarations
******************************************************************************/
void TCProfileGetAddress(uint8_t* addr);
void Profile_Appl_Evt_Notify(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);
void Init_Profile(void);
void Advertize(void);
static void TC_Application_Process(void);
static void TC_Process_Time(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/

/**
 * @brief  Initialize the Application Context for using the TC profile methods
 * @param  None
 * @retval None
 */
void TC_Appl_Context_Init(void)
{
  if (profileApplContext.profileGetAddressFunc == NULL)
  {
    profileApplContext.profileGetAddressFunc = TCProfileGetAddress;
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
    profileApplContext.profileStateMachineFunc = TimeClient_StateMachine;
  }
  
  if (profileApplContext.profileApplicationProcessFunc == NULL)
  {
    profileApplContext.profileApplicationProcessFunc = TC_Application_Process;
  }
  
  if (profileApplContext.profileProcessMeasurementFunc == NULL)
  {
    profileApplContext.profileProcessMeasurementFunc = TC_Process_Time;
  }

}

/**
 * @brief This function sets TC specific address
 * @param The profile address
 * @retval None
 */
void TCProfileGetAddress(uint8_t* addr)
{
  uint8_t PROFILE_ADDR[] = {0x12, 0x34, 0x11, 0xE1, 0x80, 0x02};
  
  BLUENRG_memcpy(addr, PROFILE_ADDR, sizeof(PROFILE_ADDR));
}

/**
 * @brief Notify the event to the profile application
 * @param Event
 * @param Event length
 * @param Event data
 */
void Profile_Appl_Evt_Notify (tNotificationEvent event,uint8_t evtLen,uint8_t* evtData)
{  
  switch(event)
  {
  case EVT_MP_ADVERTISING_TIMEOUT:
    {
      APPL_MESG_DBG(profiledbgfile,"enable advertising\n");
      TimeClient_Make_Discoverable(0x00);
    }
    break;
  case EVT_TC_INITIALIZED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_TC_INITIALIZED\n");
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
    }
    break;
  case EVT_TC_DISCOVERY_CMPLT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_TC_DISCOVERY_CMPLT\n");
      BLE_Profile_Write_DeviceState(APPL_UPDATE);
    }
    break;
  case EVT_TC_READ_REF_TIME_INFO_CHAR:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_TC_READ_REF_TIME_INFO_CHAR\n");
      APPL_MESG_DATA(profiledbgfile,evtData,evtLen);
      
      APPL_MESG_DBG(profiledbgfile,"\n");
    }
    break;    
  }
  
}/* end Profile_Appl_Evt_Notify() */


/**
 * @brief This function initializes the Time Client Profile by calling the
 *        function TimeClient_Init().
 * @param None
 * @param None
 */
void Init_Profile()
{
  PRINTF ("Test Application: Initializing Time Client Profile \n" );
  if (TimeClient_Init(BLE_Profiles_Evt_Notify_Cb) == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Initialized Time Client Profile \n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Failed Time Client Profile \n" );
  }
}/* end Init_Profile() */


/**
 * @brief This function enables advertising.
 * @param None
 * @param None
 */
void Advertize()
{
  APPL_MESG_DBG(profiledbgfile,"enable advertising\n");
  TimeClient_Make_Discoverable(0x00);
}/* end Advertize() */


/**
 * @brief  Time Client Application specific processing
 * @param  None
 * @retval None
 */
static void TC_Application_Process(void)
{
  switch(BLE_Profile_Read_DeviceState())
  { 
  case APPL_INIT_DONE:
    {
      if(TC_Start_Timer == 1)
      {
        /**
    	 * Make sure to stop the timer only when it has been started at least once
    	 * The Timer ID must be existing to call TIMER_Stop();
    	 */
    	TIMER_Stop(profileApplContext.profileTimer_Id);
    	TC_Start_Timer = 0;
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
      if (TC_Start_Timer == 0)
      {
        TIMER_Start(profileApplContext.profileTimer_Id, (uint16_t)(TC_TIMER_VALUE/TIMERSERVER_TICK_VALUE));
        TC_Start_Timer = 1;
      }
    }
    break;
  case APPL_GET_REF_TIME_UPDATE:	
    {
      TimeClient_Update_Reference_Time_On_Server(0x01);
    }	
    break;
  case APPL_CANCEL_REF_TIME_UPDATE:	
    {
      TimeClient_Update_Reference_Time_On_Server(0x02);
    }	
    break;
  case APPL_TIME_UPDATE_NOTIFICATION:
    {
      TimeClient_SetReset_Time_Update_Notification(0x01);
    }	
    break;
  case APPL_GET_SERV_TIME_UPDATE_STATE: 
    {
      TimeClient_Get_Server_Time_Update_State();
    }
    break;
  case APPL_READ_NEXT_DST_CHANGE_TIME: 		 
    {
      TimeClient_Get_Next_DST_Change_Time();
    }
    break;
  case APPL_READ_LOCAL_TIME_INFORM:
    {
      TimeClient_Get_Local_Time_Information();
    }
    break;
  case APPL_READ_CURRENT_TIME:
    {
      TimeClient_Get_Current_Time();
    }
    break;
  case APPL_GET_TIME_ACCU_INFO_SERVER:	
    {
      TimeClient_Get_Time_Accuracy_Info_Of_Server();
    }
    break;
  default:
    break;
  }
  return;
}

static void TC_Process_Time(void)
{
  return;
}

#endif  /* (BLE_CURRENT_PROFILE_ROLES & TIME_CLIENT) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
