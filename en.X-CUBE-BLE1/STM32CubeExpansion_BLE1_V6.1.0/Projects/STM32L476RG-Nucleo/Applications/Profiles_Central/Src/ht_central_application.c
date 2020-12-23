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
*   FILENAME        -  profile_HT_central_test_application.c
*
*******************************************************************************
*
*   CHANGE HISTORY
*  ---------------------------
*
*   $Date$:      29/01/2015
*   $Revision$:  first version
*   $Author$:    AMS RF Application team
*   Comments:    Profiles PTS test application for Health Thermometer  Collector. 
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

#if (BLE_CURRENT_PROFILE_ROLES & HEALTH_THERMOMETER_COLLECTOR)
#include <health_thermometer_service.h>
#include <health_thermometer_collector.h>
#include <health_thermometer_collector_config.h>

/*******************************************************************************
 * Macros
*******************************************************************************/
/* PERIPHERAL peer address */
#define PERIPHERAL_PEER_ADDRESS PTS_PERIPHERAL_ADDRESS //PTS_PERIPHERAL_ADDRESS or PEER_ADDRESS

/* Health Thermometer Collector test configuration for some PTS tests */

/* General operations: discovery, connection, disconnection, pairing, clear security database */
#define APPL_DISCOVER_HEALTH_THEMOMETER            'a' // Start Discovery procedure for detecting a Health Thermometer 
#define APPL_CONNECT_HEALTH_THEMOMETER             'b' // Start Connection procedure for connecting to the discovered Health Thermometer 
#define APPL_DISCONNECT_HEALTH_THEMOMETER          'c' // Terminate the current connection with the Health Thermometer 
#define APPL_PAIR_WITH_HEALTH_THEMOMETER           'd' // Start the pairing  procedure (or enable the encryption) with the connected Health Thermometer 
#define APPL_CLEAR_SECURITY_DATABASE               'e' // Clear the security database (it removes any bonded device) 

//---------------------- DISCOVERY SERVICES AND CHARACTERISTICS
#define APPL_DISCOVER_HEALTH_THERMOMETER_SERVICES               'f' //Discover All Primary Services:
                                                                    //Discover Health Thermometer Sensor Services
                                                                    //Discover Discover Device Information Service 
#define APPL_DISCOVER_TEMPERATURE_MEASUREMENT_CHAR              'g' //Discover All Characteristics of Health Thermometer Services
                                                                    //Discover Temperature Measurement Characteristic
                                                                    //Discover Temperature Type Characteristic
                                                                    //Discover Intermediate Temperature Characteristic 
                                                                    //Discover Measurement Interval Characteristic   
#define APPL_DISCOVER_TEMPERATURE_MEASUREMENT_CHAR_DESCRIPTOR   'h' //Discover Temperature Measurement Client Characteristic Configuration Descriptor

#define APPL_DISCOVER_INTERMEDIATE_TEMPERATURE_CHAR_DESCRIPTOR  'k' //Discover Intermediate Temperature Client Characteristic Configuration Descriptor

#define APPL_DISCOVER_MEASUREMENT_INTERVAL_CHAR_DESCRIPTOR      'i' //Discover Measurement Interval Client Characteristic Configuration Descriptor

#define APPL_DISCOVER_DIS_CHARACTERISTICS                       'l' // Discover Device Information Service (DIS) Characteristics
#define APPL_READ_DIS_MANUFACTURER_NAME_CHAR                    'm' // TP/THS/CO/BV-14-I [Read Device Information Service Characteristics] 
#define APPL_READ_DIS_MODEL_NUMBER_CHAR                         'n' // TP/THS/CO/BV-14-I [Read Device Information Service Characteristics] 
#define APPL_READ_DIS_SYSTEM_ID_CHAR                            'o' // TP/THS/CO/BV-14-I [Read Device Information Service Characteristics] 


//-------------------------- ENABLE NOTIFICATIONS and INDICATIONS
#define APPL_ENABLE_TEMPERATURE_MEASUREMENT_CHAR_INDICATION     'p' //Configure Temperature Measurement for Indication
#define APPL_ENABLE_INTERMEDIATE_TEMPERATURE_CHAR_NOTIFICATION  'q' //Configure Intermediate Temperature for Notification
#define APPL_ENABLE_MEASUREMENT_INTERVAL_CHAR_INDICATION        'r' //Configure Measurement Interval for Indication

