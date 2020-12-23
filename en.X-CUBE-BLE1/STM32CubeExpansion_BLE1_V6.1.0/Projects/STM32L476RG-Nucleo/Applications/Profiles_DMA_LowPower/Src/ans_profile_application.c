/**
******************************************************************************
* @file    ans_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    06-October-2014
* @brief   This file implements the main function executing the Alert
*          Notification Server profile.
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
#include <alertNotification_Server.h>

#include "hci_tl_interface.h"

#if (BLE_CURRENT_PROFILE_ROLES & ALERT_NOTIFICATION_SERVER)
/*******************************************************************************
 * Macros
*******************************************************************************/
#define APPL_UPDATE_NEW_ALERT
//#define APPL_UPDATE_UNREAD_ALERT_STATUS
#define ANS_TIMER_VALUE	 1000000 /**< Value in us */

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
int ANS_Start_Timer = 0;

tTextInfo textInfo;
uint8_t alert_count = 0x00;
uint8_t str[4] = {'J','O','H','N'}; //{0x4A,0x4F,0x48,0x4E}

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */
extern volatile uint8_t send_measurement;

/******************************************************************************
 * Function Declarations
******************************************************************************/
void ANSProfileGetAddress(uint8_t* addr);
void Init_Mail_Alert_Text(void);
void Profile_Appl_Evt_Notify(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);
void Init_Profile(void);
void Advertize(void);
static void ANS_Application_Process(void);
static void ANS_Process_Notification(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/

/**
 * @brief  Initialize the Application Context for using the Alert Notification
 *         Server profile methods
 * @param  None
 * @retval None
 */
void ANS_Appl_Context_Init(void)
{
  if (profileApplContext.profileGetAddressFunc == NULL)
  {
    profileApplContext.profileGetAddressFunc = ANSProfileGetAddress;
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
    profileApplContext.profileStateMachineFunc = ANSProfile_StateMachine;
  }
  
  if (profileApplContext.profileApplicationProcessFunc == NULL)
  {
    profileApplContext.profileApplicationProcessFunc = ANS_Application_Process;
  }
  
  if (profileApplContext.profileProcessMeasurementFunc == NULL)
  {
    profileApplContext.profileProcessMeasurementFunc = ANS_Process_Notification;
  }
  
  Init_Mail_Alert_Text();
}

/**
 * @brief This function sets Alert Notification Server specific address
 * @param The profile address
 * @retval None
 */
void ANSProfileGetAddress(uint8_t* addr)
{
  uint8_t PROFILE_ADDR[] = {0x12, 0x34, 0x07, 0xE1, 0x80, 0x02};
  
  BLUENRG_memcpy(addr, PROFILE_ADDR, sizeof(PROFILE_ADDR));
}

/**
 * @brief  Initialize the Mail Alert Text
 * @param  None
 * @retval None
 */
void Init_Mail_Alert_Text(void)
{
  BLUENRG_memset((void*)&textInfo, 0, sizeof(textInfo));
  
  textInfo.len = sizeof(str);
  textInfo.str = str;
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
  case EVT_ANS_INITIALIZED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_ANS_INITIALIZED %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
    }
    break;
  }
  
}/* end Profile_Appl_Evt_Notify() */


/**
 * @brief This function initializes the Alert Notification Server Profile by calling the
 *        function ANS_Init().
 * @param None
 * @retval None
 */
void Init_Profile(void)
{
  PRINTF ("Test Application: Initializing Alert Notification Service profile\n");
  uint8_t alertCategory[2] = {0x02,0x00}; //Just Category Email enabled
  uint8_t unreadAlertCategory[2] = {0xFF,0x00};
  
  /* Configure the User Button in EXTI Mode */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

  if (ANS_Init(alertCategory,unreadAlertCategory,BLE_Profiles_Evt_Notify_Cb) == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Initialized Alert Notification Server Profile \n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Failed Alert Notification Server Profile \n" );
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
  ANS_Advertize(0x00);
}/* end Advertize() */


/**
 * @brief  Application specific processing
 * @param  None
 * @retval None
 */
static void ANS_Application_Process(void)
{
  switch(BLE_Profile_Read_DeviceState())
  {
  // The following 'case' must be enabled for debug purpose only
  //case APPL_UNINITIALIZED:
    //break;
  case APPL_INIT_DONE:
    {
      if(ANS_Start_Timer == 1)
      {
        /**
    	 * Make sure to stop the timer only when it has been started at least once
    	 * The Timer ID must be existing to call TIMER_Stop();
    	 */
    	TIMER_Stop(profileApplContext.profileTimer_Id);
    	ANS_Start_Timer = 0;
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
      if (ANS_Start_Timer == 0)
      {
        TIMER_Start(profileApplContext.profileTimer_Id, (uint16_t)(ANS_TIMER_VALUE/TIMERSERVER_TICK_VALUE));
        ANS_Start_Timer = 1;
      }
    }
    break;
  default:      
    break;
  }
  return;
}

static void ANS_Process_Notification(void)
{
  int32_t uwPRIMASK_Bit;
  
  if (send_measurement != 0)
  {
    uwPRIMASK_Bit = __get_PRIMASK();	/**< backup PRIMASK bit */
    __disable_irq(); /**< Disable all interrupts by setting PRIMASK bit on Cortex*/
    send_measurement = 0;
    __set_PRIMASK(uwPRIMASK_Bit);	/**< Restore PRIMASK bit*/
    
#if defined(APPL_UPDATE_NEW_ALERT)
    alert_count++;
	ANS_Update_New_Alert(CATEGORY_ID_EMAIL,alert_count,textInfo);
#elif defined(APPL_UPDATE_UNREAD_ALERT_STATUS)
    ANS_Update_Unread_Alert_Status(CATEGORY_ID_SMS_MMS,0x01);
#endif
  }
}

#endif  /* (BLE_CURRENT_PROFILE_ROLES & ALERT_NOTIFICATION_SERVER) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
