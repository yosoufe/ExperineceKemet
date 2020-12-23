/**
******************************************************************************
* @file    fmt_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    02-October-2014
* @brief   This file implements the main function executing the Find Me Target
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
#include "profile_application.h"
#include <findme_target.h>
#include <findme_locator.h>

#include "hci_tl_interface.h"

#if (BLE_CURRENT_PROFILE_ROLES & FIND_ME_TARGET)
/*******************************************************************************
 * Macros
*******************************************************************************/
#define FMT_TIMER_VALUE	 1000000 /**< Value in us */
#define MILD_ALERT_BLINK 6
#define HIGH_ALERT_BLINK 2
#define REPEAT_BLINK     9

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
int FMT_Start_Timer = 0;
uint8_t  blinkTimer_Id;
uint32_t blink_delay;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */
volatile uint8_t alert_level;

/******************************************************************************
 * Function Declarations
******************************************************************************/
void FMTProfileGetAddress(uint8_t* addr);
void FMT_Set_Alert_Level(uint8_t level);
void FMT_Set_Alert_Blink(uint8_t level);
void Blink_Handler(void);
tBleStatus Blink_Timer_Start(uint32_t expiryTime,
                             TIMER_TIMEOUT_NOTIFY_CALLBACK_TYPE timercb,
                             tTimerID *timerID);
void Profile_Appl_Evt_Notify(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);
void Init_Profile(void);
void Advertize(void);
static void FMT_Application_Process(void);
static void FMT_Process_Alert(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/

/**
 * @brief  Initialize the Application Context for using the Find Me Target
 *         profile methods
 * @param  None
 * @retval None
 */
void FMT_Appl_Context_Init(void)
{
  if (profileApplContext.profileGetAddressFunc == NULL)
  {
    profileApplContext.profileGetAddressFunc = FMTProfileGetAddress;
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
    profileApplContext.profileStateMachineFunc = FMTProfile_StateMachine;
  }
  
  if (profileApplContext.profileApplicationProcessFunc == NULL)
  {
    profileApplContext.profileApplicationProcessFunc = FMT_Application_Process;
  }
  
  if (profileApplContext.profileProcessMeasurementFunc == NULL)
  {
    profileApplContext.profileProcessMeasurementFunc = FMT_Process_Alert;
  }
  
}

/**
 * @brief This function sets Find Me Target specific address
 * @param The profile address
 * @retval None
 */
void FMTProfileGetAddress(uint8_t* addr)
{
  uint8_t PROFILE_ADDR[] = {0x12, 0x34, 0x05, 0xE1, 0x80, 0x02};
  
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
  case EVT_FMT_INITIALIZED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_FMT_INITIALIZED %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
    }
    break;
  case EVT_FMT_ALERT:
    {
      APPL_MESG_DBG("EVT_FMT_ALERT %x\n",evtData[0]);
      FMT_Set_Alert_Level(evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_FMT_ALERT_LEVEL);
    }
    break;
  }
  
}/* end Profile_Appl_Evt_Notify() */


/**
 * @brief  This function initializes the Find Me Target Profile by calling the
 *         function FindMeTarget_Init().
 * @param  None
 * @retval None
 */
void Init_Profile(void)
{
  PRINTF ("Test Application: Initializing Find me profile\n");
  
  /* LED2 is used for signaling the presence of the target */
  /* It is used instead of an acustic alert not present on the board */
  BSP_LED_Init(LED2);
  
  if (FindMeTarget_Init(BLE_Profiles_Evt_Notify_Cb) == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Initialized Find me Profile \n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Failed Find me Profile \n" );
  }
  
}/* end Init_Profile() */


/**
 * @brief This function enables advertising.
 * @param None
 * @param None
 */
void Advertize(void)
{
  APPL_MESG_DBG(profiledbgfile,"Test Application: advertize\n");
  FMT_Advertize();
}/* end Advertize() */


/**
 * @brief  Application specific processing
 * @param  None
 * @retval None
 */
static void FMT_Application_Process(void)
{
  switch(BLE_Profile_Read_DeviceState())
  { 
  case APPL_INIT_DONE:
    {
      if(FMT_Start_Timer == 1)
      {
        /**
    	 * Make sure to stop the timer only when it has been started at least once
    	 * The Timer ID must be existing to call TIMER_Stop();
    	 */
    	TIMER_Stop(profileApplContext.profileTimer_Id);
    	FMT_Start_Timer = 0;
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
      if (FMT_Start_Timer == 0)
      {
        TIMER_Start(profileApplContext.profileTimer_Id, (uint16_t)(FMT_TIMER_VALUE/TIMERSERVER_TICK_VALUE));
        FMT_Start_Timer = 1;
      }
    }
    break;
  case APPL_FMT_ALERT_LEVEL:
    {
      switch(alert_level)
      {
      case NO_ALERT:
        BSP_LED_Off(LED2);
        break;
      case MILD_ALERT:
      case HIGH_ALERT:
        FMT_Set_Alert_Blink(alert_level);//BSP_LED_On(LED2);
        break;
      }
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  default:
    break;
  }
  return;
}

void FMT_Set_Alert_Level(uint8_t level) {
  alert_level = level;
}

void FMT_Set_Alert_Blink(uint8_t level) {
  
  if (level == MILD_ALERT) blink_delay = MILD_ALERT_BLINK;
  if (level == HIGH_ALERT) blink_delay = HIGH_ALERT_BLINK;
  
  BSP_LED_Off(LED2);
  Blink_Timer_Start(blink_delay, Blink_Handler, &blinkTimer_Id);
  
  return;
}

void Blink_Handler(void)
{
  static uint8_t repeat = REPEAT_BLINK;
  
  Blue_NRG_Timer_Stop(blinkTimer_Id);
  
  if (repeat > 0) {
    BSP_LED_Toggle(LED2);
    Blink_Timer_Start(blink_delay, Blink_Handler, &blinkTimer_Id);    
    repeat--;
  }
  else {    
    repeat = REPEAT_BLINK;
  }
  return;
}

tBleStatus Blink_Timer_Start(uint32_t expiryTime,
                                TIMER_TIMEOUT_NOTIFY_CALLBACK_TYPE timercb,
                                tTimerID *timerID)
{
  TIMER_Create(eTimerModuleID_BlueNRG_Profile, timerID, eTimerMode_SingleShot, (pf_TIMER_TimerCallBack_t) timercb);
  TIMER_Start(*timerID, expiryTime*100000/TIMERSERVER_TICK_VALUE);
  return (BLE_STATUS_SUCCESS);
}

static void FMT_Process_Alert(void)
{
  return;
}

#endif  /* (BLE_CURRENT_PROFILE_ROLES & FIND_ME_TARGET) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
