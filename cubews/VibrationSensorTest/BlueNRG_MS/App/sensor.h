/******************** (C) COPYRIGHT 2015 STMicroelectronics ********************
 * File Name          : sensor.h
 * Author             : AMS - VMA RF Application team
 * Version            : V1.0.0
 * Date               : 23-November-2015
 * Description        : Header file for sensor.c
 ********************************************************************************
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *******************************************************************************/

#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

#define IDB04A1 0
#define IDB05A1 1
#define SENSOR_DEMO_NAME   'B','l','u','e','N','R','G'
#define BDADDR_SIZE        6

void Set_DeviceConnectable(void);
void user_notify(void * pData);

extern uint8_t Application_Max_Attribute_Records[];

#endif /* SENSOR_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