//----------------------------- NOTIFICATIONS and INDICATIONS
//Receive Temperature Measurement Indications
//Receive Temperature Measurement Indications with reserved flags
//Receive Temperature Measurement Indications with reserved temperature type
//Receive Temperature Measurement Indications with additional octets not represented in the flags field
//Receive multiple Temperature Measurement Indications

//Receive Intermediate Measurement Notifications
//Receive Intermediate Measurement Notifications with reserved flags
//Receive Intermediate Measurement Notifications with reserved temperature type
//Receive Intermediate Measurement Notifications with additional octets not represented in the flags field
//Receive multiple Intermediate Measurement Notifications

//Receive Measurement Interval Indications

//--------------------- READ AND WRITE HEALTH THERMOMETER SERVICE CHARACTERISTICS 
#define APPL_READ_MEASUREMENT_INTERVAL_CHAR               's' //Read Measurement Interval characteristic
#define APPL_WRITE_MEASUREMENT_INTERVAL_CHAR              't' //Write Measurement Interval characteristic 
#define APPL_READ_MEASUREMENT_INTERVAL_VALID_RANGE_DESC   'u' //Read Measurement Interval Valid Range descriptor

#define APPL_READ_TEMPERATURE_TYPE_CHAR                   'v' //Read Temperature Type characteristic

#define APPL_START_FULL_CONFIGURATION                     'Z' // Starts all the connection, service discovery, characteristic discovery, .....

#define DISPLAY_PTS_MENU                                  '?' //Display PTS application command menu


/******************************************************************************
 * Local Variable Declarations
******************************************************************************/

/******************************************************************************
 * Global Variable Declarations
******************************************************************************/

BOOL bReadValidRangeDescr = FALSE;

extern tProfileApplContext profileApplContext; /* Profile Application Context */
extern initDevType initParam;

/* HT collector discovery variables */
//uint8_t addr[6]= MASTER_PUBLIC_ADDRESS; //TBR
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
 * for using the HT central profile methods
 * @param  None
 * @retval None
 */
