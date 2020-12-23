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
*   FILENAME        -  profile_TimeClient_central_test_application.c
*
*******************************************************************************
*
*   CHANGE HISTORY
*  ---------------------------
*
*   $Date$:      01/07/2015
*   $Revision$:  first version
*   $Author$:    AMS RF Application team
*   Comments:    Profiles PTS test application for Time Client Central role
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

#if (BLE_CURRENT_PROFILE_ROLES & TIME_CLIENT)
#include <time_client_config.h>
#include <time_client.h>

/*******************************************************************************
 * Macros
*******************************************************************************/
/* PERIPHERAL peer address */
#define PERIPHERAL_PEER_ADDRESS PTS_PERIPHERAL_ADDRESS //PTS_PERIPHERAL_ADDRESS //PTS_PERIPHERAL_ADDRESS or PEER_ADDRESS

/* --------------------- User Inputs for Time Client PTS tests -----------------*/

/* General operations: discovery, connection, disconnection, bonding, clear security database */
#define APPL_DISCOVER_TIME_SERVER                            'a' // Start Discovery procedure for detecting a time server
#define APPL_CONNECT_TIME_SERVER                             'b' // Start Connection procedure for connecting to the discovered device
#define APPL_DISCONNECT_TIME_SERVER                          'c' // Terminate the current connection with the device
#define APPL_PAIR_WITH_TIME_SERVER                           'd' // Start the pairing  procedure (or enable the encryption) with the connected device
#define APPL_CLEAR_SECURITY_DATABASE                         'e' // Clear the security database (it removes any bonded device) 

/* ---------- 4.3: Discover Services and Characteristics tests */
#define APPL_DISCOVER_TIME_SERVICES             'f'    // TP/TPD/CO/BV-01-I [Discover Current Time]
                                                       // TP/TPD/CO/BV-02-I [Discover next DST  Service]
                                                       // TP/TPD/CO/BV-03-I [Discover RTU  Service]

#define APPL_DISCOVER_CTS_CHARACTERISTICS       'h' // [Discover Current Time Service Characteristic]
                                                                  
#define APPL_DISCOVER_CT_CHAR_DESCRIPTOR        'k' // [Discover Current Time Characteristic  – Client Characteristic Configuration Descriptor]

#define APPL_DISCOVER_NEXT_DST_CHARACTERISTICS  'l' // [Discover next DST Service Characteristics]

#define APPL_DISCOVER_RTU_CHARACTERISTICS       'm' // [Discover RTU Service Characteristics]

#define APPL_GET_REF_TIME_UPDATE		'o' // Starts a gatt write without response procedure to write the time update control point characteristic  (0x01)
#define APPL_TIME_UPDATE_NOTIFICATION		'p' // Enable Client Characteristic Configuration Descriptor for Current time characteristic


#define APPL_READ_LOCAL_TIME_INFORM		'q' // Read local time characteristic (current time service)
#define APPL_READ_CURRENT_TIME			'r' // Read current time characteristic (current time service)
#define APPL_GET_REF_TIME_INFO_ON_SERVER	's' // Read reference time characteristic (current time service)
#define APPL_READ_NEXT_DST_CHANGE_TIME 		't' // Read time with DST characteristic (next DST change service)
#define APPL_GET_SERV_TIME_UPDATE_STATE	        'u' // Read server time update state (RTU service)
#define APPL_CANCEL_REF_TIME_UPDATE		'v' //Starts a gatt write without response procedure to write the time update control point characteristic  on the server (0x02)

#define APPL_START_FULL_CONFIGURATION           'Z' // Starts all the connection, service discovery, characteristic discovery, .....
#define DISPLAY_PTS_MENU                        '?' //Display PTS application command menu

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

/******************************************************************************
 * Function Definitions 
******************************************************************************/
extern void _Error_Handler(char *, int);

