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
*   FILENAME        -  profiles_ProximityMonitor_central_test_application.c
*
*******************************************************************************
*
*   CHANGE HISTORY
*  ---------------------------
*
*   $Date$:      09/01/2015
*   $Revision$:  first version
*   $Author$:    AMS VMA RF Application team
*   Comments:    Profiles PTS test application for Proximity Monitor. 
*                It uses the BlueNRG Host Profiles Library
*
*******************************************************************************
*
*  File Description 
*  ---------------------
*  Test application file for proximity monitor profile central role 
*
*******************************************************************************/

/*******************************************************************************
 * Include Files
*******************************************************************************/
#include "hci_tl.h"
#include "stdio.h"
#include "debug.h"
#include "timer.h"
#include "hci.h"
#include "link_layer.h"
#include "bluenrg_gatt_server.h"
#include "uuid.h"
#include "master_basic_profile.h"

#if (BLE_CURRENT_PROFILE_ROLES & PROXIMITY_MONITOR)

#include "proximity_monitor.h"
#include "proximity_monitor_config.h"

/*******************************************************************************
 * Macros
*******************************************************************************/

/* --------------------- User Inputs for Proximity Monitor PTS tests -----------------*/

/* General operations: discovery, connection, disconnection, bonding, clear security database */
#define APPL_DISCOVER_PROXIMITY_REPORTER                        'a' // Start Discovery procedure for detecting a proximity reporter
#define APPL_CONNECT_PROXIMITY_REPORTER                         'b' // Start Connection procedure for connecting to the discovered proximity reporter
#define APPL_DISCONNECT_PROXIMITY_REPORTER                      'c' // Terminate the current connection with the proximity reporter
#define APPL_PAIR_WITH_PROXIMITY_REPORTER                       'd' // Start the pairing  procedure (or enable the encryption) with the connected proximity reporter
#define APPL_CLEAR_SECURITY_DATABASE                            'e' // Clear the security database (it removes any bonded device) 
#define APPL_START_FULL_CONFIGURATION                           'z' // Starts all the connection, service discovery, characteristic discovery, configuration level...
#define APPL_PTS_COMMAND_MENU                                   'h' // PTS Command Menu help

/* ---------- 4.3: Discover Services and Characteristics tests */
#define APPL_DISCOVERY_PROXIMITY_REPORTER_SERVICES              'f' // TP/PXS/PM/BV-04-I [Discover Link Loss Service using GATT for LE]
                                                                    // TP/PXS/PM/BV-05-I [Discover Immediate Alert Service using GATT for LE]
                                                                    // TP/PXS/PM/BV-06-I [Discover Tx Power Service using GATT for LE]
#define APPL_DISCOVERY_ALERT_LEVEL_LINK_LOSS_CHAR               'g' // TP/PXS/PM/BV-07-I [Discover Alert Level Characteristic for Link Loss]
#define APPL_DISCOVERY_ALERT_LEVEL_IMMED_ALERT_CHAR             'i' // TP/PXS/PM/BV-08-I [Discover Alert Level Characteristic for Immediate Alert]
#define APPL_DISCOVERY_TX_POWER_LEVEL_CHAR                      'j' // TP/PXS/PM/BV-09-I [Discover Tx Power Level Characteristic]
#define APPL_DISCOVERY_TX_POWER_LEVEL_CHARAC_DESC               'k' // TP/PXS/PM/BV-10-I [Discover Tx Power Level - Characteristic Presentation Format Descriptor]
                                                                    // TP/PXS/PM/BV-11-I [Discover Tx Power Level - Client Characteristic Configuration Descriptor]
#define APPL_CONFIGURE_ALERT_LEVEL_OF_LINK_LOSS_SERVICE         'l' // TP/PXF/PM/BV-01-I [Configuration of Alert Level]
#define APPL_READ_TX_POWER                                      'm' // TP/PXF/PM/BV-03-I [Read Tx Power]
#define APPL_ENABLE_TX_POWER_NOTIFICATION                       'n' // TP/PXF/PM/BV-04-I [Tx Power Notification]
#define APPL_START_IMMEDIATE_ALERT                              'o' // TP/PXF/PM/BV-05-I [Alert on Exceeding Path Loss]
#define APPL_STOP_IMMEDIATE_ALERT                               'p' // TP/PXF/PM/BV-06-I [Cancel Alert on Reducing Path Loss]

