/*******************************************************************************
*
* THE PRESENT CODE AIMS AT PROVIDING CUSTOMERS WITH CODING INFORMATION
* REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* (C) 2014 STMicroelectronics International NV
*
*   FILENAME        -  profile_TimeServer_central_test_application.c
*
*******************************************************************************
*
*   CHANGE HISTORY
*  ---------------------------
*
*   $Date$:      02/23/2015
*   $Revision$:  first version
*   $Author$:    AMS RF Application team
*   Comments:    Profiles PTS test application for Time Server
*                Central role.
*                It uses the BlueNRG Host Profiles Library.
*
*******************************************************************************
*
*  File Description 
*  ---------------------
* 
*******************************************************************************/

/*******************************************************************************
 * Include Files
*******************************************************************************/
#include "hci.h"
#include "hci_tl.h"
#include "bluenrg_aci.h"
#include "bluenrg_gatt_server.h"
#include "hci_const.h"
#include "bluenrg_gap.h"
#include "sm.h"
#include <stdio.h>

#include <host_config.h>
#if (PLATFORM_WINDOWS == 1)
#include <Windows.h>
#endif
#include <stdio.h>
#include <debug.h>
#include <hci.h>
#include "master_basic_profile.h"

#if (BLE_CURRENT_PROFILE_ROLES & TIME_SERVER)
#include <time_server.h>
#include <time_server_config.h>

/*******************************************************************************
 * Macros
*******************************************************************************/
/* PERIPHERAL peer address */
#define PERIPHERAL_PEER_ADDRESS PTS_PERIPHERAL_ADDRESS //PTS_PERIPHERAL_ADDRESS or PEER_ADDRESS

/* application states */
#define APPL_UNINITIALIZED		(0x00) //TBR
#define APPL_INIT_DONE			(0x01) //TBR

/* Time Server test configuration for some PTS tests */

/* --------------------- User Inputs for Time Server PTS tests -----------------*/

/* General operations: discovery, connection, disconnection, bonding, clear security database */
#define APPL_DISCOVER_TIME_CLIENT      'a' // Start Discovery procedure for detecting a time client
#define APPL_CONNECT_TIME_CLIENT       'b' // Start Connection procedure for connecting to the discovered time client
#define APPL_DISCONNECT_TIME_CLIENT    'c' // Terminate the current connection with the time client
#define APPL_PAIR_WITH_TIME_CLIENT     'd' // Start the pairing  procedure (or enable the encryption) with the connected time client
#define APPL_CLEAR_SECURITY_DATABASE   'e' // Clear the security database (it removes any bonded device) 

#define APPL_ADJUST_REASON_MANUAL_TIME_UPDATE               'f'
#define APPL_ADJUST_REASON_CHANGE_OF_TIME_ZONE              'g' 
#define APPL_ADJUST_REASON_CHANGE_OF_DST                    'h'
#define APPL_ADJUST_REASON_EXTERNAL_REFERENCE_TIME_UPDATE   'k'
            
// User Inputs For Time Server 
#define APPL_UPDATE_CURRENT_TIME                'i' 

/* It is used for PTS test TC_CSP_BV_06_C */
#define APPL_DECREASE_CURRENT_TIME              'j'      
/* It is used for PTS test TC_CSP_BV_06_C */
#define APPL_UPDATE_CURRENT_TIME_MORE_THAN_15  'l'

#define APPL_UPDATE_LOCAL_TIME			'm' 
#define APPL_UPDATE_REFERENCE_TIME		'n' 
#define APPL_UPDATE_NEXT_DSTCHANGE_TIME		'o' 
#define APPL_UPDATE_REFERENCE_TIME_INFO		'p'

#define DISPLAY_PTS_MENU                                         '?' //Display PTS application command menu
/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
uint8_t deviceState = APPL_UNINITIALIZED;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern void *profiledbgfile;
extern initDevType initParam;

uint8_t status;

tCurrentTime time;

/******************************************************************************
 * Function Declarations
******************************************************************************/

