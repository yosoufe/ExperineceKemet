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
*   FILENAME        -  profile_PhoneAlertStatusServer_central_test_application.c
*
*******************************************************************************
*
*   CHANGE HISTORY
*  ---------------------------
*
*   $Date$:      01/19/2015
*   $Revision$:  first version
*   $Author$:    AMS RF Application team
*   Comments:    Profiles PTS test application for Phone Alert Status Server
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

#if (BLE_CURRENT_PROFILE_ROLES & PHONE_ALERT_STATUS_SERVER)
#include <phone_alert_status_service.h>
#include <phone_alert_status_server.h>
#include <phone_alert_status_server_config.h>

/*******************************************************************************
 * Macros
*******************************************************************************/
/* PERIPHERAL peer address */
#define PERIPHERAL_PEER_ADDRESS PTS_PERIPHERAL_ADDRESS //PTS_PERIPHERAL_ADDRESS or PEER_ADDRESS

/* application states */
#define APPL_UNINITIALIZED		(0x00) //TBR
#define APPL_INIT_DONE			(0x01) //TBR

/* Phone Alert Status Server test configuration for some PTS tests */

/* --------------------- User Inputs for Phone Alert Status Server PTS tests -----------------*/

/* General operations: discovery, connection, disconnection, bonding, clear security database */
#define APPL_DISCOVER_PHONE_ALERT_STATUS_CLIENT      'a' // Start Discovery procedure for detecting a phone alert status client
#define APPL_CONNECT_PHONE_ALERT_STATUS_CLIENT       'b' // Start Connection procedure for connecting to the discovered phone alert status client
#define APPL_DISCONNECT_PHONE_ALERT_STATUS_CLIENT    'c' // Terminate the current connection with the phone alert status client
#define APPL_PAIR_WITH_PHONE_ALERT_STATUS_CLIENT     'd' // Start the pairing  procedure (or enable the encryption) with the connected phone alert status client
#define APPL_CLEAR_SECURITY_DATABASE                 'e' // Clear the security database (it removes any bonded device) 

/* 4.3	Service Definition	
   4.3.1	TP/SD/BV-01-C	[Service Definition] */

/* 4.4	Characteristic Declaration 
   4.4.1	TP/DEC/BV-01-C	[Characteristic Declaration – Alert Status] 
   4.4.2	TP/DEC/BV-02-C	[Characteristic Declaration – Ringer Setting] 
   4.4.3	TP/DEC/BV-03-C	[Characteristic Declaration – Ringer Control Point]
*/
/* 4.5	Characteristic Descriptor 
   4.5.1	TP/CDD/BV-01-C	[Alert Status - Client Characteristic Configuration Descriptor] 
   4.5.2	TP/CDD/BV-02-C	[Ringer Settings - Client Characteristic Configuration Descriptor]
*/
/* 4.6 	Configure Notification	
   4.6.1	TP/CCC/BV-01-C	[Configure Notification - Alert Status]
   4.6.2	TP/CCC/BV-02-C	[Configure Notification - Alert Status] 
   4.6.3	TP/CCC/BV-03-C	[Configure Notification – Ringer Setting]
   4.6.4	TP/CCC/BV-04-C	[Configure Notification – Ringer Setting]
*/
/* 4.7	Characteristic Read	
   4.7.1	TP/CR/BV-01-C	[Characteristic Read – Alert Status] 
   4.7.2	TP/CR/BV-02-C	[Characteristic Read – Ringer Setting] 
*/
/* 4.8  Characteristic Write Without Response 	
   4.8.1	TP/CW/BV-01-C	[Ringer Control Point] 
*/
/* 4.9	Service Procedures */	
#define APPL_SET_ALERT_STATUS_NO_ALERTS        'f' // 4.9.1 TP/SP/BV-01-C [Alert Status characteristic - Alert Status shows current status of the server]
#define APPL_SET_ALERT_STATUS_RINGER_STATE     'g' // 4.9.1 TP/SP/BV-01-C   
#define APPL_SET_ALERT_STATUS_DISPLAY_STATE    'h' // 4.9.1 TP/SP/BV-01-C
#define APPL_SET_ALERT_STATUS_VIBRATOR_STATE   'k' // 4.9.1 TP/SP/BV-01-C

