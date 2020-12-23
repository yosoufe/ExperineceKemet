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
*   FILENAME        -  profile_FindMeTarget_central_test_application.c
*
*******************************************************************************
*
*   CHANGE HISTORY
*  ---------------------------
*
*   $Date$:      01/26/2015
*   $Revision$:  first version
*   $Author$:    AMS RF Application team
*   Comments:    Profiles PTS test application for Find Me Target 
*                Central role.
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

#if (BLE_CURRENT_PROFILE_ROLES & FIND_ME_TARGET)
#include <find_me_target.h>
#include <find_me_target_config.h>

/*******************************************************************************
 * Macros
*******************************************************************************/
/* PERIPHERAL peer address */
#define PERIPHERAL_PEER_ADDRESS PTS_PERIPHERAL_ADDRESS //PTS_PERIPHERAL_ADDRESS or PEER_ADDRESS
   
/* application states */
#define APPL_UNINITIALIZED		(0x00) //TBR
#define APPL_INIT_DONE			(0x01) //TBR


/* --------------------- User Inputs for Profile Central PTS tests -----------------*/

/* General operations: discovery, connection, disconnection, bonding, clear security database */
#define APPL_DISCOVER_FIND_ME_LOCATOR      'a' // Start Discovery procedure for detecting a device 
#define APPL_CONNECT_FIND_ME_LOCATOR       'b' // Start Connection procedure for connecting to the discovered device
#define APPL_DISCONNECT_FIND_ME_LOCATOR    'c' // Terminate the current connection with a device
#define APPL_PAIR_WITH_FIND_ME_LOCATOR     'd' // Start the pairing  procedure (or enable the encryption) with the connected device
#define APPL_CLEAR_SECURITY_DATABASE       'e' // Clear the security database (it removes any bonded device) 

/* 4.3	Service Definition 
   4.3.1 TP/SD/BV-01-C [Service Definition] 
*/

/* 4.4	Characteristic Declaration
   4.4.1 TP/DEC/BV-01-C [Characteristic Declaration – Alert Level] 
*/

/* 4.5	Characteristic Write without Response 
   4.5.1 TP/CW/BV-01-C [Characteristic Write without Response – Alert Level] 
*/

/* 4.6	Service Procedures
   4.6.1 TP/SP/BV-01-C [Writing Alert Level Behavior]
*/

#define APPL_GET_ALERT_LEVEL               'f' /* utility */
#define DISPLAY_PTS_MENU                   '?' //Display PTS application command menu
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
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_FIND_ME_LOCATOR: %c\n",APPL_DISCOVER_FIND_ME_LOCATOR);
  
  APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_FIND_ME_LOCATOR: %c\n",APPL_CONNECT_FIND_ME_LOCATOR);
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_FIND_ME_LOCATOR: %c\n",APPL_DISCONNECT_FIND_ME_LOCATOR);
  APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_FIND_ME_LOCATOR: %c\n",APPL_PAIR_WITH_FIND_ME_LOCATOR);
  APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: %c\n",APPL_CLEAR_SECURITY_DATABASE);
  
  APPL_MESG_DBG(profiledbgfile,"APPL_GET_ALERT_LEVEL: %c\n",APPL_GET_ALERT_LEVEL);
  
  APPL_MESG_DBG(profiledbgfile,"DISPLAY_PTS_MENU: %c\n",DISPLAY_PTS_MENU);          
    
}/* end Display_Appl_Menu() */
 
void Device_Init(void)
{
  fmtInitDevType InitDev; 
  uint8_t public_addr[6]=FMT_PUBLIC_ADDRESS;
  uint8_t device_name[]=FMT_DEVICE_NAME;
  tBleStatus ble_status;

  InitDev.public_addr =  public_addr;
  InitDev.txPower =      FMT_TX_POWER_LEVEL;
  InitDev.device_name_len = sizeof(device_name);
  InitDev.device_name = device_name;
  
  /* Init Profile Central Role */ 
  ble_status = FMT_Init(InitDev);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"FMT_Init() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FMT_Init() Error: %02X\n", ble_status);
  }
}

void Device_SetSecurity(void)
{
  fmtSecurityType param;
  tBleStatus ble_status;
  
  param.ioCapability =  FMT_IO_CAPABILITY;
  param.mitm_mode =     FMT_MITM_MODE;
  param.oob_enable =    FMT_OOB_ENABLE;
  param.bonding_mode =  FMT_BONDING_MODE;
  param.use_fixed_pin = FMT_USE_FIXED_PIN;
  param.fixed_pin =     FMT_FIXED_PIN;
  
  ble_status = FMT_SecuritySet(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"FMT_SecuritySet() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FMT_SecuritySet() Error: %02X\n", ble_status);
  }
}