void HT_Appl_Context_Init(void)
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
      case HEALTH_THERMOMETER_SERVICE_UUID:
        PACK_2_BYTE_PARAM16(uuid_array, uuid);
        notify_uart(uuid_array, sizeof(uuid_array), SERVICE_UUID);
	APPL_MESG_INFO(profiledbgfile,"****    Health Thermometer Service UUID found\r\n");
	break;
      case DEVICE_INFORMATION_SERVICE_UUID:
	APPL_MESG_INFO(profiledbgfile,"****    Device Information Service UUID found\r\n");
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
    case TEMPERATURE_MEASUREMENT_CHAR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Temperature Measurement Characteristic found\r\n");
      break;
    case TEMPERATURE_TYPE_CHAR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Temperature Type Characteristic found\r\n");
      break;
    case INTERMEDIATE_TEMPERATURE_CHAR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Intermediate Temperature Characteristic found\r\n");
      break;
      
    case MEASUREMENT_INTERVAL_CHAR_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Measurement Interval Characteristic found\r\n");
      break;  
    
    case MANUFACTURER_NAME_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Device Information Manufacturer Name Characteristic found\r\n");
      break;
   
    case MODEL_NUMBER_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Device Information Model Number Characteristic found\r\n");
      break;
        
    case SYSTEM_ID_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Device Information System ID Characteristic found\r\n");
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
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_HEALTH_THEMOMETER: %c\n", APPL_DISCOVER_HEALTH_THEMOMETER);
  APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_HEALTH_THEMOMETER: %c\n",APPL_CONNECT_HEALTH_THEMOMETER);  
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_HEALTH_THEMOMETER: %c\n",APPL_DISCONNECT_HEALTH_THEMOMETER);  
  APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_HEALTH_THEMOMETER: %c\n",APPL_PAIR_WITH_HEALTH_THEMOMETER);
  APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: %c\n",APPL_CLEAR_SECURITY_DATABASE);  
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_HEALTH_THERMOMETER_SERVICES: %c\n",APPL_DISCOVER_HEALTH_THERMOMETER_SERVICES);
 
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_TEMPERATURE_MEASUREMENT_CHAR: %c\n",APPL_DISCOVER_TEMPERATURE_MEASUREMENT_CHAR);
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_TEMPERATURE_MEASUREMENT_CHAR_DESCRIPTOR: %c\n",APPL_DISCOVER_TEMPERATURE_MEASUREMENT_CHAR_DESCRIPTOR);
  
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_INTERMEDIATE_TEMPERATURE_CHAR_DESCRIPTOR: %c\n",APPL_DISCOVER_INTERMEDIATE_TEMPERATURE_CHAR_DESCRIPTOR);
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_MEASUREMENT_INTERVAL_CHAR_DESCRIPTOR: %c\n",APPL_DISCOVER_MEASUREMENT_INTERVAL_CHAR_DESCRIPTOR);

  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_DIS_CHARACTERISTICS: %c\n",APPL_DISCOVER_DIS_CHARACTERISTICS);
  APPL_MESG_DBG(profiledbgfile,"APPL_READ_DIS_MANUFACTURER_NAME_CHAR: %c\n",APPL_READ_DIS_MANUFACTURER_NAME_CHAR);
  APPL_MESG_DBG(profiledbgfile,"APPL_READ_DIS_MODEL_NUMBER_CHAR: %c\n",APPL_READ_DIS_MODEL_NUMBER_CHAR);
  APPL_MESG_DBG(profiledbgfile,"APPL_READ_DIS_SYSTEM_ID_CHAR: %c\n",APPL_READ_DIS_SYSTEM_ID_CHAR);
  APPL_MESG_DBG(profiledbgfile,"APPL_ENABLE_TEMPERATURE_MEASUREMENT_CHAR_INDICATION: %c\n",APPL_ENABLE_TEMPERATURE_MEASUREMENT_CHAR_INDICATION);
  APPL_MESG_DBG(profiledbgfile,"APPL_ENABLE_INTERMEDIATE_TEMPERATURE_CHAR_NOTIFICATION: %c\n",APPL_ENABLE_INTERMEDIATE_TEMPERATURE_CHAR_NOTIFICATION);
  APPL_MESG_DBG(profiledbgfile,"APPL_ENABLE_MEASUREMENT_INTERVAL_CHAR_INDICATION: %c\n",APPL_ENABLE_MEASUREMENT_INTERVAL_CHAR_INDICATION);  
  APPL_MESG_DBG(profiledbgfile,"APPL_READ_MEASUREMENT_INTERVAL_CHAR: %c\n",APPL_READ_MEASUREMENT_INTERVAL_CHAR);
  APPL_MESG_DBG(profiledbgfile,"APPL_WRITE_MEASUREMENT_INTERVAL_CHAR: %c\n",APPL_WRITE_MEASUREMENT_INTERVAL_CHAR);
  APPL_MESG_DBG(profiledbgfile,"APPL_READ_MEASUREMENT_INTERVAL_VALID_RANGE_DESC: %c\n",APPL_READ_MEASUREMENT_INTERVAL_VALID_RANGE_DESC);
  APPL_MESG_DBG(profiledbgfile,"APPL_READ_TEMPERATURE_TYPE_CHAR: %c\n",APPL_READ_TEMPERATURE_TYPE_CHAR);  
  
 // APPL_MESG_DBG(profiledbgfile,"APPL_VERIFY_BOND_STATUS_ON_RECONNECTION: %u\n",APPL_VERIFY_BOND_STATUS_ON_RECONNECTION);
  APPL_MESG_DBG(profiledbgfile,"APPL_START_FULL_CONFIGURATION: %c\n",APPL_START_FULL_CONFIGURATION);
  APPL_MESG_DBG(profiledbgfile,"DISPLAY_PTS_MENU: %c\n",DISPLAY_PTS_MENU);  
  
}/* end Display_Appl_Menu() */


void Device_Init(void)
{
  htcInitDevType InitDev; 
  uint8_t public_addr[6]=HTC_PUBLIC_ADDRESS;
  uint8_t device_name[]=HTC_DEVICE_NAME;
  tBleStatus ble_status;

  InitDev.public_addr =  public_addr;
  InitDev.txPower =      HTC_TX_POWER_LEVEL;
  InitDev.device_name_len = sizeof(device_name);
  InitDev.device_name = device_name;
  
  ble_status = HT_Collector_Init(InitDev);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"HT_Collector_Init() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Central initialized.\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HT_Collector_Init() Error: %02X\n", ble_status);
  }
}

