/**
******************************************************************************
* @file    fml_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    10-October-2014
* @brief   This file implements the main function executing the Find Me
*          Locator profile.
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
#include <findme_locator.h>

#include "hci_tl_interface.h"

#if (BLE_CURRENT_PROFILE_ROLES & FIND_ME_LOCATOR)
/*******************************************************************************
 * Macros
*******************************************************************************/
#define FML_TIMER_VALUE	 1000000 /**< Value in us */

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
int FML_Start_Timer = 0;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */
extern tSecurityParams Security_Param;

/******************************************************************************
 * Function Declarations
******************************************************************************/
void FMLProfileGetAddress(uint8_t* addr);
void FML_Set_Alert_level(uint8_t level);
void Profile_Appl_Evt_Notify(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);
void Init_Profile(void);
void Advertize(void);
static int FML_Application_Process(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/

/**
 * @brief  Initialize the Application Context for using the Fidn Me Locator
 *         profile methods
 * @param  None
 * @retval None
 */
void FML_Appl_Context_Init(void)
{
  if (profileApplContext.profileGetAddressFunc == NULL)
  {
    profileApplContext.profileGetAddressFunc = FMLProfileGetAddress;
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
    profileApplContext.profileStateMachineFunc = FMLProfile_StateMachine;
  }
  
  if (profileApplContext.profileApplicationProcessFunc == NULL)
  {
    profileApplContext.profileApplicationProcessFunc = FML_Application_Process;
  }
  
}

/**
 * @brief This function sets Find Me Locator specific address
 * @param The profile address
 * @retval None
 */
void FMLProfileGetAddress(uint8_t* addr)
{
  uint8_t PROFILE_ADDR[] = {0x12, 0x34, 0x06, 0xE1, 0x80, 0x02};
  
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
  case EVT_FML_DISCOVERY_CMPLT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_FML_DISCOVERY_CMPLT : reason %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_LOCATOR_ALERT_TARGET);
    }
    break;
  case EVT_FML_INITIALIZED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_FML_INITIALIZED\n");
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
    }
    break;
  }
  
}/* end Profile_Appl_Evt_Notify() */


/**
 * @brief  This function initializes the Find Me Locator Profile by calling the
 *         function FindMeLocator_Init().
 * @param  None
 * @retval None
 */
void Init_Profile()
{
  PRINTF ("Test Application: Initializing Find me profile\n");
  
  /* LED2 is used for signaling the presence of the target */
  /* It is used instead of an acustic alert not present on the board */
  BSP_LED_Init(LED2);

  if (FindMeLocator_Init(&Security_Param,BLE_Profiles_Evt_Notify_Cb) == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Initialized Find me Profile \n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Failed Find me Profile \n" );
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
  FML_Advertize();
}/* end Advertize() */


/**
 * @brief  Application specific processing.
 * @param  None
 * @retval int
 */
static int FML_Application_Process(void)
{
  int ret_val = 1;
  
  BSP_LED_Off(LED2);
  
  switch(BLE_Profile_Read_DeviceState())
  {
  case APPL_UNINITIALIZED:
    break;
  case APPL_INIT_DONE:
    {
      if(FML_Start_Timer == 1)
      {
        /**
    	 * Make sure to stop the timer only when it has been started at least once
    	 * The Timer ID must be existing to call TIMER_Stop();
    	 */
    	TIMER_Stop(profileApplContext.profileTimer_Id);
    	FML_Start_Timer = 0;
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
      if (FML_Start_Timer == 0)
      {
        TIMER_Start(profileApplContext.profileTimer_Id, (uint16_t)(FML_TIMER_VALUE/TIMERSERVER_TICK_VALUE));
        FML_Start_Timer = 1;
      }
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  case APPL_LOCATOR_ALERT_TARGET:
    {
      BSP_LED_On(LED2); //FIXME The API to be called is: FML_ALert_Target(0x00);
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  default:
    ret_val = 0;
    break;
  }
  return ret_val;
}

#endif  /* (BLE_CURRENT_PROFILE_ROLES & FIND_ME_LOCATOR) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
