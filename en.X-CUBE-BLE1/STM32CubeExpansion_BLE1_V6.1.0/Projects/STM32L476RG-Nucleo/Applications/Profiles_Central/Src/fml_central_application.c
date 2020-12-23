/*******************************************************************************
*
* THE PRESENT CODE AIMS AT PROVIDING CUSTOMERS WITH CODING INFORMATION
* REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* (C) 2015 STMicroelectronics International NV
*
*   FILENAME        -  profile_FindMeLocator_central_test_application.c
*
*******************************************************************************
*
*   CHANGE HISTORY
*  ---------------------------
*
*   $Date$:      01/26/2015
*   $Revision$:  first version
*   $Author$:    AMS RF Application team
*   Comments:    Profiles PTS test application for Find Me Locator.
*                It uses the BlueNRG Host Profiles Library 
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

#include "uart_support.h"
#include <string.h>

#include <host_config.h>
#if (PLATFORM_WINDOWS == 1)
#include <Windows.h>
#endif
#include <stdio.h>
#include <debug.h>
#include <hci.h>
#include "master_basic_profile.h"

#if (BLE_CURRENT_PROFILE_ROLES & FIND_ME_LOCATOR)
#include <find_me_locator.h>
#include <find_me_locator_config.h>


/*******************************************************************************
 * Macros
*******************************************************************************/
/* PERIPHERAL peer address */
#define PERIPHERAL_PEER_ADDRESS PTS_PERIPHERAL_ADDRESS //PTS_PERIPHERAL_ADDRESS or PEER_ADDRESS

/* ---------- User Inputs for Profile Central role PTS tests -----------------*/

/* General operations: discovery, connection, disconnection, bonding, clear security database */
#define APPL_DISCOVER_FIND_ME_TARGET      'a' // Start Discovery procedure for detecting a find me target
#define APPL_CONNECT_FIND_ME_TARGET       'b' // Start Connection procedure for connecting to the discovered find me target
#define APPL_DISCONNECT_FIND_ME_TARGET    'c' // Terminate the current connection with the find me target
#define APPL_PAIR_WITH_FIND_ME_TARGET     'd' // Start the pairing  procedure (or enable the encryption) with the connected glucose sensor
#define APPL_CLEAR_SECURITY_DATABASE      'e' // Clear the security database (it removes any bonded device) 

/* ---------- 4.3: Discover Services and Characteristics tests */
#define APPL_DISCOVER_IAS_SERVICE                 'f' // 4.3.2 TP/FMS/FL/BV-02-I [Discover Immediate Alert Service using GA for LE]
#define APPL_DISCOVER_ALERT_LEVEL_CHARACTERISTIC  'g' // 4.3.3 	TP/FMS/FL/BV-03-I [Discover Alert Level Characteristic for Immediate Alert]
/* 4.4	Find Me Profile Features */
#define APPL_SET_ALERT                            'h' // 4.4.1 	TP/FMF/FL/BV-01-I [Alert Peer Device] 
#define APPL_CANCEL_ALERT                         'k' // 4.4.2 	TP/FMF/FL/BV-02-I [Cancel Alert on Peer Device]
// 4.4.3 	TP/FMF/FL/BV-03-I [Verify Bond Status on Reconnection] 

#define APPL_START_FULL_CONFIGURATION                            'Z' // Starts all the connection, service discovery, characteristic discovery, .....

#define DISPLAY_PTS_MENU                          '?' //Display PTS application command menu


/* --------------- Define values required for PTS test cases ------------------ */
/* Alert level values */ 
#define SET_ALERT    0x01 
#define CANCEL_ALERT 0x00

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/

extern tProfileApplContext profileApplContext; /* Profile Application Context */
extern initDevType initParam;

uint8_t status;

/******************************************************************************
 * Function Declarations
******************************************************************************/
extern void _Error_Handler(char *, int);

void Device_Init(void);
void Device_SetSecurity(void);
uint8_t Device_Discovery_Procedure(void);
void Device_Disconnection(void);
void FML_CP(uint8_t* params, uint8_t parmas_len);

/******************************************************************************
 * Function Definitions 
******************************************************************************/
/**
 * @brief  Initialize the Application Context
 * for using the FML central profile methods
 * @param  None
 * @retval None
 */
