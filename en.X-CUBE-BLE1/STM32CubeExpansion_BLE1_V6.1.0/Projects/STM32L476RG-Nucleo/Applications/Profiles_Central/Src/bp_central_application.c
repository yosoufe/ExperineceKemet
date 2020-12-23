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
*   FILENAME        -  profile_BloodPressure_central_test_application.c
*
*******************************************************************************
*
*   CHANGE HISTORY
*  ---------------------------
*
*   $Date$:      09/02/2015
*   $Revision$:  first version
*   $Author$:    AMS RF Application team
*   Comments:    Profiles PTS test application for Blood Pressure  Collector. 
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

#if (BLE_CURRENT_PROFILE_ROLES & BLOOD_PRESSURE_COLLECTOR)
#include <blood_pressure_service.h>
#include <blood_pressure_collector.h>
#include <blood_pressure_collector_config.h>

/*******************************************************************************
 * Macros
*******************************************************************************/

/* PERIPHERAL peer address */
#define PERIPHERAL_PEER_ADDRESS PTS_PERIPHERAL_ADDRESS //PTS_PERIPHERAL_ADDRESS or PEER_ADDRESS

/* Blood_Pressure Collector test configuration for some PTS tests */

/* General operations: discovery, connection, disconnection, bonding, clear security database */
#define APPL_DISCOVER_BLOOD_PRESSURE            'a' // Start Discovery procedure for detecting a Blood Pressure 
#define APPL_CONNECT_BLOOD_PRESSURE             'b' // Start Connection procedure for connecting to the discovered Blood Pressure 
#define APPL_DISCONNECT_BLOOD_PRESSURE          'c' // Terminate the current connection with the Blood Pressure 
#define APPL_PAIR_WITH_BLOOD_PRESSURE           'd' // Start the bonding  procedure (or enable the encryption) with the connected Blood Pressure 
#define APPL_CLEAR_SECURITY_DATABASE            'e' // Clear the security database (it removes any bonded device) 

//---------------------- DISCOVERY SERVICES AND CHARACTERISTICS
#define APPL_DISCOVER_PRIMARY_SERVICES                  'f' //Discover All Primary Services                                                           

#define APPL_DISCOVER_BP_CHARACTERISTICS                'g' //Discover All Characteristics of Blood Pressure Services                                                
                                                            //Discover Blood Pressure Measurement Characteristic
                                                            //Discover Intermediate Cuff Pressure Characteristic 
                                                            //Discover Blood Pressure Feature Characteristic   
#define APPL_DISCOVER_BP_MEASUREMENT_CHAR_DESCRIPTOR    'h' //Discover Blood Pressure Measurement Client Characteristic Configuration Descriptor
#define APPL_DISCOVER_ICP_CHAR_DESCRIPTOR               'i' //Discover Intermediate Cuff Pressure Client Characteristic Configuration Descriptor

#define APPL_DISCOVER_DIS_CHARACTERISTICS               'k' // Discover Device Information Service (DIS) Characteristics
#define APPL_READ_DIS_MANUFACTURER_NAME_CHAR            'l' // TP/BPD/CO/BV-10-I [Read Device Information Service Characteristics] 
#define APPL_READ_DIS_MODEL_NUMBER_CHAR                 'm' // TP/BPD/CO/BV-10-I [Read Device Information Service Characteristics] 
#define APPL_READ_DIS_SYSTEM_ID_CHAR                    'n' // TP/BPD/CO/BV-10-I [Read Device Information Service Characteristics] 

//-------------------------- ENABLE NOTIFICATIONS and INDICATIONS
#define APPL_ENABLE_BP_MEASUREMENT_CHAR_INDICATION      'o' //Configure Blood Pressure Measurement for Indication
#define APPL_ENABLE_ICP_CHAR_NOTIFICATION               'p' //Configure Intermediate Cuff Pressure for Notification

