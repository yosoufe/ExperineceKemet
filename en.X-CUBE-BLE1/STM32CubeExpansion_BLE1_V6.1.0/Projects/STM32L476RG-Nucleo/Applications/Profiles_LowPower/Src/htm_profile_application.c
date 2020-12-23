/**
******************************************************************************
* @file    htm_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    22-September-2014
* @brief   This file implements the main function executing the Health
*          Termometer Monitor profile.
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
#include <health_thermometer.h>

#include "hci_tl_interface.h"

#if (BLE_CURRENT_PROFILE_ROLES & HEALTH_THERMOMETER)
/*******************************************************************************
 * Macros
*******************************************************************************/
#define INTERMEDIATE_TEMP 0
#define HEALTH_THERMOMETER_TIMER_VALUE	1000000 /**< Value in us */

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
tTempMeasurementVal tempMeasurementVal;
int HT_Start_Timer = 0;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */
extern volatile uint8_t send_measurement;
static uint8_t HTUM_sent = 0;

/******************************************************************************
 * Function Declarations
******************************************************************************/
void HTProfileGetAddress(uint8_t* addr);
void Init_temperature(void);
void Profile_Appl_Evt_Notify(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);
void Init_Profile(void);
void Advertize(void);
static int HT_Application_Process(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/

/**
 * @brief  Initialize the Application Context for using the HTM profile methods
 * @param  None
 * @retval None
 */
void HT_Appl_Context_Init(void)
{
  if (profileApplContext.profileGetAddressFunc == NULL)
  {
    profileApplContext.profileGetAddressFunc = HTProfileGetAddress;
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
    profileApplContext.profileStateMachineFunc = HT_StateMachine;
  }
  
  if (profileApplContext.profileApplicationProcessFunc == NULL)
  {
    profileApplContext.profileApplicationProcessFunc = HT_Application_Process;
  }
  
  Init_temperature();
}

/**
 * @brief This function sets HT specific address
 * @param The profile address
 * @retval None
 */
void HTProfileGetAddress(uint8_t* addr)
{
  uint8_t PROFILE_ADDR[] = {0x12, 0x34, 0x02, 0xE1, 0x80, 0x02};
  
  BLUENRG_memcpy(addr, PROFILE_ADDR, sizeof(PROFILE_ADDR));
}

/**
 * @brief  This function initializes HT specific parameters to default values
 * @param  None
 * @retval None
 */
void Init_temperature(void)
{
  BLUENRG_memset((void*)&tempMeasurementVal, 0, sizeof(tempMeasurementVal));
  
  //tempMeasurementVal.flags = (FLAG_TEMPERATURE_UNITS_FARENHEIT|FLAG_TIMESTAMP_PRESENT|FLAG_TEMPERATURE_TYPE);
  tempMeasurementVal.flags = (FLAG_TIMESTAMP_PRESENT|FLAG_TEMPERATURE_TYPE);
  //tempMeasurementVal.temperatureType = TEMP_MEASURED_FOR_BODY;
  tempMeasurementVal.temperatureType = TEMP_MEASURED_AT_ARMPIT;
  tempMeasurementVal.timeStamp.year = 2015;
  tempMeasurementVal.timeStamp.month = 10;
  tempMeasurementVal.timeStamp.day = 26;
  tempMeasurementVal.timeStamp.hours = 19;
  tempMeasurementVal.timeStamp.minutes = 42;
  tempMeasurementVal.timeStamp.seconds = 50;
}


/**
 * @brief  Profile Application Event Notify
 * @param  Event
 * @param  Event Length
 * @param  Event Data
 * @retval None
 */
void Profile_Appl_Evt_Notify (tNotificationEvent event,uint8_t evtLen,uint8_t* evtData)
{  
  switch(event)
  {
  case EVT_HT_INITIALIZED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_HT_INITIALIZED %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
      APPL_MESG_DBG(profiledbgfile,"EVT_HT_INITIALIZED  AND DEVICE STATE IS %d\n",BLE_Profile_Read_DeviceState());
    }
    break;
  case EVT_HT_TEMPERATURE_CHAR_UPDATE_CMPLT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_HT_TEMPERATURE_CHAR_UPDATE_CMPLT %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_CONNECTED);
    }
    break;
  case EVT_HT_INTERMEDIATE_TEMP_CHAR_UPDATE_CMPLT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_HT_INTERMEDIATE_TEMP_CHAR_UPDATE_CMPLT %x\n",evtData[0]);
      BLE_Profile_Write_DeviceState(APPL_CONNECTED);
    }
    break;
  case EVT_HT_MEASUREMENT_INTERVAL_RECEIVED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_HT_MEASUREMENT_INTERVAL_RECEIVED %x %x\n",evtData[0],evtData[1]);
      BLE_Profile_Write_DeviceState(APPL_UPDATE_MEASUREMENT_INTERVAL);
      //uint16_t interval = evtData[1] | evtData[0] << 8;
    }
    break;
  case EVT_HT_MEASUREMENT_INTERVAL_UPDATE_CMPLT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_HT_MEASUREMENT_INTERVAL_UPDATE_CMPLT\n");
      BLE_Profile_Write_DeviceState(APPL_CONNECTED);
    }
    break;
  }
  
}/* end Profile_Appl_Evt_Notify() */


