/**
******************************************************************************
* @file    gs_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    22-September-2014
* @brief   This file implements the main function executing the Glucose Sensor
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
#include "profile_application.h"
/* Include record measurement database record */
#include <glucose_database.h>

#include "hci_tl_interface.h"

#if (BLE_CURRENT_PROFILE_ROLES & GLUCOSE_SENSOR)
/*******************************************************************************
 * Macros
*******************************************************************************/
#define CHANGE_DB_1 1
#define GLUCOSE_SENSOR_TIMER_VALUE	1000000 /**< Value in us */

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
tGlucoseMeasurementVal glucoseMeasurementVal;
int GL_Start_Timer = 0;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */

/******************************************************************************
 * Function Declarations
******************************************************************************/
void GLProfileGetAddress(uint8_t* addr);
void Init_glucose(void);
void Profile_Appl_Evt_Notify(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);
void Init_Profile(void);
void Advertize(void);
static void GL_Application_Process(void);
static void GL_Process_Measurement(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/

/**
 * @brief  Initialize the Application Context for using the Glucose Sensor
 *         profile methods
 * @param  None
 * @retval None
 */
void GL_Appl_Context_Init(void)
{
  if (profileApplContext.profileGetAddressFunc == NULL)
  {
    profileApplContext.profileGetAddressFunc = GLProfileGetAddress;
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
    profileApplContext.profileStateMachineFunc = GL_StateMachine;
  }
  
  if (profileApplContext.profileApplicationProcessFunc == NULL)
  {
    profileApplContext.profileApplicationProcessFunc = GL_Application_Process;
  }
  
  if (profileApplContext.profileProcessMeasurementFunc == NULL)
  {
    profileApplContext.profileProcessMeasurementFunc = GL_Process_Measurement;
  }
  
  Init_glucose();  
}

/**
 * @brief This function sets Glucose specific address
 * @param The profile address
 * @retval None
 */
void GLProfileGetAddress(uint8_t* addr)
{
  uint8_t PROFILE_ADDR[] = {0x12, 0x34, 0x03, 0xE1, 0x80, 0x02};
  
  BLUENRG_memcpy(addr, PROFILE_ADDR, sizeof(PROFILE_ADDR));
}

/**
 * @brief  This function initializes Glucose Sensor specific parameters to
 *         default values.
 * @param  None
 * @retval None
 */
void Init_glucose(void)
{
  BLUENRG_memset((void*)&glucoseMeasurementVal, 0, sizeof(glucoseMeasurementVal));
    
  glucoseMeasurementVal.flags = GLUCOSE_MEASUREMENT_CONTEXT_FLAG_ALL_PRESENT;
  glucoseMeasurementVal.sequenceNumber = 1;
  glucoseMeasurementVal.baseTime.year = 2015;
  glucoseMeasurementVal.baseTime.month = 10;
  glucoseMeasurementVal.baseTime.day = 26;
  glucoseMeasurementVal.baseTime.hours = 19;
  glucoseMeasurementVal.baseTime.minutes = 42;
  glucoseMeasurementVal.baseTime.seconds = 50;
  glucoseMeasurementVal.timeOffset = 16;
  glucoseMeasurementVal.typeSampleLocation = GLUCOSE_SAMPLE_LOCATION_FINGER;
  glucoseMeasurementVal.glucoseConcentration = 150;
  glucoseMeasurementVal.sensorStatusAnnunciation = GLUCOSE_SENSOR_STATUS_ANNUNCIATION_DEVICE_BATTERY_LOW;
}

/**
 * @brief  Update parameters before sending them.
 * @param  Update interval (in sec)
 * @retval None
 */
void Update_glucose(uint8_t time_interval)
{
  glucoseMeasurementVal.sequenceNumber++;
  glucoseMeasurementVal.baseTime.seconds += time_interval;
  if (glucoseMeasurementVal.baseTime.seconds >= 60)
  {
    glucoseMeasurementVal.baseTime.seconds = glucoseMeasurementVal.baseTime.seconds % 60;
    glucoseMeasurementVal.baseTime.minutes++;
  }
  if (glucoseMeasurementVal.baseTime.minutes == 60)
  {
    glucoseMeasurementVal.baseTime.minutes = 0;
    glucoseMeasurementVal.baseTime.hours++;
  }
  if (glucoseMeasurementVal.baseTime.hours == 23)
  {
    glucoseMeasurementVal.baseTime.hours = 0;
    glucoseMeasurementVal.baseTime.day++;
  }  
  glucoseMeasurementVal.glucoseConcentration++;
  if (glucoseMeasurementVal.glucoseConcentration > 204)
  {
    glucoseMeasurementVal.glucoseConcentration = 150;
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
  case EVT_GL_INITIALIZED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_GL_INITIALIZED %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
      APPL_MESG_DBG(profiledbgfile,"EVT_GL_INITIALIZED  AND DEVICE STATE IS %d\n",BLE_Profile_Read_DeviceState());
    }
    break;
  }
  
}/* end Profile_Appl_Evt_Notify() */


