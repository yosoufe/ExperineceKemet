/**
******************************************************************************
* @file    hrm_profile_application.c 
* @author  AAS / CL
* @version V1.0.0
* @date    04-July-2014
* @brief   This file implements the main function executing the Heart Rate
*          Monitor profile.
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
#include "master_basic_profile.h"

#if (BLE_CURRENT_PROFILE_ROLES & HEART_RATE_COLLECTOR)
#include <heart_rate_service.h>
#include <heart_rate_collector_config.h>
#include <heart_rate_collector.h>

/*******************************************************************************
 * Macros
*******************************************************************************/
/* PERIPHERAL peer address */
#define PERIPHERAL_PEER_ADDRESS PTS_PERIPHERAL_ADDRESS //PTS_PERIPHERAL_ADDRESS or PEER_ADDRESS

/* Heart_Rate Collector test configuration for some PTS tests */

/* General operations: discovery, connection, disconnection, bonding, clear security database */
#define APPL_DISCOVER_HR_SENSOR                    'a' // Start Discovery procedure for detecting a heart rate sensor
#define APPL_CONNECT_HR_SENSOR                     'b' // Start Connection procedure for connecting to the discovered heart rate sensor
#define APPL_DISCONNECT_HR_SENSOR                  'c' // Terminate the current connection with the heart rate sensor
#define APPL_PAIR_WITH_HR_SENSOR                   'd' // Start the bonding  procedure (or enable the encryption) with the connected heart rate sensor
#define APPL_CLEAR_SECURITY_DATABASE               'e' // Clear the security database (it removes any bonded device) 

//------------- Discovery
#define APPL_DISCOVER_HEART_RATE_SERVICES          'f' //Discover All Primary Services
                                                       //Discover Heart Rate Sensor Services
                                                       //Discover Discover Device Information Service 
#define APPL_DISCOVER_DEVICE_INFORMATION_SERVICES  'g' //if the previous test is not performed
                                                       //Discover All Primary Services      

//#define APPL_VERIFY_VALID_BR_EDR                        '' //Verify Valid BR/EDR Behavior for Heart Rate Service

#define APPL_DISCOVER_HR_CHARACTERISTICS                'h' //Discover All Characteristics of Services
                                                            //Discover Heart Rate Measurement Characteristic
                                                            //Discover Body Sensor Location Characteristic
                                                            //Discover Heart Rate Control Point Characteristic                        
#define APPL_DISCOVER_HR_MEASUREMENT_CHAR_DESCRIPTOR    'i' //Discover Heart Rate Measurement Client Characteristic Configuration Descriptor
//#define APPL_DISCOVER_BODY_SENSOR_LOCATION_CHAR         'j' //Discover Body Sensor Location Characteristic
//#define APPL_DISCOVER_HR_CTRL_POINT_CHAR                'k' //Discover Heart Rate Control Point Characteristic

#define APPL_DISCOVER_DIS_CHARACTERISTICS               'l' // Discover Device Information Service (DIS) Characteristics
#define APPL_READ_DIS_MANUFACTURER_NAME_CHAR            'm' // Read Device Information Service Characteristics 

#define APPL_ENABLE_HR_MEASUREMENT_CHAR_NOTIFICATION    'n' //Configure Heart Rate Measurement for Notification

//----------------------------- NOTIFICATIONS
//Receive Heart Rate Measurement Notifications
//Receive Heart Rate Measurement Notifications with reserved flags
//Receive Heart Rate Measurement Notifications with additional octets not represented in the flags field
//Receive multiple Heart Rate Measurement Notifications

//----------------------------- READ BODY SENSOR LOCATION 
#define APPL_READ_BODY_SENSOR_LOCATION_CHAR             'o' //Read Body Sensor Location characteristic

#define APPL_WRITE_HR_CTRL_POINT_CHAR                   'p' //Write Heart Rate Control Point characteristic – reset energy expended

#define APPL_VERIFY_BOND_STATUS_ON_RECONNECTION         'q' //Verify Bond Status on Reconnection

#define APPL_START_FULL_CONFIGURATION                   'Z' // Starts all the connection, service discovery, characteristic discovery, .....

#define DISPLAY_PTS_MENU                                '?' //Display PTS application command menu


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

/******************************************************************************
 * Function Definitions 
******************************************************************************/
/**
 * @brief  Initialize the Application Context
 * for using the HR central profile methods
 * @param  None
 * @retval None
 */
