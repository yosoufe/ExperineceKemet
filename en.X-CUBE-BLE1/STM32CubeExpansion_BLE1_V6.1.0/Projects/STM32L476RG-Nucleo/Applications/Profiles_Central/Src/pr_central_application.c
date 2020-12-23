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
*   FILENAME        -  profile_ProximityReporter_central_test_application.c
*
*******************************************************************************
*
*   CHANGE HISTORY
*  ---------------------------
*
*   $Date$:      03/05/2015
*   $Revision$:  first version
*   $Author$:    AMS RF Application team
*   Comments:    Profiles PTS test application for Proximity Reporter
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
//#include <platform_config.h> 
#include "master_basic_profile.h"

#if (BLE_CURRENT_PROFILE_ROLES & PROXIMITY_REPORTER) 
#include <proximity_reporter.h>
#include <proximity_reporter_config.h>

/*******************************************************************************
 * Macros
*******************************************************************************/
/* PERIPHERAL peer address */
#define PERIPHERAL_PEER_ADDRESS PEER_ADDRESS //PTS_PERIPHERAL_ADDRESS or PEER_ADDRESS

/* application states */
#define APPL_UNINITIALIZED		(0x00) 
#define APPL_INIT_DONE			(0x01) 

/* Test configuration: PTS SW tools doesnt allow to test Proximity Reporter Central Role */

/* --------------------- User Inputs for Phone Alert Status Server PTS tests -----------------*/

/* General operations: discovery, connection, disconnection, bonding, clear security database */
#define APPL_DISCOVER_PEER_DEVICE     'a' // Start Discovery procedure for detecting a peer device
#define APPL_CONNECT_PEER_DEVICE      'b' // Start Connection procedure for connecting to the discovered  peer device
#define APPL_DISCONNECT_PEER_DEVICE   'c' // Terminate the current connection with the  peer device
#define APPL_PAIR_WITH_PEER_DEVICE    'd' // Start the pairing  procedure (or enable the encryption) with the connected  peer device
#define APPL_CLEAR_SECURITY_DATABASE  'e' // Clear the security database (it removes any bonded device) 

#define DISPLAY_PTS_MENU              '?' //Display PTS application command menu
/******************************************************************************
 * Local Variable Declarations
******************************************************************************/
uint8_t deviceState = APPL_UNINITIALIZED;

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/

void *profiledbgfile;

uint8_t status;

/******************************************************************************
 * Function Declarations
******************************************************************************/

/******************************************************************************
 * Function Definitions 
******************************************************************************/


void Display_Appl_Menu()
{
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_PEER_DEVICE: %c\n",APPL_DISCOVER_PEER_DEVICE);
  
  APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_PEER_DEVICE: %c\n",APPL_CONNECT_PEER_DEVICE);
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_PEER_DEVICE: %c\n",APPL_DISCONNECT_PEER_DEVICE);
  APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: %c\n",APPL_CLEAR_SECURITY_DATABASE);
  APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_PEER_DEVICE: %c\n",APPL_PAIR_WITH_PEER_DEVICE);
  
  APPL_MESG_DBG(profiledbgfile,"DISPLAY_PTS_MENU: %c\n",DISPLAY_PTS_MENU);          
    
}/* end Display_Appl_Menu() */

void Device_Init(void)
{
  pxrInitDevType InitDev; 
  uint8_t public_addr[6]=PR_PUBLIC_ADDRESS;
  uint8_t device_name[]=PR_DEVICE_NAME;
  tBleStatus ble_status;

  InitDev.public_addr =  public_addr;
  InitDev.txPower =      PR_TX_POWER_LEVEL;
  InitDev.device_name_len = sizeof(device_name);
  InitDev.device_name = device_name;
  
  /* Init Proximity Reporter Profile Central Role with 
     Immediate alert and TX power services*/
  InitDev.immAlertTxPowerServSupport = 1;
  
  ble_status = PXR_Init(InitDev);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_Init() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_Init() Error: %02X\n", ble_status);
  }
}

