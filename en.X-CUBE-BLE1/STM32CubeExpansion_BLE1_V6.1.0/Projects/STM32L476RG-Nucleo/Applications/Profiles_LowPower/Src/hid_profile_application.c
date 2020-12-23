/**
******************************************************************************
* @file    hid_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    21-January-2015
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
#include <hid_device.h>

#include "hci_tl_interface.h"

#if (BLE_CURRENT_PROFILE_ROLES & HID_DEVICE)
/*******************************************************************************
 * Macros
*******************************************************************************/
#define HID_TIMER_VALUE	1000000 /**< Value in us */

#define KEYBOARD_IP
//#define MOUSE_IP

#define APPL_UPDATE_INPUT_REPORT
//#define APPL_UPDATE_BOOT_KEYBOARD_IP_REPORT
//#define APPL_UPDATE_BOOT_MOUSE_IP_REPORT
//#define APPL_UPDATE_BATTERY_LEVEL
//#define APPL_UPDATE_SCAN_REFRESH_CHARAC
//#define APPL_UPDATE_FEATURE_REPORT

#if defined(KEYBOARD_IP)
#define REPORT_DESC_SIZE	63//76//67//68
#define REPORT_IP_LEN_0		8
#define KEY_NULL			0
#define KEY_A				4
#define KEY_B				5

#elif defined(MOUSE_IP)
#define REPORT_DESC_SIZE	52
#define REPORT_IP_LEN_0		4
#define MOUSE_OFFSET_X		20
#define MOUSE_OFFSET_Y		20
#endif

#define REPORT_IP_LEN_1         8
#define REPORT_IP_LEN_2         8
#define REPORT_IP_LEN_3         8

#define CLOSE_CONN_TIMEOUT      18 //sec

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
tApplDataForHidServ appHidServData[HID_SERVICE_MAX];
uint8_t battRead = 0;
uint8_t battIdx;

int HID_Start_Timer = 0;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */
extern volatile uint8_t send_measurement;

// Central device address
extern tCentralDevice centralDevice;

volatile uint8_t skip_hid_tick_inc = 1;
volatile uint8_t hid_tick = 0;

/******************************************************************************
 * Function Declarations
******************************************************************************/
void HIDProfileGetAddress(uint8_t* addr);
void Profile_Appl_Evt_Notify(tNotificationEvent event,uint8_t evtLen,uint8_t* evtData);
void Init_Profile(void);
void Advertize(void);
static int HID_Application_Process(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/

/**
 * @brief  Initialize the Application Context for using the HID profile methods
 * @param  None
 * @retval None
 */
void HID_Appl_Context_Init(void)
{
  if (profileApplContext.profileGetAddressFunc == NULL)
  {
    profileApplContext.profileGetAddressFunc = HIDProfileGetAddress;
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
    profileApplContext.profileStateMachineFunc = HIDProfile_StateMachine;
  }
  
  if (profileApplContext.profileApplicationProcessFunc == NULL)
  {
    profileApplContext.profileApplicationProcessFunc = HID_Application_Process;
  }

}

/**
 * @brief This function sets HID specific address
 * @param The profile address
 * @retval None
 */
void HIDProfileGetAddress(uint8_t* addr)
{
  uint8_t PROFILE_ADDR[] = {0x12, 0x34, 0x14, 0xE1, 0x80, 0x02};
  
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
    case EVT_HID_INITIALIZED:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_HID_INITIALIZED\n");
      BLE_Profile_Write_DeviceState(APPL_INIT_DONE);
    }
    break;
    case EVT_BATT_LEVEL_READ_REQ:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_BATT_LEVEL_READ_REQ\n");
      /* BAS service test: TC_CR_BV_01_C  */
      battRead = 0x01;
      battIdx = evtData[0];
      HidDevice_Update_Battery_Level(battIdx,71);
    }
    break;
    case EVT_HID_UPDATE_CMPLT:
    {
      APPL_MESG_DBG(profiledbgfile,"EVT_HID_UPDATE_CMPLT %x %x %x %x\n",evtData[0],evtData[1],evtData[2],evtData[3]);
      BLE_Profile_Write_DeviceState(APPL_CONNECTED);
      if(battRead)
      {
        APPL_MESG_DBG(profiledbgfile,"allow read\n");
        Allow_BatteryLevel_Char_Read(battIdx);
        battRead = 0x00;
      }
    }
    break;
  }
  
}/* end Profile_Appl_Evt_Notify() */


/**
 * @brief This function initializes the HID Profile by calling the
 *        function HID_Init().
 * @param None
 * @retval None
 */