/******************************************************************************
 * Function Definitions 
******************************************************************************/
void Init_Current_Time()
{
  time.date = 22;
  time.day_of_week = 02;
  time.hours = 4;
  time.minutes = 1; /* keep it to 1 as default value x PTS tests */
  time.seconds = 4;
  
  time.adjustReason = 0x01; /* ADJUST_REASON_MANUAL_TIME_UPDATE: TP/CSP/BV-02-C */
  
  time.fractions256 = 0x00;
  time.year = 2015;
  time.month = 2;

}

void Display_Appl_Menu()
{
  APPL_MESG_INFO(profiledbgfile,"APPL_DISCOVER_TIME_CLIENT: %c\n",APPL_DISCOVER_TIME_CLIENT);
  
  APPL_MESG_INFO(profiledbgfile,"APPL_CONNECT_TIME_CLIENT: %c\n",APPL_CONNECT_TIME_CLIENT);
  APPL_MESG_INFO(profiledbgfile,"APPL_DISCONNECT_TIME_CLIENT: %c\n",APPL_DISCONNECT_TIME_CLIENT);
  APPL_MESG_INFO(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: %c\n",APPL_CLEAR_SECURITY_DATABASE);
  APPL_MESG_INFO(profiledbgfile,"APPL_PAIR_WITH_TIME_CLIENT: %c\n",APPL_PAIR_WITH_TIME_CLIENT);
  
  APPL_MESG_INFO(profiledbgfile,"APPL_ADJUST_REASON_MANUAL_TIME_UPDATE: %c\n",APPL_ADJUST_REASON_MANUAL_TIME_UPDATE);
  APPL_MESG_INFO(profiledbgfile,"APPL_ADJUST_REASON_CHANGE_OF_TIME_ZONE: %c\n",APPL_ADJUST_REASON_CHANGE_OF_TIME_ZONE);
  APPL_MESG_INFO(profiledbgfile,"APPL_ADJUST_REASON_CHANGE_OF_DST: %c\n",APPL_ADJUST_REASON_CHANGE_OF_DST);
  APPL_MESG_INFO(profiledbgfile,"APPL_ADJUST_REASON_EXTERNAL_REFERENCE_TIME_UPDATE: %c\n",APPL_ADJUST_REASON_EXTERNAL_REFERENCE_TIME_UPDATE);
  
  APPL_MESG_INFO(profiledbgfile,"APPL_UPDATE_CURRENT_TIME: %u\n",APPL_UPDATE_CURRENT_TIME);
  APPL_MESG_INFO(profiledbgfile,"APPL_UPDATE_LOCAL_TIME: %u\n",APPL_UPDATE_LOCAL_TIME);
  APPL_MESG_INFO(profiledbgfile,"APPL_UPDATE_REFERENCE_TIME: %u\n",APPL_UPDATE_REFERENCE_TIME);
  APPL_MESG_INFO(profiledbgfile,"APPL_UPDATE_NEXT_DSTCHANGE_TIME: %u\n",APPL_UPDATE_NEXT_DSTCHANGE_TIME);
  APPL_MESG_INFO(profiledbgfile,"APPL_UPDATE_REFERENCE_TIME_INFO: %u\n",APPL_UPDATE_REFERENCE_TIME_INFO);	 
  
  APPL_MESG_INFO(profiledbgfile,"DISPLAY_PTS_MENU: %c\n",DISPLAY_PTS_MENU);          
    
}/* end Display_Appl_Menu() */

void Device_Init(void)
{
  tipsInitDevType InitDev; 
  uint8_t public_addr[6]=TIME_SERVER_PUBLIC_ADDRESS;
  uint8_t device_name[]=TIME_SERVER_DEVICE_NAME;
  tBleStatus ble_status;

  InitDev.public_addr =  public_addr;
  InitDev.txPower =      TIME_SERVER_TX_POWER_LEVEL;
  InitDev.device_name_len = sizeof(device_name);
  InitDev.device_name = device_name;
  
  /* Init Time Server Profile Central Role with optional supported services */
  InitDev.servicesToBeSupported = NEXT_DST_CHANGE_SERVICE_BITMASK|REFERENCE_TIME_UPDATE_SERVICE_BITMASK;
  
  ble_status = TimeServer_Init(InitDev);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"TimeServer_Init() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeServer_Init() Error: %02X\n", ble_status);
  }
}