void FML_Appl_Context_Init(void)
{
  if (profileApplContext.initDeviceFunc == NULL)
  {
    profileApplContext.initDeviceFunc = Device_Init;
  }
  if (profileApplContext.deviceSetSecurityFunc == NULL)
  {
    profileApplContext.deviceSetSecurityFunc = Device_SetSecurity;
  }
  if (profileApplContext.deviceDiscoveryProcedureFunc == NULL)
  {
    profileApplContext.deviceDiscoveryProcedureFunc = Device_Discovery_Procedure;
  }
  if (profileApplContext.deviceDisconnectionFunc == NULL)
  {
    profileApplContext.deviceDisconnectionFunc = Device_Disconnection;
  }
  if (profileApplContext.cpFunc == NULL)
  {
    profileApplContext.cpFunc = FML_CP;
  }
  
  profileApplContext.deviceState = APPL_UNINITIALIZED;
  
  profileApplContext.deviceMasterInited = FALSE;
  
  profileApplContext.startDeviceConn = FALSE;
}

void printDebugServices(uint8_t numReporterServices, uint8_t *reporterServices)
{
  uint8_t i , j;
  uint16_t handle, uuid;
  uint8_t uuid_array[2];

  APPL_MESG_INFO(profiledbgfile,"****    Number of Primary Service: %d\r\n", numReporterServices);
  j = 0;
  for (i=0; i<numReporterServices; i++) {
    if (reporterServices[j] == UUID_TYPE_16) {
      BLUENRG_memcpy(&uuid, &reporterServices[j+5], 2);
      switch (uuid) {
      case IMMEDIATE_ALERT_SERVICE_UUID:
        PACK_2_BYTE_PARAM16(uuid_array, uuid);
        notify_uart(uuid_array, sizeof(uuid_array), SERVICE_UUID);
	APPL_MESG_INFO(profiledbgfile,"****    Immediate Service UUID found\r\n");
	break;
      default:
	APPL_MESG_INFO(profiledbgfile,"****    UUID 0x%04x\r\n", uuid);
      }
    }
    j++;
    BLUENRG_memcpy(&handle, &reporterServices[j], 2);
    APPL_MESG_INFO(profiledbgfile,"****       Start Handle = 0x%04x\r\n", handle);
    j += 2;
    BLUENRG_memcpy(&handle, &reporterServices[j], 2);
    APPL_MESG_INFO(profiledbgfile,"****       End Handle = 0x%04x\r\n", handle);
    j += 2;
    if (reporterServices[j-5] == UUID_TYPE_16) {
      j += 2;
    } else {
      j += 16;
    }
  }
}

void printDebugCharac(uint8_t numCharac, uint8_t *charac)
{
  uint8_t i, index, sizeElement, properties;
  uint16_t value_handle, charac_handle, uuid_charac;

  APPL_MESG_INFO(profiledbgfile,"****    Number of Charac of a Service: %d\r\n", numCharac);
  index = 0;
  for (i=0; i<numCharac; i++) { 
    if (charac[index] == 1) {
      sizeElement = 8;
    BLUENRG_memcpy(&uuid_charac, &charac[index+6], 2);
    } else {
      sizeElement = 22;
    }

    switch(uuid_charac) {
    case ALERT_LEVEL_CHARACTERISTIC_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Alert Level Characteristic found\r\n");
      break;
    
    default:
      APPL_MESG_INFO(profiledbgfile,"****      Characteristic found\r\n");
    }

    BLUENRG_memcpy(&charac_handle, &charac[index+1], 2);
    properties = charac[index+3];
    BLUENRG_memcpy(&value_handle, &charac[index+4], 2);
    APPL_MESG_INFO(profiledbgfile,"****      Characteristic Handle = 0x%04x\r\n", charac_handle);
    APPL_MESG_INFO(profiledbgfile,"****      Properties = 0x%02x\r\n", properties);
    APPL_MESG_INFO(profiledbgfile,"****      ValueHandle = 0x%04x\r\n", value_handle);
    APPL_MESG_INFO(profiledbgfile,"****      UUID = 0x%04x\r\n", uuid_charac);

    index += sizeElement;
  }
}