/* 4.9.2 TP/SP/BV-02-C	[Alert Status characteristic - The server notifies the current alert status]: same user inputs as 4.9.1 */

                                       
#define APPL_SET_RINGER_SETTING_NORMAL         'i' //4.9.3	TP/SP/BV-04-C	[Ringer Setting characteristic – Show the current status] 
#define APPL_SET_RINGER_SETTING_SILENCE        'j' //4.9.3

/* 4.9.4	TP/SP/BV-05-C	[Ringer Setting characteristic – Notify the change for Ringer Setting]: same user inputs as 4.9.3 */

/* 4.9.5	TP/SP/BV-06-C	[Ringer Control Point characteristic – Receive the Set Silent Mode command] 
   4.9.6	TP/SP/BV-07-C	[Ringer Control Point characteristic – Receive the Cancel Silent Mode command]
   4.9.7	TP/SP/BV-08-C	[Ringer Control Point characteristic – Receive the Mute Once command]
   4.9.8	TP/SP/BI-01-C	[Ringer Control Point characteristic – Receive the unsupported command]
*/

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

/******************************************************************************
 * Function Declarations
******************************************************************************/

/******************************************************************************
 * Function Definitions 
******************************************************************************/


void Display_Appl_Menu()
{
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_PHONE_ALERT_STATUS_CLIENT: %c\n",APPL_DISCOVER_PHONE_ALERT_STATUS_CLIENT);
  
  APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_PHONE_ALERT_STATUS_CLIENT: %c\n",APPL_CONNECT_PHONE_ALERT_STATUS_CLIENT);
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_PHONE_ALERT_STATUS_CLIENT: %c\n",APPL_DISCONNECT_PHONE_ALERT_STATUS_CLIENT);
  APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: %c\n",APPL_CLEAR_SECURITY_DATABASE);
  APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_PHONE_ALERT_STATUS_CLIENT: %c\n",APPL_PAIR_WITH_PHONE_ALERT_STATUS_CLIENT);
  
  APPL_MESG_DBG(profiledbgfile,"APPL_SET_ALERT_STATUS_NO_ALERTS: %c\n",APPL_SET_ALERT_STATUS_NO_ALERTS);
  APPL_MESG_DBG(profiledbgfile,"APPL_SET_ALERT_STATUS_RINGER_STATE: %c\n",APPL_SET_ALERT_STATUS_RINGER_STATE);
  APPL_MESG_DBG(profiledbgfile,"APPL_SET_ALERT_STATUS_DISPLAY_STATE: %c\n",APPL_SET_ALERT_STATUS_DISPLAY_STATE);
  APPL_MESG_DBG(profiledbgfile,"APPL_SET_ALERT_STATUS_VIBRATOR_STATE: %c\n",APPL_SET_ALERT_STATUS_VIBRATOR_STATE);
  APPL_MESG_DBG(profiledbgfile,"APPL_SET_RINGER_SETTING_NORMAL: %c\n",APPL_SET_RINGER_SETTING_NORMAL);
  APPL_MESG_DBG(profiledbgfile,"APPL_SET_RINGER_SETTING_SILENCE: %c\n",APPL_SET_RINGER_SETTING_SILENCE);
  
  APPL_MESG_DBG(profiledbgfile,"DISPLAY_PTS_MENU: %c\n",DISPLAY_PTS_MENU);          
    
}/* end Display_Appl_Menu() */

void Device_Init(void)
{
  passInitDevType InitDev; 
  uint8_t public_addr[6]=PASS_PUBLIC_ADDRESS;
  uint8_t device_name[]=PASS_DEVICE_NAME;
  tBleStatus ble_status;

  InitDev.public_addr =  public_addr;
  InitDev.txPower =      PASS_TX_POWER_LEVEL;
  InitDev.device_name_len = sizeof(device_name);
  InitDev.device_name = device_name;
  
 /* Init Phone Alert Status Server Profile Central Role with ALERT_STATUS_RINGER_VIBRATOR_DISPLAY,
     ALERT_STATUS_RINGER_SETTING_NORMAL*/
  InitDev.alert_status_init_value = ALERT_STATUS_RINGER_VIBRATOR_DISPLAY;
  InitDev.ringer_setting_init_value = ALERT_STATUS_RINGER_SETTING_NORMAL; 
  
  ble_status = PASS_Init(InitDev);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_Init() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_Init() Error: %02X\n", ble_status);
  }
}