void Device_SetSecurity(void)
{
  tipsSecurityType param;
  tBleStatus ble_status;
  
  param.ioCapability =  TIME_SERVER_IO_CAPABILITY;
  param.mitm_mode =     TIME_SERVER_MITM_MODE;
  param.oob_enable =    TIME_SERVER_OOB_ENABLE;
  param.bonding_mode =  TIME_SERVER_BONDING_MODE;
  param.use_fixed_pin = TIME_SERVER_USE_FIXED_PIN;
  param.fixed_pin =     TIME_SERVER_FIXED_PIN;
  
  ble_status = TimeServer_SecuritySet(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"TimeServer_SecuritySet() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeServer_SecuritySet() Error: %02X\n", ble_status);
  }
}

void Device_Discovery_Procedure(void)
{
  tipsDevDiscType param;
  tBleStatus ble_status;
  
  param.own_addr_type = PUBLIC_ADDR;
  param.scanInterval =  TIME_SERVER_LIM_DISC_SCAN_INT;
  param.scanWindow =    TIME_SERVER_LIM_DISC_SCAN_WIND;
  ble_status = TimeServer_DeviceDiscovery(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"TimeServer_DeviceDiscovery() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeServer_DeviceDiscovery() Error: %02X\n", ble_status);
  }
}

void Device_Connection_Procedure(void)
{
  tipsConnDevType param;
  tBleStatus ble_status;
  uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS;

  param.fastScanDuration =         TIME_SERVER_FAST_SCAN_DURATION;
  param.fastScanInterval =         TIME_SERVER_FAST_SCAN_INTERVAL;
  param.fastScanWindow =           TIME_SERVER_FAST_SCAN_WINDOW;
  param.reducedPowerScanInterval = TIME_SERVER_REDUCED_POWER_SCAN_INTERVAL;
  param.reducedPowerScanWindow =   TIME_SERVER_REDUCED_POWER_SCAN_WINDOW;
  param.peer_addr_type =           PUBLIC_ADDR;
  param.peer_addr =                peer_addr;
  param.own_addr_type =            PUBLIC_ADDR;
  param.conn_min_interval =        TIME_SERVER_FAST_MIN_CONNECTION_INTERVAL;
  param.conn_max_interval =        TIME_SERVER_FAST_MAX_CONNECTION_INTERVAL;
  param.conn_latency =             TIME_SERVER_FAST_CONNECTION_LATENCY;
  param.supervision_timeout =      TIME_SERVER_SUPERVISION_TIMEOUT;
  param.min_conn_length =          TIME_SERVER_MIN_CONN_LENGTH;
  param.max_conn_length =          TIME_SERVER_MAX_CONN_LENGTH;
  ble_status = TimeServer_DeviceConnection(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"TimeServer_DeviceConnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeServer_DeviceConnection() Error: %02X\n", ble_status);
  }
}
 
void Device_StartPairing(void)
{
  tBleStatus ble_status;
  ble_status = TimeServer_StartPairing();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"TimeServer_StartPairing() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeServer_StartPairing() Error: %02X\n", ble_status);
  }
}

void Device_Disconnection(void)
{
  tBleStatus ble_status;
  ble_status = TimeServer_DeviceDisconnection();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"TimeServer_DeviceDisconnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeServer_DeviceDisconnection() Error: %02X\n", ble_status);
  }
}

