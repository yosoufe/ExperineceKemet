#ifndef __DEMO_APPLICATION_H__
#define __DEMO_APPLICATION_H__

/* Includes *********************************************************************/
#include "stdint.h"

/* Macros *********************************************************************/
/* Little Endian buffer to host endianess conversion */
#define LE_TO_HOST_16(ptr)  (uint16_t) ( ((uint16_t) \
                                           *((uint8_t *)ptr)) | \
                                          ((uint16_t) \
                                           *((uint8_t *)ptr + 1) << 8 ) )

uint8_t application_init(void);
uint8_t application_ADV_Start(void);
void APP_Tick(void);

#endif // __DEMO_APPLICATION_H__