void Device_Init(void);
void Device_SetSecurity(void);
uint8_t Device_Discovery_Procedure(void);
void Device_Disconnection(void);

/******************************************************************************
 * Function Definitions 
******************************************************************************/
/**
 * @brief  Initialize the Application Context
 * for using the TC central profile methods
 * @param  None
 * @retval None
 */
void TC_Appl_Context_Init(void)
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
      case CURRENT_TIME_SERVICE_UUID:
        PACK_2_BYTE_PARAM16(uuid_array, uuid);
        notify_uart(uuid_array, sizeof(uuid_array), SERVICE_UUID);
	APPL_MESG_INFO(profiledbgfile,"****    Current Time UUID found\r\n");
	break;
      case NEXT_DST_CHANGE_SERVICE_UUID:
	APPL_MESG_INFO(profiledbgfile,"****    Next DST Change  UUID found\r\n");
	break;
      case REFERENCE_UPDATE_TIME_SERVICE_UUID:
	APPL_MESG_INFO(profiledbgfile,"****    Reference Time Update UUID found\r\n");
	break;
      default:
	APPL_MESG_INFO(profiledbgfile,"****    UUID 0x%04x\r\n", uuid);
        break;
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
    case CURRENT_TIME_CHAR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Current Time Characteristic found\r\n");
      break;
    case LOCAL_TIME_INFORMATION_CHAR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Local Time Information Characteristic found\r\n");
      break;
    case REFERENCE_TIME_INFORMATION_CHAR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Reference Time Information Characteristic found\r\n");
      break;
      
    case TIME_WITH_DST_CHAR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Time with DST Characteristic found\r\n");
      break;  
    
    case TIME_UPDATE_CONTROL_POINT_CHAR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Time Update COntrol Point Characteristic found\r\n");
      break;
   
    case TIME_UPDATE_STATE_CHAR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Time Update State Characteristic found\r\n");
      break;
        
    default:
      APPL_MESG_INFO(profiledbgfile,"****      Characteristic found\r\n");
      break;
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

void printDebugCharacDesc(uint8_t numCharac, uint8_t *charac)
{
  uint8_t i, index, sizeElement;
  uint16_t handle_characDesc, uuid_characDesc;

  APPL_MESG_INFO(profiledbgfile,"****    Number of Charac Descriptor: %d\r\n", numCharac);
  index = 0;
  for (i=0; i<numCharac; i++) { 
    if (charac[index] == 1) {
      sizeElement = 5;
      BLUENRG_memcpy(&uuid_characDesc, &charac[index+3], 2);
    } else {
      sizeElement = 19;
    }

    BLUENRG_memcpy(&handle_characDesc, &charac[index+1], 2);

    if (CHAR_PRESENTATION_FORMAT_DESCRIPTOR_UUID) {
    }
    switch(uuid_characDesc) {
    case CHAR_PRESENTATION_FORMAT_DESCRIPTOR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Characteristic Presentation Format Descriptor found\r\n");
      break;
    case CLIENT_CHAR_CONFIG_DESCRIPTOR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Client Characteristic Configuration Descriptor found\r\n");
      break;
    default:
      APPL_MESG_INFO(profiledbgfile,"****      Characteristic Descriptor found\r\n");
      break;
    }

    APPL_MESG_INFO(profiledbgfile,"****      Characteristic Descriptor Handle = 0x%04x\r\n", handle_characDesc);
    APPL_MESG_INFO(profiledbgfile,"****      UUID = 0x%04x\r\n", uuid_characDesc);

    index += sizeElement;
  }
}