void HR_Appl_Context_Init(void)
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
      case HEART_RATE_SERVICE_UUID:
        PACK_2_BYTE_PARAM16(uuid_array, uuid);
        notify_uart(uuid_array, sizeof(uuid_array), SERVICE_UUID);
	APPL_MESG_INFO(profiledbgfile,"****    Heart Rate Service UUID found\r\n");
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
    case HEART_RATE_MEASURMENT_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Heart Rate Measurement Characteristic found\r\n");
      break;
    case SENSOR_LOCATION_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Heart Rate Body Sensor Location Characteristic found\r\n");
      break;
    case CONTROL_POINT_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Heart Rate Control Point Characteristic found\r\n");
      break; 
    
    case MANUFACTURER_NAME_UUID:
      APPL_MESG_INFO(profiledbgfile,"****      Device Information Manufacturer Name Characteristic found\r\n");
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
    default:
      APPL_MESG_INFO(profiledbgfile,"****      Characteristic Descriptor found\r\n");
    }

    APPL_MESG_INFO(profiledbgfile,"****      Characteristic Descriptor Handle = 0x%04x\r\n", handle_characDesc);
    APPL_MESG_INFO(profiledbgfile,"****      UUID = 0x%04x\r\n", uuid_characDesc);

    index += sizeElement;
  }
}

void Display_Appl_Menu()
{
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_HR_SENSOR: %c\n", APPL_DISCOVER_HR_SENSOR);
  APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_HR_SENSOR: %c\n",APPL_CONNECT_HR_SENSOR);  
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_HR_SENSOR: %c\n",APPL_DISCONNECT_HR_SENSOR);
  APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_HR_SENSOR: %c\n",APPL_PAIR_WITH_HR_SENSOR);
  APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: %c\n",APPL_CLEAR_SECURITY_DATABASE);
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_HEART_RATE_SERVICES: %c\n",APPL_DISCOVER_HEART_RATE_SERVICES);
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_DEVICE_INFORMATION_SERVICES: %c\n",APPL_DISCOVER_DEVICE_INFORMATION_SERVICES);
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_HR_CHARACTERISTICS: %c\n",APPL_DISCOVER_HR_CHARACTERISTICS);
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_HR_MEASUREMENT_CHAR_DESCRIPTOR: %c\n",APPL_DISCOVER_HR_MEASUREMENT_CHAR_DESCRIPTOR);
  //APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_BODY_SENSOR_LOCATION_CHAR: %c\n",APPL_DISCOVER_BODY_SENSOR_LOCATION_CHAR); //TBR???
  //APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_HR_CTRL_POINT_CHAR: %c\n",APPL_DISCOVER_HR_CTRL_POINT_CHAR); //TBR??
  APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_DIS_CHARACTERISTICS: %c\n",APPL_DISCOVER_DIS_CHARACTERISTICS);
  APPL_MESG_DBG(profiledbgfile,"APPL_READ_DIS_MANUFACTURER_NAME_CHAR: %c\n",APPL_READ_DIS_MANUFACTURER_NAME_CHAR);
  APPL_MESG_DBG(profiledbgfile,"APPL_ENABLE_HR_MEASUREMENT_CHAR_NOTIFICATION: %c\n",APPL_ENABLE_HR_MEASUREMENT_CHAR_NOTIFICATION);
  APPL_MESG_DBG(profiledbgfile,"APPL_READ_BODY_SENSOR_LOCATION_CHAR: %c\n",APPL_READ_BODY_SENSOR_LOCATION_CHAR);
  APPL_MESG_DBG(profiledbgfile,"APPL_WRITE_HR_CTRL_POINT_CHAR: %c\n",APPL_WRITE_HR_CTRL_POINT_CHAR);
  APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_HR_SENSOR: %c\n",APPL_CONNECT_HR_SENSOR);
 // APPL_MESG_DBG(profiledbgfile,"APPL_VERIFY_BOND_STATUS_ON_RECONNECTION: %u\n",APPL_VERIFY_BOND_STATUS_ON_RECONNECTION);

  
  APPL_MESG_DBG(profiledbgfile,"APPL_START_FULL_CONFIGURATION: %c\n",APPL_START_FULL_CONFIGURATION);
  APPL_MESG_DBG(profiledbgfile,"DISPLAY_PTS_MENU: %c\n",DISPLAY_PTS_MENU);  
  
}/* end Display_Appl_Menu() */