//----------------------------- NOTIFICATIONS and INDICATIONS
//Receive Blood Pressure Measurement Indications
//Receive Blood Pressure Measurement Indications with reserved flags
//Receive Blood Pressure Measurement Indications with reserved measurement status bits
//Receive Blood Pressure Measurement Indications with additional octets not comprehended
//Receive multiple Blood Pressure Measurement Indications

//Receive Intermediate Cuff Pressure Notifications
//Receive Intermediate Cuff Pressure Notifications with reserved flags
//Receive Intermediate Cuff Pressure Notifications with reserved measurement status bits
//Receive Intermediate Cuff Pressure Notifications with additional octets not comprehended
//Receive multiple Intermediate Cuff Pressure Notifications


//--------------------- READ AND WRITE BLOOD PRESSURE SERVICE CHARACTERISTICS 
#define APPL_READ_BP_FEATURE_CHAR                       'q' //Read Blood Pressure Feature characteristic

//----------------------------- OTHERS

#define APPL_START_FULL_CONFIGURATION                   'Z' // Starts all the connection, service discovery, characteristic discovery, .....

#define DISPLAY_PTS_MENU                                '?' //Display PTS application command menu


/******************************************************************************
 * Local Variable Declarations
******************************************************************************/

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/

///BOOL bReadValidRangeDescr = FALSE; ///cambiare

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

/******************************************************************************
 * Function Definitions 
******************************************************************************/
/**
 * @brief  Initialize the Application Context
 * for using the BP central profile methods
 * @param  None
 * @retval None
 */
void BP_Appl_Context_Init(void)
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
      case BLOOD_PRESSURE_SERVICE_UUID:
        PACK_2_BYTE_PARAM16(uuid_array, uuid);
        notify_uart(uuid_array, sizeof(uuid_array), SERVICE_UUID);
	APPL_MESG_INFO(profiledbgfile,"****    Blood Pressure Service UUID found\r\n");
	break;
      case DEVICE_INFORMATION_SERVICE_UUID:
	APPL_MESG_INFO(profiledbgfile,"****    Device Information Service UUID found\r\n");
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
    case BLOOD_PRESSURE_MEASUREMENT_CHAR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Blood Pressure Measurement Characteristic found\r\n");
      break;
    case INTERMEDIATE_CUFF_PRESSURE_CHARAC_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Intermediate Cuff Pressure Characteristic found\r\n");
      break;
    case BLOOD_PRESSURE_FEATURE_CHARAC:
      APPL_MESG_INFO(profiledbgfile,"****      Blood Pressure Feature Characteristic found\r\n");
      break; 
    
    case MANUFACTURER_NAME_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Device Information Manufacturer Name Characteristic found\r\n");
      break;
    case SYSTEM_ID_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Device Information System ID found\r\n");
      break;
    case MODEL_NUMBER_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Device Information Model Number found\r\n");
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
    case VALID_RANGE_DESCRIPTOR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Valid Range Descriptor Characteristic found\r\n");
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
  printf("APPL_DISCOVER_BLOOD_PRESSURE: %c\n", APPL_DISCOVER_BLOOD_PRESSURE);
  printf("APPL_CONNECT_BLOOD_PRESSURE: %c\n",APPL_CONNECT_BLOOD_PRESSURE);  
  printf("APPL_DISCONNECT_BLOOD_PRESSURE: %c\n",APPL_DISCONNECT_BLOOD_PRESSURE);  
  printf("APPL_PAIR_WITH_BLOOD_PRESSURE: %c\n",APPL_PAIR_WITH_BLOOD_PRESSURE);
  printf("APPL_CLEAR_SECURITY_DATABASE: %c\n",APPL_CLEAR_SECURITY_DATABASE); 
  
  printf("APPL_DISCOVER_PRIMARY_SERVICES: %c\n",APPL_DISCOVER_PRIMARY_SERVICES);

  printf("APPL_DISCOVER_BP_CHARACTERISTICS: %c\n",APPL_DISCOVER_BP_CHARACTERISTICS);    
  printf("APPL_DISCOVER_BP_MEASUREMENT_CHAR_DESCRIPTOR: %c\n",APPL_DISCOVER_BP_MEASUREMENT_CHAR_DESCRIPTOR);
  printf("APPL_DISCOVER_ICP_CHAR_DESCRIPTOR: %c\n",APPL_DISCOVER_ICP_CHAR_DESCRIPTOR);

  printf("APPL_DISCOVER_DIS_CHARACTERISTICS: %c\n",APPL_DISCOVER_DIS_CHARACTERISTICS);
  printf("APPL_READ_DIS_MANUFACTURER_NAME_CHAR: %c\n",APPL_READ_DIS_MANUFACTURER_NAME_CHAR);
  printf("APPL_READ_DIS_MODEL_NUMBER_CHAR: %c\n",APPL_READ_DIS_MODEL_NUMBER_CHAR);
  printf("APPL_READ_DIS_SYSTEM_ID_CHAR: %c\n",APPL_READ_DIS_SYSTEM_ID_CHAR); 
  
  printf("APPL_ENABLE_BP_MEASUREMENT_CHAR_INDICATION: %c\n",APPL_ENABLE_BP_MEASUREMENT_CHAR_INDICATION);
  printf("APPL_ENABLE_ICP_CHAR_NOTIFICATION: %c\n",APPL_ENABLE_ICP_CHAR_NOTIFICATION); 
  
  printf("APPL_READ_BP_FEATURE_CHAR: %c\n",APPL_READ_BP_FEATURE_CHAR);
  
  APPL_MESG_DBG(profiledbgfile,"APPL_START_FULL_CONFIGURATION: %c\n",APPL_START_FULL_CONFIGURATION);
  printf("DISPLAY_PTS_MENU: %c\n",DISPLAY_PTS_MENU);  
  
}/* end Display_Appl_Menu() */