void Display_Appl_Menu()
{
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_FIND_ME_TARGET: %c\n",APPL_DISCOVER_FIND_ME_TARGET);
  APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_FIND_ME_TARGET: %c\n",APPL_CONNECT_FIND_ME_TARGET);
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_FIND_ME_TARGET: %c\n",APPL_DISCONNECT_FIND_ME_TARGET);
  APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_FIND_ME_TARGET: %c\n",APPL_PAIR_WITH_FIND_ME_TARGET);
  APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: %c\n",APPL_CLEAR_SECURITY_DATABASE);
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_IAS_SERVICE: %c\n",APPL_DISCOVER_IAS_SERVICE);
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_GL_MEASUREMENT_CONTEXT_CHAR_DESCRIPTOR: %c\n",APPL_DISCOVER_ALERT_LEVEL_CHARACTERISTIC);
  APPL_MESG_DBG(profiledbgfile,"APPL_SET_ALERT: %c\n",APPL_SET_ALERT);
  APPL_MESG_DBG(profiledbgfile,"APPL_CANCEL_ALERT: %c\n",APPL_CANCEL_ALERT);
   
  APPL_MESG_DBG(profiledbgfile,"APPL_START_FULL_CONFIGURATION: %c\n",APPL_START_FULL_CONFIGURATION);
  APPL_MESG_DBG(profiledbgfile,"DISPLAY_PTS_MENU: %c\n",DISPLAY_PTS_MENU);          
}/* end Display_Appl_Menu() */
 
void Device_Init(void)
{
  fmlInitDevType InitDev; 
  uint8_t public_addr[6]=FML_PUBLIC_ADDRESS;
  uint8_t device_name[]=FML_DEVICE_NAME;
  tBleStatus ble_status;

  InitDev.public_addr =  public_addr;
  InitDev.txPower =      FML_TX_POWER_LEVEL;
  InitDev.device_name_len = sizeof(device_name);
  InitDev.device_name = device_name;
  
  ble_status = FML_Init(InitDev);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"FML_Init() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Central initialized.\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FML_Init() Error: %02X\n", ble_status);
  }
}

void Device_SetSecurity(void)
{
  fmlSecurityType param;
  tBleStatus ble_status;
  
  param.ioCapability =  FML_IO_CAPABILITY;
  param.mitm_mode =     FML_MITM_MODE;
  param.oob_enable =    FML_OOB_ENABLE;
  param.bonding_mode =  FML_BONDING_MODE;
  param.use_fixed_pin = FML_USE_FIXED_PIN;
  param.fixed_pin =     FML_FIXED_PIN;
  
  ble_status = FML_SecuritySet(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"FML_SecuritySet() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Security params set.\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FML_SecuritySet() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_Discovery_Procedure(void)
{
  fmlDevDiscType param;
  tBleStatus ble_status;
  
  param.own_addr_type = PUBLIC_ADDR;
  param.scanInterval =  FML_LIM_DISC_SCAN_INT;
  param.scanWindow =    FML_LIM_DISC_SCAN_WIND;
  ble_status = FML_DeviceDiscovery(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"FML_DeviceDiscovery() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Scanning...\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FML_DeviceDiscovery() Error: %02X\n", ble_status);
  }
  return ble_status;
}

uint8_t Device_Connection_Procedure(uint8_t *peer_addr)
{
  fmlConnDevType param;
  tBleStatus ble_status;
  //uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS;

  param.fastScanDuration =         FML_FAST_SCAN_DURATION;
  param.fastScanInterval =         FML_FAST_SCAN_INTERVAL;
  param.fastScanWindow =           FML_FAST_SCAN_WINDOW;
  param.reducedPowerScanInterval = FML_REDUCED_POWER_SCAN_INTERVAL;
  param.reducedPowerScanWindow =   FML_REDUCED_POWER_SCAN_WINDOW;
  param.peer_addr_type =           PUBLIC_ADDR;
  param.peer_addr =                peer_addr;
  param.own_addr_type =            PUBLIC_ADDR;
  param.conn_min_interval =        FML_FAST_MIN_CONNECTION_INTERVAL;
  param.conn_max_interval =        FML_FAST_MAX_CONNECTION_INTERVAL;
  param.conn_latency =             FML_FAST_CONNECTION_LATENCY;
  param.supervision_timeout =      FML_SUPERVISION_TIMEOUT;
  param.min_conn_length =          FML_MIN_CONN_LENGTH;
  param.max_conn_length =          FML_MAX_CONN_LENGTH;
  ble_status = FML_DeviceConnection(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"FML_DeviceConnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FML_DeviceConnection() Error: %02X\n", ble_status);
  }
  return ble_status;
}
 
void Device_StartPairing(void)
{
  tBleStatus ble_status;
  ble_status = FML_StartPairing();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"FML_StartPairing() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FML_StartPairing() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_ServicesDiscovery(void)
{
  tBleStatus ble_status;
  ble_status = FML_ServicesDiscovery();

  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"FML_ServicesDiscovery() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FML_ServicesDiscovery() Error: %02X\n", ble_status);
  }
  return ble_status;
}