void Device_Discovery_Procedure(void)
{
  fmtDevDiscType param;
  tBleStatus ble_status;
  
  param.own_addr_type = PUBLIC_ADDR;
  param.scanInterval =  FMT_LIM_DISC_SCAN_INT;
  param.scanWindow =    FMT_LIM_DISC_SCAN_WIND;
  ble_status = FMT_DeviceDiscovery(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"FMT_DeviceDiscovery() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FMT_DeviceDiscovery() Error: %02X\n", ble_status);
  }
}

void Device_Connection_Procedure(void)
{
  fmtConnDevType param;
  tBleStatus ble_status;
  uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS;

  param.fastScanDuration =         FMT_FAST_SCAN_DURATION;
  param.fastScanInterval =         FMT_FAST_SCAN_INTERVAL;
  param.fastScanWindow =           FMT_FAST_SCAN_WINDOW;
  param.reducedPowerScanInterval = FMT_REDUCED_POWER_SCAN_INTERVAL;
  param.reducedPowerScanWindow =   FMT_REDUCED_POWER_SCAN_WINDOW;
  param.peer_addr_type =           PUBLIC_ADDR;
  param.peer_addr =                peer_addr;
  param.own_addr_type =            PUBLIC_ADDR;
  param.conn_min_interval =        FMT_FAST_MIN_CONNECTION_INTERVAL;
  param.conn_max_interval =        FMT_FAST_MAX_CONNECTION_INTERVAL;
  param.conn_latency =             FMT_FAST_CONNECTION_LATENCY;
  param.supervision_timeout =      FMT_SUPERVISION_TIMEOUT;
  param.min_conn_length =          FMT_MIN_CONN_LENGTH;
  param.max_conn_length =          FMT_MAX_CONN_LENGTH;
  ble_status = FMT_DeviceConnection(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"FMT_DeviceConnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FMT_DeviceConnection() Error: %02X\n", ble_status);
  }
}
 
void Device_StartPairing(void)
{
  tBleStatus ble_status;
  ble_status = FMT_StartPairing();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"FMT_StartPairing() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FMT_StartPairing() Error: %02X\n", ble_status);
  }
}

void Device_Disconnection(void)
{
  tBleStatus ble_status;
  ble_status = FMT_DeviceDisconnection();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"FMT_DeviceDisconnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FMT_DeviceDisconnection() Error: %02X\n", ble_status);
  }
}