void Device_SetSecurity(void)
{
  passSecurityType param;
  tBleStatus ble_status;
  
  param.ioCapability =  PASS_IO_CAPABILITY;
  param.mitm_mode =     PASS_MITM_MODE;
  param.oob_enable =    PASS_OOB_ENABLE;
  param.bonding_mode =  PASS_BONDING_MODE;
  param.use_fixed_pin = PASS_USE_FIXED_PIN;
  param.fixed_pin =     PASS_FIXED_PIN;
  
  ble_status = PASS_SecuritySet(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_SecuritySet() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_SecuritySet() Error: %02X\n", ble_status);
  }
}

void Device_Discovery_Procedure(void)
{
  passDevDiscType param;
  tBleStatus ble_status;
  
  param.own_addr_type = PUBLIC_ADDR;
  param.scanInterval =  PASS_LIM_DISC_SCAN_INT;
  param.scanWindow =    PASS_LIM_DISC_SCAN_WIND;
  ble_status = PASS_DeviceDiscovery(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_DeviceDiscovery() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_DeviceDiscovery() Error: %02X\n", ble_status);
  }
}

void Device_Connection_Procedure(void)
{
  passConnDevType param;
  tBleStatus ble_status;
  uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS;

  param.fastScanDuration =         PASS_FAST_SCAN_DURATION;
  param.fastScanInterval =         PASS_FAST_SCAN_INTERVAL;
  param.fastScanWindow =           PASS_FAST_SCAN_WINDOW;
  param.reducedPowerScanInterval = PASS_REDUCED_POWER_SCAN_INTERVAL;
  param.reducedPowerScanWindow =   PASS_REDUCED_POWER_SCAN_WINDOW;
  param.peer_addr_type =           PUBLIC_ADDR;
  param.peer_addr =                peer_addr;
  param.own_addr_type =            PUBLIC_ADDR;
  param.conn_min_interval =        PASS_FAST_MIN_CONNECTION_INTERVAL;
  param.conn_max_interval =        PASS_FAST_MAX_CONNECTION_INTERVAL;
  param.conn_latency =             PASS_FAST_CONNECTION_LATENCY;
  param.supervision_timeout =      PASS_SUPERVISION_TIMEOUT;
  param.min_conn_length =          PASS_MIN_CONN_LENGTH;
  param.max_conn_length =          PASS_MAX_CONN_LENGTH;
  ble_status = PASS_DeviceConnection(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_DeviceConnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_DeviceConnection() Error: %02X\n", ble_status);
  }
}
 
void Device_StartPairing(void)
{
  tBleStatus ble_status;
  ble_status = PASS_StartPairing();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_StartPairing() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_StartPairing() Error: %02X\n", ble_status);
  }
}

void Device_Disconnection(void)
{
  tBleStatus ble_status;
  ble_status = PASS_DeviceDisconnection();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_DeviceDisconnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_DeviceDisconnection() Error: %02X\n", ble_status);
  }
}

