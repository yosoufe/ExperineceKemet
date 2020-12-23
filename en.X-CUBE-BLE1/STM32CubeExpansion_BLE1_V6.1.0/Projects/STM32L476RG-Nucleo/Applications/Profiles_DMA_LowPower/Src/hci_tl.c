/**
  ******************************************************************************
  * @file    hci_dma_lp.c 
  * @author  AMS/HESA Application Team
  * @brief   Function for managing HCI interface.
  ******************************************************************************
  *
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  */ 

#include "bluenrg_types.h"
#include "bluenrg_def.h"
#include "hal.h"
#include "hci_const.h"
#include "gp_timer.h"

#include "hci_tl_interface.h"
#include "stm32xx_timerserver.h"

extern SPI_HandleTypeDef SpiHandle;

#if BLE_CONFIG_DBG_ENABLE
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define HCI_LOG_ON 0

#define HCI_READ_PACKET_NUM_MAX 		 (5)

#define MIN(a,b)            ((a) < (b) )? (a) : (b)
#define MAX(a,b)            ((a) > (b) )? (a) : (b)

typedef enum
{
  Event_Requested,
  Waiting_For_Buffer
} Event_Request_Status_t;

static void enqueue_packet(tHciDataPacket * hciReadPacket);
static void hci_timeout_callback(void);
static void Event_Request(void);

tListNode hciReadPktPool;
tListNode hciReadPktRxQueue;
/* pool of hci read packets */
static tHciDataPacket     hciReadPacketBuffer[HCI_READ_PACKET_NUM_MAX];

static volatile uint8_t readPacketListFull=FALSE;

static uint8_t *hci_buffer = NULL;
static volatile uint16_t hci_pckt_len;
static volatile uint8_t hci_timer_id;
static volatile uint8_t hci_timeout;
static volatile Event_Request_Status_t Event_Request_Status;

uint8_t header[HCI_HDR_SIZE + HCI_COMMAND_HDR_SIZE];

static void Event_Request(void)
{
  tHciDataPacket * hciReadPacket;

  if(Event_Request_Status == Waiting_For_Buffer)
  {
    if (list_is_empty (&hciReadPktPool) == FALSE)
    {
      Event_Request_Status = Event_Requested;
      list_remove_head (&hciReadPktPool, (tListNode **)&hciReadPacket);
      Hal_Event_Request ((uint8_t*)hciReadPacket, HCI_READ_PACKET_SIZE);
    }
  }

  return;
}

void hci_timeout_callback(void)
{
	hci_timeout = 1;

	return;
}

void hci_init(void(* UserEvtRx)(void* pData), void* pConf)
{
    uint8_t index;
    
    /* Initialize list heads of ready and free hci data packet queues */
    list_init_head (&hciReadPktPool);
    list_init_head (&hciReadPktRxQueue);
    
    /* Initialize the queue of free hci data packets */
    for (index = 0; index < HCI_READ_PACKET_NUM_MAX; index++)
    {
        list_insert_tail(&hciReadPktPool, (tListNode *)&hciReadPacketBuffer[index]);
    }

    Event_Request_Status = Waiting_For_Buffer;
}

#define HCI_PCK_TYPE_OFFSET                 0
#define  EVENT_PARAMETER_TOT_LEN_OFFSET     2

void Hal_Init_Event_Request(void)
{  
  Event_Request();
  
  return;
}

static volatile hci_packet_complete_callback packet_complete_callback = NULL;

static void hci_set_packet_complete_callback(hci_packet_complete_callback cb)
{
	packet_complete_callback = cb;
}

