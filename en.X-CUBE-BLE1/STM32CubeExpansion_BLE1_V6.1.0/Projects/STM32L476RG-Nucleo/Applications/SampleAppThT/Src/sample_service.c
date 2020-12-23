/**
  ******************************************************************************
  * @file    sample_service.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    04-July-2014
  * @brief   Add a sample service using a vendor specific profile.
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
/* Includes ------------------------------------------------------------------*/
#include "sample_service.h"

/** @addtogroup Applications
 *  @{
 */

/** @addtogroup SampleAppThT
 *  @{
 */
 
/** @defgroup SAMPLE_SERVICE 
 * @{
 */

/** @defgroup SAMPLE_SERVICE_Private_Variables
 * @{
 */
/* Private variables ---------------------------------------------------------*/
volatile int connected = FALSE;
volatile uint8_t set_connectable = 1;
volatile uint16_t connection_handle = 0;
volatile uint8_t notification_enabled = FALSE;
volatile uint8_t start_read_tx_char_handle = FALSE;
volatile uint8_t start_read_rx_char_handle = FALSE;
volatile uint8_t end_read_tx_char_handle = FALSE;
volatile uint8_t end_read_rx_char_handle = FALSE;

uint16_t tx_handle;
uint16_t rx_handle;

uint16_t sampleServHandle, TXCharHandle, RXCharHandle;

extern uint8_t bnrg_expansion_board;
extern BLE_RoleTypeDef BLE_Role;
extern uint8_t throughput_test;
/**
 * @}
 */

/** @defgroup SAMPLE_SERVICE_Private_Macros
 * @{
 */
/* Private macros ------------------------------------------------------------*/
#define NUM_PACKETS 500 // Only used for throughput test (define THROUGHPUT_TEST)

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
  do {\
  	uuid_struct.uuid128[0] = uuid_0; uuid_struct.uuid128[1] = uuid_1; uuid_struct.uuid128[2] = uuid_2; uuid_struct.uuid128[3] = uuid_3; \
	uuid_struct.uuid128[4] = uuid_4; uuid_struct.uuid128[5] = uuid_5; uuid_struct.uuid128[6] = uuid_6; uuid_struct.uuid128[7] = uuid_7; \
	uuid_struct.uuid128[8] = uuid_8; uuid_struct.uuid128[9] = uuid_9; uuid_struct.uuid128[10] = uuid_10; uuid_struct.uuid128[11] = uuid_11; \
	uuid_struct.uuid128[12] = uuid_12; uuid_struct.uuid128[13] = uuid_13; uuid_struct.uuid128[14] = uuid_14; uuid_struct.uuid128[15] = uuid_15; \
  }while(0)
/**
 * @}
 */

/** @defgroup SAMPLE_SERVICE_Exported_Functions 
 * @{
 */ 
/**
 * @brief  Add a sample service using a vendor specific profile
 * @param  None
 * @retval Status
 */
