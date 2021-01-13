/**
* File Name          : gatt_db.h
* Author             :
* Version            : V1.0.0
* Date               : 16-September-2015
* Description        : Header file for gatt_db.c
*/

#ifndef GATT_DB_H
#define GATT_DB_H

#include <stdint.h>
#include <stdlib.h>
#include "bluenrg_def.h"

/** Documentation for C union Service_UUID_t */
typedef union Service_UUID_t_s {
  /** 16-bit UUID
  */
  uint16_t Service_UUID_16;
  /** 128-bit UUID
  */
  uint8_t Service_UUID_128[16];
} Service_UUID_t;

/** Documentation for C union Char_UUID_t */
typedef union Char_UUID_t_s {
  /** 16-bit UUID
  */
  uint16_t Char_UUID_16;
  /** 128-bit UUID
  */
  uint8_t Char_UUID_128[16];
} Char_UUID_t;

tBleStatus Add_MyBle_Service(void);
void Read_Request_CB(uint16_t handle);
void Write_Request_CB(uint16_t handle, uint8_t* data);
tBleStatus Ble_Threshold_Update();
tBleStatus Ble_CurrentValue_Update();


extern uint8_t Services_Max_Attribute_Records[];

#endif /* GATT_DB_H */
