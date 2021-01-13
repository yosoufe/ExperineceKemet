/**
* File Name          : gatt_db.c
* Author             :
* Version            : V1.0.0
* Date               : 16-September-2015
* Description        : Functions to build GATT DB and handle GATT events.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bluenrg_def.h"
#include "gatt_db.h"
#include "bluenrg_conf.h"
#include "bluenrg_gatt_aci.h"

#include "vibrationCalculations.h" // to access the threshold, and the measurement value

/** @brief Macro that stores Value into a buffer in Little Endian Format (2 bytes)*/
#define HOST_TO_LE_16(buf, val)    ( ((buf)[0] =  (uint8_t) (val)    ) , \
                                   ((buf)[1] =  (uint8_t) (val>>8) ) )

/** @brief Macro that stores Value into a buffer in Little Endian Format (4 bytes) */
#define HOST_TO_LE_32(buf, val)    ( ((buf)[0] =  (uint8_t) (val)     ) , \
                                   ((buf)[1] =  (uint8_t) (val>>8)  ) , \
                                   ((buf)[2] =  (uint8_t) (val>>16) ) , \
                                   ((buf)[3] =  (uint8_t) (val>>24) ) )

/** @brief Macro that stores Value into a buffer in Little Endian Format (8 bytes) */
#define HOST_TO_LE_64(buf, val)    HOST_TO_LE_32(buf, val); HOST_TO_LE_32(buf+4, val >> 32)

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

/* Switch Characteristics Service */
#define COPY_MYBLE_SERVICE_UUID(uuid_struct)    	COPY_UUID_128(uuid_struct,0x00,0x00,0x00,0x00,0x00,0x03,0x11,0xe1,0x9a,0xb4,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
// To receive the threshold value
#define COPY_THRESHOLD_CHAR_UUID(uuid_struct)		COPY_UUID_128(uuid_struct,0x20,0x00,0x00,0x00,0x00,0x01,0x11,0xe1,0xac,0x36,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
// To send a notification if value passed the threshold
#define COPY_NOTIFICATION_CHAR_UUID(uuid_struct)        COPY_UUID_128(uuid_struct,0x21,0x00,0x00,0x00,0x00,0x01,0x11,0xe1,0xac,0x36,0x00,0x02,0xa5,0xd5,0xc5,0x1b)



uint16_t MyBleServHandle, ThresholdCharHandle, NotificationCharHandle;


extern uint16_t SWITCH_STATUS;

/* UUIDS */
Service_UUID_t service_uuid;
Char_UUID_t char_uuid;

extern uint16_t connection_handle;
extern uint32_t start_time;

tBleStatus Add_MyBle_Service(void)
{
  tBleStatus ret;
  uint8_t uuid[16];

  COPY_MYBLE_SERVICE_UUID(uuid);
  BLUENRG_memcpy(&service_uuid.Service_UUID_128, uuid, 16);
  // about max_attr_records: https://community.st.com/s/question/0D50X00009Xki9c/bluenrg-acigattaddserv-description-of-parameter-maxattrrecords
  ret = aci_gatt_add_serv(UUID_TYPE_128,
			  service_uuid.Service_UUID_128,
			  PRIMARY_SERVICE,
                          1+3+2, /* 1 service, 3 for a characteristic with Notification, 2 for a characteristic without notification */
			  &MyBleServHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }

  COPY_THRESHOLD_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(MyBleServHandle,
			   UUID_TYPE_128,
			   char_uuid.Char_UUID_128,
                           8,
			   CHAR_PROP_WRITE_WITHOUT_RESP | CHAR_PROP_READ,
                           ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_ATTRIBUTE_WRITE | GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &ThresholdCharHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }

  COPY_NOTIFICATION_CHAR_UUID(uuid);
  BLUENRG_memcpy(&char_uuid.Char_UUID_128, uuid, 16);
  ret =  aci_gatt_add_char(MyBleServHandle,
			   UUID_TYPE_128,
			   char_uuid.Char_UUID_128,
			   8,
			   CHAR_PROP_READ | CHAR_PROP_NOTIFY,
			   ATTR_PERMISSION_NONE,
			   GATT_NOTIFY_ATTRIBUTE_WRITE,
			   16, 0, &NotificationCharHandle);

  if (ret != BLE_STATUS_SUCCESS) {
    goto fail;
  }

  return BLE_STATUS_SUCCESS;

fail:
  return BLE_STATUS_ERROR;
}

void Read_Request_CB(uint16_t handle)
{
  tBleStatus ret;
  if (handle == ThresholdCharHandle + 1)
  {
    // update the threshold
    Ble_Threshold_Update();
  } else if(handle == NotificationCharHandle + 1)
  {
    // update the current value
    Ble_CurrentValue_Update();
  }

  if(connection_handle !=0)
  {
    ret = aci_gatt_allow_read(connection_handle);
    if (ret != BLE_STATUS_SUCCESS)
    {
      PRINTF("aci_gatt_allow_read() failed: 0x%02x\r\n", ret);
    }
  }
}

void Write_Request_CB(uint16_t handle, uint8_t* data)
{
  if(handle == ThresholdCharHandle +1 )
  {
    uint64_t thr = ((uint64_t)(data[0]) |
	((uint64_t)(data[1]) << 8*1) |
	((uint64_t)(data[2]) << 8*2) |
	((uint64_t)(data[3]) << 8*3) |
	((uint64_t)(data[4]) << 8*4) |
	((uint64_t)(data[5]) << 8*5) |
	((uint64_t)(data[6]) << 8*6) |
	((uint64_t)(data[7]) << 8*7));
    mean_square_threshold = (float)(thr);
  }
}

tBleStatus Ble_Threshold_Update()
{
  tBleStatus ret;
  static unsigned char buff[8];
  HOST_TO_LE_64(buff,(uint64_t)(mean_square_threshold));
  ret = aci_gatt_update_char_value(MyBleServHandle, ThresholdCharHandle,
                                   0, 8, buff);
  if (ret != BLE_STATUS_SUCCESS){
    PRINTF("Error Ble_Threshold_Update: 0x%04X\n",ret) ;
    return BLE_STATUS_ERROR ;
  }
  return BLE_STATUS_SUCCESS;
}

tBleStatus Ble_CurrentValue_Update()
{
  tBleStatus ret;
  static uint8_t buff[8];
  HOST_TO_LE_64(buff,(uint64_t)(vibrationMeanSquare.mean_square));
  ret = aci_gatt_update_char_value(MyBleServHandle,
				   NotificationCharHandle,
                                   0, 8, buff);

  if (ret != BLE_STATUS_SUCCESS){
    PRINTF("Error Ble_CurrentValue_Update: 0x%04X\n",ret) ;
    return BLE_STATUS_ERROR ;
  }

  return BLE_STATUS_SUCCESS;
}