tBleStatus Add_Sample_Service(void)
{
  tBleStatus ret;
  
  /*
  UUIDs:
  D973F2E0-B19E-11E2-9E96-0800200C9A66
  D973F2E1-B19E-11E2-9E96-0800200C9A66
  D973F2E2-B19E-11E2-9E96-0800200C9A66
  */
  
  const uint8_t service_uuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe0,0xf2,0x73,0xd9};
  const uint8_t charUuidTX[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe1,0xf2,0x73,0xd9};
  const uint8_t charUuidRX[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe2,0xf2,0x73,0xd9};
  
  ret = aci_gatt_add_serv(UUID_TYPE_128, service_uuid, PRIMARY_SERVICE, 7, &sampleServHandle); /* original is 9?? */
  if (ret != BLE_STATUS_SUCCESS) goto fail;    
  
  ret = aci_gatt_add_char(sampleServHandle, UUID_TYPE_128, charUuidTX, 20, CHAR_PROP_NOTIFY, ATTR_PERMISSION_NONE, 0,
                           16, 1, &TXCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;
  
  ret = aci_gatt_add_char(sampleServHandle, UUID_TYPE_128, charUuidRX, 20, CHAR_PROP_WRITE|CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,
                           16, 1, &RXCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;
  
  PRINTF("Sample Service added.\nTX Char Handle %04X, RX Char Handle %04X\n", TXCharHandle, RXCharHandle);
  return BLE_STATUS_SUCCESS; 
  
fail:
  PRINTF("Error while adding Sample Service.\n");
  return BLE_STATUS_ERROR ;
}

/**
 * @brief  Make the device connectable
 * @param  None 
 * @retval None
 */
void Make_Connection(void)
{  
  tBleStatus ret;
  
  
  if (BLE_Role == CLIENT) {
    
    PRINTF("Client Create Connection\n");
    tBDAddr bdaddr = {0xaa, 0x00, 0x00, 0xE1, 0x80, 0x02};
    
    BSP_LED_On(LED2); //To indicate the start of the connection and discovery phase
    
    /*
    Scan_Interval, Scan_Window, Peer_Address_Type, Peer_Address, Own_Address_Type, Conn_Interval_Min, 
    Conn_Interval_Max, Conn_Latency, Supervision_Timeout, Conn_Len_Min, Conn_Len_Max    
    */
    ret = aci_gap_create_connection(SCAN_P, SCAN_L, PUBLIC_ADDR, bdaddr, PUBLIC_ADDR, CONN_P1, CONN_P2, 0,
                                    SUPERV_TIMEOUT, CONN_L1 , CONN_L2); 
    
    if (ret != 0) {
      PRINTF("Error while starting connection.\n");
      Clock_Wait(100);
    }
    
  } else {
    
    const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'B','l','u','e','N','R','G','_','C','h','a','t'};
    
    /* disable scan response */
    hci_le_set_scan_resp_data(0,NULL);
    
    PRINTF("General Discoverable Mode ");
    /*
    Advertising_Event_Type, Adv_Interval_Min, Adv_Interval_Max, Address_Type, Adv_Filter_Policy,
    Local_Name_Length, Local_Name, Service_Uuid_Length, Service_Uuid_List, Slave_Conn_Interval_Min,
    Slave_Conn_Interval_Max
    */
    ret = aci_gap_set_discoverable(ADV_DATA_TYPE, 0, 0, PUBLIC_ADDR, NO_WHITE_LIST_USE,
                                   13, local_name, 0, NULL, 0, 0);
    PRINTF("%d\n",ret);
  }
}

/**
 * @brief  Discovery TX characteristic handle by UUID 128 bits
 * @param  None 
 * @retval None
 */
void startReadTXCharHandle(void)
{
  if (!start_read_tx_char_handle)
  {    
    PRINTF("Start reading TX Char Handle\n");
    
    const uint8_t charUuid128_TX[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe1,0xf2,0x73,0xd9};
    aci_gatt_disc_charac_by_uuid(connection_handle, 0x0001, 0xFFFF, UUID_TYPE_128, charUuid128_TX);
    start_read_tx_char_handle = TRUE;
  }
}

/**
 * @brief  Discovery RX characteristic handle by UUID 128 bits
 * @param  None 
 * @retval None
 */
void startReadRXCharHandle(void)
{  
  if (!start_read_rx_char_handle)
  {
    PRINTF("Start reading RX Char Handle\n");
    
    const uint8_t charUuid128_RX[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe2,0xf2,0x73,0xd9};
    aci_gatt_disc_charac_by_uuid(connection_handle, 0x0001, 0xFFFF, UUID_TYPE_128, charUuid128_RX);
    start_read_rx_char_handle = TRUE;
  }
}

/**
 * @brief  This function is used to receive data related to the sample service
 *         (received over the air from the remote board).
 * @param  data_buffer : pointer to store in received data
 * @param  Nb_bytes : number of bytes to be received
 * @retval None
 */
void receiveData(uint8_t* data_buffer, uint8_t Nb_bytes)
{
  BSP_LED_Toggle(LED2);

  for (int i = 0; i < Nb_bytes; i++) {
    PRINTF("%c", data_buffer[i]);
  }
}

/**
 * @brief  This function is used to send data related to the sample service
 *         (to be sent over the air to the remote board).
 * @param  data_buffer : pointer to data to be sent
 * @param  Nb_bytes : number of bytes to send
 * @retval None
 */