void Device_Disconnection(void)
{
  tBleStatus ble_status;
  ble_status = FML_DeviceDisconnection();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"FML_DeviceDisconnection() Call: OK\n" );
    notify_uart(&ble_status, sizeof(ble_status), CLOSE_CONNECTION);
    APPL_MESG_DBG(profiledbgfile,"Disconnection complete.\n" );
    profileApplContext.deviceState = APPL_INIT_DONE;
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"FML_DeviceDisconnection() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_Discovery_CharacServ(uint16_t uuid_service)
{
  tBleStatus ble_status;
  ble_status = FML_DiscCharacServ(uuid_service);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"Device_Discovery_CharacServ() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"Device_Discovery_CharacServ() Error: %02X, uuid_service: 0x%02X\n", ble_status,uuid_service);
  }
  return ble_status;
}

void Device_StartFullConfig(void)
{
  fmlConnDevType connParam;
  //fmlConfDevType confParam;
  uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS; 

  /* Connection Parameter */
  connParam.fastScanDuration =         FML_FAST_SCAN_DURATION;
  connParam.fastScanInterval =         FML_FAST_SCAN_INTERVAL;
  connParam.fastScanWindow =           FML_FAST_SCAN_WINDOW;
  connParam.reducedPowerScanInterval = FML_REDUCED_POWER_SCAN_INTERVAL;
  connParam.reducedPowerScanWindow =   FML_REDUCED_POWER_SCAN_WINDOW;
  connParam.peer_addr_type =           PUBLIC_ADDR;
  connParam.peer_addr =                peer_addr;
  connParam.own_addr_type =            PUBLIC_ADDR;
  connParam.conn_min_interval =        FML_FAST_MIN_CONNECTION_INTERVAL;
  connParam.conn_max_interval =        FML_FAST_MAX_CONNECTION_INTERVAL;
  connParam.conn_latency =             FML_FAST_CONNECTION_LATENCY;
  connParam.supervision_timeout =      FML_SUPERVISION_TIMEOUT;
  connParam.min_conn_length =          FML_MIN_CONN_LENGTH;
  connParam.max_conn_length =          FML_MAX_CONN_LENGTH;

  /* Configuration Parameter */

  if (FML_ConnConf(connParam) != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"Error in FML_ConnConf()\r\n");
  } 
  //else {
    //glucoseCollectorContext.useConnConf = TRUE;
  //}
}

void FML_CP(uint8_t* params, uint8_t parmas_len)
{
  uint8_t alert_state = params[0];
  
  status = FML_ALert_Target(alert_state);
  if (status!= BLE_STATUS_SUCCESS) {
    APPL_MESG_DBG(profiledbgfile,"Error in the FML_ALert_Target(%d) call: %02X\n", alert_state, status);
  } else {
    APPL_MESG_DBG(profiledbgfile,"Alert set successfully\n");
  }
}