void Host_Profile_Test_Application (void)
{
  deviceState = APPL_UNINITIALIZED;
  
  APPL_MESG_INFO(profiledbgfile,"\n Test Application: Starting testing the BLE Profiles (? for menu) \n" );
  
  /* Init Phone Alert Status Server Profile Central Role with ALERT_STATUS_RINGER_VIBRATOR_DISPLAY,
     ALERT_STATUS_RINGER_SETTING_NORMAL*/
  Device_Init();

  /* Init Phone Alert Status Server Profile Security */
  Device_SetSecurity();
  
  /* Add device service & characteristics */
  status =  PASS_Add_Services_Characteristics();
  if (status == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_Add_Services_Characteristics() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PASS_Add_Services_Characteristics() Error: %02X\n", status);
  }
  
  deviceState = APPL_INIT_DONE;
  while(1)
  {
    hci_user_evt_proc();
    
    /* process the timer Q */
    //Blue_NRG_Timer_Process_Q(); //TBR: timer not used
    
    /* Call the Profile Master role state machine */
    Master_Process(&initParam);
    
    //PASS_StateMachine(); //TBR: PASS_StateMachine() not used

    /* if the profiles have nothing more to process, then
     * wait for application input
     */ 
    if(deviceState >= APPL_INIT_DONE) //TBR
    {      
      uint8_t input = APPL_DISCOVER_PHONE_ALERT_STATUS_CLIENT;//200;
      deviceState = input;
      if (input>0)
      {
        APPL_MESG_DBG(profiledbgfile,"io--- input: %c\n",input); 

        switch(input)
        {   
          case DISPLAY_PTS_MENU:
             
          case APPL_DISCOVER_PHONE_ALERT_STATUS_CLIENT:
          case APPL_CONNECT_PHONE_ALERT_STATUS_CLIENT:
          case APPL_DISCONNECT_PHONE_ALERT_STATUS_CLIENT:
          case APPL_PAIR_WITH_PHONE_ALERT_STATUS_CLIENT:
          case APPL_CLEAR_SECURITY_DATABASE:
            
          case APPL_SET_ALERT_STATUS_NO_ALERTS: 
          case APPL_SET_ALERT_STATUS_RINGER_STATE:
          case APPL_SET_ALERT_STATUS_DISPLAY_STATE:
          case APPL_SET_ALERT_STATUS_VIBRATOR_STATE:
            
          //case APPL_SEND_ALERT_STATUS_CHAR_VALUE: //TBR
          case APPL_SET_RINGER_SETTING_NORMAL: 
          case APPL_SET_RINGER_SETTING_SILENCE:
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
      
      case APPL_DISCOVER_PHONE_ALERT_STATUS_CLIENT:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_PHONE_ALERT_STATUS_CLIENT: call Device_Discovery_Procedure() \n"); 
        Device_Discovery_Procedure();
      }
      break;
      case APPL_CONNECT_PHONE_ALERT_STATUS_CLIENT:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_PHONE_ALERT_STATUS_CLIENT: call Device_Connection_Procedure() \n"); 
        Device_Connection_Procedure();
      }
      break;
      case APPL_DISCONNECT_PHONE_ALERT_STATUS_CLIENT:
      {
         APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_PHONE_ALERT_STATUS_CLIENT: call Device_Disconnection() \n"); 
         Device_Disconnection();
      }
      break;
      case APPL_PAIR_WITH_PHONE_ALERT_STATUS_CLIENT:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_PHONE_ALERT_STATUS_CLIENT: call Device_StartPairing() \n"); 
        Device_StartPairing();
      }
      break;
      case APPL_CLEAR_SECURITY_DATABASE:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: call Clear_Security_Database() \n"); 
        status = PASS_Clear_Security_Database();
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"PASS_Clear_Security_Database() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"PASS_Clear_Security_Database() Error: %02X\n", status);
        }
      }
      break; 
      
      case APPL_SET_ALERT_STATUS_NO_ALERTS: /* 4.9.1 */
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_SET_ALERT_STATUS_NO_ALERTS: call PASS_Set_AlertStatus_Value(ALERT_STATUS_NO_ALERTS) \n"); 
        
        /* It sets the alert status value to no alerts */
        status = PASS_Set_AlertStatus_Value(ALERT_STATUS_NO_ALERTS);
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"PASS_Set_AlertStatus_Value() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"Error during the PASS_Set_AlertStatus_Value() call %02X\n", status);
        }
      }
      break;
      case APPL_SET_ALERT_STATUS_RINGER_STATE: /* 4.9.1 */
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_SET_ALERT_STATUS_RINGER_STATE: call PASS_Set_AlertStatus_Value(ALERT_STATUS_BIT_RINGER_STATE)\n"); 
        
        /* It sets the alert status value to ringer state */
        status = PASS_Set_AlertStatus_Value(ALERT_STATUS_BIT_RINGER_STATE);
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"PASS_Set_AlertStatus_Value() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"Error during the PASS_Set_AlertStatus_Value() call %02X\n", status);
        }
      }
      break;
      
      case APPL_SET_ALERT_STATUS_DISPLAY_STATE: /* 4.9.1 */
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_SET_ALERT_STATUS_DISPLAY_STATE: call PASS_Set_AlertStatus_Value(ALERT_STATUS_BIT_DISPLAY_ALERT_STATUS_STATE) \n"); 
        
        /* It sets the alert status value to display state */
        status = PASS_Set_AlertStatus_Value(ALERT_STATUS_BIT_DISPLAY_ALERT_STATUS_STATE);
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"PASS_Set_AlertStatus_Value() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"Error during the PASS_Set_AlertStatus_Value() call %02X\n", status);
        }
      }
      break;
      
      case APPL_SET_ALERT_STATUS_VIBRATOR_STATE: /* 4.9.1 */
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_SET_ALERT_STATUS_VIBRATOR_STATE: call PASS_Set_AlertStatus_Value(ALERT_STATUS_BIT_VIBRATOR_STATE) \n"); 
        
        /* It sets the alert status value to vibrator state */
        status = PASS_Set_AlertStatus_Value(ALERT_STATUS_BIT_VIBRATOR_STATE);
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"PASS_Set_AlertStatus_Value() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"Error during the PASS_Set_AlertStatus_Value() call %02X\n", status);
        }
      }
      break;
        
      case APPL_SET_RINGER_SETTING_NORMAL: /* 4.9.3 */
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_SET_RINGER_SETTING_NORMAL: call PASS_Set_RingerSetting_Value(ALERT_STATUS_RINGER_SETTING_NORMAL) \n"); 
        
        /* It sets the ringer setting value to normal state */
        status = PASS_Set_RingerSetting_Value(ALERT_STATUS_RINGER_SETTING_NORMAL);
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"PASS_Set_RingerSetting_Value() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"Error during the PASS_Set_RingerSetting_Value() call %02X\n", status);
        }
      }
      break;
      
      case APPL_SET_RINGER_SETTING_SILENCE: /* 4.9.3 */
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_SET_RINGER_SETTING_NORMAL: call PASS_Set_RingerSetting_Value(ALERT_STATUS_RINGER_SETTING_SILENT) \n"); 
        
        /* It sets the ringer setting value to silence state */
        status = PASS_Set_RingerSetting_Value(ALERT_STATUS_RINGER_SETTING_SILENT);
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"PASS_Set_RingerSetting_Value() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"Error during the PASS_Set_RingerSetting_Value() call %02X\n", status);
        }
      }
      break;

    }/* end switch(devicestate) */

  } /* end while(1) */
}/* end Host_Profile_Test_Application() */