void Device_SetSecurity(void)
{
  htcSecurityType param;
  tBleStatus ble_status;
  
  param.ioCapability =  HTC_IO_CAPABILITY;
  param.mitm_mode =     HTC_MITM_MODE;
  param.oob_enable =    HTC_OOB_ENABLE;
  param.bonding_mode =  HTC_BONDING_MODE;
  param.use_fixed_pin = HTC_USE_FIXED_PIN;
  param.fixed_pin =     HTC_FIXED_PIN;
  
  ble_status = HT_Collector_SecuritySet(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"HT_Collector_SecuritySet() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Security params set.\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HT_Collector_SecuritySet() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_Discovery_Procedure(void)
{
  htcDevDiscType param;
  tBleStatus ble_status;
  
  param.own_addr_type = PUBLIC_ADDR;
  param.scanInterval =  HTC_LIM_DISC_SCAN_INT;
  param.scanWindow =    HTC_LIM_DISC_SCAN_WIND;
  ble_status = HT_Collector_DeviceDiscovery(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"HT_Collector_DeviceDiscovery() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Scanning...\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HT_Collector_DeviceDiscovery() Error: %02X\n", ble_status);
  }
  
  return ble_status;
}

uint8_t Device_Connection_Procedure(uint8_t *peer_addr)
{
  htcConnDevType param;
  tBleStatus ble_status;
  //uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS;

  param.fastScanDuration =         HTC_FAST_SCAN_DURATION;
  param.fastScanInterval =         HTC_FAST_SCAN_INTERVAL;
  param.fastScanWindow =           HTC_FAST_SCAN_WINDOW;
  param.reducedPowerScanInterval = HTC_REDUCED_POWER_SCAN_INTERVAL;
  param.reducedPowerScanWindow =   HTC_REDUCED_POWER_SCAN_WINDOW;
  param.peer_addr_type =           PUBLIC_ADDR;
  param.peer_addr =                peer_addr;
  param.own_addr_type =            PUBLIC_ADDR;
  param.conn_min_interval =        HTC_FAST_MIN_CONNECTION_INTERVAL;
  param.conn_max_interval =        HTC_FAST_MAX_CONNECTION_INTERVAL;
  param.conn_latency =             HTC_FAST_CONNECTION_LATENCY;
  param.supervision_timeout =      HTC_SUPERVISION_TIMEOUT;
  param.min_conn_length =          HTC_MIN_CONN_LENGTH;
  param.max_conn_length =          HTC_MAX_CONN_LENGTH;
  ble_status = HT_Collector_DeviceConnection(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"HT_Collector_DeviceConnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HT_Collector_DeviceConnection() Error: %02X\n", ble_status);
  }
  return ble_status;
}
 
void Device_StartPairing(void)
{
  tBleStatus ble_status;
  ble_status = HT_Collector_StartPairing();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"HT_Collector_StartPairing() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HT_Collector_StartPairing() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_ServicesDiscovery(void)
{
  tBleStatus ble_status;
  ble_status = HT_Collector_ServicesDiscovery();

  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"HT_Collector_ServicesDiscovery() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HT_Collector_ServicesDiscovery() Error: %02X\n", ble_status);
  }
  
  return ble_status;
}