void Device_Init(void)
{
  bpcInitDevType InitDev; 
  uint8_t public_addr[6]=BPC_PUBLIC_ADDRESS;
  uint8_t device_name[]=BPC_DEVICE_NAME;
  
  tBleStatus ble_status;

  InitDev.public_addr =  public_addr;
  InitDev.txPower =      BPC_TX_POWER_LEVEL;
  InitDev.device_name_len = sizeof(device_name);
  InitDev.device_name = device_name;
  
  ble_status = BPC_Init(InitDev);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"BPC_Init() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Central initialized.\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"BPC_Init() Error: %02X\n", ble_status);
  }
}

void Device_SetSecurity(void)
{
  bpcSecurityType param;
  tBleStatus ble_status;
  
  param.ioCapability =  BPC_IO_CAPABILITY;
  param.mitm_mode =     BPC_MITM_MODE;
  param.oob_enable =    BPC_OOB_ENABLE;
  param.bonding_mode =  BPC_BONDING_MODE;
  param.use_fixed_pin = BPC_USE_FIXED_PIN;
  param.fixed_pin =     BPC_FIXED_PIN;
  
  ble_status = BPC_SecuritySet(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"BPC_SecuritySet() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Security params set.\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"BPC_SecuritySet() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_Discovery_Procedure(void)
{
  bpcDevDiscType param;
  tBleStatus ble_status;
  
  param.own_addr_type = PUBLIC_ADDR;
  param.scanInterval =  BPC_LIM_DISC_SCAN_INT;
  param.scanWindow =    BPC_LIM_DISC_SCAN_WIND;
  ble_status = BPC_DeviceDiscovery(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"BPC_DeviceDiscovery() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Scanning...\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"BPC_DeviceDiscovery() Error: %02X\n", ble_status);
  }
  return ble_status;
}

uint8_t Device_Connection_Procedure(uint8_t *peer_addr)
{
  bpcConnDevType param;
  tBleStatus ble_status;
  //uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS;

  param.fastScanDuration =         BPC_FAST_SCAN_DURATION;
  param.fastScanInterval =         BPC_FAST_SCAN_INTERVAL;
  param.fastScanWindow =           BPC_FAST_SCAN_WINDOW;
  param.reducedPowerScanInterval = BPC_REDUCED_POWER_SCAN_INTERVAL;
  param.reducedPowerScanWindow =   BPC_REDUCED_POWER_SCAN_WINDOW;
  param.peer_addr_type =           PUBLIC_ADDR;
  param.peer_addr =                peer_addr;
  param.own_addr_type =            PUBLIC_ADDR;
  param.conn_min_interval =        BPC_FAST_MIN_CONNECTION_INTERVAL;
  param.conn_max_interval =        BPC_FAST_MAX_CONNECTION_INTERVAL;
  param.conn_latency =             BPC_FAST_CONNECTION_LATENCY;
  param.supervision_timeout =      BPC_SUPERVISION_TIMEOUT;
  param.min_conn_length =          BPC_MIN_CONN_LENGTH;
  param.max_conn_length =          BPC_MAX_CONN_LENGTH;
  ble_status = BPC_DeviceConnection(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"BPC_DeviceConnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"BPC_DeviceConnection() Error: %02X\n", ble_status);
  }
  return ble_status;
}

void Device_StartPairing(void)
{
  tBleStatus ble_status;
  ble_status = BPC_StartPairing();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"BPC_StartPairing() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"BPC_StartPairing() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_ServicesDiscovery(void)
{
  tBleStatus ble_status;
  ble_status = BPC_ServicesDiscovery();

  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"BPC_ServicesDiscovery() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"BPC_ServicesDiscovery() Error: %02X\n", ble_status);
  }
  return ble_status;
}

void Device_Disconnection(void)
{
  tBleStatus ble_status;
  ble_status = BPC_DeviceDisconnection();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"BPC_DeviceDisconnection() Call: OK\n" );
    notify_uart(&ble_status, sizeof(ble_status), CLOSE_CONNECTION);
    APPL_MESG_DBG(profiledbgfile,"Disconnection complete.\n" );
    profileApplContext.deviceState = APPL_INIT_DONE;
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"BPC_DeviceDisconnection() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_Discovery_CharacServ(uint16_t uuid_service)
{
  tBleStatus ble_status;
  ble_status = BPC_DiscCharacServ(uuid_service);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"BPC_DiscCharacServ() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"BPC_DiscCharacServ() Error: %02X, uuid_service: 0x%02X\n", ble_status,uuid_service);
  }
  return ble_status;
}

void Device_StartFullConfig(void)
{
  bpcConnDevType connParam;
  uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS; 

  /* Connection Parameter */
  connParam.fastScanDuration =         BPC_FAST_SCAN_DURATION;
  connParam.fastScanInterval =         BPC_FAST_SCAN_INTERVAL;
  connParam.fastScanWindow =           BPC_FAST_SCAN_WINDOW;
  connParam.reducedPowerScanInterval = BPC_REDUCED_POWER_SCAN_INTERVAL;
  connParam.reducedPowerScanWindow =   BPC_REDUCED_POWER_SCAN_WINDOW;
  connParam.peer_addr_type =           PUBLIC_ADDR;
  connParam.peer_addr =                peer_addr;
  connParam.own_addr_type =            PUBLIC_ADDR;
  connParam.conn_min_interval =        BPC_FAST_MIN_CONNECTION_INTERVAL;
  connParam.conn_max_interval =        BPC_FAST_MAX_CONNECTION_INTERVAL;
  connParam.conn_latency =             BPC_FAST_CONNECTION_LATENCY;
  connParam.supervision_timeout =      BPC_SUPERVISION_TIMEOUT;
  connParam.min_conn_length =          BPC_MIN_CONN_LENGTH;
  connParam.max_conn_length =          BPC_MAX_CONN_LENGTH;

  /* Configuration Parameter */

  if (BPC_ConnConf(connParam) != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"Error in BPC_ConnConf()\r\n");
  } 
  //else {
    //bloodPressureCollectorContext.useConnConf = TRUE;
  //}
}


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

      /* process the timer Q */
      Blue_NRG_Timer_Process_Q();
      
      /* Call the Profile Master role state machine */
      Master_Process(&initParam);
      
      if (bloodPressureCollectorContext.fullConf) 
        BPC_StateMachine();
      
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
        ret = Device_Discovery_CharacServ(BLOOD_PRESSURE_SERVICE_UUID);
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
        }
        profileApplContext.deviceState = APPL_INIT_DONE; // Transitory
      }
      
      /* Get BP Measurement Char Descriptor */
      if(profileApplContext.deviceState == APPL_CHARS_DISCOVERED){
        ret = BPC_Start_Blood_Pressure_Measurement_Client_Char_Descriptor_Discovery();
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
        }
        profileApplContext.deviceState = APPL_INIT_DONE; // Transitory
      }

      /* Read Feature */
      if(profileApplContext.deviceState == APPL_GOT_CHAR_DESC){
        ret = BPC_Read_BP_Feature();
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
        }
        profileApplContext.deviceState = APPL_INIT_DONE; // Transitory
      }
      
      /* Enable Indication */
      if(profileApplContext.deviceState == APPL_VALUE_READ){
        ret = BPC_Enable_BP_Measurement_Char_Indication();
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
          profileApplContext.deviceState = APPL_INIT_DONE;
        } else {
          profileApplContext.deviceState = APPL_INDICATION_ENABLED;
        }
      }