void Host_Profile_Test_Application (void)
{
  deviceState = APPL_UNINITIALIZED;
  
  APPL_MESG_INFO(profiledbgfile,"\n Test Application: Starting testing the BLE Profiles (? for menu) \n" );
  
  /* Init user time */
  Init_Current_Time();
  
  /* Init Time Server Profile Central Role */
  Device_Init();

  /* Init Time Server Profile Security */
  Device_SetSecurity();
  
  /* Add device service & characteristics */
  status =  TimeServer_Add_Services_Characteristics();
  if (status == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"Time_Server_Add_Services_Characteristics() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Time_Server_Add_Services_Characteristics() Error: %02X\n", status);
  }
  
  deviceState = APPL_INIT_DONE;
  while(1)
  {
    hci_user_evt_proc();
    
    /* process the timer Q */
    Blue_NRG_Timer_Process_Q(); 
    
    /* Call the Profile Master role state machine */
    Master_Process(&initParam);
    
    TimeServer_StateMachine(); 

    /* if the profiles have nothing more to process, then
     * wait for application input
     */ 
    if(deviceState >= APPL_INIT_DONE) //TBR
    {      
      uint8_t input = APPL_DISCOVER_TIME_CLIENT;//200;
      deviceState = input;
      if (input>0)
      {
        APPL_MESG_INFO(profiledbgfile,"io--- input: %c\n",input); 

        switch(input)
        {   
          case DISPLAY_PTS_MENU:
             
          case APPL_DISCOVER_TIME_CLIENT:
          case APPL_CONNECT_TIME_CLIENT:
          case APPL_DISCONNECT_TIME_CLIENT:
          case APPL_PAIR_WITH_TIME_CLIENT:
          case APPL_CLEAR_SECURITY_DATABASE:
            
          case APPL_ADJUST_REASON_MANUAL_TIME_UPDATE:
          case APPL_ADJUST_REASON_CHANGE_OF_TIME_ZONE: 
          case APPL_ADJUST_REASON_CHANGE_OF_DST:     
          case APPL_ADJUST_REASON_EXTERNAL_REFERENCE_TIME_UPDATE: 
            
          case APPL_UPDATE_CURRENT_TIME:            
          case APPL_UPDATE_LOCAL_TIME:				 
          case APPL_UPDATE_REFERENCE_TIME:			 
          case APPL_UPDATE_NEXT_DSTCHANGE_TIME:		 
          case APPL_UPDATE_REFERENCE_TIME_INFO:	
          case APPL_DECREASE_CURRENT_TIME:
          case APPL_UPDATE_CURRENT_TIME_MORE_THAN_15:
          {
            deviceState = input;
          }
          break;
          default:
          break; //continue
        }/* end switch(input) */
      } /* end if _IO.... */
    }/* end if(deviceState >= APPL_CONNECTED) */
            
    /* application specific processing */	
    switch(deviceState)
    {
      case DISPLAY_PTS_MENU:
      {
        Display_Appl_Menu();
      }
      break;
      
      case APPL_DISCOVER_TIME_CLIENT:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_TIME_CLIENT: call Device_Discovery_Procedure() \n"); 
        Device_Discovery_Procedure();
      }
      break;
      case APPL_CONNECT_TIME_CLIENT:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_TIME_CLIENT: call Device_Connection_Procedure() \n"); 
        Device_Connection_Procedure();
      }
      break;
      case APPL_DISCONNECT_TIME_CLIENT:
      {
         APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_TIME_CLIENT: call Device_Disconnection() \n"); 
         Device_Disconnection();
      }
      break;
      case APPL_PAIR_WITH_TIME_CLIENT:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_TIME_CLIENT: call Device_StartPairing() \n"); 
        Device_StartPairing();
      }
      break;
      case APPL_CLEAR_SECURITY_DATABASE:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: call Clear_Security_Database() \n"); 
        status = TimeServer_Clear_Security_Database();
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Clear_Security_Database() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Clear_Security_Database() Error: %02X\n", status);
        }
      }
      break; 
      
      case APPL_ADJUST_REASON_MANUAL_TIME_UPDATE:
      {
        time.adjustReason = 0x01; /* ADJUST_REASON_MANUAL_TIME_UPDATE: TP/CSP/BV-02-C */
      }
      break;
      
      case APPL_ADJUST_REASON_CHANGE_OF_TIME_ZONE:
      {
        time.adjustReason = 0x04; /* ADJUST_REASON_CHANGE_OF_TIME_ZONE: TP/CSP/BV-03-C */
      }
      break;
      case APPL_ADJUST_REASON_CHANGE_OF_DST:
      {
        time.adjustReason = 0x08; /* ADJUST_REASON_CHANGE_OF_DST:     TP/CSP/BV-04-C */
      }
      break;
      case APPL_ADJUST_REASON_EXTERNAL_REFERENCE_TIME_UPDATE:
      {
        time.adjustReason = 0x02; /* ADJUST_REASON_EXTERNAL_REFERENCE_TIME_UPDATE: TP/CSP/BV-05-C, TP/CSP/BV-06-C*/
      }
      break;
      case APPL_UPDATE_CURRENT_TIME:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_UPDATE_CURRENT_TIME (minute: %d) as +2 minutes\n", time.minutes); 
        
        //time.minutes += 1;
        //if(time.minutes == 60)
        //{
          //time.minutes = 0;
          //time.hours+=1;
        //}
        
        //TBR: new code for PTS tests including also PTS TEST TC_CSP_BV_06_C */
        if ((time.minutes + 2) >= 60)
        {
          time.minutes = (time.minutes + 2) - 60;
          time.hours+=1;
        }
        else
          time.minutes += 2;

        status = TimeServer_Update_Current_Time_Value(time);
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Update_Current_Time_Value() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Update_Current_Time_Value() Error: %02X\n", status);
        }
      }
      break;
      
      case APPL_DECREASE_CURRENT_TIME: //New code only for PTS TEST TC_CSP_BV_06_C 
      {
        /* Adjusting time less than 1 minute from previous one */
        APPL_MESG_DBG(profiledbgfile,"APPL_DECREASE_CURRENT_TIME (minute: %d) less than 1 minute (decrease 1 second)\n",time.minutes);
        /* Just decrease current time of 1 second: required for PTS test TC_CSP_BV_06_C */
        time.seconds -= 1;

        status = TimeServer_Update_Current_Time_Value(time);
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Update_Current_Time_Value() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Update_Current_Time_Value() Error: %02X\n", status);
        }
      }
      break;
      case APPL_UPDATE_CURRENT_TIME_MORE_THAN_15://New code (only for PTS TEST TC_CSP_BV_06_C?  TBR if needed)
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_UPDATE_CURRENT_TIME_MORE_THAN_15 (minute: %d) as +15 minutes\n", time.minutes);
        if ((time.minutes + 15) >= 60)
        {
          time.minutes = (time.minutes + 15) - 60;
          time.hours+=1;
        }
        else
          time.minutes += 15;
        status = TimeServer_Update_Current_Time_Value(time);
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Update_Current_Time_Value() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Update_Current_Time_Value() Error: %02X\n", status);
        }
      }
      break;
        
      case APPL_UPDATE_LOCAL_TIME:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_UPDATE_LOCAL_TIME\n");
        tLocalTimeInformation localTimeInfo;
        localTimeInfo.timeZone = 0x03;
        localTimeInfo.dstOffset = 0x12;
        TimeServer_Update_Local_Time_Information(localTimeInfo);
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Update_Local_Time_Information() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Update_Local_Time_Information() Error: %02X\n", status);
        }
      }
      break;
      case APPL_UPDATE_NEXT_DSTCHANGE_TIME:
      {
        tTimeWithDST timeDST;
        timeDST.date = 06;
        timeDST.year = 2000;
        timeDST.month = 04;
        timeDST.hours = 1;
        timeDST.minutes = 1;
        timeDST.seconds = 4;
               
        APPL_MESG_DBG(profiledbgfile,"APPL_UPDATE_NEXT_DSTCHANGE_TIME\n");
        TimeServer_Update_Next_DST_Change(timeDST);
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Update_Next_DST_Change() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Update_Next_DST_Change() Error: %02X\n", status);
        }
      }
      break;
      case APPL_UPDATE_REFERENCE_TIME_INFO:
      {
        tReferenceTimeInformation refTimeInfo;
        refTimeInfo.source = 0x01;	
        refTimeInfo.accuracy = 0x02;	 
        refTimeInfo.daysSinceUpdate = 0x05;	 
        refTimeInfo.hoursSinceUpdate = 0x03;	 
        APPL_MESG_DBG(profiledbgfile,"APPL_UPDATE_REFERENCE_TIME_INFO\n");
        TimeServer_Update_Reference_Time_Information(refTimeInfo);
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Update_Reference_Time_Information() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"TimeServer_Update_Reference_Time_Information() Error: %02X\n", status);
        }    
      }
      break;

    }/* end switch(devicestate) */

  } /* end while(1) */
}/* end Host_Profile_Test_Application() */