void Device_Disconnection(void)
{
  tBleStatus ble_status;
  ble_status = HT_Collector_DeviceDisconnection();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"HT_Collector_DeviceDisconnection() Call: OK\n" );
    notify_uart(&ble_status, sizeof(ble_status), CLOSE_CONNECTION);
    APPL_MESG_DBG(profiledbgfile,"Disconnection complete.\n" );
    profileApplContext.deviceState = APPL_INIT_DONE;
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HT_Collector_DeviceDisconnection() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_Discovery_CharacServ(uint16_t uuid_service)
{
  tBleStatus ble_status;
  ble_status = HT_Collector_DiscCharacServ(uuid_service);
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
  htcConnDevType connParam;
  //htcConfDevType confParam;
  uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS; 

  /* Connection Parameter */
  connParam.fastScanDuration =         HTC_FAST_SCAN_DURATION;
  connParam.fastScanInterval =         HTC_FAST_SCAN_INTERVAL;
  connParam.fastScanWindow =           HTC_FAST_SCAN_WINDOW;
  connParam.reducedPowerScanInterval = HTC_REDUCED_POWER_SCAN_INTERVAL;
  connParam.reducedPowerScanWindow =   HTC_REDUCED_POWER_SCAN_WINDOW;
  connParam.peer_addr_type =           PUBLIC_ADDR;
  connParam.peer_addr =                peer_addr;
  connParam.own_addr_type =            PUBLIC_ADDR;
  connParam.conn_min_interval =        HTC_FAST_MIN_CONNECTION_INTERVAL;
  connParam.conn_max_interval =        HTC_FAST_MAX_CONNECTION_INTERVAL;
  connParam.conn_latency =             HTC_FAST_CONNECTION_LATENCY;
  connParam.supervision_timeout =      HTC_SUPERVISION_TIMEOUT;
  connParam.min_conn_length =          HTC_MIN_CONN_LENGTH;
  connParam.max_conn_length =          HTC_MAX_CONN_LENGTH;

  /* Configuration Parameter */

  if (HT_Collector_ConnConf(connParam) != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"Error in HT_Collector_ConnConf()\r\n");
  } 
  //else {
    //glucoseCollectorContext.useConnConf = TRUE;
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
      
      if (healthThermometerCollectorContext.fullConf) 
        HT_Collector_StateMachine();

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
        ret = Device_Discovery_CharacServ(HEALTH_THERMOMETER_SERVICE_UUID);
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
        }
        profileApplContext.deviceState = APPL_INIT_DONE; // Transitory
      }
      
      /* Get HT Temperature Measurement Char Descriptor */
      if(profileApplContext.deviceState == APPL_CHARS_DISCOVERED){
        ret = HT_Collector_Start_Temperature_Measurement_Client_Char_Descriptor_Discovery();
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
        }
        profileApplContext.deviceState = APPL_INIT_DONE; // Transitory
      }
      
      /* Enable Indication */
      if(profileApplContext.deviceState == APPL_GOT_CHAR_DESC){
        ret = HT_Collector_Enable_Temperature_Measurement_Char_Indication();
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
      uint8_t input = APPL_DISCOVER_HEALTH_THEMOMETER;//200;
      deviceState = input;
      if (input>0)
      {
        APPL_MESG_DBG(profiledbgfile,"io--- input: %c\n",input); 

        switch(input)
        {   
	  case DISPLAY_PTS_MENU:             
                        
          case APPL_DISCOVER_HEALTH_THEMOMETER:
	  case APPL_CONNECT_HEALTH_THEMOMETER:
	  case APPL_DISCONNECT_HEALTH_THEMOMETER:
	  case APPL_PAIR_WITH_HEALTH_THEMOMETER:
	  case APPL_CLEAR_SECURITY_DATABASE:
	  case APPL_DISCOVER_HEALTH_THERMOMETER_SERVICES:
	  case APPL_DISCOVER_TEMPERATURE_MEASUREMENT_CHAR:
	  case APPL_DISCOVER_TEMPERATURE_MEASUREMENT_CHAR_DESCRIPTOR:
	  
	  case APPL_DISCOVER_INTERMEDIATE_TEMPERATURE_CHAR_DESCRIPTOR :
          case APPL_DISCOVER_MEASUREMENT_INTERVAL_CHAR_DESCRIPTOR:
          case APPL_DISCOVER_DIS_CHARACTERISTICS:
	  case APPL_READ_DIS_MANUFACTURER_NAME_CHAR: 
          case APPL_READ_DIS_MODEL_NUMBER_CHAR: 
          case APPL_READ_DIS_SYSTEM_ID_CHAR:   
	  case APPL_ENABLE_TEMPERATURE_MEASUREMENT_CHAR_INDICATION:
	  case APPL_ENABLE_INTERMEDIATE_TEMPERATURE_CHAR_NOTIFICATION:
	  case APPL_ENABLE_MEASUREMENT_INTERVAL_CHAR_INDICATION:
	  case APPL_READ_MEASUREMENT_INTERVAL_CHAR:            
          case APPL_WRITE_MEASUREMENT_INTERVAL_CHAR:
	  case APPL_READ_MEASUREMENT_INTERVAL_VALID_RANGE_DESC:
	  case APPL_READ_TEMPERATURE_TYPE_CHAR:            
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
      case APPL_DISCOVER_HEALTH_THEMOMETER:
      {
        PROFILE_MESG_DBG(profiledbgfile,"APPL_DISCOVER_HEALTH_THEMOMETER : call Device_Discovery_Procedure() \n"); 
        Device_Discovery_Procedure();
      }
      break;
      case APPL_CONNECT_HEALTH_THEMOMETER:
      {
        PROFILE_MESG_DBG(profiledbgfile,"APPL_CONNECT_HT_SENSOR : call Device_Connection_Procedure() \n"); 
        Device_Connection_Procedure();
      }
      break;
      case APPL_DISCONNECT_HEALTH_THEMOMETER:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_HT_SENSOR: call Device_Disconnection() \n"); 
        Device_Disconnection();
      }
      break;
      case APPL_PAIR_WITH_HEALTH_THEMOMETER :
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_HT_SENSOR: call Device_StartPairing() \n"); 
        Device_StartPairing();
      }
      break;
      case APPL_CLEAR_SECURITY_DATABASE:
      {
        APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: call HT_Collector_Clear_Security_Database()\n"); 
        status = HT_Collector_Clear_Security_Database();
        if (status == BLE_STATUS_SUCCESS) 
        {
          APPL_MESG_DBG(profiledbgfile,"HT_Collector_Clear_Security_Database() Call: OK\n" );
        }
        else
        {
          APPL_MESG_DBG(profiledbgfile,"HT_Collector_Clear_Security_Database() Error: %02X\n", status);
        }
      }
      break; 
      case APPL_DISCOVER_HEALTH_THERMOMETER_SERVICES :  /* 4.3.1 TP/THS/CO/BV-01-I */                                                   
      {
        /* It discover the services of the connected Health Thermometer  device */
        APPL_MESG_DBG(profiledbgfile,"Call Device_ServicesDiscovery() x Health Thermometer Device \n"); 
        Device_ServicesDiscovery();
      }
      break;
        
      case APPL_DISCOVER_TEMPERATURE_MEASUREMENT_CHAR:  /* 4.3.3 TP/THS/CO/BV-04-I */  
      {
        /* It discovers all the characteristics of the connected Health Thermometer service */
        APPL_MESG_DBG(profiledbgfile,"Call Device_Discovery_CharacServ(HEALTH_THERMOMETER_SERVICE_UUID) x Health Thermometer Service\n"); 
        Device_Discovery_CharacServ(HEALTH_THERMOMETER_SERVICE_UUID);
      }
      break;
      
      case APPL_DISCOVER_TEMPERATURE_MEASUREMENT_CHAR_DESCRIPTOR: /* 4.3.4 TP/THS/CO/BV-05-I */             
      {
        /* It discovers the temperature measurrment characteristic descriptor of the connected Health Thermometer Sensor  */
        APPL_MESG_DBG(profiledbgfile,"Call HT_Collector_Start_Temperature_Measurement_Client_Char_Descriptor_Discovery() \n"); 
        status = HT_Collector_Start_Temperature_Measurement_Client_Char_Descriptor_Discovery();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_Start_Temperature_Measurement_Client_Char_Descriptor_Discovery() call: %02X\n", status);
        }
      }
      break; 
      
      case APPL_DISCOVER_INTERMEDIATE_TEMPERATURE_CHAR_DESCRIPTOR: /* 4.3.7 TP/THS/CO/BV-09-I */             
      {
        /* It discovers the intermediate characteristic descriptor of the connected Health Thermometer Sensor  */
        APPL_MESG_DBG(profiledbgfile,"Call HT_Collector_Start_Intermediate_Temperature_Client_Char_Descriptor_Discovery() \n"); 
        status = HT_Collector_Start_Intermediate_Temperature_Client_Char_Descriptor_Discovery();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_Start_Intermediate_Temperature_Client_Char_Descriptor_Discovery() call: %02X\n", status);
        }
      }
      break;
      
      case APPL_DISCOVER_MEASUREMENT_INTERVAL_CHAR_DESCRIPTOR: /* 4.3.9 TP/THS/CO/BV-11-I */             
      {
        /* It discovers the measurement interval characteristic descriptor of the connected Health Thermometer Sensor */
        APPL_MESG_DBG(profiledbgfile,"Call HT_Collector_Start_Measurement_Interval_Client_Char_Descriptor_Discovery() \n"); 
        status = HT_Collector_Start_Measurement_Interval_Client_Char_Descriptor_Discovery();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_Start_Measurement_Interval_Client_Char_Descriptor_Discovery() call: %02X\n", status);
        }
      }
      break;
      
      case APPL_DISCOVER_DIS_CHARACTERISTICS:  /* 4.3.11 TP/THS/CO/BV-13-I */  
      {
         /* It discovers all the characteristics of the connected device information service */
        APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_DIS_CHARACTERISTICS: call Device_Discovery_CharacServ() x Device Information Service\n"); 
        Device_Discovery_CharacServ(DEVICE_INFORMATION_SERVICE_UUID);
      }
      break;
      
      case APPL_READ_DIS_MANUFACTURER_NAME_CHAR: /* 4.3.12 TP/THS/CO/BV-14-I */
      {
        APPL_MESG_DBG(profiledbgfile,"Read Device Information Service: Manufacturer Name Characteristic \n");        
        /* Read Device Info Manufacturer Name Characteristic */
        status = HT_Collector_ReadDISManufacturerNameChar();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_ReadDISManufacturerNameChar() call: %02X\n", status);
        }
      }
      break;
      
      case APPL_READ_DIS_MODEL_NUMBER_CHAR:  /* 4.3.12 TP/THS/CO/BV-14-I */
      {
        APPL_MESG_DBG(profiledbgfile,"Read Device Information Service: Model Number Characteristic \n");        
        /* Read Device Info Model Number Characteristic */ 
        status = HT_Collector_ReadDISModelNumberChar();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_ReadDISModelNumberChar() call: %02X\n", status);
        }
      }  
      break;
      case APPL_READ_DIS_SYSTEM_ID_CHAR: /* 4.3.12 TP/THS/CO/BV-14-I */
      {
        APPL_MESG_DBG(profiledbgfile,"Read Device Information Service: System ID Characteristic \n");        
        /* Read Device Info System ID Characteristic */ 
        status = HT_Collector_ReadDISSystemIDChar();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_ReadDISSystemIDChar() call: %02X\n", status);
        }
      }  
      break;

    case APPL_ENABLE_TEMPERATURE_MEASUREMENT_CHAR_INDICATION :  /* 4.4.3 TP/THF/CO/BV-01-I */
      {        
        /* Enable Temperature Measurement Char Descriptor for indication */
        APPL_MESG_DBG(profiledbgfile,"Call HT_Collector_Enable_Temperature_Measurement_Char_Indication() \n");
        status = HT_Collector_Enable_Temperature_Measurement_Char_Indication();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_Enable_Temperature_Measurement_Char_Indication() call: %02X\n", status);
        }
      }
      break;
       
      case APPL_ENABLE_INTERMEDIATE_TEMPERATURE_CHAR_NOTIFICATION :  /* 4.4.9 TP/THF/CO/BV-04-I */
      {        
        /* Enable Intermediate Temperature Char Descriptor for notifications */
        APPL_MESG_DBG(profiledbgfile,"Call HT_Collector_Enable_Intermediate_Temperature_Char_Notification() \n");
        status = HT_Collector_Enable_Intermediate_Temperature_Char_Notification();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_Enable_Intermediate_Temperature_Char_Notification() call: %02X\n", status);
        }
      }
      break;
      case APPL_ENABLE_MEASUREMENT_INTERVAL_CHAR_INDICATION :  /* 4.4.15 TP/THF/CO/BV-08-I */
      {        
        /* Enable Health Thermometer Measurement Char Descriptor for notifications */
        APPL_MESG_DBG(profiledbgfile,"Call HT_Collector_Enable_Measurement_Interval_Char_Indication() \n");
        status = HT_Collector_Enable_Measurement_Interval_Char_Indication();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_Enable_Measurement_Interval_Char_Indication() call: %02X\n", status);
        }
      }
      break;
      
      case APPL_READ_MEASUREMENT_INTERVAL_CHAR:  /* 4.4.17 TP/THF/CO/BV-10-I */
      {
        APPL_MESG_DBG(profiledbgfile,"Read Measurement Interval Characteristic Value.\n");
        /* Read Measurement Interval Characteristic */
        status = HT_Collector_Read_Measurement_Interval();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_Read_Measurement_Interval() call (Measurement Interval Characteristic): %02X\n", status);
        }
      }
      break;
            
      case APPL_WRITE_MEASUREMENT_INTERVAL_CHAR:  /* 4.4.18 TP/THF/CO/BV-11-I */
      {
        APPL_MESG_DBG(profiledbgfile,"Write Measurement Interval Characteristic Value.\n");
        /* Write Measurement Interval Characteristi */
        status = HT_Collector_Write_Measurement_Interval(2);
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_Write_Measurement_Interval() call (Measurement Interval Characteristic): %02X\n", status);
        }
      }
      break;

      case APPL_READ_MEASUREMENT_INTERVAL_VALID_RANGE_DESC:  /* 4.4.19 TP/THF/CO/BV-12-I */
      {
        bReadValidRangeDescr = TRUE;
        APPL_MESG_DBG(profiledbgfile,"Read Measurement Interval Valid Range Descriptor.\n");
        /* Read Measurement Interval Valid Range Descriptor */
        status = HT_Collector_Read_Measurement_Interval_Valid_Range_Descr();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_Read_Measurement_Interval_Valid_Range_Descr() call (Measurement Interval Valid Range Descriptor): %02X\n", status);
        }
      }
      break;

      case APPL_READ_TEMPERATURE_TYPE_CHAR:  /* 4.4.20 TP/THF/CO/BV-13-I */
      {
        APPL_MESG_DBG(profiledbgfile,"Read Temperature Type Characteristic Value.\n");
        /* Read Temperature Type Characteristic */
        status = HT_Collector_Read_Temperature_Type();
        if (status!= BLE_STATUS_SUCCESS)
        {
          APPL_MESG_DBG(profiledbgfile,"Error in the HT_Collector_Read_Temperature_Type() call (Temperature Type Characteristic): %02X\n", status);
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

void HT_Collector_DeviceDiscovery_CB(uint8_t status, uint8_t addr_type, uint8_t *addr, 
                                     uint8_t data_length, uint8_t *data, 
                                     uint8_t RSSI)
{
  uint8_t i;
  uint8_t name_len = 17;
  uint8_t offset = name_len;

  /* Check if the device found is a HealthThermometer */
  if (!memcmp(&data[5], "HealthThermometer", name_len)) {
    
    // name + | + *addr_type + | + addr
    // 17    + 1 + 1          + 1 + 6 = 26
    uint8_t dev[26];
    strcpy((char*)dev, "HealthThermometer");
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

void HT_Collector_ConnectionStatus_CB(uint8_t connection_evt, uint8_t status)
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

void HT_Collector_ServicesDiscovery_CB(uint8_t status, uint8_t numServices, uint8_t *services)
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

void HT_Collector_CharacOfService_CB(uint8_t status, uint8_t numCharac, uint8_t *charac)
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

void HT_Collector_CharacDesc_CB(uint8_t status, uint8_t numCharac, uint8_t *charac)
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

void HT_Collector_DataValueRead_CB(uint8_t status, uint16_t data_len, uint8_t *data)
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

void HT_Collector_FullConfError_CB(uint8_t error_type, uint8_t code)
{
  if (healthThermometerCollectorContext.fullConf) {
    APPL_MESG_INFO(profiledbgfile,"***** HT_Collector_FullConf_CB() Error Type = 0x%0x with Code = 0x%0x\r\n", error_type, code);
  }
}

void HT_Collector_PinCodeRequired_CB(void)
{
  uint8_t ret;
  uint32_t pinCode=0;

  APPL_MESG_INFO(profiledbgfile,"**** Required Security Pin Code\r\n");

  /* Insert the pin code according the glucose collector indication */
  //pinCode = ....

  ret = HT_Collector_SendPinCode(pinCode);
  if (ret != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"**** Error during the Pairing procedure -> Pin Code set (Status = 0x%x)\r\n", ret);
  }
}
                                        
void HT_Collector_NotificationReceived_CB(uint8_t handle, uint8_t length, uint8_t * data_value)
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
  
void HT_Collector_IndicationReceived_CB(uint8_t handle, uint8_t length, uint8_t * data_value)
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

void HT_Collector_Pairing_CB(uint16_t conn_handle, uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"Pairing CB conn_handle = 0x%04x with status = 0x%02x***************\n", conn_handle, status);
  
}

void HT_Collector_EnableNotificationIndication_CB(uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"**** Enable Notification/Indication procedure ended with Status = 0x%0x\r\n", status);
}

#endif    