/******************************************************************************
 * Global Variables
******************************************************************************/
uint8_t useConnConf = FALSE;
extern initDevType initParam;

/******************************************************************************
 * Function Declarations
******************************************************************************/

/******************************************************************************
 * Function Definitions 
******************************************************************************/

void printDebugServices(uint8_t numReporterServices, uint8_t *reporterServices)
{
  uint8_t i , j;
  uint16_t handle, uuid;

  printf("****    Number of Primary Service: %d\r\n", numReporterServices);
  j = 0;
  for (i=0; i<numReporterServices; i++) {
    if (reporterServices[j] == UUID_TYPE_16) {
      memcpy(&uuid, &reporterServices[j+5], 2);
      switch (uuid) {
      case LINK_LOSS_SERVICE_UUID:
	printf("****    Link Loss Service UUID found\r\n");
	break;
      case TX_POWER_SERVICE_UUID:
	printf("****    Tx Power Service UUID found\r\n");
	break;
      case IMMEDIATE_ALERT_SERVICE_UUID:
	printf("****    Immediate Alert Service UUID found \r\n");
	break;
      default:
	printf("****    UUID 0x%04x\r\n", uuid);
      }
    }
    j++;
    memcpy(&handle, &reporterServices[j], 2);
    printf("****       Start Handle = 0x%04x\r\n", handle);
    j += 2;
    memcpy(&handle, &reporterServices[j], 2);
    printf("****       End Handle = 0x%04x\r\n", handle);
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

  printf("****    Number of Charac of a Service: %d\r\n", numCharac);
  index = 0;
  for (i=0; i<numCharac; i++) { 
    if (charac[index] == 1) {
      sizeElement = 8;
    memcpy(&uuid_charac, &charac[index+6], 2);
    } else {
      sizeElement = 22;
    }

    switch(uuid_charac) {
    case ALERT_LEVEL_CHARACTERISTIC_UUID:
      printf("****      Alert Level Characteristic found\r\n");
      break;
    case TX_POWER_LEVEL_CHARACTERISTIC_UUID:
      printf("****      Tx Power Level Characteristic found\r\n");
      break;
    default:
      printf("****      Characteristic found\r\n");
    }

    memcpy(&charac_handle, &charac[index+1], 2);
    properties = charac[index+3];
    memcpy(&value_handle, &charac[index+4], 2);
    printf("****      Characteristic Handle = 0x%04x\r\n", charac_handle);
    printf("****      Properties = 0x%02x\r\n", properties);
    printf("****      ValueHandle = 0x%04x\r\n", value_handle);
    printf("****      UUID = 0x%04x\r\n", uuid_charac);

    index += sizeElement;
  }
}

void printDebugCharacDesc(uint8_t numCharac, uint8_t *charac)
{
  uint8_t i, index, sizeElement;
  uint16_t handle_characDesc, uuid_characDesc;

  printf("****    Number of Charac Descriptor: %d\r\n", numCharac);
  index = 0;
  for (i=0; i<numCharac; i++) { 
    if (charac[index] == 1) {
      sizeElement = 5;
      memcpy(&uuid_characDesc, &charac[index+3], 2);
    } else {
      sizeElement = 19;
    }

    memcpy(&handle_characDesc, &charac[index+1], 2);

    if (CHAR_PRESENTATION_FORMAT_DESCRIPTOR_UUID) {
    }
    switch(uuid_characDesc) {
    case CHAR_PRESENTATION_FORMAT_DESCRIPTOR_UUID:
      printf("****      Characteristic Presentation Format Descriptor found\r\n");
      break;
    case CLIENT_CHAR_CONFIG_DESCRIPTOR_UUID:
      printf("****      Client Characteristic Configuration Descriptor found\r\n");
      break;
    default:
      printf("****      Characteristic Descriptor found\r\n");
    }

    printf("****      Characteristic Descriptor Handle = 0x%04x\r\n", handle_characDesc);
    printf("****      UUID = 0x%04x\r\n", uuid_characDesc);

    index += sizeElement;
  }
}

void Display_Appl_Menu()
{
  printf("APPL_DISCOVER_PROXIMITY_REPORTER:                 %c\n", APPL_DISCOVER_PROXIMITY_REPORTER);
  printf("APPL_CONNECT_PROXIMITY_REPORTER:                  %c\n", APPL_CONNECT_PROXIMITY_REPORTER);
  printf("APPL_DISCONNECT_PROXIMITY_REPORTER:               %c\n", APPL_DISCONNECT_PROXIMITY_REPORTER); 
  printf("APPL_PAIR_WITH_PROXIMITY_REPORTER:                %c\n", APPL_PAIR_WITH_PROXIMITY_REPORTER);
  printf("APPL_CLEAR_SECURITY_DATABASE:                     %c\n", APPL_CLEAR_SECURITY_DATABASE);
  printf("APPL_DISCOVERY_PROXIMITY_REPORTER_SERVICES:       %c\n", APPL_DISCOVERY_PROXIMITY_REPORTER_SERVICES);
  printf("APPL_DISCOVERY_ALERT_LEVEL_LINK_LOSS_CHAR:        %c\n", APPL_DISCOVERY_ALERT_LEVEL_LINK_LOSS_CHAR);
  printf("APPL_DISCOVERY_ALERT_LEVEL_IMMED_ALERT_CHAR:      %c\n", APPL_DISCOVERY_ALERT_LEVEL_IMMED_ALERT_CHAR);
  printf("APPL_DISCOVERY_TX_POWER_LEVEL_CHAR:               %c\n", APPL_DISCOVERY_TX_POWER_LEVEL_CHAR);
  printf("APPL_DISCOVERY_TX_POWER_LEVEL_CHARAC_DESC:        %c\n", APPL_DISCOVERY_TX_POWER_LEVEL_CHARAC_DESC);
  printf("APPL_CONFIGURE_ALERT_LEVEL_OF_LINK_LOSS_SERVICE:  %c\n", APPL_CONFIGURE_ALERT_LEVEL_OF_LINK_LOSS_SERVICE);
  printf("APPL_READ_TX_POWER:                               %c\n", APPL_READ_TX_POWER);
  printf("APPL_ENABLE_TX_POWER_NOTIFICATION:                %c\n", APPL_ENABLE_TX_POWER_NOTIFICATION);
  printf("APPL_START_IMMEDIATE_ALERT:                       %c\n", APPL_START_IMMEDIATE_ALERT);
  printf("APPL_STOP_IMMEDIATE_ALERT:                        %c\n", APPL_STOP_IMMEDIATE_ALERT);
  printf("APPL_START_FULL_CONFIGURATION:                    %c\n", APPL_START_FULL_CONFIGURATION);
  printf("APPL_PTS_COMMAND_MENU                             %c\n", APPL_PTS_COMMAND_MENU);
    
}/* end Display_Appl_Menu() */

void Device_Init(void)
{
  pxmInitDevType pmInitDev; 
  uint8_t public_addr[6]=PM_PUBLIC_ADDRESS;
  uint8_t device_name[]=PM_DEVICE_NAME;

  useConnConf = FALSE;

  pmInitDev.public_addr =  public_addr;
  pmInitDev.txPower =      PM_TX_POWER_LEVEL;
  pmInitDev.device_name_len = sizeof(device_name);
  pmInitDev.device_name = device_name;
  if (PXM_Init(pmInitDev) != BLE_STATUS_SUCCESS)
    printf("Error in PXM_Init()\r\n");
}

void Device_SetSecurity(void)
{
  pxmSecurityType param;

  param.ioCapability =  PM_IO_CAPABILITY;
  param.mitm_mode =     PM_MITM_MODE;
  param.oob_enable =    PM_OOB_ENABLE;
  param.bonding_mode =  PM_BONDING_MODE;
  param.use_fixed_pin = PM_USE_FIXED_PIN;
  param.fixed_pin =     PM_FIXED_PIN;
  if (PXM_SecuritySet(param) != BLE_STATUS_SUCCESS) {
    printf("Error in PXM_SecuritySet() function call\r\n");
  }
}

void Device_Discovery_Procedure(void)
{
  pxmDevDiscType param;

  param.own_addr_type = PUBLIC_ADDR;
  param.scanInterval =  PM_LIM_DISC_SCAN_INT;
  param.scanWindow =    PM_LIM_DISC_SCAN_WIND;
  if (PXM_DeviceDiscovery(param) != BLE_STATUS_SUCCESS) {
    printf("Error in PXM_DeviceDiscovery() function call\r\n");
  }
}

void Device_Connection_Procedure(void)
{
  pxmConnDevType param;
  uint8_t pts_peer_addr[6] = PTS_PERIPHERAL_ADDRESS; 

  param.fastScanDuration =         PM_FAST_SCAN_DURATION;
  param.fastScanInterval =         PM_FAST_SCAN_INTERVAL;
  param.fastScanWindow =           PM_FAST_SCAN_WINDOW;
  param.reducedPowerScanInterval = PM_REDUCED_POWER_SCAN_INTERVAL;
  param.reducedPowerScanWindow =   PM_REDUCED_POWER_SCAN_WINDOW;
  param.peer_addr_type =           PUBLIC_ADDR;
  param.peer_addr =                pts_peer_addr;
  param.own_addr_type =            PUBLIC_ADDR;
  param.conn_min_interval =        PM_FAST_MIN_CONNECTION_INTERVAL;
  param.conn_max_interval =        PM_FAST_MAX_CONNECTION_INTERVAL;
  param.conn_latency =             PM_FAST_CONNECTION_LATENCY;
  param.supervision_timeout =      PM_SUPERVISION_TIMEOUT;
  param.min_conn_length =          PM_MIN_CONN_LENGTH;
  param.max_conn_length =          PM_MAX_CONN_LENGTH;
  if (PXM_DeviceConnection(param) != BLE_STATUS_SUCCESS) {
    printf("Error in PXM_DeviceConnection()\r\n");
  }
}
 
void Device_StartPairing(void)
{
  if (PXM_StartPairing()) {
    printf("Error in PXM_StartPairing()\r\n");
  }
}

void Device_ServicesDiscovery(void)
{
  if (PXM_ServicesDiscovery()) {
    printf("Error in PXM_ServicesDiscovery()\r\n");
  }
}

void Device_Disconnection(void)
{
  if (PXM_DeviceDisconnection()) {
    printf("Error in PXM_DisconnectionDevice()\r\n");
  }
}

void Device_Discovery_CharacServ(uint16_t uuid_service)
{
  if (PXM_DiscCharacServ(uuid_service)) {
    printf("Error in PXM_DiscCharacServ()\r\n");
  }
}

void Device_Discovery_TxPwdLvl_CharacDesc(void)
{
  if (PXM_TxPwrLvl_DiscCharacDesc()) {
    printf("Error in PXM_TxPwrLvl_DiscCharacDesc()\r\n");
  }
}

void Device_ConfigureAlertLevel(void)
{
  if (PXM_ConfigureLinkLossAlert(MILD_ALERT)) {
    printf("Error in PXM_ConfigureLinkLossAlert()\r\n");
  }
}

void Device_ReadTxPower(void) 
{
  if (PXM_ReadTxPower()) {
    printf("Error in PXM_ReadTxPower()\r\n");
  }
}

void Device_EnableTxPowerNotification(void)
{
  if (PXM_EnableTxPowerNotification()) {
    printf("Error in PXM_EnableTxPowerNotification()\r\n");
  }
}

void Device_StartImmediateAlert(void) 
{
  if (PXM_ConfigureImmediateAlert(HIGH_ALERT)) {
    printf("Error in PXM_ConfigureImmediateAlert()\r\n");
  }
}

void Device_StopImmediateAlert(void)
{
  if (PXM_ConfigureImmediateAlert(NO_ALERT)) {
    printf("Error in PXM_ConfigureImmediateAlert()\r\n");
  }
}

void Device_StartFullConfig(void)
{
  pxmConnDevType connParam;
  pxmConfDevType confParam;
  uint8_t pts_peer_addr[6] = PTS_PERIPHERAL_ADDRESS; //PERIPHERAL_PEER_ADDRESS; 

  /* Connection Parameter */
  connParam.fastScanDuration =         PM_FAST_SCAN_DURATION;
  connParam.fastScanInterval =         PM_FAST_SCAN_INTERVAL;
  connParam.fastScanWindow =           PM_FAST_SCAN_WINDOW;
  connParam.reducedPowerScanInterval = PM_REDUCED_POWER_SCAN_INTERVAL;
  connParam.reducedPowerScanWindow =   PM_REDUCED_POWER_SCAN_WINDOW;
  connParam.peer_addr_type =           PUBLIC_ADDR;
  connParam.peer_addr =                pts_peer_addr;
  connParam.own_addr_type =            PUBLIC_ADDR;
  connParam.conn_min_interval =        PM_FAST_MIN_CONNECTION_INTERVAL;
  connParam.conn_max_interval =        PM_FAST_MAX_CONNECTION_INTERVAL;
  connParam.conn_latency =             PM_FAST_CONNECTION_LATENCY;
  connParam.supervision_timeout =      PM_SUPERVISION_TIMEOUT;
  connParam.min_conn_length =          PM_MIN_CONN_LENGTH;
  connParam.max_conn_length =          PM_MAX_CONN_LENGTH;

  /* Configuration Parameter */
  confParam.llAlertLevel = HIGH_ALERT;
  confParam.iaAlertLevel = MILD_ALERT;
  confParam.alertPathLossEnabled = PM_ALERT_PATH_LOSS_ENABLED;
  confParam.pathLossTheshold = PM_PATH_LOSS_THRESHOLD;
  confParam.pathLossTimeout = PM_PATH_LOSS_TIMEOUT;

  if (PXM_ConnConf(connParam, confParam) != BLE_STATUS_SUCCESS) {
    printf("Error in PXM_ConnConf()\r\n");
  } else {
    useConnConf = TRUE;
  }
}

void runProcedure(uint8_t input)
{
  switch(input) {
  case APPL_DISCOVER_PROXIMITY_REPORTER:
    Device_Discovery_Procedure();
    break;
  case APPL_CONNECT_PROXIMITY_REPORTER:
    Device_Connection_Procedure();
    break;
  case APPL_DISCONNECT_PROXIMITY_REPORTER:
    Device_Disconnection();
    break;
  case APPL_PAIR_WITH_PROXIMITY_REPORTER:
    Device_StartPairing();
    break;
  case APPL_DISCOVERY_PROXIMITY_REPORTER_SERVICES:
    Device_ServicesDiscovery();
    break;
  case APPL_DISCOVERY_ALERT_LEVEL_LINK_LOSS_CHAR:
    Device_Discovery_CharacServ(LINK_LOSS_SERVICE_UUID);
    break;
  case APPL_DISCOVERY_ALERT_LEVEL_IMMED_ALERT_CHAR:
    Device_Discovery_CharacServ(IMMEDIATE_ALERT_SERVICE_UUID);
    break;
  case APPL_DISCOVERY_TX_POWER_LEVEL_CHAR:
    Device_Discovery_CharacServ(TX_POWER_SERVICE_UUID);
    break;
  case APPL_DISCOVERY_TX_POWER_LEVEL_CHARAC_DESC:
    Device_Discovery_TxPwdLvl_CharacDesc();
    break;
  case APPL_CONFIGURE_ALERT_LEVEL_OF_LINK_LOSS_SERVICE:
    Device_ConfigureAlertLevel();
    break;
  case APPL_READ_TX_POWER:
    Device_ReadTxPower();
    break;
  case APPL_ENABLE_TX_POWER_NOTIFICATION:
    Device_EnableTxPowerNotification();
    break;
  case APPL_START_IMMEDIATE_ALERT:
    Device_StartImmediateAlert();
    break;
  case APPL_STOP_IMMEDIATE_ALERT:
    Device_StopImmediateAlert();
    break;
  case APPL_START_FULL_CONFIGURATION:
    Device_StartFullConfig();
    break;
  case APPL_PTS_COMMAND_MENU:
    Display_Appl_Menu();
    break;
  }
}

void Host_Profile_Test_Application (void)
{
  uint8_t input = APPL_DISCOVER_PROXIMITY_REPORTER;
  
  printf("Test Application: Starting testing the BLE Proximity Profile \n" );
  printf("Digit h for to print instruction supported\n");

  /* Init Proximity Monitor Central Role */  
  Device_Init();

  /* Init Profile Security */
  Device_SetSecurity();

  while(1)
  {
    hci_user_evt_proc();
    
    /* process the timer Q */
    Blue_NRG_Timer_Process_Q();
    
    /* Call the Profile Master role state machine */
    Master_Process(&initParam);
    
    if (useConnConf) 
      PXM_StateMachine();
  
    /* if the profiles have nothing more to process, then
     * wait for application input
     */ 
    {      
      if (input>0)
      {
        printf("io--- input: %c\n",input); 
	runProcedure(input);
      }
    }
  }
}

void PXM_DeviceDiscovery_CB(uint8_t status, uint8_t addr_type, uint8_t *addr, 
			    uint8_t data_length, uint8_t *data, 
			    uint8_t RSSI)
{
  uint8_t addr_to_find[6]=PTS_PERIPHERAL_ADDRESS;
  
  if (memcmp(addr, addr_to_find, 6) == 0) {
    printf("****************************************\r\n");
    printf("*\r\n");
    printf("**** Proximity Reporter Found\r\n");
    printf("*\r\n");
    printf("****************************************\r\n");
  }
}

void PXM_ConnectionStatus_CB(uint8_t connection_evt, uint8_t status)
{
  printf("****************************************\r\n");
  switch(connection_evt) {
  case CONNECTION_ESTABLISHED_EVT:
    if (status == BLE_STATUS_SUCCESS) {
      printf("****  Connection Established with Success\r\n");
    }
    break;
  case CONNECTION_FAILED_EVT:
    printf("****  Connection Failed with Status = 0x%02x\r\n", status);
    break;
  case DISCONNECTION_EVT:
    printf("****  Disconnection with peer device\r\n");
    break;
  }
  printf("****************************************\r\n");
}

void PXM_ServicesDiscovery_CB(uint8_t status, uint8_t numServices, uint8_t *services)
{
  printf("****************************************\r\n");
  printf("**** Primary Service Discovery Procedure\r\n");
  printf("****    Status = 0x%02x\r\n", status);
  if (status == BLE_STATUS_SUCCESS)
    printDebugServices(numServices, services);
  printf("****************************************\r\n");  
}

void PXM_CharacOfService_CB(uint8_t status, uint8_t numCharac, uint8_t *charac)
{
  printf("****************************************\r\n");
  printf("**** Discovery Characterisitc of a Service Procedure\r\n");
  printf("****    Status = 0x%02x\r\n", status);
  if (status == BLE_STATUS_SUCCESS)
    printDebugCharac(numCharac, charac);
  printf("****************************************\r\n");  
}

void PXM_CharacDesc_CB(uint8_t status, uint8_t numCharac, uint8_t *charac)
{
  printf("****************************************\r\n");
  printf("**** Discovery Characterisitc Descriptor Procedure\r\n");
  printf("****    Status = 0x%02x\r\n", status);
  if (status == BLE_STATUS_SUCCESS)
    printDebugCharacDesc(numCharac, charac);
  printf("****************************************\r\n");    
}

void PXM_ConfigureAlert_CB(uint8_t status)
{
  printf("Alert Level Configured with status 0x%0x\r\n", status);
}

void PXM_DataValueRead_CB(uint8_t status, uint16_t data_len, uint8_t *data)
{
  uint8_t i;

  printf("****************************************\r\n");
  printf("**** Data Read Value Procedure\r\n");
  printf("****   Status = 0x%02x\r\n", status);
  if (status == BLE_STATUS_SUCCESS) {
    printf("****    Len data Read = %d\r\n", data_len);
    printf("****    Data: ");
    for (i=0; i<data_len; i++) {
      printf("0x%0x (%d)", data[i], (int8_t)data[i]);
    }
    printf("\r\n");
  }
}

void PXM_LinkLossAlert(uint8_t level)
{
  printf("**** Alert on Link Loss with Level %d\r\n", level);
}

void PXM_PathLossAlert(uint8_t level)
{
  printf("**** Alert on Path Loss with Level %d\r\n", level);
}

void PXM_TxPowerNotificationReceived(int8_t data_value)
{
  printf("**** Tx Power Level Notification Received---> Level = %d dBm\r\n", data_value);
}

void PXM_FullConfError_CB(uint8_t error_type, uint8_t code)
{
  if (useConnConf) {
    printf("***** PXM_FullConf_CB() Error Type = 0x%0x with Code = 0x%0x\r\n", error_type, code);
  }
}

void PXM_PinCodeRequired_CB(void)
{
  uint8_t ret;
  uint32_t pinCode=0;

  printf("**** Required Security Pin Code\r\n");

  /* Insert the pin code according the proximity reporter indication */
  //pinCode = ....

  ret = PXM_SendPinCode(pinCode);
  if (ret != BLE_STATUS_SUCCESS) {
    printf("**** Error during the Pairing procedure -> Pin Code set (Status = 0x%x)\r\n", ret);
  }
}

void PXM_EnableNotification_CB(uint8_t status)
{
  printf("**** Enable Tx Power Notification procedure ended with Status = 0x%0x\r\n", status);
}

#endif //BLE_CURRENT_PROFILE_ROLES & PROXIMITY_MONITOR