void sendData(uint8_t* data_buffer, uint8_t Nb_bytes)
{
  if (BLE_Role == SERVER) {
    
    if (throughput_test) {
      
      do {
        uint8_t data[20] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',0,0,0,0};
        
        static uint32_t packets = 0;
        static tClockTime time, time2;
        
        HOST_TO_LE_32(data+16, packets);
        
        if (packets==0) {
          PRINTF("Test start\n");
          time = Clock_Time();
        }      
        
        struct timer t;
        Timer_Set(&t, CLOCK_SECOND*10); 
        
        while (aci_gatt_update_char_value(sampleServHandle, TXCharHandle, 0, 20, data)==BLE_STATUS_INSUFFICIENT_RESOURCES) {
          // Radio is busy (buffer full).
          //PRINTF("Radio is busy (buffer full)\n");
          if (Timer_Expired(&t)) {
            break;
          }
        }
        
        packets++;
        
        if (packets != 0 && packets%NUM_PACKETS == 0) {
          time2 = Clock_Time();
          tClockTime diff = time2-time;
          PRINTF("%d packets. Elapsed time: %d ms. App throughput: %d kbps.\n", NUM_PACKETS, diff, (int)((float)NUM_PACKETS*20*8/diff));
          time = Clock_Time();
        }
      } while (1);
      
    } else {
      aci_gatt_update_char_value(sampleServHandle,TXCharHandle, 0, Nb_bytes, data_buffer);
    }

  } else {
    aci_gatt_write_without_response(connection_handle, rx_handle+1, Nb_bytes, data_buffer);
  }
}

/**
 * @brief  Enable notification
 * @param  None 
 * @retval None
 */
void enableNotification(void)
{
  uint8_t client_char_conf_data[] = {0x01, 0x00}; // Enable notifications
  struct timer t;
  Timer_Set(&t, CLOCK_SECOND*10);
  
  while (aci_gatt_write_charac_descriptor(connection_handle, tx_handle+2, 2, client_char_conf_data)==BLE_STATUS_NOT_ALLOWED) {
    /* Radio is busy */
    if (Timer_Expired(&t)) break;
  }
  notification_enabled = TRUE;
}

/**
 * @brief  This function is called when an attribute gets modified
 * @param  handle : handle of the attribute
 * @param  data_length : size of the modified attribute data
 * @param  att_data : pointer to the modified attribute data
 * @retval None
 */
void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data)
{
  if (handle == RXCharHandle + 1) {
    receiveData(att_data, data_length);
  } else if (handle == TXCharHandle + 2) {        
    if (att_data[0] == 0x01)
      notification_enabled = TRUE;
  }
}

/**
 * @brief  This function is called when there is a LE Connection Complete event.
 * @param  addr : Address of peer device
 * @param  handle : Connection handle
 * @retval None
 */
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle)
{  
  connected = TRUE;
  connection_handle = handle;
  
  PRINTF("Connected to device:");
  for (int i = 5; i > 0; i--) {
    PRINTF("%02X-", addr[i]);
  }
  PRINTF("%02X\n", addr[0]);
}

/**
 * @brief  This function is called when the peer device get disconnected.
 * @param  None 
 * @retval None
 */
void GAP_DisconnectionComplete_CB(void)
{
  connected = FALSE;
  
  PRINTF("Disconnected\n");
  /* Make the device connectable again. */
  set_connectable = TRUE;
  notification_enabled = FALSE;
  start_read_tx_char_handle = FALSE;
  start_read_rx_char_handle = FALSE;
  end_read_tx_char_handle = FALSE;
  end_read_rx_char_handle = FALSE;
}

/**
 * @brief  This function is called when there is a notification from the sever.
 * @param  attr_handle Handle of the attribute
 * @param  attr_len    Length of attribute value in the notification
 * @param  attr_value  Attribute value in the notification
 * @retval None
 */
void GATT_Notification_CB(uint16_t attr_handle, uint8_t attr_len, uint8_t *attr_value)
{
  if (throughput_test && BLE_Role == CLIENT) {
    static tClockTime time, time2;
    static uint32_t packets=0;
    static uint32_t n_packet1, n_packet2, lost_packets = 0;
    
    if (attr_handle == tx_handle+1) { 
        if (packets==0) {
            PRINTF("Test start\n");
            time = Clock_Time();
            n_packet1 = LE_TO_HOST_32(attr_value+16) - 1;
        }
               
        n_packet2 = LE_TO_HOST_32(attr_value+16);
        if (n_packet2 != n_packet1 + 1) {
          lost_packets += n_packet2-(n_packet1+1);
        }
        n_packet1 = n_packet2;
        
        packets++;
        //PRINTF("packet %d\n", packets);
        
        if (packets != 0 && packets%NUM_PACKETS == 0) {
            time2 = Clock_Time();
            tClockTime diff = time2-time;
            PRINTF("%d packets. Elapsed time: %d ms. App throughput: %d kbps.\n", NUM_PACKETS, diff, (int)((float)NUM_PACKETS*20*8/diff));            
            
            if (lost_packets) {
              PRINTF("%d lost packet(s)\n", lost_packets);
            }
            time = Clock_Time();
            lost_packets=0;
        }
        
    }
  } else {
    
    if (attr_handle == tx_handle+1) {
      receiveData(attr_value, attr_len);
    }
  }
}