/***************  Public Function callback ******************/

void TimeServer_DeviceDiscovery_CB(uint8_t status, uint8_t addr_type, uint8_t *addr, 
                                     uint8_t data_length, uint8_t *data, 
                                     uint8_t RSSI)
{
  uint8_t addr_to_find[6]=PERIPHERAL_PEER_ADDRESS;
  
  if (memcmp(addr, addr_to_find, 6) == 0) {
    APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
    APPL_MESG_INFO(profiledbgfile,"*\r\n");
    APPL_MESG_INFO(profiledbgfile,"**** Peer Device Found\r\n");
    APPL_MESG_INFO(profiledbgfile,"**** Addr = 0x");
    for (uint8_t i=0; i<6; i++)
      APPL_MESG_INFO(profiledbgfile,"%02x", addr[i]);
    APPL_MESG_INFO(profiledbgfile,"\r\n");
    APPL_MESG_INFO(profiledbgfile,"*\r\n");
    APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
  }
}

void TimeServer_ConnectionStatus_CB(uint8_t connection_evt, uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
  switch(connection_evt) {
  case CONNECTION_ESTABLISHED_EVT:
    if (status == BLE_STATUS_SUCCESS) {
      APPL_MESG_INFO(profiledbgfile,"****  Connection Established with Success\r\n");
    }
    break;
  case CONNECTION_FAILED_EVT:
    APPL_MESG_INFO(profiledbgfile,"****  Connection Failed with Status = 0x%02x\r\n", status);
    break;
  case DISCONNECTION_EVT:
    APPL_MESG_INFO(profiledbgfile,"****  Disconnection with peer device\r\n");
    break;
  }
  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
}