//TBR
void Host_Profile_Test_Application (void)
{
  deviceState = APPL_UNINITIALIZED;
  
  APPL_MESG_INFO(profiledbgfile,"\n Test Application: Starting testing the BLE Profiles (? for menu) \n" );
  
  /* Init Profile Central */
  Device_Init();
  /* Init Profile Security */
  Device_SetSecurity();
  
  /* Add device service & characteristics */
  status =  FMT_Add_Services_Characteristics();
  if (status == BLE_STATUS_SUCCESS)
  {
    FMT_DBG_MSG(profiledbgfile,"FMT_Add_Services_Characteristics() Call: OK\n" );
  }
  else
  {
    FMT_DBG_MSG(profiledbgfile,"FMT_Add_Services_Characteristics() Error: %02X\n", status);
  }
  
  /* Clear Security database: is it needed ?TBR */
  status = FMT_Clear_Security_Database(); 
  if (status == BLE_STATUS_SUCCESS)
  {
    FMT_DBG_MSG(profiledbgfile,"FMT_Clear_Security_Database() Call: OK\n" );
  }
  else
  {
    FMT_DBG_MSG(profiledbgfile,"FMT_Clear_Security_Database() Error: %02X\n", status);
  }
  
  deviceState = APPL_INIT_DONE;
  while(1)
  {
    hci_user_evt_proc();
    
    /* process the timer Q */
    //Blue_NRG_Timer_Process_Q(); //TBR: timer not used
    
    /* Call the Profile Master role state machine */
    Master_Process(&initParam);
    
    //FMT_StateMachine(); //TBR Is is needed?
  
    /* if the profiles have nothing more to process, then
     * wait for application input
     */ 
    if(deviceState >= APPL_INIT_DONE) //TBR
    {      
      uint8_t input = APPL_DISCOVER_FIND_ME_LOCATOR;//200;
      deviceState = input;
      if (input>0)
      {
        APPL_MESG_INFO(profiledbgfile,"io--- input: %c\n",input); 

        switch(input)
        {   
          case DISPLAY_PTS_MENU:
          case APPL_DISCOVER_FIND_ME_LOCATOR: 
          case APPL_CONNECT_FIND_ME_LOCATOR:   
          case APPL_DISCONNECT_FIND_ME_LOCATOR:   
          case APPL_PAIR_WITH_FIND_ME_LOCATOR: 
          case APPL_CLEAR_SECURITY_DATABASE:
          case APPL_GET_ALERT_LEVEL:
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
      
      case APPL_DISCOVER_FIND_ME_LOCATOR:
      {
        APPL_MESG_INFO(profiledbgfile,"APPL_DISCOVER_FIND_ME_LOCATOR: call Device_Discovery_Procedure() \n"); 
        Device_Discovery_Procedure();
      }
      break;
      case APPL_CONNECT_FIND_ME_LOCATOR:
      {
        APPL_MESG_INFO(profiledbgfile,"APPL_CONNECT_FIND_ME_LOCATOR: call Device_Connection_Procedure() \n"); 
        Device_Connection_Procedure();
      }
      break;
      case APPL_DISCONNECT_FIND_ME_LOCATOR:
      {
         APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_FIND_ME_LOCATOR: call Device_Disconnection() \n"); 
         Device_Disconnection();
      }
      break;
      case APPL_PAIR_WITH_FIND_ME_LOCATOR:
      {
        APPL_MESG_INFO(profiledbgfile,"APPL_PAIR_WITH_FIND_ME_LOCATOR: call Device_StartPairing() \n"); 
        Device_StartPairing();
      }
      break;
      case APPL_CLEAR_SECURITY_DATABASE:
      {
        APPL_MESG_INFO(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: call Clear_Security_Database() \n"); 
        status = FMT_Clear_Security_Database();
        if (status == BLE_STATUS_SUCCESS) 
        {
          FMT_DBG_MSG(profiledbgfile,"FMT_Clear_Security_Database() Call: OK\n" );
        }
        else
        {
          FMT_DBG_MSG(profiledbgfile,"FMT_Clear_Security_Database() Error: %02X\n", status);
        }
      }
      break; 
      
      case APPL_GET_ALERT_LEVEL: /* user utility: it could be used for checking current alert level value */
      {
        uint8_t level; 
        
        level = FMT_Get_Alert_Level_Value();
        APPL_MESG_INFO(profiledbgfile,"APPL_GET_ALERT_LEVEL: call FMT_Get_Alert_Level_Value() 0x%0X\n", level);
        
      }
      break;
    
    }/* end switch(devicestate) */

  } /* end while(1) */
}/* end Host_Profile_Test_Application() */

/***************  Public Function callback ******************/

void FMT_DeviceDiscovery_CB(uint8_t status, uint8_t addr_type, uint8_t *addr, 
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

void FMT_ConnectionStatus_CB(uint8_t connection_evt, uint8_t status)
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

void FMT_PinCodeRequired_CB(void)
{
  uint8_t ret;
  uint32_t pinCode=0;

  APPL_MESG_INFO(profiledbgfile,"**** Required Security Pin Code\r\n");

  /* Insert the pin code according the glucose collector indication */
  //pinCode = ....

  ret = FMT_SendPinCode(pinCode);
  if (ret != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"**** Error during the Pairing procedure -> Pin Code set (Status = 0x%x)\r\n", ret);
  }
}

void FMT_Pairing_CB(uint16_t conn_handle, uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"Pairing CB conn_handle = 0x%04x with status = 0x%02x***************\n", conn_handle, status);
}

/**
  * @brief  User Callback which is called each time alert level 
  *         value is received: user specific implementation 
  *         action should be done accordingly.
  * @param  None
  * @retval None
  */
void FMT_Alert_Level_Value_CB(uint8_t alert_level)
{
  switch(alert_level)
  {
    case (NO_ALERT):
      APPL_MESG_INFO(profiledbgfile,"Current Alert Level is NO_ALERT: 0x%0X\n",alert_level);
      /* do user specific implementation action ..... */
    break;
    case(MILD_ALERT):
      APPL_MESG_INFO(profiledbgfile,"Current Alert Level is MILD_ALERT: 0x%0X\n",alert_level);
      /* do user specific implementation action ..... */
    break;
        
    case(HIGH_ALERT):
      APPL_MESG_INFO(profiledbgfile,"Current Alert Level is HIGH_ALERT: 0x%0X\n",alert_level);
      /* do user specific implementation action ..... */
    break;
    default:
      APPL_MESG_INFO(profiledbgfile,"Current Alert Level is Unknown: 0x%0X\n",alert_level);
      /* do nothing ..... */
    break;
  }
}/* end FMT_Alert_Level_Value_CB() */

#endif    

