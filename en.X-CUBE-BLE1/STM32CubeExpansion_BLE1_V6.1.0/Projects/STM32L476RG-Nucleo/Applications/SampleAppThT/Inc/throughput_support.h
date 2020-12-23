/******************** (C) COPYRIGHT 2018 STMicroelectronics ********************
* File Name          : throughput_support.h
* Author             : Central Lab
* Version            : V1.0.0
* Date               : 01-June-2018
* Description        : 
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
#ifndef __THROUGHPUT_SUPPORT_H__
#define __THROUGHPUT_SUPPORT_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "stm32l4xx_hal.h"

/******************************************************************************
 * Macros
 *****************************************************************************/
/* Store Value into a buffer in Little Endian Format */
#define HOST_TO_LE_32(buf, val)    (((buf)[0] = (uint8_t) (val)     ) , \
                                    ((buf)[1] = (uint8_t) (val>>8)  ) , \
                                    ((buf)[2] = (uint8_t) (val>>16) ) , \
                                    ((buf)[3] = (uint8_t) (val>>24) )) 

#define LE_TO_HOST_32(ptr)   (uint32_t) ( ((uint32_t) \
                                          *((uint8_t *)ptr)) | \
                                          ((uint32_t) \
                                          *((uint8_t *)ptr + 1) << 8)  | \
                                          ((uint32_t) \
                                          *((uint8_t *)ptr + 2) << 16) | \
                                          ((uint32_t) \
                                          *((uint8_t *)ptr + 3) << 24) )

/******************************************************************************
 * Types
 *****************************************************************************/

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/


#endif /* __THROUGHPUT_SUPPORT_H__ */
