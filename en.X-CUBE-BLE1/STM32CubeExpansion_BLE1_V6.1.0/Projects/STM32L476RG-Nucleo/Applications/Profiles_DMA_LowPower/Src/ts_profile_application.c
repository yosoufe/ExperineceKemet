/**
******************************************************************************
* @file    ts_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    14-October-2014
* @brief   This file implements the main function executing the Time Server
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
#include <time_server.h>
#include <time_profile_types.h>

#include "hci_tl_interface.h"

#if (BLE_CURRENT_PROFILE_ROLES & TIME_SERVER)
/*******************************************************************************
 * Macros
*******************************************************************************/
#define TS_TIMER_VALUE	1000000 /**< Value in us */

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
tCurrentTime time;
int TS_Start_Timer = 0;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */

/******************************************************************************
 * Function Declarations
******************************************************************************/
void TSProfileGetAddress(uint8_t* addr);
void Init_Current_Time(void);
void Profile_Appl_Evt_Notify(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);
void Init_Profile(void);
void Advertize(void);
static void TS_Application_Process(void);
static void TS_Process_Time(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/

/**
 * @brief  Initialize the Application Context for using the Time Server
 *         profile methods
 * @param  None
 * @retval None
 */
void TS_Appl_Context_Init(void)
{
  if (profileApplContext.profileGetAddressFunc == NULL)
  {
    profileApplContext.profileGetAddressFunc = TSProfileGetAddress;
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
    profileApplContext.profileStateMachineFunc = TimeServer_StateMachine;
  }
  
  if (profileApplContext.profileApplicationProcessFunc == NULL)
  {
    profileApplContext.profileApplicationProcessFunc = TS_Application_Process;
  }
  
  if (profileApplContext.profileProcessMeasurementFunc == NULL)
  {
    profileApplContext.profileProcessMeasurementFunc = TS_Process_Time;
  }
  
  Init_Current_Time();
}

/**
 * @brief This function sets TS specific address
 * @param The profile address
* @retval None
 */
void TSProfileGetAddress(uint8_t* addr)
{
  uint8_t PROFILE_ADDR[] = {0x12, 0x34, 0x10, 0xE1, 0x80, 0x02};
  
  BLUENRG_memcpy(addr, PROFILE_ADDR, sizeof(PROFILE_ADDR));
}

/** 
* @brief  Initialize the current time.
* @param  None
* @retval None
*/
void Init_Current_Time(void)
{
  time.date = 14;
  time.day_of_week = WEDNESDAY;
  time.hours = 1;
  time.minutes = 1;
  time.seconds = 4;
  
  /* NOTES: Values to be set for running PTS tests CTS, CSP */
  time.adjustReason = 0x01; /* ADJUST_REASON_MANUAL_TIME_UPDATE: TP/CSP/BV-02-C */
  //time.adjustReason = 0x04; /* ADJUST_REASON_CHANGE_OF_TIME_ZONE: TP/CSP/BV-03-C */
  //time.adjustReason = 0x08; /* ADJUST_REASON_CHANGE_OF_DST:     TP/CSP/BV-04-C */
  //time.adjustReason = 0x02; /* ADJUST_REASON_EXTERNAL_REFERENCE_TIME_UPDATE: TP/CSP/BV-05-C, TP/CSP/BV-06-C*/
  
  time.fractions256 = 0x00;
  time.month = 10;
  time.year = 2015;

}

/**
 * @brief  Update time before sending.
 * @param  Update interval (in sec)
 * @retval None
 */
void Update_Current_Time(uint8_t time_interval)
{
  time.seconds += time_interval;
  if (time.seconds >= 60)
  {
    time.seconds = time.seconds % 60;
    time.minutes++;
  }
  if (time.minutes == 60)
  {
    time.minutes = 0;
    time.hours++;
  }  
}

/**
 * @brief  Profile Application Event Notify.
 * @param  Event
 * @param  Event length
 * @param  Event data
 * @retval None
 */
void Profile_Appl_Evt_Notify (tNotificationEvent event,uint8_t evtLen,uint8_t* evtData)
{  
  switch(event)
  {
  case EVT_TS_INITIALIZED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_TS_INITIALIZED\n");
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
    }
    break;
  case EVT_TS_START_REFTIME_UPDATE:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_TS_START_REFTIME_UPDATE\n");
      TimeServer_Update_Current_Time_Value(time);
    }
    break;
  case EVT_TS_CHAR_UPDATE_CMPLT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_TS_CHAR_UPDATE_CMPLT\n");
      BLE_Profile_Write_DeviceState(APPL_CONNECTED);
    }
    break;
  case EVT_TS_CURTIME_READ_REQ:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_TS_CURTIME_READ_REQ\n");
      Allow_Curtime_Char_Read();
    }
    break;
  }
  
}/* end Profile_Appl_Evt_Notify() */


/**
 * @brief  This function initializes the Time Server Profile by calling the
 *         function TimeServer_Init().
 * @param  None
 * @retval None
 */
void Init_Profile(void)
{
  PRINTF ("Test Application: Initializing Find me profile\n");
  if (TimeServer_Init (NEXT_DST_CHANGE_SERVICE_BITMASK|REFERENCE_TIME_UPDATE_SERVICE_BITMASK, 
                       BLE_Profiles_Evt_Notify_Cb) == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Initialized Time Server Profile \n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Failed Time Server Profile \n" );
  }
  
}/* end Init_Profile() */


/**
 * @brief  This function enables advertising.
 * @param  None
 * @retval None
 */
void Advertize(void)
{  
  APPL_MESG_DBG(profiledbgfile,"enable advertising\n");
  TimeServer_Make_Discoverable(0x00);
}/* end Advertize() */


/**
 * @brief  Application specific processing
 * @param  None
 * @retval None
 */
static void TS_Application_Process(void)
{
  switch(BLE_Profile_Read_DeviceState())
  { 
  case APPL_INIT_DONE:
    {
      if(TS_Start_Timer == 1)
      {
        /**
    	 * Make sure to stop the timer only when it has been started at least once
    	 * The Timer ID must be existing to call TIMER_Stop();
    	 */
    	TIMER_Stop(profileApplContext.profileTimer_Id);
    	TS_Start_Timer = 0;
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
      if (TS_Start_Timer == 0)
      {
        TIMER_Start(profileApplContext.profileTimer_Id, (uint16_t)(TS_TIMER_VALUE/TIMERSERVER_TICK_VALUE));
        TS_Start_Timer = 1;
      }
    }
    break;
  default:      
    break;
  }
  return;
}

static void TS_Process_Time(void)
{
  uint8_t status;
  
  Update_Current_Time((uint8_t)(UPDATE_INTERVAL/1000));

  status = TimeServer_Update_Current_Time_Value(time);
  APPL_MESG_DBG(profiledbgfile,"updating current time status is %d\n", status);  
}

#endif  /* (BLE_CURRENT_PROFILE_ROLES & TIME_SERVER) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