/**
 * @brief  This function initializes the Glucose Sensor Profile by calling the
 *         function GLProfile_Init().
 * @param  None
 * @retval None
 */
void Init_Profile(void)
{
  PRINTF ("Test Application: Initializing Glucose Sensor profile\n");
  
  if (GL_Init(GLUCOSE_NUMBER_OF_STORED_RECORDS, &glucoseDatabase_MeasurementRecords[0], &glucoseDataBase_MeasurementContextRecords[0], BLE_Profiles_Evt_Notify_Cb) == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Initialized Glucose Sensor Profile \n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Failed Glucose Sensor Profile \n" );
  }
  
}/* end Init_Profile() */


/**
 * @brief  This function enables advertising.
 * @param  None
 * @retval None
 */
void Advertize(void)
{
  APPL_MESG_DBG(profiledbgfile,"glucose enable advertising\n");
  
  /* Set Glucose Feature characteristic value: all features except multi bond */
  GL_Set_Glucose_Feature_Value(GLUCOSE_FEATURE_ALL_SUPPORTED_NO_MULTI_BOND);
  
  /* Start advertising (no whitelist, fast) */
  GL_Advertize(0x00);

}/* end Advertize() */


/**
 * @brief  Application specific processing
 * @param  None
 * @retval None
 */
static void GL_Application_Process(void)
{
  switch(BLE_Profile_Read_DeviceState())
  {  
  case APPL_INIT_DONE:
    {
      if(GL_Start_Timer == 1)
      {
        /**
    	 * Make sure to stop the timer only when it has been started at least once
    	 * The Timer ID must be existing to call TIMER_Stop();
    	 */
    	TIMER_Stop(profileApplContext.profileTimer_Id);
    	GL_Start_Timer = 0;
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
      if (GL_Start_Timer == 0)
      {
        TIMER_Start(profileApplContext.profileTimer_Id, (uint16_t)(GLUCOSE_SENSOR_TIMER_VALUE/TIMERSERVER_TICK_VALUE));
        GL_Start_Timer = 1;
      }
    }
    break;    
  default:      
    break;
    
  }
  return;
}

static void GL_Process_Measurement(void)
{
  Update_glucose((uint8_t)(UPDATE_INTERVAL/1000));

#if CHANGE_DB_1
  /* Change database manually: store new sequence number and reset flags field on 
     current database measurement for simulating new records measurements */
  glucoseDatabase_Change_Database_Manually(1, SET_FLAG);
#else
  /* Change database manually: store new sequence number and reset flags field on 
     current database measurement for simulating new records measurements */
  glucoseDatabase_Change_Database_Manually(3, UNSET_FLAG);
#endif

 GL_Send_Glucose_Measurement(glucoseMeasurementVal);      
}

#endif     /* (BLE_CURRENT_PROFILE_ROLES & GLUCOSE_SENSOR) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