//TBR
void Host_Profile_Test_Application (void)
{
    uint8_t ret;
  
  while(1)
  {
    HAL_StatusTypeDef uart_status;
    
    uart_status = HAL_UART_Receive(&UartHandle, (uint8_t *)uart_header, UARTHEADERSIZE, 10);
    if( uart_status != HAL_OK && uart_status != HAL_TIMEOUT)
    {
      _Error_Handler(__FILE__, __LINE__);
    }
    
    if (uart_status == HAL_OK) {
      // Process the command
      handle_uart_cmd(uart_header, UARTHEADERSIZE, PROFILE_CENTRAL);
    }
    
    if(profileApplContext.deviceMasterInited) {
      
      hci_user_evt_proc();

      /* Call the Profile Master role state machine */
      Master_Process(&initParam);
      
      if (findMeLocatorContext.fullConf) 
        FML_StateMachine();
      
      /* Start the connection procedure */
      if (profileApplContext.startDeviceConn) {
        ret = Device_Connection_Procedure(profileApplContext.peerAddr);
        notify_uart(&ret, sizeof(ret), CONNECTION_STATUS);
        profileApplContext.startDeviceConn = FALSE;
      }

      /* Services Discovery */
      if(profileApplContext.deviceState == APPL_CONNECTED){
        ret = Device_ServicesDiscovery();
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
        }
        profileApplContext.deviceState = APPL_INIT_DONE; // Transitory
      }
      
      /* Chars Discovery */
      if(profileApplContext.deviceState == APPL_SERVICES_DISCOVERED){
        ret = Device_Discovery_CharacServ(IMMEDIATE_ALERT_SERVICE_UUID);
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
        }
        profileApplContext.deviceState = APPL_INIT_DONE; // Transitory
      }
      
#if 0      
    /* if the profiles have nothing more to process, then
     * wait for application input
     */ 
    if(deviceState >= APPL_INIT_DONE) 
    {      
      uint8_t input = APPL_DISCOVER_FIND_ME_TARGET;//200;
      deviceState = input;
      if (input>0)
      {
        APPL_MESG_DBG(profiledbgfile,"io--- input: %c\n",input); 

        switch(input)
        {           
          case DISPLAY_PTS_MENU: 
          case APPL_DISCOVER_FIND_ME_TARGET:
          case APPL_CONNECT_FIND_ME_TARGET:
          case APPL_DISCONNECT_FIND_ME_TARGET:
          case APPL_PAIR_WITH_FIND_ME_TARGET:
          case APPL_CLEAR_SECURITY_DATABASE:
          case APPL_DISCOVER_IAS_SERVICE:
          case APPL_DISCOVER_ALERT_LEVEL_CHARACTERISTIC:
          case APPL_SET_ALERT:
          case APPL_CANCEL_ALERT:
          case APPL_START_FULL_CONFIGURATION:
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
      case APPL_DISCOVER_FIND_ME_TARGET:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_FIND_ME_TARGET: call Device_Discovery_Procedure() \n"); 
        Device_Discovery_Procedure();
      }
      break;
      case APPL_CONNECT_FIND_ME_TARGET:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_FIND_ME_TARGET: call Device_Connection_Procedure() \n"); 
        Device_Connection_Procedure();
      }
      break;
      case APPL_DISCONNECT_FIND_ME_TARGET:
      {
         FML_DBG_MSG(profiledbgfile,"APPL_DISCONNECT_FIND_ME_TARGET: call Device_Disconnection() \n"); 
         Device_Disconnection();
      }
      break;
      case APPL_PAIR_WITH_FIND_ME_TARGET:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_FIND_ME_TARGET: call Device_StartPairing() \n"); 
        Device_StartPairing();
      }
      break;
      case APPL_CLEAR_SECURITY_DATABASE:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: call Clear_Security_Database() \n"); 
        status = FML_Clear_Security_Database();
        if (status == BLE_STATUS_SUCCESS) 
        {
          FML_DBG_MSG(profiledbgfile,"FML_Clear_Security_Database() Call: OK\n" );
        }
        else
        {
          FML_DBG_MSG(profiledbgfile,"FML_Clear_Security_Database() Error: %02X\n", status);
        }
      }
      break; 
      case APPL_DISCOVER_IAS_SERVICE: /* 4.3.2 */
      {
        /* It discover all the primary services of the connected find me target device */
        APPL_MESG_DBG(profiledbgfile,"Call Device_ServicesDiscovery() x IAS Service\n"); 
        Device_ServicesDiscovery();
      }
      break;
      
      case APPL_DISCOVER_ALERT_LEVEL_CHARACTERISTIC: /* 4.3.3 */ 
      {
        /* It discovers all the characteristics of the connected device  */
        APPL_MESG_DBG(profiledbgfile,"Call Device_Discovery_CharacServ() x Immediate Alert Service\n"); 
        Device_Discovery_CharacServ(IMMEDIATE_ALERT_SERVICE_UUID);
      }
      break;
      
      case APPL_SET_ALERT: /* 4.4.1 */
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_SET_ALERT (%d)\n",SET_ALERT);
        /* Set alert  */
        status = FML_ALert_Target(SET_ALERT);
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the FML_ALert_Target(%d) call: %02X\n", SET_ALERT, status);
        }
      }
      break;
      case APPL_CANCEL_ALERT: /* 4.4.2 */
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_CANCEL_ALERT (%d)\n",CANCEL_ALERT);
        /* Cancel alert  */
        status = FML_ALert_Target(CANCEL_ALERT);
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the FML_ALert_Target(%d) call: %02X\n", CANCEL_ALERT, status);
        }
      }
      break;
       case APPL_START_FULL_CONFIGURATION:
        APPL_MESG_DBG(profiledbgfile,"Call Device_StartFullConfig()\n");
        Device_StartFullConfig();
      break;
    }/* end switch(devicestate) */
