/**
******************************************************************************
* @file    bps_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    23-September-2014
* @brief   This file implements the main function executing the Blood Pressure
*          Sensor profile.
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
#include "low_power_conf.h"
#include "stm32xx_timerserver.h"
#include "profile_application.h"
#include <blood_pressure.h>

#include "hci_tl_interface.h"

#if (BLE_CURRENT_PROFILE_ROLES & BLOOD_PRESSURE_SENSOR)
/*******************************************************************************
 * Macros
*******************************************************************************/
#define INTERMEDIATE_CUFF_PRESSURE 0
#define BLOOD_PRESSURE_TIMER_VALUE	1000000 /**< Value in us */

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
tBPMValue bpmval;
tICPValue icpval;
int BP_Start_Timer = 0;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */
extern volatile uint8_t send_measurement;

/******************************************************************************
 * Function Declarations
******************************************************************************/
void BPProfileGetAddress(uint8_t* addr);
void Init_bpm_values(void);
void Profile_Appl_Evt_Notify(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);
void Init_Profile(void);
void Advertize(void);
static int BP_Application_Process(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/

/**
 * @brief  Initialize the Application Context for using the Blood Pressure Sensor
 *         profile methods
 * @param  None
 * @retval None
 */
void BP_Appl_Context_Init(void)
{
  if (profileApplContext.profileGetAddressFunc == NULL)
  {
    profileApplContext.profileGetAddressFunc = BPProfileGetAddress;
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
    profileApplContext.profileStateMachineFunc = BPS_StateMachine;
  }
  
  if (profileApplContext.profileApplicationProcessFunc == NULL)
  {
    profileApplContext.profileApplicationProcessFunc = BP_Application_Process;
  }
    
  Init_bpm_values();
}

/**
 * @brief This function sets BPS specific address
 * @param The profile address
 * @retval None
 */
void BPProfileGetAddress(uint8_t* addr)
{
  uint8_t PROFILE_ADDR[] = {0x12, 0x34, 0x04, 0xE1, 0x80, 0x02};
  
  BLUENRG_memcpy(addr, PROFILE_ADDR, sizeof(PROFILE_ADDR));
}

/**
 * @brief  This function initializes BPS specific parameters to default values
 * @param  None
 * @retval None
 */
void Init_bpm_values(void)
{
  BLUENRG_memset((void*)&icpval, 0, sizeof(icpval));
  
#if INTERMEDIATE_CUFF_PRESSURE
  icpval.flags = (FLAG_USER_ID_PRESENT|FLAG_TIME_STAMP_PRESENT|
                  FLAG_PULSE_RATE_PRESENT|FLAG_MEASUREMENT_STATUS_PRESENT);
  icpval.icp = 0xAAAA;
  icpval.userID = 0x01;
  icpval.measurementStatus.bodyMovementDetectionFlag = 0x01;
  icpval.measurementStatus.cuffFitDetectionFlag = 0x01;
  icpval.measurementStatus.measurementPositionDetectionFlag = 0x01;  
#else
  bpmval.flags = (FLAG_TIME_STAMP_PRESENT|FLAG_PULSE_RATE_PRESENT|
                  FLAG_USER_ID_PRESENT|FLAG_MEASUREMENT_STATUS_PRESENT);
  bpmval.systolic = 0x0072;//0xAAAA;
  bpmval.diastolic = 0x0072;
  bpmval.map = 0x0072;
  bpmval.pulseRate = 80;
  bpmval.userID = 0x01;
  bpmval.timeStamp.year = 2015;
  bpmval.timeStamp.month = 10;
  bpmval.timeStamp.day = 26;
  bpmval.timeStamp.hours = 19;
  bpmval.timeStamp.minutes = 42;
  bpmval.timeStamp.seconds = 50;
  bpmval.measurementStatus.bodyMovementDetectionFlag = 0x01;
  bpmval.measurementStatus.cuffFitDetectionFlag = 0x01;
  bpmval.measurementStatus.measurementPositionDetectionFlag = 0x01;
#endif
}

/**
 * @brief  Update time before sending.
 * @param  Update interval (in sec)
 * @retval None
 */
void Update_Time(uint8_t time_interval)
{
  bpmval.pulseRate++;
  if (bpmval.pulseRate > 90)
  {
    bpmval.pulseRate = 80;
  }
        
  bpmval.timeStamp.seconds += time_interval;
  if (bpmval.timeStamp.seconds >= 60)
  {
    bpmval.timeStamp.seconds = bpmval.timeStamp.seconds % 60;
    bpmval.timeStamp.minutes++;
  }
  if (bpmval.timeStamp.minutes == 60)
  {
    bpmval.timeStamp.minutes = 0;
    bpmval.timeStamp.hours++;
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
  case EVT_BPS_INITIALIZED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_BPS_INITIALIZED %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
    }
    break;
  case EVT_BPS_BPM_CHAR_UPDATE_CMPLT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_BPS_BPM_CHAR_UPDATE_CMPLT %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_CONNECTED);
    }
    break;
  case EVT_BPS_ICP_CHAR_UPDATE_CMPLT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_BPS_ICP_CHAR_UPDATE_CMPLT %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_CONNECTED);
    }
    break;
  }
  
}/* end Profile_Appl_Evt_Notify() */