void HCI_Input(tHciDataPacket * hciReadPacket)
{
    uint8_t byte;
    hci_acl_hdr *acl_hdr;

	static hci_state state = WAITING_TYPE;

    uint16_t collected_payload_len = 0;
	uint16_t payload_len = 0;
    
    hci_buffer = hciReadPacket->dataBuff;
    
    if(state == WAITING_TYPE)
        hci_pckt_len = 0;
    
    while(hci_pckt_len < HCI_READ_PACKET_SIZE){

        byte = hci_buffer[hci_pckt_len++];

        if(state == WAITING_TYPE){
            /* Only ACL Data and Events packets are accepted. */
            if(byte == HCI_EVENT_PKT){
                 state = WAITING_EVENT_CODE;
            }
//            else if(byte == HCI_ACLDATA_PKT){
//                state = WAITING_HANDLE;
//            }
            else{
                /* Incorrect type. Reset state machine. */
                state = WAITING_TYPE;
                break;
            }
        }
        else if(state == WAITING_EVENT_CODE)
            state = WAITING_PARAM_LEN;
        else if(state == WAITING_HANDLE)
            state = WAITING_HANDLE_FLAG;
        else if(state == WAITING_HANDLE_FLAG)
            state = WAITING_DATA_LEN1;
        else if(state == WAITING_DATA_LEN1)
            state = WAITING_DATA_LEN2;

        else if(state == WAITING_DATA_LEN2){
            acl_hdr = (void *)&hci_buffer[HCI_HDR_SIZE];
            payload_len = acl_hdr->dlen;
            collected_payload_len = 0;
            state = WAITING_PAYLOAD;
        }
        else if(state == WAITING_PARAM_LEN){
             payload_len = byte;
             collected_payload_len = 0;
             state = WAITING_PAYLOAD;
        }
        else if(state == WAITING_PAYLOAD){
            collected_payload_len += 1;
            if(collected_payload_len >= payload_len){
                /* Reset state machine. */
                state = WAITING_TYPE;
                enqueue_packet(hciReadPacket);
                
                if(packet_complete_callback){
                  uint16_t len = hci_pckt_len;
                  packet_complete_callback(hci_buffer, len);
                }
                break;
            }
        }
    }        
}

void enqueue_packet(tHciDataPacket * hciReadPacket)
{
    hci_uart_pckt *hci_pckt = (void*)hciReadPacket->dataBuff;
    hci_event_pckt *event_pckt = (void*)hci_pckt->data;
    
    // Do not enqueue Command Complete or Command Status events
    
    if((hci_pckt->type != HCI_EVENT_PKT) ||
       event_pckt->evt == EVT_CMD_COMPLETE ||
           event_pckt->evt == EVT_CMD_STATUS){
        // Insert the packet back into the pool.
        list_insert_tail(&hciReadPktPool, (tListNode *)hciReadPacket);
    }
    else {    
        // Insert the packet into the queue of events to be processed.
        list_insert_tail(&hciReadPktRxQueue, (tListNode *)hciReadPacket);
        HCI_Process_Notification_Request();
    }
}

__weak void HCI_Event_CB(void *pckt)
{
  
}

void HCI_Process(void)
{
    //uint8_t data_len;
    //uint8_t buffer[HCI_PACKET_SIZE];
    tHciDataPacket * hciReadPacket = NULL;

    uint8_t list_empty = list_is_empty(&hciReadPktRxQueue);        
    /* process any pending events read */
    while(list_empty == FALSE)
    {
        list_remove_head (&hciReadPktRxQueue, (tListNode **)&hciReadPacket);
        HCI_Event_CB(hciReadPacket->dataBuff);
        list_insert_tail(&hciReadPktPool, (tListNode *)hciReadPacket);
        Event_Request();
        list_empty = list_is_empty(&hciReadPktRxQueue);
    }
}

BOOL HCI_Queue_Empty(void)
{
  return list_is_empty(&hciReadPktRxQueue);
}

void hci_notify_asynch_evt(uint8_t *buffer, uint8_t event_payload_len)
{
  Event_Request_Status = Waiting_For_Buffer;
  
  if(event_payload_len > 0){
    
    HCI_Input((tHciDataPacket*)buffer);
    // Packet will be inserted to te correct queue by 
  }
  else {
    // Insert the packet back into the pool.
    list_insert_head(&hciReadPktPool, (tListNode*)buffer);
  }

  Event_Request();
}

void hci_write(const void* data1, const void* data2, uint16_t n_bytes1, uint16_t n_bytes2){
#if  HCI_LOG_ON
  PRINTF("HCI <- ");
  for(int i=0; i < n_bytes1; i++)
    PRINTF("%02X ", *((uint8_t*)data1 + i));
  for(int i=0; i < n_bytes2; i++)
    PRINTF("%02X ", *((uint8_t*)data2 + i));
  PRINTF("\n");    
#endif
  
  Hal_Write_Serial(data1, data2, n_bytes1, n_bytes2);
}