void Device_Init(void)
{
  hrcInitDevType InitDev; 
  uint8_t public_addr[6]= HRC_PUBLIC_ADDRESS;
  uint8_t device_name[]=HRC_DEVICE_NAME;
  tBleStatus ble_status;

  //InitDev.public_addr =  public_addr;
  InitDev.public_addr =  public_addr;
  InitDev.txPower =      HRC_TX_POWER_LEVEL;
  InitDev.device_name_len = sizeof(device_name);
  InitDev.device_name = device_name;
  
  ble_status = HRC_Init(InitDev);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"HRC_Init() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Central initialized.\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HRC_Init() Error: %02X\n", ble_status);
  }
}

void Device_SetSecurity(void)
{
  hrcSecurityType param;
  tBleStatus ble_status;
  
  param.ioCapability =  HRC_IO_CAPABILITY;
  param.mitm_mode =     HRC_MITM_MODE;
  param.oob_enable =    HRC_OOB_ENABLE;
  param.bonding_mode =  HRC_BONDING_MODE;
  param.use_fixed_pin = HRC_USE_FIXED_PIN;
  param.fixed_pin =     HRC_FIXED_PIN;
  
  ble_status = HRC_SecuritySet(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"HRC_SecuritySet() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Security params set.\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HRC_SecuritySet() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_Discovery_Procedure(void)
{
  hrcDevDiscType param;
  tBleStatus ble_status;
  
  param.own_addr_type = PUBLIC_ADDR;
  param.scanInterval =  HRC_GEN_DISC_SCAN_INT;
  param.scanWindow =    HRC_GEN_DISC_SCAN_WIND;
  ble_status = HRC_DeviceDiscovery(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"HRC_DeviceDiscovery() Call: OK\n" );
    APPL_MESG_DBG(profiledbgfile,"Scanning...\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HRC_DeviceDiscovery() Error: %02X\n", ble_status);
  }
  
  return ble_status;
}

uint8_t Device_Connection_Procedure(uint8_t *peer_addr)
{
  hrcConnDevType param;
  tBleStatus ble_status;
  //uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS;

  param.fastScanDuration =         HRC_FAST_SCAN_DURATION;
  param.fastScanInterval =         HRC_FAST_SCAN_INTERVAL;
  param.fastScanWindow =           HRC_FAST_SCAN_WINDOW;
  param.reducedPowerScanInterval = HRC_REDUCED_POWER_SCAN_INTERVAL;
  param.reducedPowerScanWindow =   HRC_REDUCED_POWER_SCAN_WINDOW;
  param.peer_addr_type =           PUBLIC_ADDR;
  param.peer_addr =                peer_addr;
  param.own_addr_type =            PUBLIC_ADDR;
  param.conn_min_interval =        HRC_FAST_MIN_CONNECTION_INTERVAL;
  param.conn_max_interval =        HRC_FAST_MAX_CONNECTION_INTERVAL;
  param.conn_latency =             HRC_FAST_CONNECTION_LATENCY;
  param.supervision_timeout =      HRC_SUPERVISION_TIMEOUT;
  param.min_conn_length =          HRC_MIN_CONN_LENGTH;
  param.max_conn_length =          HRC_MAX_CONN_LENGTH;
  ble_status = HRC_DeviceConnection(param);
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"HRC_DeviceConnection() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HRC_DeviceConnection() Error: %02X\n", ble_status);
  }
  return ble_status;
}
 
void Device_StartPairing(void)
{
  tBleStatus ble_status;
  ble_status = HRC_StartPairing();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"HRC_StartPairing() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HRC_StartPairing() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_ServicesDiscovery(void)
{
  tBleStatus ble_status;
  ble_status = HRC_ServicesDiscovery();

  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_DBG(profiledbgfile,"HRC_ServicesDiscovery() Call: OK\n" );
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HRC_ServicesDiscovery() Error: %02X\n", ble_status);
  }
  
  return ble_status;
}

void Device_Disconnection(void)
{
  tBleStatus ble_status;
  ble_status = HRC_DeviceDisconnection();
  if (ble_status == BLE_STATUS_SUCCESS) 
  {
    //APPL_MESG_DBG(profiledbgfile,"HRC_DeviceDisconnection() Call: OK\n" );
    notify_uart(&ble_status, sizeof(ble_status), CLOSE_CONNECTION);
    APPL_MESG_DBG(profiledbgfile,"Disconnection complete.\n" );
    profileApplContext.deviceState = APPL_INIT_DONE;
  }
  else
  {
    APPL_MESG_DBG(profiledbgfile,"HRC_DeviceDisconnection() Error: %02X\n", ble_status);
  }
}