void Display_Appl_Menu()
{
  APPL_MESG_INFO(profiledbgfile,"APPL_DISCOVER_TIME_SERVER: %c\n",APPL_DISCOVER_TIME_SERVER);
  APPL_MESG_INFO(profiledbgfile,"APPL_CONNECT_TIME_SERVER: %c\n",APPL_CONNECT_TIME_SERVER);
  APPL_MESG_INFO(profiledbgfile,"APPL_DISCONNECT_TIME_SERVER: %c\n",APPL_DISCONNECT_TIME_SERVER);
  APPL_MESG_INFO(profiledbgfile,"APPL_PAIR_WITH_TIME_SERVER: %c\n",APPL_PAIR_WITH_TIME_SERVER);
  APPL_MESG_INFO(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: %c\n",APPL_CLEAR_SECURITY_DATABASE);
  
  APPL_MESG_INFO(profiledbgfile,"APPL_DISCOVER_TIME_SERVICES : %c\n",APPL_DISCOVER_TIME_SERVICES);   
  APPL_MESG_INFO(profiledbgfile,"APPL_DISCOVER_CTS_CHARACTERISTICS : %c\n",APPL_DISCOVER_CTS_CHARACTERISTICS);   
  APPL_MESG_INFO(profiledbgfile,"APPL_DISCOVER_CT_CHAR_DESCRIPTOR : %c\n",APPL_DISCOVER_CT_CHAR_DESCRIPTOR);   
  APPL_MESG_INFO(profiledbgfile,"APPL_DISCOVER_NEXT_DST_CHARACTERISTICS : %c\n",APPL_DISCOVER_NEXT_DST_CHARACTERISTICS);
  APPL_MESG_INFO(profiledbgfile,"APPL_DISCOVER_RTU_CHARACTERISTICS : %c\n",APPL_DISCOVER_RTU_CHARACTERISTICS);  
  APPL_MESG_INFO(profiledbgfile,"APPL_GET_REF_TIME_UPDATE : %c\n",APPL_GET_REF_TIME_UPDATE);   
  APPL_MESG_INFO(profiledbgfile,"APPL_TIME_UPDATE_NOTIFICATION : %c\n",APPL_TIME_UPDATE_NOTIFICATION);   
  APPL_MESG_INFO(profiledbgfile,"APPL_READ_LOCAL_TIME_INFORM : %c\n",APPL_READ_LOCAL_TIME_INFORM);   
  APPL_MESG_INFO(profiledbgfile,"APPL_READ_CURRENT_TIME : %c\n",APPL_READ_CURRENT_TIME);   
  APPL_MESG_INFO(profiledbgfile,"APPL_GET_REF_TIME_INFO_ON_SERVER : %c\n",APPL_GET_REF_TIME_INFO_ON_SERVER);   
  APPL_MESG_INFO(profiledbgfile,"APPL_READ_NEXT_DST_CHANGE_TIME : %c\n",APPL_READ_NEXT_DST_CHANGE_TIME); 
  APPL_MESG_INFO(profiledbgfile,"APPL_GET_SERV_TIME_UPDATE_STATE: %c\n",APPL_GET_SERV_TIME_UPDATE_STATE); 
  APPL_MESG_INFO(profiledbgfile,"APPL_CANCEL_REF_TIME_UPDATE : %c\n",APPL_CANCEL_REF_TIME_UPDATE);  
  APPL_MESG_INFO(profiledbgfile,"APPL_START_FULL_CONFIGURATION : %c\n",APPL_START_FULL_CONFIGURATION);  
                                                       
  APPL_MESG_DBG(profiledbgfile,"DISPLAY_PTS_MENU: %c\n",DISPLAY_PTS_MENU);          
    
}/* end Display_Appl_Menu() */
 
void Device_Init(void)
{
  tipcInitDevType InitDev; 
  uint8_t public_addr[6]= TIME_CLIENT_PUBLIC_ADDRESS;
  uint8_t device_name[]=TIME_CLIENT_DEVICE_NAME;
  tBleStatus ble_status;

  InitDev.public_addr =  public_addr;
  InitDev.txPower =      TIME_CLIENT_TX_POWER_LEVEL;
  InitDev.device_name_len = sizeof(device_name);
  InitDev.device_name = device_name;
  
  ble_status = TimeClient_Init(InitDev);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"TimeClient_Init() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Central initialized.\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeClient_Init() Error: %02X\n", ble_status);
  }
}