#endif /* 0 */
    }

  } /* end while(1) */
}/* end Host_Profile_Test_Application() */

/***************  Public Function callback ******************/

void FML_DeviceDiscovery_CB(uint8_t status, uint8_t addr_type, uint8_t *addr, 
                                     uint8_t data_length, uint8_t *data, 
                                     uint8_t RSSI)
{
  uint8_t i;
  uint8_t name_len = 12;
  uint8_t offset = name_len;

  /* Check if the device found is a FindMeTarget */
  if (!memcmp(&data[5], "FindMeTarget", name_len)) {
    
    // name + | + *addr_type + | + addr
    // 12    + 1 + 1          + 1 + 6 = 21
    uint8_t dev[21];
    strcpy((char*)dev, "FindMeTarget");
    dev[offset++] = '|';
    dev[offset++] = addr_type;
    dev[offset++] = '|';
    for (i=0; i<6; i++) {
      dev[offset+i] = addr[i];
    }
    notify_uart(dev, sizeof(dev), SCAN_RES);
  }
    
  if (status == DEVICE_DISCOVERY_PROCEDURE_TIMEOUT) {
    notify_uart(&status, sizeof(status), DISCOVERY_TIMEOUT);
    APPL_MESG_DBG(profiledbgfile,"Scanning complete.\n" );
  }
  
  /*
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
  */
}

void FML_ConnectionStatus_CB(uint8_t connection_evt, uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
  switch(connection_evt) {
  case CONNECTION_ESTABLISHED_EVT:
    if (status == BLE_STATUS_SUCCESS) {
      APPL_MESG_INFO(profiledbgfile,"****  Connection Established with Success\r\n");
      profileApplContext.deviceState = APPL_CONNECTED;
    }
    break;
  case CONNECTION_FAILED_EVT:
    APPL_MESG_INFO(profiledbgfile,"****  Connection Failed with Status = 0x%02x\r\n", status);
    break;
  case DISCONNECTION_EVT:
    // TBC
    if(profileApplContext.deviceState == APPL_CHARS_DISCOVERED) {
        tBleStatus ble_status = BLE_STATUS_SUCCESS;
        notify_uart(&ble_status, sizeof(ble_status), CLOSE_CONNECTION);
        APPL_MESG_INFO(profiledbgfile,"****  Disconnection with peer device\r\n");
    }
    break;
  }
  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
}

void FML_ServicesDiscovery_CB(uint8_t status, uint8_t numServices, uint8_t *services)
{
  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
  APPL_MESG_INFO(profiledbgfile,"**** Primary Service Discovery Procedure\r\n");
  APPL_MESG_INFO(profiledbgfile,"****    Status = 0x%02x\r\n", status);
  if (status == BLE_STATUS_SUCCESS) {
    printDebugServices(numServices, services);
    profileApplContext.deviceState = APPL_SERVICES_DISCOVERED;
  }
  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");  
}

void FML_CharacOfService_CB(uint8_t status, uint8_t numCharac, uint8_t *charac)
{
  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
  APPL_MESG_INFO(profiledbgfile,"**** Discovery Characteristics of a Service Procedure\r\n");
  APPL_MESG_INFO(profiledbgfile,"****    Status = 0x%02x\r\n", status);
  if (status == BLE_STATUS_SUCCESS) {
    printDebugCharac(numCharac, charac);
    profileApplContext.deviceState = APPL_CHARS_DISCOVERED;
  }
  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");  
}

void FML_FullConfError_CB(uint8_t error_type, uint8_t code)
{
  if (findMeLocatorContext.fullConf) {
    APPL_MESG_INFO(profiledbgfile,"***** FML_FullConf_CB() Error Type = 0x%0x with Code = 0x%0x\r\n", error_type, code);
  }
}

void FML_PinCodeRequired_CB(void)
{
  uint8_t ret;
  uint32_t pinCode=0;

  APPL_MESG_INFO(profiledbgfile,"**** Required Security Pin Code\r\n");

  /* Insert the pin code according the glucose collector indication */
  //pinCode = ....

  ret = FML_SendPinCode(pinCode);
  if (ret != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"**** Error during the Pairing procedure -> Pin Code set (Status = 0x%x)\r\n", ret);
  }
}

void FML_Pairing_CB(uint16_t conn_handle, uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"Pairing CB conn_handle = 0x%04x with status = 0x%02x***************\n", conn_handle, status);
  
}
#endif    