/**
 * @brief  This function initializes the Health Thermometer Profile by calling the
 *         function HTProfile_Init().
 * @param  None
 * @retval None
 */
void Init_Profile(void)
{
  PRINTF ("Test Application: Initializing Health Thermometer Profile \n" );
  
  if (HT_Init(INTERMEDIATE_TEMPERATURE_CHAR|MEASUREMENT_INTERVAL_CHAR,
          2, /* minValidInterval 2 */
          60, /* maxValidInterval */
          BLE_Profiles_Evt_Notify_Cb) == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Initialized Health Thermometer Profile \n" );
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
  APPL_MESG_DBG(profiledbgfile,"enable advertising\n");
  //char systemId[8]={0x00,0x01,'.',0x00,0x00,0x00,0x00,0x01};
  char pnpid[7]={0x01,0x01,0x01,0x01,0x01,0x01,0x01};
  //BLE_Profile_Update_DIS_SystemID(8,systemId);
  //BLE_Profile_Update_DIS_ModelNum(8,systemId);
  //BLE_Profile_Update_DIS_SerialNum(8,systemId);
  //BLE_Profile_Update_DIS_FirmwareRev(8,systemId);
  //BLE_Profile_Update_DIS_HardwareRev(8,systemId);
  //BLE_Profile_Update_DIS_SoftwareRev(8,systemId);
  //BLE_Profile_Update_DIS_manufacturerName(8,systemId);
  //BLE_Profile_Update_DIS_IEEECertification(8,systemId);
  BLE_Profile_Update_DIS_pnpId(7,pnpid);
  HT_Update_Temperature_Type(0x00);
  HT_Advertize(0x00);

}/* end Advertize() */


/**
 * @brief  Update time before sending.
 * @param  Update interval (in sec)
 * @retval None
 */
void Update_Time(uint8_t time_interval)
{
  tempMeasurementVal.timeStamp.seconds += time_interval;
  if (tempMeasurementVal.timeStamp.seconds >= 60)
  {
    tempMeasurementVal.timeStamp.seconds = tempMeasurementVal.timeStamp.seconds % 60;
    tempMeasurementVal.timeStamp.minutes++;
  }
  if (tempMeasurementVal.timeStamp.minutes == 60)
  {
    tempMeasurementVal.timeStamp.minutes = 0;
    tempMeasurementVal.timeStamp.hours++;
  }  
}

/**
 * @brief  Application specific processing
 * @param  None
 * @retval int
 */
static int HT_Application_Process(void)
{
  int ht;
  int ret_val = 1;
  int32_t uwPRIMASK_Bit = __get_PRIMASK();	/**< backup PRIMASK bit */

  switch(BLE_Profile_Read_DeviceState())
  {
  case APPL_UNINITIALIZED:
    break;
  case APPL_INIT_DONE:
    {
      if(HT_Start_Timer == 1)
      {
        /**
    	 * Make sure to stop the timer only when it has been started at least once
    	 * The Timer ID must be existing to call TIMER_Stop();
    	 */
    	TIMER_Stop(profileApplContext.profileTimer_Id);
    	HT_Start_Timer = 0;
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
      if (HT_Start_Timer == 0)
      {
        TIMER_Start(profileApplContext.profileTimer_Id, (uint16_t)(HEALTH_THERMOMETER_TIMER_VALUE/TIMERSERVER_TICK_VALUE));
        HT_Start_Timer = 1;
      }
      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  case APPL_UPDATE_MEASUREMENT_INTERVAL:
    {
      APPL_MESG_DBG(profiledbgfile,"Call HT_Update_Measurement_Interval()\n");
      HT_Update_Measurement_Interval(30);
      HTUM_sent = 1;
    }
    break;
  default:
    
      if (HTUM_sent == 0)
      {
        HT_Update_Measurement_Interval(30); 
        HTUM_sent = 1;
      }

      if (send_measurement != 0) {
        ht = (rand()&0x07)+35;
        __disable_irq(); /**< Disable all interrupts by setting PRIMASK bit on Cortex*/
        send_measurement = 0;
        __set_PRIMASK(uwPRIMASK_Bit); /**< Restore PRIMASK bit*/
        BLE_Profile_Write_DeviceState(APPL_WAIT);
        Update_Time((uint8_t)(UPDATE_INTERVAL/1000));
        tempMeasurementVal.temperature = ht;
#if INTERMEDIATE_TEMP
        HT_Send_Intermediate_Temperature(tempMeasurementVal);
#else
        HT_Send_Temperature_Measurement(tempMeasurementVal);
#endif
        
      }else{
        ret_val = 0;
      }

    break;

  }
  return ret_val;
}

#endif   /* (BLE_CURRENT_PROFILE_ROLES & HEALTH_THERMOMETER) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
