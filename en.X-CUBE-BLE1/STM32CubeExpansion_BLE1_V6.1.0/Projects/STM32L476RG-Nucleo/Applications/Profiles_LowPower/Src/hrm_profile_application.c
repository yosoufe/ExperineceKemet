/**
******************************************************************************
* @file    hrm_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    04-July-2014
* @brief   This file implements the main function executing the Heart Rate
*          Monitor profile.
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
#include <ble_profile.h>
#include <ble_events.h>
#include <timer.h>
#include "stm32xx_timerserver.h"
#include "profile_application.h"
#include <heart_rate.h>

#include "hci_tl_interface.h"
#include "bluenrg_aci.h"

#if (BLE_CURRENT_PROFILE_ROLES & HEART_RATE_SENSOR)
/*******************************************************************************
 * Macros
*******************************************************************************/
#define HEART_RATE_TIMER_VALUE	1000000 /**< Value in us */

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
tHeartRateMeasure heartRateMeasurmentValue;
int HR_Start_Timer = 0;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */
extern volatile uint8_t send_measurement;
extern uint8_t bnrg_expansion_board;

/******************************************************************************
 * Function Declarations
******************************************************************************/
void HRProfileGetAddress(uint8_t* addr);
void Init_heartRateMeas(void);
void Profile_Appl_Evt_Notify(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);
void Init_Profile(void);
void Advertize(void);
static int HR_Application_Process(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/

/**
 * @brief  Initialize the Application Context for using the HRM profile methods
 * @param  None
 * @retval None
 */
void HR_Appl_Context_Init(void)
{
  if (profileApplContext.profileGetAddressFunc == NULL)
  {
    profileApplContext.profileGetAddressFunc = HRProfileGetAddress;
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
    profileApplContext.profileStateMachineFunc = HRProfile_StateMachine;
  }
  
  if (profileApplContext.profileApplicationProcessFunc == NULL)
  {
    profileApplContext.profileApplicationProcessFunc = HR_Application_Process;
  }

  Init_heartRateMeas();
}

/**
 * @brief This function sets HR specific address
 * @param The profile address
 * @retval None
 */
void HRProfileGetAddress(uint8_t* addr)
{
  uint8_t PROFILE_ADDR[] = {0x12, 0x34, 0x01, 0xE1, 0x80, 0x02};
  
  BLUENRG_memcpy(addr, PROFILE_ADDR, sizeof(PROFILE_ADDR));
}

/**
 * @brief  This function initializes HRM specific parameters to default values
 * @param  None
 * @retval None
 */
void Init_heartRateMeas(void)
{
  int indx;
  
  BLUENRG_memset((void*)&heartRateMeasurmentValue, 0, sizeof(heartRateMeasurmentValue));
  
  heartRateMeasurmentValue.valueformat = HRM_VALUE_FORMAT_UINT8;
  heartRateMeasurmentValue.sensorContact = HRM_SENSOR_CONTACT_PRESENT;
  heartRateMeasurmentValue.energyExpendedStatus = HRM_ENERGY_EXPENDED_PRESENT;
  heartRateMeasurmentValue.rrIntervalStatus = HRM_RR_INTERVAL_PRESENT;
  heartRateMeasurmentValue.enrgyExpended = 0x00;
  heartRateMeasurmentValue.numOfRRIntervalvalues = 0x05;
  
  for (indx=0; indx<9; indx++) {
    heartRateMeasurmentValue.rrIntervalValues[indx] = (uint16_t)(indx + 0);
  }
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
  case EVT_HRS_INITIALIZED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_HRS_INITIALIZED\n");
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
    }
    break;
  case EVT_HRS_CHAR_UPDATE_CMPLT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_HRS_CHAR_UPDATE_CMPLT %x:%x:%x:%x:%x\n",evtData[0],evtData[1],evtData[2],evtData[3],evtData[4]);
    }
    break;
  case EVT_HRS_RESET_ENERGY_EXPENDED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_HRS_RESET_ENERGY_EXPENDED\n");
    }
    break;
  }
  
}/* end Profile_Appl_Evt_Notify() */


/**
 * @brief This function initializes the Heart Rate Profile by calling the
 *        function HRProfile_Init().
 * @param None
 * @retval None
 */
void Init_Profile(void)
{
  PRINTF ("Test Application: Initializing Heart Rate Profile \n" );
  
  if (HRProfile_Init((uint8_t)0xFF,
                     BLE_Profiles_Evt_Notify_Cb,
                     BODY_SENSOR_LOCATION_HAND) == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Initialized Heart Rate Profile \n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Failed Heart Rate Profile \n" );
  }
  
}/* end Init_Profile() */


/**
 * @brief This function enables advertising.
 * @param None
 * @retval None
 */
void Advertize(void)
{
  uint8_t useBoundedDeviceList = 0;

  APPL_MESG_DBG(profiledbgfile,"set sensor location\n");
  HRProfile_Set_Body_Sensor_Location(BODY_SENSOR_LOCATION_HAND);
  APPL_MESG_DBG(profiledbgfile,"enable advertising\n");
  HR_Sensor_Make_Discoverable(useBoundedDeviceList);

}/* end Advertize() */

/**
 * @brief  Application specific processing
 * @param  None
 * @retval int
 */
static int HR_Application_Process()
{
  int hr;
  int ret_val = 1;
  int32_t uwPRIMASK_Bit = __get_PRIMASK();	/**< backup PRIMASK bit */

  switch(BLE_Profile_Read_DeviceState())
  { 
  case APPL_UNINITIALIZED:
    break;
  case APPL_INIT_DONE:
    {
      if(HR_Start_Timer == 1)
      {
        /**
         * Make sure to stop the timer only when it has been started at least once
         * The Timer ID must be existing to call TIMER_Stop();
         */
        TIMER_Stop(profileApplContext.profileTimer_Id);
        HR_Start_Timer = 0;
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
      if (HR_Start_Timer == 0)
      {
        TIMER_Start(profileApplContext.profileTimer_Id, (uint16_t)(HEART_RATE_TIMER_VALUE/TIMERSERVER_TICK_VALUE));
        HR_Start_Timer = 1;
      }
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  default:
      if (send_measurement != 0)
        {
          hr = (rand()&0x1F)+60;

          __disable_irq(); /**< Disable all interrupts by setting PRIMASK bit on Cortex*/
          send_measurement = 0;
          __set_PRIMASK(uwPRIMASK_Bit);	/**< Restore PRIMASK bit*/
          BLE_Profile_Write_DeviceState(APPL_WAIT);

          /* Just update the HR field value before sending */
          heartRateMeasurmentValue.heartRateValue = hr;

          HRProfile_Send_HRM_Value (heartRateMeasurmentValue);

        }
        else{
          ret_val = 0;
        }
    break;

  }
  return ret_val;
}

#endif   /* (BLE_CURRENT_PROFILE_ROLES & HEART_RATE_SENSOR) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