void hci_send_cmd(uint16_t ogf, uint16_t ocf, uint8_t plen, void *param)
{
  hci_command_hdr hc;
  
  hc.opcode = htobs(cmd_opcode_pack(ogf, ocf));
  hc.plen= plen;
  
  //uint8_t header[HCI_HDR_SIZE + HCI_COMMAND_HDR_SIZE];
  header[0] = HCI_COMMAND_PKT;
  BLUENRG_memcpy(header+1, &hc, sizeof(hc));
  
  hci_write(header, param, sizeof(header), plen);
}

static uint8_t new_packet;

void new_hci_event(void *pckt, uint16_t len)
{
  
  new_packet = TRUE;
}

/**
 * FIXME: Param async is unused, it has been introduced to align the interface
 * to DK 1.6.0 HCI stack
 */
/* 'to' is timeout in system clock ticks.  */
int hci_send_req(struct hci_request *r, BOOL async)
{
  uint8_t *ptr;
  uint16_t opcode = htobs(cmd_opcode_pack(r->ogf, r->ocf));
  hci_event_pckt *event_pckt;
  hci_uart_pckt *hci_hdr;
  int try;
  uint32_t to = HCI_DEFAULT_TIMEOUT_MS/10;
  
  new_packet = FALSE;
  HCI_Cmd_Status(BUSY);
  hci_set_packet_complete_callback(new_hci_event);
  hci_send_cmd(r->ogf, r->ocf, r->clen, r->cparam);
  
  try = 10;
  while (try--) {
    evt_cmd_complete *cc;
    evt_cmd_status *cs;
    evt_le_meta_event *me;
    int len;
    
    /* Minimum timeout is 1. */
    if(to == 0)
      to = 1;
    
    hci_timeout = 0;
    Blue_NRG_HCI_Timer_Start(to, hci_timeout_callback, (uint8_t*)&hci_timer_id);
    
    while(1){
      if(hci_timeout){
        Blue_NRG_HCI_Timer_Stop(hci_timer_id);
        goto failed;
      }
      if(new_packet){
        Blue_NRG_HCI_Timer_Stop(hci_timer_id);
        break;
      }
      HCI_Wait_For_Response();
    }
    
    hci_hdr = (void *)hci_buffer;
    if(hci_hdr->type != HCI_EVENT_PKT){
      new_packet = FALSE;
      continue;
    }
    
    event_pckt = (void *) (hci_hdr->data);
    
    ptr = hci_buffer + (1 + HCI_EVENT_HDR_SIZE);
    len = hci_pckt_len - (1 + HCI_EVENT_HDR_SIZE);
    
    switch (event_pckt->evt) {
      
    case EVT_CMD_STATUS:
      cs = (void *) ptr;
      
      if (cs->opcode != opcode)
        break;
      
      if (r->event != EVT_CMD_STATUS) {
        if (cs->status) {
          goto failed;
        }
        break;
      }
      
      r->rlen = MIN(len, r->rlen);
      BLUENRG_memcpy(r->rparam, ptr, r->rlen);
      goto done;
      
    case EVT_CMD_COMPLETE:
      cc = (void *) ptr;
      
      if (cc->opcode != opcode)
        break;
      
      ptr += EVT_CMD_COMPLETE_SIZE;
      len -= EVT_CMD_COMPLETE_SIZE;
      
      r->rlen = MIN(len, r->rlen);
      BLUENRG_memcpy(r->rparam, ptr, r->rlen);
      goto done;
      
    case EVT_LE_META_EVENT:
      me = (void *) ptr;
      
      if (me->subevent != r->event)
        break;
      
      len -= 1;
      r->rlen = MIN(len, r->rlen);
      BLUENRG_memcpy(r->rparam, me->data, r->rlen);
      goto done;
      
    case EVT_HARDWARE_ERROR:            
      goto failed;
      
    default:
      break; // In the meantime there could be other events from the controller.
    }
    
    new_packet = FALSE;
    
  }
  
failed:
  hci_set_packet_complete_callback(NULL);
  HCI_Cmd_Status(AVAILABLE);
  return -1;
  
done:
  hci_set_packet_complete_callback(NULL);
  HCI_Cmd_Status(AVAILABLE);
  return 0;
}