#if 0
    /* if the profiles have nothing more to process, then
     * wait for application input
     */ 
    if(deviceState >= APPL_INIT_DONE) //TBR
    {      
      uint8_t input = APPL_DISCOVER_BLOOD_PRESSURE;//200;
      deviceState = input;
      if (input>0)
      {
        printf("io--- input: %c\n",input); 

        switch(input)
        {   
	  case DISPLAY_PTS_MENU:             
          case APPL_DISCOVER_BLOOD_PRESSURE:
	  case APPL_CONNECT_BLOOD_PRESSURE:
	  case APPL_DISCONNECT_BLOOD_PRESSURE:
	  case APPL_PAIR_WITH_BLOOD_PRESSURE:
	  case APPL_CLEAR_SECURITY_DATABASE:            
	  case APPL_DISCOVER_PRIMARY_SERVICES:              
          case APPL_DISCOVER_BP_CHARACTERISTICS:
	  case APPL_DISCOVER_BP_MEASUREMENT_CHAR_DESCRIPTOR:
          case APPL_DISCOVER_ICP_CHAR_DESCRIPTOR:         
          case APPL_DISCOVER_DIS_CHARACTERISTICS:
	  case APPL_READ_DIS_MANUFACTURER_NAME_CHAR: 
          case APPL_READ_DIS_MODEL_NUMBER_CHAR: 
          case APPL_READ_DIS_SYSTEM_ID_CHAR:              
	  case APPL_ENABLE_BP_MEASUREMENT_CHAR_INDICATION:
	  case APPL_ENABLE_ICP_CHAR_NOTIFICATION:            
	  case APPL_READ_BP_FEATURE_CHAR:           
	  case APPL_START_FULL_CONFIGURATION :
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
      case APPL_DISCOVER_BLOOD_PRESSURE:
      {
        PROFILE_MESG_DBG(profiledbgfile,"APPL_DISCOVER_BLOOD_PRESSURE : call Device_Discovery_Procedure() \n"); 
        Device_Discovery_Procedure();
      }
      break;
      case APPL_CONNECT_BLOOD_PRESSURE:
      {
        PROFILE_MESG_DBG(profiledbgfile,"APPL_CONNECT_BP_SENSOR : call Device_Connection_Procedure() \n"); 
        Device_Connection_Procedure();
      }
      break;
      case APPL_DISCONNECT_BLOOD_PRESSURE:
      {
        BP_DBG_MSG(profiledbgfile,"APPL_DISCONNECT_BP_SENSOR: call Device_Disconnection() \n"); 
        Device_Disconnection();
      }
      break;
      case APPL_PAIR_WITH_BLOOD_PRESSURE :
      {
        BP_DBG_MSG(profiledbgfile,"APPL_BOND_WITH_BP_SENSOR: call Device_StartPairing() \n"); 
        Device_StartPairing();
      }
      break;
      case APPL_CLEAR_SECURITY_DATABASE:
      {
        BP_DBG_MSG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: call BP_Collector_Clear_Security_Database()\n"); 
        status = BPC_Clear_Security_Database();
        if (status == BLE_STATUS_SUCCESS) 
        {
          BP_DBG_MSG(profiledbgfile,"BPC_Clear_Security_Database() Call: OK\n" );
        }
        else
        {
          BP_DBG_MSG(profiledbgfile,"BPC_Clear_Security_Database() Error: %02X\n", status);
        }
      }
      break; 
      case APPL_DISCOVER_PRIMARY_SERVICES :  /* 4.3.1 TP/THS/CO/BV-01-I */                                                   
      {
        /* It discover the Blood Pressure services of the connected Blood Pressure  device */
        BP_DBG_MSG(profiledbgfile,"Call Device_ServicesDiscovery() x Blood Pressure Service\n"); 
        Device_ServicesDiscovery();
      }
      break;  

      case APPL_DISCOVER_BP_CHARACTERISTICS:  /* 4.3.4 TP/BPD/CO/BV-04-I */  
      {
        /* It discovers all the characteristics of the connected Blood Pressure service */
        BP_DBG_MSG(profiledbgfile,"Call Device_Discovery_CharacServ(BLOOD_PRESSURE_SERVICE_UUID) x Blood Pressure Service\n"); 
        Device_Discovery_CharacServ(BLOOD_PRESSURE_SERVICE_UUID);
      }
      break;
      
      case APPL_DISCOVER_BP_MEASUREMENT_CHAR_DESCRIPTOR: /* 4.3.5 TP/BPD/CO/BV-05-I */             
      {
        /* It discovers the characteristic descriptors of the connected Blood Pressure Measurement Context Characteristic */
        BP_DBG_MSG(profiledbgfile,"Call BPC_Start_Blood_Pressure_Measurement_Client_Char_Descriptor_Discovery() x Blood Pressure Measurement Characteristic\n"); 
        status = BPC_Start_Blood_Pressure_Measurement_Client_Char_Descriptor_Discovery();
        if (status!= BLE_STATUS_SUCCESS)
        {
          BP_DBG_MSG(profiledbgfile,"Error in the BPC_Start_Blood_Pressure_Measurement_Client_Char_Descriptor_Discovery() call: %02X\n", status);
        }
      }
      break; 
      
      case APPL_DISCOVER_ICP_CHAR_DESCRIPTOR: /* 4.3.7 TP/BPD/CO/BV-07-I */             
      {
        /* It discovers the characteristic descriptors of the connected Blood Pressure Sensor Measurement Context Characteristic */
        BP_DBG_MSG(profiledbgfile,"Call BPC_Start_ICP_Client_Char_Descriptor_Discovery() x Intermediate Cuff Pressure Characteristic\n"); 
        status = BPC_Start_ICP_Client_Char_Descriptor_Discovery();
        if (status!= BLE_STATUS_SUCCESS)
        {
          BP_DBG_MSG(profiledbgfile,"Error in the BPC_Start_ICP_Client_Char_Descriptor_Discovery() call: %02X\n", status);
        }
      }
      break;

      case APPL_DISCOVER_DIS_CHARACTERISTICS:  /* 4.3.9 TP/BPD/CO/BV-09-I */  
      {
         /* It discovers all the characteristics of the connected device information service */
        BP_DBG_MSG(profiledbgfile,"APPL_DISCOVER_DIS_CHARACTERISTICS: call Device_Discovery_CharacServ(DEVICE_INFORMATION_SERVICE_UUID) x Device Information Service\n"); 
        Device_Discovery_CharacServ(DEVICE_INFORMATION_SERVICE_UUID);
      }
      break;
      
      case APPL_READ_DIS_MANUFACTURER_NAME_CHAR: /* 4.3.10 TP/BPD/CO/BV-10-I */
      {
        BP_DBG_MSG(profiledbgfile,"Read Device Information Service: Manufacturer Name Characteristic \n");        
        /* Read Device Info Manufacturer Name Characteristic */
        status = BPC_ReadDISManufacturerNameChar();
        if (status!= BLE_STATUS_SUCCESS)
        {
          BP_DBG_MSG(profiledbgfile,"Error in the BPC_ReadDISManufacturerNameChar() call: %02X\n", status);
        }
      }
      break;
      
      case APPL_READ_DIS_MODEL_NUMBER_CHAR:  /* 4.3.10 TP/BPD/CO/BV-10-I */
      {
        BP_DBG_MSG(profiledbgfile,"Read Device Information Service: Model Number Characteristic \n");        
        /* Read Device Info Model Number Characteristic */ 
        status = BPC_ReadDISModelNumberChar();
        if (status!= BLE_STATUS_SUCCESS)
        {
          BP_DBG_MSG(profiledbgfile,"Error in the BPC_ReadDISModelNumberChar() call: %02X\n", status);
        }
      }  
      break;
      case APPL_READ_DIS_SYSTEM_ID_CHAR: /* 4.3.10 TP/BPD/CO/BV-10-I */
      {
        BP_DBG_MSG(profiledbgfile,"Read Device Information Service: System ID Characteristic \n");        
        /* Read Device Info System ID Characteristic */ 
        status = BPC_ReadDISSystemIDChar();
        if (status!= BLE_STATUS_SUCCESS)
        {
          BP_DBG_MSG(profiledbgfile,"Error in the BPC_ReadDISSystemIDChar() call: %02X\n", status);
        }
      }  
      break;
      
      case APPL_ENABLE_BP_MEASUREMENT_CHAR_INDICATION:  /* 4.4.9 TP/BPF/CO/BV-09-I */
      {        
        /* Enable Temperature Measurement Char Descriptor for indication */
        BP_DBG_MSG(profiledbgfile,"Call BPC_Enable_BP_Measurement_Char_Indication() \n");
        status = BPC_Enable_BP_Measurement_Char_Indication();
        if (status!= BLE_STATUS_SUCCESS)
        {
          BP_DBG_MSG(profiledbgfile,"Error in the BPC_Enable_BP_Measurement_Char_Indication() call: %02X\n", status);
        }
      }
      break;
       
      case APPL_ENABLE_ICP_CHAR_NOTIFICATION :  /* 4.4.15 TP/BPF/CO/BV-12-I */
      {        
        /* Enable Intermediate Temperature Char Descriptor for notifications */
        BP_DBG_MSG(profiledbgfile,"Call BPC_Enable_ICP_Char_Notification() \n");
        status = BPC_Enable_ICP_Char_Notification();
        if (status!= BLE_STATUS_SUCCESS)
        {
          BP_DBG_MSG(profiledbgfile,"Error in the BPC_Enable_ICP_Char_Notification() call: %02X\n", status);
        }
      }
      break;
      
      case APPL_READ_BP_FEATURE_CHAR:  /* 4.4.21 TP/THF/CO/BV-15-I */
      {
        BP_DBG_MSG(profiledbgfile,"Read Blood Pressure Feature Characteristic Value.\n");
        status = BPC_Read_BP_Feature();
        if (status!= BLE_STATUS_SUCCESS)
        {
          BP_DBG_MSG(profiledbgfile,"Error in the BPC_Read_BP_Feature() call (Blood Pressure Feature Characteristic): %02X\n", status);
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

void BPC_DeviceDiscovery_CB(uint8_t status, uint8_t addr_type, uint8_t *addr, 
                                     uint8_t data_length, uint8_t *data, 
                                     uint8_t RSSI)
{
  uint8_t i;
  uint8_t name_len = 19;
  uint8_t offset = name_len;

  /* Check if the device found is a BloodPressureSensor */
  if (!memcmp(&data[5], "BloodPressureSensor", name_len)) {
    
    // name + | + *addr_type + | + addr
    // 19    + 1 + 1          + 1 + 6 = 28
    uint8_t dev[28];
    strcpy((char*)dev, "BloodPressureSensor");
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

void BPC_ConnectionStatus_CB(uint8_t connection_evt, uint8_t status)
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
    if(profileApplContext.deviceState == APPL_INDICATION_ENABLED) {
        tBleStatus ble_status = BLE_STATUS_SUCCESS;
        notify_uart(&ble_status, sizeof(ble_status), CLOSE_CONNECTION);
        APPL_MESG_INFO(profiledbgfile,"****  Disconnection with peer device\r\n");
    }
    break;
  }
  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
}

void BPC_ServicesDiscovery_CB(uint8_t status, uint8_t numServices, uint8_t *services)
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

void BPC_CharacOfService_CB(uint8_t status, uint8_t numCharac, uint8_t *charac)
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

void BPC_CharacDesc_CB(uint8_t status, uint8_t numCharac, uint8_t *charac)
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

void BPC_DataValueRead_CB(uint8_t status, uint16_t data_len, uint8_t *data)
{
  uint8_t i;

  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
  APPL_MESG_INFO(profiledbgfile,"**** Data Read Value Procedure: Status = 0x%02x\r\n", status);
  if (status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_INFO(profiledbgfile,"****    Data: ");
    for (i=0; i<data_len; i++) {
      APPL_MESG_INFO(profiledbgfile,"0x%0x (%d)", data[i], data[i]);
    }
    APPL_MESG_INFO(profiledbgfile,"\r\n");
    profileApplContext.deviceState = APPL_VALUE_READ;
  }
}

void BPC_FullConfError_CB(uint8_t error_type, uint8_t code)
{
  if (bloodPressureCollectorContext.fullConf) {
    APPL_MESG_INFO(profiledbgfile,"***** BPC_FullConf_CB() Error Type = 0x%0x with Code = 0x%0x\r\n", error_type, code);
  }
}

void BPC_PinCodeRequired_CB(void)
{
  uint8_t ret;
  uint32_t pinCode=0;

  APPL_MESG_INFO(profiledbgfile,"**** Required Security Pin Code\r\n");

  /* Insert the pin code according the blood pressure collector indication */
  //pinCode = ....

  ret = BPC_SendPinCode(pinCode);
  if (ret != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"**** Error during the Pairing procedure -> Pin Code set (Status = 0x%x)\r\n", ret);
  }
}
                                        
void BPC_NotificationReceived_CB(uint8_t handle, uint8_t length, uint8_t * data_value)
{
  if(profileApplContext.deviceState != APPL_NOTIFICATION_ENABLED) {
    return;
  }
  
  APPL_MESG_DBG(profiledbgfile,"NOTIFICATION_DATA_RECEIVED: handle=0x%04x, data length=%d, data:\r\n",
	   handle, length);
    
  for (uint8_t i=0; i<length; i++) {
    APPL_MESG_DBG(profiledbgfile,"%02x ", data_value[i]);
  }
  APPL_MESG_DBG(profiledbgfile,"\r\n");  
}
  
void BPC_IndicationReceived_CB(uint8_t handle, uint8_t length, uint8_t * data_value)
{
  if(profileApplContext.deviceState != APPL_INDICATION_ENABLED) {
    return;
  }
  
  APPL_MESG_DBG(profiledbgfile,"INDICATION_DATA_RECEIVED: handle=0x%04x, data length=%d, data:\r\n",
	   handle, length);
    
  for (uint8_t i=0; i<length; i++) {
    APPL_MESG_DBG(profiledbgfile,"%02x ", data_value[i]);
  }
  APPL_MESG_DBG(profiledbgfile,"\r\n");  
}

void BPC_Pairing_CB(uint16_t conn_handle, uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"Pairing CB conn_handle = 0x%04x with status = 0x%02x***************\n", conn_handle, status);
  
}

void BPC_EnableIndication_CB(uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"**** Enable Indication procedure ended with Status = 0x%0x\r\n", status);
}

void BPC_EnableNotification_CB(uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"**** Enable Notification procedure ended with Status = 0x%0x\r\n", status);
}


#endif    