void Device_SetSecurity(void)
{
  pxrSecurityType param;
  tBleStatus ble_status;
  
  param.ioCapability =  PR_IO_CAPABILITY;
  param.mitm_mode =     PR_MITM_MODE;
  param.oob_enable =    PR_OOB_ENABLE;
  param.bonding_mode =  PR_BONDING_MODE;
  param.use_fixed_pin = PR_USE_FIXED_PIN;
  param.fixed_pin =     PR_FIXED_PIN;
  
  ble_status = PXR_SecuritySet(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_SecuritySet() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_SecuritySet() Error: %02X\n", ble_status);
  }
}

void Device_Discovery_Procedure(void)
{
  pxrDevDiscType param;
  tBleStatus ble_status;
  
  param.own_addr_type = PUBLIC_ADDR;
  param.scanInterval =  PR_LIM_DISC_SCAN_INT;
  param.scanWindow =    PR_LIM_DISC_SCAN_WIND;
  ble_status = PXR_DeviceDiscovery(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_DeviceDiscovery() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_DeviceDiscovery() Error: %02X\n", ble_status);
  }
}

void Device_Connection_Procedure(void)
{
  pxrConnDevType param;
  tBleStatus ble_status;
  uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS;

  param.fastScanDuration =         PR_FAST_SCAN_DURATION;
  param.fastScanInterval =         PR_FAST_SCAN_INTERVAL;
  param.fastScanWindow =           PR_FAST_SCAN_WINDOW;
  param.reducedPowerScanInterval = PR_REDUCED_POWER_SCAN_INTERVAL;
  param.reducedPowerScanWindow =   PR_REDUCED_POWER_SCAN_WINDOW;
  param.peer_addr_type =           PUBLIC_ADDR;
  param.peer_addr =                peer_addr;
  param.own_addr_type =            PUBLIC_ADDR;
  param.conn_min_interval =        PR_FAST_MIN_CONNECTION_INTERVAL;
  param.conn_max_interval =        PR_FAST_MAX_CONNECTION_INTERVAL;
  param.conn_latency =             PR_FAST_CONNECTION_LATENCY;
  param.supervision_timeout =      PR_SUPERVISION_TIMEOUT;
  param.min_conn_length =          PR_MIN_CONN_LENGTH;
  param.max_conn_length =          PR_MAX_CONN_LENGTH;
  ble_status = PXR_DeviceConnection(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_DeviceConnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_DeviceConnection() Error: %02X\n", ble_status);
  }
}
 
void Device_StartPairing(void)
{
  tBleStatus ble_status;
  ble_status = PXR_StartPairing();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_StartPairing() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_StartPairing() Error: %02X\n", ble_status);
  }
}

