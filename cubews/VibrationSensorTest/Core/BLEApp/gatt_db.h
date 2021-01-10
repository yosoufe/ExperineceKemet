/******************** (C) COPYRIGHT 2015 STMicroelectronics ********************
* File Name          : gatt_db.h
* Author             :
* Version            : V1.0.0
* Date               : 16-September-2015
* Description        : Header file for gatt_db.c
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifndef GATT_DB_H
#define GATT_DB_H

#include <stdint.h>
#include <stdlib.h>
#include "bluenrg_def.h"

#define X_OFFSET 200
#define Y_OFFSET 50
#define Z_OFFSET 1000


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

tBleStatus Add_Switch_Service(void);
void Read_Request_CB(uint16_t handle);
void Write_Request_CB(uint16_t handle, uint8_t* data);
tBleStatus BlueMS_Switch_Update(uint16_t switch_status);

extern uint8_t Services_Max_Attribute_Records[];

#endif /* GATT_DB_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