void Device_SetSecurity(void)
{
  tipcSecurityType param;
  tBleStatus ble_status;
  
  param.ioCapability =  TIME_CLIENT_IO_CAPABILITY;
  param.mitm_mode =     TIME_CLIENT_MITM_MODE;
  param.oob_enable =    TIME_CLIENT_OOB_ENABLE;
  param.bonding_mode =  TIME_CLIENT_BONDING_MODE;
  param.use_fixed_pin = TIME_CLIENT_USE_FIXED_PIN;
  param.fixed_pin =     TIME_CLIENT_FIXED_PIN;
  
  ble_status = TimeClient_SecuritySet(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"TimeClient_SecuritySet() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Security params set.\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeClient_SecuritySet() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_Discovery_Procedure(void)
{
  tipcDevDiscType param;
  tBleStatus ble_status;
  
  param.own_addr_type = PUBLIC_ADDR;
  param.scanInterval =  TIME_CLIENT_LIM_DISC_SCAN_INT;
  param.scanWindow =    TIME_CLIENT_LIM_DISC_SCAN_WIND;
  ble_status = TimeClient_DeviceDiscovery(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"TimeClient_DeviceDiscovery() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Scanning...\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeClient_DeviceDiscovery() Error: %02X\n", ble_status);
  }
  return ble_status;
}

uint8_t Device_Connection_Procedure(uint8_t *peer_addr)
{
  tipcConnDevType param;
  tBleStatus ble_status;
  //uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS;

  param.fastScanDuration =         TIME_CLIENT_FAST_SCAN_DURATION;
  param.fastScanInterval =         TIME_CLIENT_FAST_SCAN_INTERVAL;
  param.fastScanWindow =           TIME_CLIENT_FAST_SCAN_WINDOW;
  param.reducedPowerScanInterval = TIME_CLIENT_REDUCED_POWER_SCAN_INTERVAL;
  param.reducedPowerScanWindow =   TIME_CLIENT_REDUCED_POWER_SCAN_WINDOW;
  param.peer_addr_type =           PUBLIC_ADDR;
  param.peer_addr =                peer_addr;
  param.own_addr_type =            PUBLIC_ADDR;
  param.conn_min_interval =        TIME_CLIENT_FAST_MIN_CONNECTION_INTERVAL;
  param.conn_max_interval =        TIME_CLIENT_FAST_MAX_CONNECTION_INTERVAL;
  param.conn_latency =             TIME_CLIENT_FAST_CONNECTION_LATENCY;
  param.supervision_timeout =      TIME_CLIENT_SUPERVISION_TIMEOUT;
  param.min_conn_length =          TIME_CLIENT_MIN_CONN_LENGTH;
  param.max_conn_length =          TIME_CLIENT_MAX_CONN_LENGTH;
  ble_status = TimeClient_DeviceConnection(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"TimeClient_DeviceConnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeClient_DeviceConnection() Error: %02X\n", ble_status);
  }
  return ble_status;
}
 
void Device_StartPairing(void)
{
  tBleStatus ble_status;
  ble_status = TimeClient_StartPairing();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"TimeClient_StartPairing() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeClient_StartPairing() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_ServicesDiscovery(void)
{
  tBleStatus ble_status;
  ble_status = TimeClient_ServicesDiscovery();

  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"TimeClient_ServicesDiscovery() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeClient_ServicesDiscovery() Error: %02X\n", ble_status);
  }
  return ble_status;
}