void Device_Disconnection(void)
{
  tBleStatus ble_status;
  ble_status = PXR_DeviceDisconnection();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_DeviceDisconnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_DeviceDisconnection() Error: %02X\n", ble_status);
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
  status =  PXR_Add_Services_Characteristics();
  if (status == BLE_STATUS_SUCCESS)
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_Add_Services_Characteristics() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"PXR_Add_Services_Characteristics() Error: %02X\n", status);
  }
  
  deviceState = APPL_INIT_DONE;
  while(1)
  {
    hci_user_evt_proc();
    
    /* process the timer Q */
    //Blue_NRG_Timer_Process_Q(); //TBR: timer not used
    
    /* Call the Profile Master role state machine */
    Master_Process();
    
    //PXR_StateMachine(); //TBR: PXR_StateMachine() not used

    /* if the profiles have nothing more to process, then
     * wait for application input
     */ 
    if(deviceState >= APPL_INIT_DONE) //TBR
    {      
      uint8_t input = APPL_DISCOVER_PEER_DEVICE;//200;
      deviceState = input;
      if (input>0)
      {
        APPL_MESG_DBG(profiledbgfile,"io--- input: %c\n",input); 

        switch(input)
        {   
          case DISPLAY_PTS_MENU:
             
          case APPL_DISCOVER_PEER_DEVICE:
          case APPL_CONNECT_PEER_DEVICE:
          case APPL_DISCONNECT_PEER_DEVICE:
          case APPL_PAIR_WITH_PEER_DEVICE:
          case APPL_CLEAR_SECURITY_DATABASE:
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
      
      case APPL_DISCOVER_PEER_DEVICE:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_PEER_DEVICE: call Device_Discovery_Procedure() \n"); 
        Device_Discovery_Procedure();
      }
      break;
      case APPL_CONNECT_PEER_DEVICE:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_PEER_DEVICE: call Device_Connection_Procedure() \n"); 
        Device_Connection_Procedure();
      }
      break;
      case APPL_DISCONNECT_PEER_DEVICE:
      {
         APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_PEER_DEVICE: call Device_Disconnection() \n"); 
         Device_Disconnection();
      }
      break;
      case APPL_PAIR_WITH_PEER_DEVICE:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_PEER_DEVICE: call Device_StartPairing() \n"); 
        Device_StartPairing();
      }
      break;
      case APPL_CLEAR_SECURITY_DATABASE:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: call Clear_Security_Database() \n"); 
        status = PXR_Clear_Security_Database();
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"PXR_Clear_Security_Database() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"PXR_Clear_Security_Database() Error: %02X\n", status);
        }
      }
      break; 
    }/* end switch(devicestate) */

  } /* end while(1) */
}/* end Host_Profile_Test_Application() */

/***************  Public Function callback ******************/

void PXR_DeviceDiscovery_CB(uint8_t status, uint8_t addr_type, uint8_t *addr, 
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

void PXR_ConnectionStatus_CB(uint8_t connection_evt, uint8_t status)
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

void PXR_PinCodeRequired_CB(void)
{
  uint8_t ret;
  uint32_t pinCode=0;

  APPL_MESG_INFO(profiledbgfile,"**** Required Security Pin Code\r\n");

  ret = PXR_SendPinCode(pinCode);
  if (ret != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"**** Error during the Pairing procedure -> Pin Code set (Status = 0x%x)\r\n", ret);
  }
}

void PXR_Pairing_CB(uint16_t conn_handle, uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"Pairing CB conn_handle = 0x%04x with status = 0x%02x***************\n", conn_handle, status);
  
}

/**
* @brief  User defined callback which is called each time an 
*         alert is raised: user specific implementation 
*         action should be done accordingly.
* @param  alert_type: alert type (path loss or link loss)
* @param  alert_value: alert value  
* @retval None
*/
extern void PXR_Alert_CB(uint8_t alert_type, uint8_t alert_value)
{
  /* To be customized for application purposes: 
     add implementation specific actions.
  */
  if (alert_type == EVT_PR_LINK_LOSS_ALERT)
  {
    APPL_MESG_INFO (profiledbgfile,"Link Loss Alert (%d)\n",alert_value);
    /* do user specific implementation action ..... */
  }
  else if (alert_type == EVT_PR_PATH_LOSS_ALERT)
  {
    switch (alert_value)
    {
      case (NO_ALERT):
      {
        APPL_MESG_INFO (profiledbgfile,"Path Loss Alert: NO_ALERT \n");
      }
      break;
      case (MILD_ALERT):
      {
        APPL_MESG_INFO (profiledbgfile,"Path Loss Alert: MILD_ALERT \n");
      }
      break;
      case (HIGH_ALERT):
      {
        APPL_MESG_INFO (profiledbgfile,"Path Loss Alert: HIGH_ALERT \n");
      }
      break;
      default:
      {
        APPL_MESG_INFO (profiledbgfile,"Path Loss Alert: invalid alert value (%d)\n",alert_value);
      }
      break;
    }
  }    
}/* end PXR_Alert_CB() */

#endif    