/**
 * @brief  This function initializes the Blood Pressure Sensor Profile
 *         by calling the function BPS_Init().
 * @param  None
 * @retval None
 */
void Init_Profile()
{
  PRINTF ("Test Application: Initializing Blood Pressure Sensor Profile\n" );
  if (BPS_Init(0x01,
      BODY_MOVEMENT_DETECTION_SUPPORT_BIT|CUFF_FIT_DETECTION_SUPPORT_BIT|MEASUREMENT_POSITION_DETECTION_SUPPORT_BIT,
      BLE_Profiles_Evt_Notify_Cb) == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Initialized Blood Pressure Sensor Profile \n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Failed Health Thermometer Profile \n" );
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
  BPS_Advertize(0x00);

}/* end Advertize() */


/**
 * @brief  Application specific processing
 * @param  None
 * @retval int
 */
static int BP_Application_Process(void)
{
  int ret_val = 1;
  int32_t uwPRIMASK_Bit = __get_PRIMASK();	/**< backup PRIMASK bit */

  switch(BLE_Profile_Read_DeviceState())
  {
  case APPL_UNINITIALIZED:
    break;
  case APPL_INIT_DONE:
    {
      if(BP_Start_Timer == 1)
      {
        /**
    	 * Make sure to stop the timer only when it has been started at least once
    	 * The Timer ID must be existing to call TIMER_Stop();
    	 */
    	TIMER_Stop(profileApplContext.profileTimer_Id);
    	BP_Start_Timer = 0;
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
      if (BP_Start_Timer == 0)
      {
        TIMER_Start(profileApplContext.profileTimer_Id, (uint16_t)(BLOOD_PRESSURE_TIMER_VALUE/TIMERSERVER_TICK_VALUE));
        BP_Start_Timer = 1;
      }
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  default:
    
    if (send_measurement != 0) {
      
      __disable_irq();			/**< Disable all interrupts by setting PRIMASK bit on Cortex*/
      send_measurement = 0;
      __set_PRIMASK(uwPRIMASK_Bit);	/**< Restore PRIMASK bit*/
      BLE_Profile_Write_DeviceState(APPL_WAIT);
      
      Update_Time((uint8_t)(UPDATE_INTERVAL/1000));  
      
#if INTERMEDIATE_CUFF_PRESSURE
      APPL_MESG_DBG(profiledbgfile,"Sending intermediate cuff pressure measurment\n");        
      BPS_Send_Intermediate_Cuff_Pressure(icpval);
#else        
      APPL_MESG_DBG(profiledbgfile,"Sending blood pressure measurment\n");
      BPS_Send_Blood_Pressure_Measurement(bpmval);
#endif
      
    }else{
      ret_val = 0;
    }
    
    break;
    
  }
  return ret_val;
}

#endif   /* (BLE_CURRENT_PROFILE_ROLES & BLOOD_PRESSURE_SENSOR) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