void Device_Disconnection(void)
{
  tBleStatus ble_status;
  ble_status = TimeClient_DeviceDisconnection();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"TimeClient_DeviceDisconnection() Call: OK\n" );
    notify_uart(&ble_status, sizeof(ble_status), CLOSE_CONNECTION);
    APPL_MESG_DBG(profiledbgfile,"Disconnection complete.\n" );
    profileApplContext.deviceState = APPL_INIT_DONE;
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"TimeClient_DeviceDisconnection() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_Discovery_CharacServ(uint16_t uuid_service)
{
  tBleStatus ble_status;
  ble_status = TimeClient_DiscCharacServ(uuid_service);
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
  tipcConnDevType connParam;
  //tipcConfDevType confParam;
  uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS; 

  /* Connection Parameter */
  connParam.fastScanDuration =         TIME_CLIENT_FAST_SCAN_DURATION;
  connParam.fastScanInterval =         TIME_CLIENT_FAST_SCAN_INTERVAL;
  connParam.fastScanWindow =           TIME_CLIENT_FAST_SCAN_WINDOW;
  connParam.reducedPowerScanInterval = TIME_CLIENT_REDUCED_POWER_SCAN_INTERVAL;
  connParam.reducedPowerScanWindow =   TIME_CLIENT_REDUCED_POWER_SCAN_WINDOW;
  connParam.peer_addr_type =           PUBLIC_ADDR;
  connParam.peer_addr =                peer_addr;
  connParam.own_addr_type =            PUBLIC_ADDR;
  connParam.conn_min_interval =        TIME_CLIENT_FAST_MIN_CONNECTION_INTERVAL;
  connParam.conn_max_interval =        TIME_CLIENT_FAST_MAX_CONNECTION_INTERVAL;
  connParam.conn_latency =             TIME_CLIENT_FAST_CONNECTION_LATENCY;
  connParam.supervision_timeout =      TIME_CLIENT_SUPERVISION_TIMEOUT;
  connParam.min_conn_length =          TIME_CLIENT_MIN_CONN_LENGTH;
  connParam.max_conn_length =          TIME_CLIENT_MAX_CONN_LENGTH;

  /* Configuration Parameter */
  if (TimeClient_ConnConf(connParam) != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"Error in TimeClient_ConnConf()\r\n");
  } 
}

//TBR
void Host_Profile_Test_Application(void)
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

      /* process the timer Q */
      Blue_NRG_Timer_Process_Q();
      
      /* Call the Profile Master role state machine */
      Master_Process(&initParam);
      
      if (TimeClientContext.fullConf) 
        TimeClient_StateMachine();
      
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
        ret = Device_Discovery_CharacServ(CURRENT_TIME_SERVICE_UUID);
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
        }
        profileApplContext.deviceState = APPL_INIT_DONE; // Transitory
      }
      
      /* Get TS Current Time Char Descriptor */
      if(profileApplContext.deviceState == APPL_CHARS_DISCOVERED){
        ret = TimeClient_Start_Current_Time_Characteristic_Descriptor_Discovery();
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
        }
        profileApplContext.deviceState = APPL_INIT_DONE; // Transitory
      }
      
      /* Enable Notification */
      if(profileApplContext.deviceState == APPL_GOT_CHAR_DESC){
        ret = TimeClient_Set_Current_Time_Char_Notification(TRUE);
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
          profileApplContext.deviceState = APPL_INIT_DONE;
        } else {
          profileApplContext.deviceState = APPL_NOTIFICATION_ENABLED;
        }
      }