void TimeServer_PinCodeRequired_CB(void)
{
  uint8_t ret;
  uint32_t pinCode=0;

  APPL_MESG_INFO(profiledbgfile,"**** Required Security Pin Code\r\n");

  /* Insert the pin code according the glucose collector indication */
  //pinCode = ....

  ret = TimeServer_SendPinCode(pinCode);
  if (ret != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"**** Error during the Pairing procedure -> Pin Code set (Status = 0x%x)\r\n", ret);
  }
}

void TimeServer_Pairing_CB(uint16_t conn_handle, uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"Pairing CB conn_handle = 0x%04x with status = 0x%02x***************\n", conn_handle, status);
  
}

void TimeServer_Notify_State_To_User_Application_CB(uint8_t event_value)
{
  switch (event_value)
  {
   case EVT_TS_START_REFTIME_UPDATE:
    {
      APPL_MESG_INFO(profiledbgfile,"EVT_TS_START_REFTIME_UPDATE: call TimeServer_Update_Current_Time_Value()\n");
      TimeServer_Update_Current_Time_Value(time);
    }
    break;
    case EVT_TS_CHAR_UPDATE_CMPLT:
    {
      APPL_MESG_INFO(profiledbgfile,"EVT_TS_CHAR_UPDATE_CMPLT\n");		
    }
    break;
    case EVT_TS_CURTIME_READ_REQ:
    {
      APPL_MESG_INFO(profiledbgfile,"EVT_TS_CURTIME_READ_REQ: call TimeServer_Allow_Curtime_Char_Read()\n");
      TimeServer_Allow_Curtime_Char_Read();
    }
    break;
    case EVT_TS_STOP_REFTIME_UPDATE:
    {
      APPL_MESG_INFO(profiledbgfile,"EVT_TS_STOP_REFTIME_UPDATE\n");
    }
    break;
  }
}
#endif    