tBleStatus Device_Discovery_CharacServ(uint16_t uuid_service)
{
  tBleStatus ble_status;
  ble_status = HRC_DiscCharacServ(uuid_service);
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
  hrcConnDevType connParam;
  //hrcConfDevType confParam;
  uint8_t peer_addr[6] = PERIPHERAL_PEER_ADDRESS; 

  /* Connection Parameter */
  connParam.fastScanDuration =         HRC_FAST_SCAN_DURATION;
  connParam.fastScanInterval =         HRC_FAST_SCAN_INTERVAL;
  connParam.fastScanWindow =           HRC_FAST_SCAN_WINDOW;
  connParam.reducedPowerScanInterval = HRC_REDUCED_POWER_SCAN_INTERVAL;
  connParam.reducedPowerScanWindow =   HRC_REDUCED_POWER_SCAN_WINDOW;
  connParam.peer_addr_type =           PUBLIC_ADDR;
  connParam.peer_addr =                peer_addr;
  connParam.own_addr_type =            PUBLIC_ADDR;
  connParam.conn_min_interval =        HRC_FAST_MIN_CONNECTION_INTERVAL;
  connParam.conn_max_interval =        HRC_FAST_MAX_CONNECTION_INTERVAL;
  connParam.conn_latency =             HRC_FAST_CONNECTION_LATENCY;
  connParam.supervision_timeout =      HRC_SUPERVISION_TIMEOUT;
  connParam.min_conn_length =          HRC_MIN_CONN_LENGTH;
  connParam.max_conn_length =          HRC_MAX_CONN_LENGTH;

  /* Configuration Parameter */

  if (HRC_ConnConf(connParam) != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"Error in HRC_ConnConf()\r\n");
  } 
  //else {
    //glucoseCollectorContext.useConnConf = TRUE;
  //}
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
      
      /* process the timer Q */
      //Blue_NRG_Timer_Process_Q();
      
      /* Call the Profile Master role state machine */
      Master_Process(&initParam);
      
      if (heartRateCollectorContext.fullConf) 
        HRC_StateMachine(); 
      
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
        ret = Device_Discovery_CharacServ(HEART_RATE_SERVICE_UUID);
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
        }
        profileApplContext.deviceState = APPL_INIT_DONE; // Transitory
      }
      
      /* Get HR Measurement Char Descriptor */
      if(profileApplContext.deviceState == APPL_CHARS_DISCOVERED){
        ret = HRC_Start_Heart_Rate_Measurement_Characteristic_Descriptor_Discovery();
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
        }
        profileApplContext.deviceState = APPL_INIT_DONE; // Transitory
      }
      
      /* Read Body Sensor Location */
      if(profileApplContext.deviceState == APPL_GOT_CHAR_DESC){
        ret = HRC_Read_Body_Sensor_Location();
        if(ret != BLE_STATUS_SUCCESS) {
          APPL_MESG_INFO(profiledbgfile,"Disconnecting...\n");
          Device_Disconnection();
        }
        profileApplContext.deviceState = APPL_INIT_DONE; // Transitory
      }
      
      /* Enable Notification */
      if(profileApplContext.deviceState == APPL_VALUE_READ){
        ret = HRC_Enable_HR_Measurement_Char_Notification();
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
        uint8_t input = APPL_DISCOVER_HR_SENSOR;//200;
        deviceState = input;
        if (input>0)
        {
          APPL_MESG_DBG(profiledbgfile,"io--- input: %c\n",input); 
          continue;
          switch(input)
          {   
	  case DISPLAY_PTS_MENU: 
            
          case APPL_DISCOVER_HR_SENSOR :
	  case APPL_CONNECT_HR_SENSOR :
	  case APPL_DISCONNECT_HR_SENSOR :
	  case APPL_PAIR_WITH_HR_SENSOR :
	  case APPL_CLEAR_SECURITY_DATABASE :
	  case APPL_DISCOVER_HEART_RATE_SERVICES :
          case APPL_DISCOVER_DEVICE_INFORMATION_SERVICES:  
            //case APPL_VERIFY_VALID_BR_EDR :
	  case APPL_DISCOVER_HR_CHARACTERISTICS :
	  case APPL_DISCOVER_HR_MEASUREMENT_CHAR_DESCRIPTOR :
            //case APPL_DISCOVER_BODY_SENSOR_LOCATION_CHAR:  //TBR??
            //case APPL_DISCOVER_HR_CTRL_POINT_CHAR : //TBR??
	  case APPL_DISCOVER_DIS_CHARACTERISTICS :
	  case APPL_READ_DIS_MANUFACTURER_NAME_CHAR :
            //case APPL_READ_DIS_MODEL_NUMBER_CHAR :
            //case APPL_READ_DIS_SYSTEM_ID_CHAR :
	  case APPL_ENABLE_HR_MEASUREMENT_CHAR_NOTIFICATION :
            //case APPL_DISABLE_HR_MEASUREMENT_CHAR_NOTIFICATION :
	  case APPL_READ_BODY_SENSOR_LOCATION_CHAR :
	  case APPL_WRITE_HR_CTRL_POINT_CHAR :
	  case APPL_VERIFY_BOND_STATUS_ON_RECONNECTION :
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
      case APPL_DISCOVER_HR_SENSOR :
        {
          APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_HR_SENSOR : call Device_Discovery_Procedure() \n"); 
          Device_Discovery_Procedure();
        }
        break;
      case APPL_CONNECT_HR_SENSOR :
        {
          APPL_MESG_DBG(profiledbgfile,"APPL_CONNECT_HR_SENSOR : call Device_Connection_Procedure();() \n"); 
          //Device_Connection_Procedure();
        }
        break;
      case APPL_DISCONNECT_HR_SENSOR :
        {
          APPL_MESG_DBG(profiledbgfile,"APPL_DISCONNECT_HR_SENSOR: call Device_Disconnection() \n"); 
          Device_Disconnection();
        }
        break;
      case APPL_PAIR_WITH_HR_SENSOR :
        {
          APPL_MESG_DBG(profiledbgfile,"APPL_PAIR_WITH_HR_SENSOR: call Device_StartPairing() \n"); 
          Device_StartPairing();
        }
        break;
      case APPL_CLEAR_SECURITY_DATABASE:
        {
          APPL_MESG_DBG(profiledbgfile,"APPL_CLEAR_SECURITY_DATABASE: call HRC_Clear_Security_Database()\n"); 
          status = HRC_Clear_Security_Database();
          if (status == BLE_STATUS_SUCCESS) 
          {
            APPL_MESG_DBG(profiledbgfile,"HRC_Collector_Clear_Security_Database() Call: OK\n" );
          }
          else
          {
            APPL_MESG_DBG(profiledbgfile,"HRC_Collector_Clear_Security_Database() Error: %02X\n", status);
          }
        }
        break; 
      case APPL_DISCOVER_HEART_RATE_SERVICES :  /* 4.3.1 TP/HRD/CO/BV-01-I */                                                   
        {
          /* It discover the  services of the connected heart rate sensor device */
          APPL_MESG_DBG(profiledbgfile,"Call Device_ServicesDiscovery() x connected Heart Rate Device \n"); 
          Device_ServicesDiscovery();
        }
        break;
        
      case APPL_DISCOVER_HR_CHARACTERISTICS: 
        {
          /* It discovers all the characteristics of the connected heart rate service */
          APPL_MESG_DBG(profiledbgfile,"Call Device_Discovery_CharacServ() x Heart Rate Service\n"); 
          Device_Discovery_CharacServ(HEART_RATE_SERVICE_UUID);
        }
        break;
        
#if 0 //TBR???
      case APPL_DISCOVER_BODY_SENSOR_LOCATION_CHAR:
        {
          /* It discovers all the characteristics of the connected heart rate service */
          APPL_MESG_DBG(profiledbgfile,"Call HRC_Start_Sensor_Location_Characteristics_Discovery() x Heart Rate Service\n"); 
          status = HRC_Start_Sensor_Location_Characteristics_Discovery();
          if (status!= BLE_STATUS_SUCCESS)
          {
            APPL_MESG_DBG(profiledbgfile,"Error in the HRC_Start_Sensor_Location_Characteristics_Discovery() call: %02X\n", status);
          }
        }
        break;
        
      case APPL_DISCOVER_HR_CTRL_POINT_CHAR: 
        {
          /* It discovers all the characteristics of the connected heart rate service */
          APPL_MESG_DBG(profiledbgfile,"Call HRC_Start_Control_Point_Characteristics_Discovery() x Heart Rate Service\n"); 
          status = HRC_Start_Control_Point_Characteristics_Discovery();
          if (status!= BLE_STATUS_SUCCESS)
          {
            APPL_MESG_DBG(profiledbgfile,"Error in the HRC_Start_Control_Point_Characteristics_Discovery() call: %02X\n", status);
          }
        }
        break;
#endif 
        
      case APPL_DISCOVER_HR_MEASUREMENT_CHAR_DESCRIPTOR :            
        {
          /* It discovers the characteristic descriptors of the connected Heart Rate Sensor Measurement Context Characteristic */
          APPL_MESG_DBG(profiledbgfile,"Call HRC_Start_Heart_Rate_Measurement_Characteristic_Descriptor_Discovery() x Heart Rate Measurement Characteristic\n"); 
          status = HRC_Start_Heart_Rate_Measurement_Characteristic_Descriptor_Discovery();
          if (status!= BLE_STATUS_SUCCESS)
          {
            APPL_MESG_DBG(profiledbgfile,"Error in the HRC_Start_Heart_Rate_Measurement_Characteristic_Descriptor_Discovery() call: %02X\n", status);
          }
        }
        break;     
        
      case APPL_DISCOVER_DIS_CHARACTERISTICS: 
        {
          /* It discovers all the characteristics of the connected device information service */
          APPL_MESG_DBG(profiledbgfile,"APPL_DISCOVER_DIS_CHARACTERISTICS: call Device_Discovery_CharacServ() x Device Information Service\n"); 
          Device_Discovery_CharacServ(DEVICE_INFORMATION_SERVICE_UUID);
        }
        break;
        
      case APPL_READ_DIS_MANUFACTURER_NAME_CHAR: /* 4.3.9 TP//HRD/CO/BV-10-I */
        {
          APPL_MESG_DBG(profiledbgfile,"Read Device Information Service: Manufacturer Name Characteristic \n");        
          /* Read Device Info Manufacturer Name Characteristic */
          status = HRC_ReadDISManufacturerNameChar();
          if (status!= BLE_STATUS_SUCCESS)
          {
            APPL_MESG_DBG(profiledbgfile,"Error in the HRC_ReadDISManufacturerNameChar() call: %02X\n", status);
          }
        }
        break;
        
      case APPL_ENABLE_HR_MEASUREMENT_CHAR_NOTIFICATION :  /* 4.4.3 TP/HRF/CO/BV-03-I */
        {        
          /* Enable Heart Rate Measurement Char Descriptor for notifications */
          APPL_MESG_DBG(profiledbgfile,"Call HRC_Enable_HR_Measurement_Char_Notification() \n");
          status = HRC_Enable_HR_Measurement_Char_Notification();
          if (status!= BLE_STATUS_SUCCESS)
          {
            APPL_MESG_DBG(profiledbgfile,"Error in the HRC_Enable_HR_Measurement_Char_Notification() call: %02X\n", status);
          }
        }
        break;
        
      case APPL_READ_BODY_SENSOR_LOCATION_CHAR : /* 4.4.9 TP/HRF/CO/BI-03-I */
        {
          APPL_MESG_DBG(profiledbgfile,"Read Body Sensor Location Characteristic Value.\n");
          /* Read Body Sensor Location Characteristic */
          status = HRC_Read_Body_Sensor_Location();
          if (status!= BLE_STATUS_SUCCESS)
          {
            APPL_MESG_DBG(profiledbgfile,"Error in the HRC_Read_Body_Sensor_Location() call (Body Sensor Location Characteristic): %02X\n", status);
          }
        }
        break;
        
        
      case APPL_WRITE_HR_CTRL_POINT_CHAR :
        {
          APPL_MESG_DBG(profiledbgfile,"Write Heart Rate Control Point Characteristic Value.\n");
          /* Write Heart Rate Control Point Characteristi */
          status = HRC_Write_HR_Control_Point();
          if (status!= BLE_STATUS_SUCCESS)
          {
            APPL_MESG_DBG(profiledbgfile,"Error in the HRC_Write_HR_Control_Point() call (Heart Rate Control Point Characteristic): %02X\n", status);
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

void HRC_DeviceDiscovery_CB(uint8_t status, uint8_t addr_type, uint8_t *addr, 
                                     uint8_t data_length, uint8_t *data, 
                                     uint8_t RSSI)
{
  uint8_t i;
  uint8_t name_len = 15;
  uint8_t offset = name_len;
  
  /* Check if the device found is a HeartRateSensor */
  if (!memcmp(&data[5], "HeartRateSensor", name_len)) {
    
    // name + | + *addr_type + | + addr
    // 15    + 1 + 1          + 1 + 6 = 24
    uint8_t dev[24];
    strcpy((char*)dev, "HeartRateSensor");
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

void HRC_ConnectionStatus_CB(uint8_t connection_evt, uint8_t status)
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

void HRC_ServicesDiscovery_CB(uint8_t status, uint8_t numServices, uint8_t *services)
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

void HRC_CharacOfService_CB(uint8_t status, uint8_t numCharac, uint8_t *charac)
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

void HRC_CharacDesc_CB(uint8_t status, uint8_t numCharac, uint8_t *charac)
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

void HRC_DataValueRead_CB(uint8_t status, uint16_t data_len, uint8_t *data)
{
  uint8_t i;

  APPL_MESG_INFO(profiledbgfile,"****************************************\r\n");
  APPL_MESG_INFO(profiledbgfile,"**** Data Read Value Procedure: Status = 0x%02x\r\n", status);
  if (status == BLE_STATUS_SUCCESS) 
  {
    APPL_MESG_INFO(profiledbgfile,"****    Len data Read = %d\r\n", data_len);
    if (data_len == HR_READ_SIZE)
    {
      APPL_MESG_INFO(profiledbgfile,"**** HR Body Sensor Location Value: 0x%02x\n",heartRateCollectorContext.BodySensorLocValue);
    }
    else
    {
      APPL_MESG_INFO(profiledbgfile,"****    Data: ");
      for (i=0; i<data_len; i++) {
        APPL_MESG_INFO(profiledbgfile,"0x%0x (%d)", data[i], data[i]);
      }
      APPL_MESG_INFO(profiledbgfile,"\r\n");
    }
    profileApplContext.deviceState = APPL_VALUE_READ;
  }
}

void HRC_FullConfError_CB(uint8_t error_type, uint8_t code)
{
  if (heartRateCollectorContext.fullConf) {
    APPL_MESG_INFO(profiledbgfile,"***** HRC_FullConf_CB() Error Type = 0x%0x with Code = 0x%0x\r\n", error_type, code);
  }
}

void HRC_PinCodeRequired_CB(void)
{
  uint8_t ret;
  uint32_t pinCode=0;

  APPL_MESG_INFO(profiledbgfile,"**** Required Security Pin Code\r\n");

  /* Insert the pin code according the glucose collector indication */
  //pinCode = ....

  ret = HRC_SendPinCode(pinCode);
  if (ret != BLE_STATUS_SUCCESS) {
    APPL_MESG_INFO(profiledbgfile,"**** Error during the Pairing procedure -> Pin Code set (Status = 0x%x)\r\n", ret);
  }
}

#if 0 //TBR
void HRC_RACP_Response_CB(uint8_t err_code)
{
}
#endif 
                                        
void HRC_NotificationReceived_CB(uint8_t handle, uint8_t length, uint8_t * data_value)
{
  if(profileApplContext.deviceState != APPL_NOTIFICATION_ENABLED) {
    return;
  }
  
  APPL_MESG_INFO(profiledbgfile,"NOTIFICATION_DATA_RECEIVED: handle=0x%04x, data length=%d, data:\r\n",
	   handle, length);
    
  for (uint8_t i=0; i<length; i++) {
    APPL_MESG_INFO(profiledbgfile,"%02x ", data_value[i]);
  }
  APPL_MESG_INFO(profiledbgfile,"\r\n");  
}
  
void HRC_Pairing_CB(uint16_t conn_handle, uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"Pairing CB conn_handle = 0x%04x with status = 0x%02x***************\n", conn_handle, status);
  
}

void HRC_EnableNotificationIndication_CB(uint8_t status)
{
  APPL_MESG_INFO(profiledbgfile,"**** Enable Notification/Indication procedure ended with Status = 0x%0x\r\n", status);
}


#endif /* (BLE_CURRENT_PROFILE_ROLES & HEART_RATE_COLLECTOR) */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