#if 0    
    /* if the profiles have nothing more to process, then
     * wait for application input
     */ 
    if(deviceState >= APPL_INIT_DONE) //TBR
    {      
      uint8_t input = APPL_DISCOVER_TIME_SERVER;//200;
      deviceState = input;
      if (input>0)
      {
        APPL_MESG_INFO(profiledbgfile,"io--- input: %c\n",input); 

        switch(input)
        {           
          case DISPLAY_PTS_MENU: 
           
          case APPL_DISCOVER_TIME_SERVER:
          case APPL_CONNECT_TIME_SERVER:
          case APPL_DISCONNECT_TIME_SERVER:
          case APPL_PAIR_WITH_TIME_SERVER:
          case APPL_CLEAR_SECURITY_DATABASE:
            
          case APPL_DISCOVER_TIME_SERVICES:      
          case APPL_DISCOVER_CTS_CHARACTERISTICS:                                                    
          case APPL_DISCOVER_CT_CHAR_DESCRIPTOR:      
          case APPL_DISCOVER_NEXT_DST_CHARACTERISTICS:  
          case APPL_DISCOVER_RTU_CHARACTERISTICS:      
          case APPL_GET_REF_TIME_UPDATE:		
          case APPL_TIME_UPDATE_NOTIFICATION:		
          case APPL_READ_LOCAL_TIME_INFORM:		
          case APPL_READ_CURRENT_TIME:			
          case APPL_GET_REF_TIME_INFO_ON_SERVER:
          case APPL_READ_NEXT_DST_CHANGE_TIME: 		
          case APPL_GET_SERV_TIME_UPDATE_STATE:
          case APPL_CANCEL_REF_TIME_UPDATE:
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
      case APPL_DISCOVER_TIME_SERVER:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_TIME_SERVER: call Device_Discovery_Procedure() \n"); 
        Device_Discovery_Procedure();
      }
      break;
      case APPL_CONNECT_TIME_SERVER:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_TIME_SERVER: call Device_Connection_Procedure() \n"); 
        Device_Connection_Procedure();
      }
      break;
      case APPL_DISCONNECT_TIME_SERVER:
      {
         APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_TIME_SERVER: call Device_Disconnection() \n"); 
         Device_Disconnection();
      }
      break;
      case APPL_PAIR_WITH_TIME_SERVER:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_TIME_SERVER: call Device_StartPairing() \n"); 
        Device_StartPairing();
      }
      break;
      case APPL_CLEAR_SECURITY_DATABASE:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: call Clear_Security_Database() \n"); 
        status = TimeClient_Clear_Security_Database();
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"TimeClient_Clear_Security_Database() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"TimeClient_Clear_Security_Database() Error: %02X\n", status);
        }
      }
      break; 
      case APPL_DISCOVER_TIME_SERVICES :
      {
        /* It discover all the primary services of the connected device device */
        APPL_MESG_DBG(profiledbgfile,"Call Device_ServicesDiscovery() x connected device device\n"); 
        Device_ServicesDiscovery();
      }
      break;
      
      case APPL_DISCOVER_CTS_CHARACTERISTICS : 
      {
        /* It discovers all the characteristics of the connected current time service */
        APPL_MESG_DBG(profiledbgfile,"Call Device_Discovery_CharacServ() x Current Time Service\n"); 
        Device_Discovery_CharacServ(CURRENT_TIME_SERVICE_UUID);
      }
      break;
      
      case APPL_DISCOVER_NEXT_DST_CHARACTERISTICS: 
      {
        /* It discovers all the characteristics of the connected Next DST Service  */
        APPL_MESG_DBG(profiledbgfile,"Call Device_Discovery_CharacServ() x Next DST Service\n"); 
        Device_Discovery_CharacServ(NEXT_DST_CHANGE_SERVICE_UUID);
      }
      break;
      
      case APPL_DISCOVER_RTU_CHARACTERISTICS  : 
      {
        /* It discovers all the characteristics of the connected Reference Update Time service */
        APPL_MESG_DBG(profiledbgfile,"Call Device_Discovery_CharacServ() x Reference Update Time Service\n"); 
        Device_Discovery_CharacServ(REFERENCE_UPDATE_TIME_SERVICE_UUID);
      }
      break;
      
      
     case APPL_DISCOVER_CT_CHAR_DESCRIPTOR:           
      {
        /* It discovers the characteristic descriptors of the connected device current time characteristic */
        APPL_MESG_DBG(profiledbgfile,"Call TimeClient_Start_Glucose_Measurement_Characteristic_Descriptor_Discovery\n"); 
        status = TimeClient_Start_Current_Time_Characteristic_Descriptor_Discovery();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the TimeClient_Start_Current_Time_Characteristic_Descriptor_Discovery() call: %02X\n", status);
        }
      }
      break;

      case APPL_GET_REF_TIME_UPDATE:	
      {
        APPL_MESG_DBG(profiledbgfile,"Call TimeClient_Update_Reference_Time_On_Server\n"); 
        status = TimeClient_Update_Reference_Time_On_Server(0x01);
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the TimeClient_Update_Reference_Time_On_Server() call: %02X\n", status);
        }
      }	
      break;
      case APPL_CANCEL_REF_TIME_UPDATE:	
      {
        APPL_MESG_DBG(profiledbgfile,"Call TimeClient_Update_Reference_Time_On_Server\n"); 
        status = TimeClient_Update_Reference_Time_On_Server(0x02);
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the TimeClient_Update_Reference_Time_On_Server() call: %02X\n", status);
        }
      }	
      break;
      case APPL_TIME_UPDATE_NOTIFICATION:
      {
        APPL_MESG_DBG(profiledbgfile,"Call TimeClient_Set_Current_Time_Char_Notification\n"); 
        status = TimeClient_Set_Current_Time_Char_Notification(TRUE);
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the TimeClient_Set_Current_Time_Char_Notification() call: %02X\n", status);
        }
      }	
      break;
      case APPL_GET_SERV_TIME_UPDATE_STATE: 
      {
        APPL_MESG_DBG(profiledbgfile,"Call TimeClient_ReadServerTimeUpdateStatusChar\n"); 
        status = TimeClient_ReadServerTimeUpdateStatusChar();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the TimeClient_ReadServerTimeUpdateStatusChar() call: %02X\n", status);
        }
      }
      break;
      case APPL_READ_NEXT_DST_CHANGE_TIME: 		 
      {
        APPL_MESG_DBG(profiledbgfile,"Call TimeClient_ReadNextDSTChangeTimeChar\n"); 
        status = TimeClient_ReadNextDSTChangeTimeChar();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the TimeClient_ReadNextDSTChangeTimeChar() call: %02X\n", status);
        }
      }
      break;
      case APPL_READ_LOCAL_TIME_INFORM:
      {
        APPL_MESG_DBG(profiledbgfile,"Call TimeClient_Get_Local_Time_Information\n"); 
        status = TimeClient_ReadLocalTimeChar();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the TimeClient_ReadLocalTimeChar() call: %02X\n", status);
        }
      }
      break;
      case APPL_READ_CURRENT_TIME:
      {
        APPL_MESG_DBG(profiledbgfile,"Call TimeClient_ReadCurrentTimeChar\n"); 
        status = TimeClient_ReadCurrentTimeChar();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the TimeClient_ReadCurrentTimeChar() call: %02X\n", status);
        }
      }
      break;
      
    case APPL_GET_REF_TIME_INFO_ON_SERVER: //APPL_GET_TIME_ACCU_INFO_SERVER:	
      {
        APPL_MESG_DBG(profiledbgfile,"Call TimeClient_ReadReferenceTimeInfoChar\n"); 
        status = TimeClient_ReadReferenceTimeInfoChar();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the TimeClient_ReadReferenceTimeInfoChar() call: %02X\n", status);
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

void TimeClient_DeviceDiscovery_CB(uint8_t status, uint8_t addr_type, uint8_t *addr, 
                                     uint8_t data_length, uint8_t *data, 
                                     uint8_t RSSI)
{
  uint8_t i;
  uint8_t name_len = 7;
  uint8_t offset = name_len;

  /* Check if the device found is a TServer */
  if (!memcmp(&data[5], "TServer", name_len)) {
    
    // name + | + *addr_type + | + addr
    // 7    + 1 + 1          + 1 + 6 = 16
    uint8_t dev[28];
    strcpy((char*)dev, "TServer");
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

void TimeClient_ConnectionStatus_CB(uint8_t connection_evt, uint8_t status)
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
    if(profileApplContext.deviceState == APPL_NOTIFICATION_ENABLED) {
        tBleStatus ble_status = BLE_STATUS_SUCCESS;
        notify_uart(&ble_status, sizeof(ble_status), CLOSE_CONNECTION);
        APPL_MESG_INFO(profiledbgfile,"****  Disconnection with peer device\r\n");
    }
    break;
  }
  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
}

void TimeClient_ServicesDiscovery_CB(uint8_t status, uint8_t numServices, uint8_t *services)
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

void TimeClient_CharacOfService_CB(uint8_t status, uint8_t numCharac, uint8_t *charac)
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

void TimeClient_CharacDesc_CB(uint8_t status, uint8_t numCharac, uint8_t *charac)
{
  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
  APPL_MESG_INFO(profiledbgfile,"**** Discovery Characteristic Descriptor Procedure\r\n");
  APPL_MESG_INFO(profiledbgfile,"****    Status = 0x%02x\r\n", status);
  if (status == BLE_STATUS_SUCCESS) {
    printDebugCharacDesc(numCharac, charac);
    profileApplContext.deviceState = APPL_GOT_CHAR_DESC;
  }
  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");    
}

void TimeClient_DataValueRead_CB(uint8_t status, uint16_t data_len, uint8_t *data)
{
  uint8_t i;
  
  APPL_MESG_DBG(profiledbgfile,"****************************************\r\n");
  APPL_MESG_INFO(profiledbgfile,"**** Data Read Value Procedure: Status = 0x%02x, \r\n", status);
  if (status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_INFO(profiledbgfile,"****    Len data Read = %d\r\n", data_len);
    for (i=0; i<data_len; i++) {
        APPL_MESG_INFO(profiledbgfile,"0x%02x (%d) ", data[i], data[i]);
    }
    
    APPL_MESG_INFO(profiledbgfile,"\r\n");
    profileApplContext.deviceState = APPL_VALUE_READ;
  }
}

void TimeClient_FullConfError_CB(uint8_t error_type, uint8_t code)
{
  if (TimeClientContext.fullConf) {
    APPL_MESG_INFO(profiledbgfile,"***** TimeClient_FullConf_CB() Error Type = 0x%0x with Code = 0x%0x\r\n", error_type, code);
  }
}

void TimeClient_PinCodeRequired_CB(void)
{
  uint8_t ret;
  uint32_t pinCode=0;

  APPL_MESG_INFO(profiledbgfile,"**** Required Security Pin Code\r\n");

  /* Insert the pin code according the time client indication */
  //pinCode = ....

  ret = TimeClient_SendPinCode(pinCode);
  if (ret != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"**** Error during the Pairing procedure -> Pin Code set (Status = 0x%x)\r\n", ret);
  }
}
                                        
void TimeClient_NotificationReceived_CB(uint8_t handle, uint8_t length, uint8_t * data_value)
{
  if(profileApplContext.deviceState != APPL_NOTIFICATION_ENABLED) {
    return;
  }
  
  APPL_MESG_INFO(profiledbgfile,"NOTIFICATION_DATA_RECEIVED (Current Time): handle=0x%04x, data length=%d, data:\r\n",
	   handle, length);
    
  for (uint8_t i=0; i<length; i++) {
    APPL_MESG_INFO(profiledbgfile,"0x%02x ", data_value[i]);
  }
  APPL_MESG_INFO(profiledbgfile,"\r\n");  
}
  
void TimeClient_Pairing_CB(uint16_t conn_handle, uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"Pairing CB conn_handle = 0x%04x with status = 0x%02x***************\n", conn_handle, status);
  
}

void TimeClient_EnableNotification_CB(uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"**** Enable Notification procedure ended with Status = 0x%0x\r\n", status);
}

#endif    