/**
 * @brief  This function is called whenever there is an ACI event to be processed.
 * @note   Inside this function each event must be identified and correctly
 *         parsed.
 * @param  pckt  Pointer to the ACI packet
 * @retval None
 */
void HCI_Event_CB(void *pckt)
{
  hci_uart_pckt *hci_pckt = pckt;
  hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;
  
  if (hci_pckt->type != HCI_EVENT_PKT)
    return;
  
  switch (event_pckt->evt) {
    
  case EVT_DISCONN_COMPLETE:
    {
      GAP_DisconnectionComplete_CB();
    }
    break;
    
  case EVT_LE_META_EVENT:
    {
      evt_le_meta_event *evt = (void *)event_pckt->data;
      
      switch (evt->subevent) {
      case EVT_LE_CONN_COMPLETE:
        {
          evt_le_connection_complete *cc = (void *)evt->data;
          GAP_ConnectionComplete_CB(cc->peer_bdaddr, cc->handle);
        }
        break;
      }
    }
    break;
    
  case EVT_VENDOR:
    {
      evt_blue_aci *blue_evt = (void*)event_pckt->data;
      switch (blue_evt->ecode) {
        
      case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
        {
          if (bnrg_expansion_board == IDB05A1) {
            evt_gatt_attr_modified_IDB05A1 *evt = (evt_gatt_attr_modified_IDB05A1*)blue_evt->data;
            Attribute_Modified_CB(evt->attr_handle, evt->data_length, evt->att_data);
          }
          else {
            evt_gatt_attr_modified_IDB04A1 *evt = (evt_gatt_attr_modified_IDB04A1*)blue_evt->data;
            Attribute_Modified_CB(evt->attr_handle, evt->data_length, evt->att_data);
          }
          
        }
        break;
      case EVT_BLUE_GATT_NOTIFICATION:
        {
          evt_gatt_attr_notification *evt = (evt_gatt_attr_notification*)blue_evt->data;
          GATT_Notification_CB(evt->attr_handle, evt->event_data_length - 2, evt->attr_value);
        }
        break;
#if 1
      case EVT_BLUE_GATT_DISC_READ_CHAR_BY_UUID_RESP:
        if (BLE_Role == CLIENT) {
          PRINTF("EVT_BLUE_GATT_DISC_READ_CHAR_BY_UUID_RESP\n");
          
          evt_gatt_disc_read_char_by_uuid_resp *resp = (void*)blue_evt->data;
          
          if (start_read_tx_char_handle && !end_read_tx_char_handle)
          {
            tx_handle = resp->attr_handle;
            PRINTF("TX Char Handle %04X\n", tx_handle);
          }
          else if (start_read_rx_char_handle && !end_read_rx_char_handle)
          {
            rx_handle = resp->attr_handle;
            PRINTF("RX Char Handle %04X\n", rx_handle);
          }
        }
        break;
        
      case EVT_BLUE_GATT_PROCEDURE_COMPLETE:
        if (BLE_Role == CLIENT) {
          /* Wait for gatt procedure complete event trigger related to Discovery Charac by UUID */
          //evt_gatt_procedure_complete *pr = (void*)blue_evt->data;
          
          if (start_read_tx_char_handle && !end_read_tx_char_handle)
          {
            end_read_tx_char_handle = TRUE;
          }
          else if (start_read_rx_char_handle && !end_read_rx_char_handle)
          {
            end_read_rx_char_handle = TRUE;
          }
        }
        break;
#endif
      }
    }
    break;
  }    
}
/**
 * @}
 */
 
/**
 * @}
 */
 
/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