void Init_Profile(void)
{
  uint8_t ipRepLen[HID_SERVICE_MAX] = {REPORT_IP_LEN_0,REPORT_IP_LEN_1,REPORT_IP_LEN_2,REPORT_IP_LEN_3};
  uint8_t opRepLen[HID_SERVICE_MAX] = {1};
  uint8_t featureRepLen[HID_SERVICE_MAX]={0};
  uint8_t ipRepIDs[HID_SERVICE_MAX] = {0,1,2,3,4};
  uint8_t opRepIDs[HID_SERVICE_MAX]={0} ;
  uint8_t featureRepIDs[HID_SERVICE_MAX]={0};
  
  /* report Map characteristic */
  uint8_t report_descriptor[REPORT_DESC_SIZE] =
#if defined(KEYBOARD_IP)
  {
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xA1, 0x01, // COLLECTION (Application)
    0x05, 0x07, // USAGE_PAGE (Keyboard)
    //0x85, 0x01,   // REPORT_ID (1)
    // 1 byte Modifier: Ctrl, Shift and other modifier keys, 8 in total
    0x19, 0xE0, // USAGE_MINIMUM (kbd LeftControl)
    0x29, 0xE7, // USAGE_MAXIMUM (kbd Right GUI)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x25, 0x01, // LOGICAL_MAXIMUM (1)
    0x75, 0x01, // REPORT_SIZE (1)
    0x95, 0x08, // REPORT_COUNT (8)
    0x81, 0x02, // INPUT (Data,Var,Abs)
    // 1 Reserved byte
    0x95, 0x01,  // REPORT_COUNT (1)
    0x75, 0x08, // REPORT_SIZE (8)
    0x81, 0x01, // INPUT (Cnst,Ary,Abs)
    // LEDs for num lock etc
    0x95, 0x05, // REPORT_COUNT (5)
    0x75, 0x01, // REPORT_SIZE (1)
    0x05, 0x08, // USAGE_PAGE (LEDs)
    // 0x85, 0x01, // REPORT_ID (1)
    0x19, 0x01, // USAGE_MINIMUM (Num Lock)
    0x29, 0x05, // USAGE_MAXIMUM (Kana)
    0x91, 0x02, // OUTPUT (Data,Var,Abs)
    // Reserved 3 bits
    0x95, 0x01, // REPORT_COUNT (1)
    0x75, 0x03, // REPORT_SIZE (3)
    0x91, 0x03, // OUTPUT (Cnst,Var,Abs)
    /* Slots for 6 keys that can be pressed down at the same time */
    0x95, 0x06, // REPORT_COUNT (6)
    0x75, 0x08, // REPORT_SIZE (8)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x25, 0x65, // LOGICAL_MAXIMUM (101)
    0x05, 0x07, // USAGE_PAGE (Keyboard)
    0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65, // USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00, // INPUT (Data,Ary,Abs)

    0xC0, // END_COLLECTION
  };
/*
    {0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x85, 0x02,
     0xA1, 0x00, 0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7,
     0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08,
     0x81, 0x02, 0x75, 0x08, 0x95, 0x01, 0x81, 0x01,
     0x75, 0x01, 0x95, 0x05, 0x05, 0x08, 0x19, 0x01,
     0x29, 0x05, 0x91, 0x02, 0x75, 0x03, 0x95, 0x01,
     0x91, 0x01, 0x75, 0x08, 0x95, 0x06, 0x15, 0x00,
     0x25, 0x65, 0x05, 0x07, 0x19, 0x00, 0x29, 0x65,
     0x81, 0x00, 0xC0, 0xC0};
*/    
/*
  {
    0x05, 0x01,                     // Usage Page (Generic Desktop)
    0x09, 0x06,                         // Usage (Keyboard)
    0xA1, 0x01,                         // Collection (Application)
    0x05, 0x07,                         //     Usage Page (Key Codes)
    0x19, 0xe0,                         //     Usage Minimum (224)
    0x29, 0xe7,                         //     Usage Maximum (231)
    0x15, 0x00,                         //     Logical Minimum (0)
    0x25, 0x01,                         //     Logical Maximum (1)
    0x75, 0x01,                         //     Report Size (1)
    0x95, 0x08,                         //     Report Count (8)
    0x81, 0x02,                         //     Input (Data, Variable, Absolute)
    0x95, 0x01,                         //     Report Count (1)
    0x75, 0x08,                         //     Report Size (8)
    0x81, 0x01,                         //     Input (Constant) reserved byte(1)
    0x95, 0x05,                         //     Report Count (5)
    0x75, 0x01,                         //     Report Size (1)
    0x05, 0x08,                         //     Usage Page (Page# for LEDs)
    0x19, 0x01,                         //     Usage Minimum (1)
    0x29, 0x05,                         //     Usage Maximum (5)
    0x91, 0x02,                         //     Output (Data, Variable, Absolute), Led report
    0x95, 0x01,                         //     Report Count (1)
    0x75, 0x03,                         //     Report Size (3)
    0x91, 0x01,                         //     Output (Data, Variable, Absolute), Led report padding
    0x95, 0x06,                         //     Report Count (6)
    0x75, 0x08,                         //     Report Size (8)
    0x15, 0x00,                         //     Logical Minimum (0)
    0x25, 0x65,                         //     Logical Maximum (101)
    0x05, 0x07,                         //     Usage Page (Key codes)
    0x19, 0x00,                         //     Usage Minimum (0)
    0x29, 0x65,                         //     Usage Maximum (101)
    0x81, 0x00,                         //     Input (Data, Array) Key array(6 bytes)
    0x09, 0x05,                         //     Usage (Vendor Defined)
    0x15, 0x00,                         //     Logical Minimum (0)
    0x26, 0xFF, 0x00,                   //     Logical Maximum (255)
    0x75, 0x08,                         //     Report Count (2)
    0x95, 0x02,                         //     Report Size (8 bit)
    0xB1, 0x02,                         //     Feature (Data, Variable, Absolute)
    0xC0                                // End Collection (Application)
  };
*/
#elif defined(MOUSE_IP)
  {
    0x05, 0x01,   /* Usage Page (Generic Desktop)*/
    0x09, 0x02,   /* Usage (Mouse) */
    0xA1, 0x01,   /* Collection (Application) */
    0x09, 0x01,   /* Usage (Pointer) */
    0xA1, 0x00,   /* Collection (Physical) */
    0x05, 0x09,   /* Usage Page (Buttons) */
    0x19, 0x01,   /* Usage Minimun (01) */
    0x29, 0x03,   /* Usage Maximum (03) */
    0x15, 0x00,   /* logical Minimun (0) */
    0x25, 0x01,   /* logical Maximum (1) */
    0x95, 0x03,   /* Report Count (3) */
    0x75, 0x01,   /* Report Size (1) */
    0x81, 0x02,   /* Input(Data, Variable, Absolute) 3 button bits */
    0x95, 0x01,   /* Report count (1) */
    0x75, 0x05,   /* Report Size (5) */
    0x81, 0x01,   /* Input (Constant), 5 bit padding */
    0x05, 0x01,   /* Usage Page (Generic Desktop) */
    0x09, 0x30,   /* Usage (X) */
    0x09, 0x31,   /* Usage (Y) */
    0x09, 0x38,   /* Usage (Z) */
    0x15, 0x81,   /* Logical Minimum (-127) */
    0x25, 0x7F,   /* Logical Maximum (127) */
    0x75, 0x08,   /* Report Size (8) */
    0x95, 0x03,   /* Report Count (2) */
    0x81, 0x06,   /* Input(Data, Variable, Relative), 2 position bytes (X & Y)*/
    0xC0,         /* end collection */
    0xC0          /* end collection */
  };
#endif
  /*
  uint8_t ipRepLen[HID_SERVICE_MAX] = {8,8,8,8,8};
  uint8_t opRepLen[HID_SERVICE_MAX] = {8};
  uint8_t featureRepLen[HID_SERVICE_MAX]={8};
  uint8_t ipRepIDs[HID_SERVICE_MAX] = {0,1,2,3,4};
  uint8_t opRepIDs[HID_SERVICE_MAX]={0} ;
  uint8_t featureRepIDs[HID_SERVICE_MAX]={0};
  */
  tnonHIDService nonHIDService ;
  PRINTF ("Test Application: Initializing HID profile\n");

  /* Configure the User Button in EXTI Mode */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

  BLUENRG_memset ( appHidServData, 0, (sizeof(tApplDataForHidServ)*HID_SERVICE_MAX) );

  appHidServData[0].inputReportNum = 1;
  appHidServData[0].ipReportLen = ipRepLen;
  appHidServData[0].ipReportID = ipRepIDs;

  appHidServData[0].outputReportNum = 0; //1;
  appHidServData[0].opReportLen = opRepLen;
  appHidServData[0].opReportID = opRepIDs;
    
  appHidServData[0].featureReportNum = 0;
  appHidServData[0].ftrReportLen = featureRepLen;
  appHidServData[0].ftrReportID = featureRepIDs;

  appHidServData[0].reportMapValueLen = REPORT_DESC_SIZE;
  appHidServData[0].reportDesc = report_descriptor;
    
  /*Default settings:  appHidServData[0].extReportCharRefNum = 0;
     NOTE: Set to 1 for HIDS tests: TP/DES/BV-03-C, TP/DR/BV-03-C */
  appHidServData[0].extReportCharRefNum = 1; //0; 
  
  nonHIDService.servUUID = 0xA00A;
  nonHIDService.charUUID = 0xA00B;
  nonHIDService.charValLen = 0x06;
  nonHIDService.isVariable = 0x00;
  appHidServData[0].nonHidServInfo = &nonHIDService;
  /* Default settings: BOOT_DEVICE_MOUSE_MASK. 
     Use BOOT_DEVICE_KEYBOARD_MASK for all PTS HIDS tests requesting boot 
     keboard input and outpur report characteristics */
  appHidServData[0].bootModeDeviceType = BOOT_DEVICE_MOUSE_MASK; //BOOT_DEVICE_MOUSE_MASK; //0x02
  
  appHidServData[0].bootIpReportLenMax = 10;
  appHidServData[0].bootOpReportLenMax = 16;
  appHidServData[0].hidInfoChar.bcdHID = 0xFADB;
  appHidServData[0].hidInfoChar.bCountryCode = 90;
  appHidServData[0].hidInfoChar.flags = 0x02;
    
  /*
  appHidServData[1].inputReportNum = 1;
  appHidServData[1].ipReportLen = ipRepLen;
  appHidServData[1].ipReportID = ipRepIDs;

  appHidServData[1].outputReportNum = 0;
  appHidServData[1].opReportLen = opRepLen;
  appHidServData[1].opReportID = opRepIDs;
    
  appHidServData[1].featureReportNum = 1;
  appHidServData[1].ftrReportLen = featureRepLen;
  appHidServData[1].ftrReportID = featureRepIDs;

  appHidServData[1].reportMapValueLen = 2;
  appHidServData[1].reportDesc = report_descriptor;
    
  appHidServData[1].extReportCharRefNum = 1;
  nonHIDService.servUUID = 0xA00A;
  nonHIDService.charUUID = 0xA00B;
  nonHIDService.charValLen = 0x06;
  nonHIDService.isVariable = 0x00;
  appHidServData[1].nonHidServInfo = &nonHIDService;
    
  appHidServData[1].bootModeDeviceType = BOOT_DEVICE_MOUSE_MASK;
  appHidServData[1].bootIpReportLenMax = 10;
  appHidServData[1].bootOpReportLenMax = 16;
  APPL_MESG_DBG(profiledbgfile,"Initializing HID Device Profile \n" );*/

  if (HidDevice_Init (1, /* numOfHIDServices */
                      appHidServData, 
                      1, /* numOfBatteryServices: 1 is default settings. Set to 2 for BAS service test TC_DES_BV_01_C */
                      (uint16_t)PNP_ID_CHAR_MASK, 
                      0x01,
                      0x01, 
                      BLE_Profiles_Evt_Notify_Cb) == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Initialized HID Device Profile \n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Failed HID Device Profile \n" );
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
  
  uint8_t useBoundedDeviceList = 0;
  
  HidDevice_Make_Discoverable(useBoundedDeviceList);
}/* end Advertize() */


/**
 * @brief  Application specific processing
 * @param  None
 * @retval int
 */
static int HID_Application_Process(void)
{
  int ret_val = 1;
  int32_t uwPRIMASK_Bit = __get_PRIMASK();	/**< backup PRIMASK bit */

  switch(BLE_Profile_Read_DeviceState())
  {
  case APPL_UNINITIALIZED:
    break;
  case APPL_INIT_DONE:
    {
      if(HID_Start_Timer == 1)
      {
        /**
    	 * Make sure to stop the timer only when it has been started at least once
    	 * The Timer ID must be existing to call TIMER_Stop();
    	 */
    	TIMER_Stop(profileApplContext.profileTimer_Id);
    	HID_Start_Timer = 0;
      }
      Advertize();
      BLE_Profile_Write_DeviceState(APPL_ADVERTISING);
    }
    break;
  case APPL_TERMINATE_LINK:
    {      
      /* on disconnection complete, the state will be changed to APPL_UNINITIALIZED
      */ 
      BLE_Profile_Disconnect();
      BLE_Profile_Write_DeviceState(APPL_UNINITIALIZED);
    }
    break;
  case APPL_CONNECTED:
    {
      // Once connected we check if the Central device is bonded, if not let's start
      // a close connection timer to store the bonding status safely
      // Only for X-NUCLEO-IDB05A1 expansion board
      tBleStatus ret = aci_gap_is_device_bonded(centralDevice.peer_bdaddr_type, centralDevice.peer_bdaddr);
      if(ret != BLE_STATUS_SUCCESS) {
        skip_hid_tick_inc = 0;
      }

      if (HID_Start_Timer == 0)
      {
        TIMER_Start(profileApplContext.profileTimer_Id, (uint16_t)(HID_TIMER_VALUE/TIMERSERVER_TICK_VALUE));
        HID_Start_Timer = 1;
      }

      BLE_Profile_Write_DeviceState(APPL_WAIT);
    }
    break;
  default:

    if(hid_tick >= CLOSE_CONN_TIMEOUT) {
      __disable_irq(); /**< Disable all interrupts by setting PRIMASK bit on Cortex*/
      hid_tick = 0;
      skip_hid_tick_inc = 1;
      __set_PRIMASK(uwPRIMASK_Bit); /**< Restore PRIMASK bit*/
      BLE_Profile_Disconnect();
    }

    if (send_measurement != 0)
    {
      __disable_irq(); /**< Disable all interrupts by setting PRIMASK bit on Cortex*/
      send_measurement = 0;
      __set_PRIMASK(uwPRIMASK_Bit);	/**< Restore PRIMASK bit*/
      BLE_Profile_Write_DeviceState(APPL_WAIT);

#if defined(APPL_UPDATE_INPUT_REPORT)

#if defined(KEYBOARD_IP)
      APPL_MESG_DBG(profiledbgfile,"Sending data...\n" );
      uint8_t ipRepVal[REPORT_IP_LEN_0] = {2,0,0,0,0,0,0,0};

      /* Press AB followed by 0 */
      ipRepVal[2] = KEY_A;
      ipRepVal[3] = KEY_B;
      HidDevice_Update_Input_Report(0,0,REPORT_IP_LEN_0,ipRepVal);

      ipRepVal[0] = KEY_NULL;
      ipRepVal[2] = KEY_NULL;
      ipRepVal[3] = KEY_NULL;
      HidDevice_Update_Input_Report(0,0,REPORT_IP_LEN_0,ipRepVal);

#elif defined(MOUSE_IP)
      static int8_t ipRepVal[REPORT_IP_LEN_0] = {0,0,0,0};
      static int8_t cnt = 0;

      if(cnt == 0) {
        ipRepVal[1] = MOUSE_OFFSET_X;
        ipRepVal[2] = MOUSE_OFFSET_Y;
        cnt++;
      } else {
        ipRepVal[1] = -MOUSE_OFFSET_X;
        ipRepVal[2] = -MOUSE_OFFSET_Y;
        cnt--;
      }

      HidDevice_Update_Input_Report(0,0,REPORT_IP_LEN_0,ipRepVal);
#endif

#elif defined(APPL_UPDATE_BOOT_KEYBOARD_IP_REPORT)
      uint8_t ipRepVal[10] = {1,2,3,4,5,6,7,8,9,10};
      //uint8_t ipRepVal1[10] = {1,2,3,4,5,6,7,8,9,11};
      HidDevice_Update_Boot_Keyboard_Input_Report(0,10,ipRepVal);

#elif defined(APPL_UPDATE_BOOT_MOUSE_IP_REPORT)
      uint8_t mouseRepVal[10] = {1,2,3,4,5,6,7,8,0,0};
      HidDevice_Update_Boot_Mouse_Input_Report(0,10,mouseRepVal);

#elif defined(APPL_UPDATE_BATTERY_LEVEL)
      APPL_MESG_DBG(profiledbgfile,"HID_Device: APPL_UPDATE_BATTERY_LEVEL state (update battery level)\n");
      HidDevice_Update_Battery_Level(battIdx,98);

#elif defined(APPL_UPDATE_SCAN_REFRESH_CHARAC)
      HidDevice_Update_Scan_Refresh_Char(0x01);

#elif defined(APPL_UPDATE_FEATURE_REPORT)
      uint8_t featRepVal[8] = {8,7,6,5,4,3,2,1};
      HidDevice_Update_Feature_Report(0,0,8,featRepVal);

#endif

    }else{
      ret_val = 0;
    }
    break;
  }
  return ret_val;

}


#endif  /* (BLE_CURRENT_PROFILE_ROLES & HID_DEVICE) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