/***************  Public Function callback ******************/

void PASS_DeviceDiscovery_CB(uint8_t status, uint8_t addr_type, uint8_t *addr, 
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

void PASS_ConnectionStatus_CB(uint8_t connection_evt, uint8_t status)
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

void PASS_PinCodeRequired_CB(void)
{
  uint8_t ret;
  uint32_t pinCode=0;

  APPL_MESG_INFO(profiledbgfile,"**** Required Security Pin Code\r\n");

  /* Insert the pin code according the glucose collector indication */
  //pinCode = ....

  ret = PASS_SendPinCode(pinCode);
  if (ret != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"**** Error during the Pairing procedure -> Pin Code set (Status = 0x%x)\r\n", ret);
  }
}

void PASS_Pairing_CB(uint16_t conn_handle, uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"Pairing CB conn_handle = 0x%04x with status = 0x%02x***************\n", conn_handle, status);
  
}

/**
* @brief  User defined callback which is called each time a 
*         Alert Control Point arrives: user specific implementation 
*         action should be done accordingly.
* @param  alert_control_point_value: received alert level control point 
* @retval None
*/
void PASS_Ringer_State_CB(uint8_t alert_control_point_value)
{
  /* To be customized for application purposes: 
     add implementation specific actions.
  */
  if (phone_alert_status_server_context.mute_once_silence)
  {
    APPL_MESG_INFO (profiledbgfile,"Mute once silence has been set ....\n");
    /* do user specific implementation action ..... */
  }
  else if (alert_control_point_value == ALERT_STATUS_RINGER_CONTROL_POINT_SILENT_MODE)
  {
    APPL_MESG_INFO (profiledbgfile,"Ringer setting state value is set to silent ...\n");
    /* do user specific implementation action ..... */
  }
  else if (alert_control_point_value == ALERT_STATUS_RINGER_CONTROL_POINT_CANCEL_SILENT_MODE)
  {
    APPL_MESG_INFO (profiledbgfile,"Ringer setting state value is set to normal ...\n");
    /* do user specific implementation action ..... */
  }
  else
  {
    APPL_MESG_INFO (profiledbgfile,"Received Unsupported Ringer Control Point Command \n");
    /* do nothing ... */
  }
}/* end PASS_Ringer_State_CB() */

#endif    

