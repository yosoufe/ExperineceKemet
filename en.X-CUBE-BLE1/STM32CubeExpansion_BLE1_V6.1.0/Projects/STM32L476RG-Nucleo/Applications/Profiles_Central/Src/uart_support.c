/**
******************************************************************************
* @file    uart_support.c 
* @author  AAS / CL
* @version V1.0.0
* @date    18-May-2015
* @brief   This file implements the generic function for UART communication.
******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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
#include <host_config.h>
#if (PLATFORM_WINDOWS == 1)
#include <Windows.h>
#endif

#include <stdarg.h>
#include <string.h>
#include <hci.h>
#include <debug.h>
#include <timer.h>

#include "hci_tl.h"
#include "hci_le.h"
#include "uart_support.h"

#include "stm32_bluenrg_ble.h"

/** @addtogroup Applications
 *  @{
 */

/** @addtogroup Profiles_Central
 *  @{
 */
 
/** @defgroup UART_SUPPORT
 *  @{
 */
 
/*******************************************************************************
 * Macros
*******************************************************************************/

/*******************************************************************************
 * Defines
*******************************************************************************/
/**
 * HCI_Event_CB is the HCI event callback name used by the 
 * Peripheral Profiles library 
 */
#define user_notify HCI_Event_CB 

/******************************************************************************
 * Local Variable Declarations
******************************************************************************/

/** @defgroup UART_SUPPORT_Exported_Variables
 *  @{
 */
/******************************************************************************
 * Global Variable Declarations
******************************************************************************/
extern tProfileApplContext profileApplContext; /* Profile Application Context */

extern UART_HandleTypeDef UartHandle;
uint8_t uart_header[UARTHEADERSIZE];
uint8_t aRxBuffer[RXBUFFERSIZE];

void *profiledbgfile;

/**
 * @}
 */
 
/******************************************************************************
 * Function Declarations
******************************************************************************/
extern void _Error_Handler(char *, int);
/**
 * The HCI event callback declared in the Peripheral Profiles library 
 */
extern void user_notify(void * pData); 

/** @defgroup UART_SUPPORT_Functions
 *  @{
 */
/******************************************************************************
 * Function Definitions 
******************************************************************************/
/**
 * @brief
 * @param
 */
void PRINT_MESG_UART(void *f, const char * format, ... )
{
  va_list ap;
  uint8_t buffer [128];
  int n;
  
  va_start(ap, format);
  n = vsnprintf ((char*)buffer, 128, format, ap);
  va_end(ap);
  
  notify_uart(buffer, n, PROFILE_CENTRAL_NOTIFY);
}

void initDeviceMaster(void)
{      
  if (!profileApplContext.deviceMasterInited) {
    
    /* Initialize the BlueNRG HCI */
    HCI_Init();
    
    /* Reset BlueNRG hardware */
    BlueNRG_RST();

    Blue_NRG_Timer_Init(); 
    
    /* Init Collector Profile Central Role */
    profileApplContext.initDeviceFunc();
    
    /* Init Profile Security */
    profileApplContext.deviceSetSecurityFunc();
    
    // FIXME: what if Discovery error
    profileApplContext.deviceDiscoveryProcedureFunc();
    
    profileApplContext.deviceState = APPL_INIT_DONE;

    profileApplContext.deviceMasterInited = TRUE;
    
  }
}

void handle_uart_cmd(uint8_t* cmd, uint8_t cmd_len, const char* payloadString)
{
  uint8_t ret;
  uint8_t params_len;
  /*
  int unsupported = 0;
  uint8_t len = 0;
  uint8_t response[PROFILE_CENTRAL_HEADER_SIZE];
  uint8_t* payloadStr;
  */
  
  /*
  response[PKT_TYPE_OFFSET] = VENDOR_PKT;
  response[RESP_VENDOR_CODE_OFFSET] = RESPONSE;

  response[RESP_STRING_LEN_OFFSET] = 0;
  response[RESP_STATUS_OFFSET] = 0;  
  */
  if (cmd[PKT_TYPE_OFFSET] == VENDOR_PKT) {
    switch (cmd[PKT_CMDCODE_OFFSET]) {
    case APP_TYPE:
      /*
      payloadStr = PROFILE_CENTRAL;
      response[RESP_STRING_LEN_OFFSET] = strlen(PROFILE_CENTRAL);
      response[RESP_CMDCODE_OFFSET] = PROFILE_CENTRAL_OPEN;
      */
      profileApplContext.deviceMasterInited = FALSE;
      notify_uart(PROFILE_CENTRAL, strlen(PROFILE_CENTRAL), PROFILE_CENTRAL_OPEN);
      initDeviceMaster();
      
      break;
      
    case SCAN_REQ:
      
      profileApplContext.deviceState = APPL_INIT_DONE;
      profileApplContext.deviceMasterInited = TRUE;
      
      ret = profileApplContext.deviceDiscoveryProcedureFunc();
      // In case of Discovery procedure error, re-init the device */
      if(ret != BLE_STATUS_SUCCESS) {
        profileApplContext.deviceMasterInited = FALSE;
        initDeviceMaster();
      }

      break;
      
    case DEVICE_CONNECTION:
      
      //response[RESP_CMDCODE_OFFSET] = cmd[PKT_CMDCODE_OFFSET];
      
      params_len = cmd[UARTHEADERSIZE-1];
      
      if (params_len > 0) {
        if(HAL_UART_Receive(&UartHandle, (uint8_t *)aRxBuffer, params_len, 300) != HAL_OK) {       
          _Error_Handler(__FILE__, __LINE__);
        }
        
        memcpy(profileApplContext.peerAddr, aRxBuffer+1, 6);
        
        profileApplContext.deviceMasterInited = TRUE;
        profileApplContext.startDeviceConn = TRUE;
      }
      
      break;
      
    case CLOSE_CONNECTION:
      //HR_RESERVED_MSG(profiledbgfile,"****   Closing connection...\r\n");
      
      //response[RESP_CMDCODE_OFFSET] = cmd[PKT_CMDCODE_OFFSET];
      
      if(profileApplContext.deviceState >= APPL_CONNECTED){
        profileApplContext.deviceDisconnectionFunc();
        
      } else {
        profileApplContext.deviceMasterInited = FALSE;
        tBleStatus ble_status = BLE_STATUS_SUCCESS;
        notify_uart(&ble_status, sizeof(ble_status), CLOSE_CONNECTION);
        APPL_MESG_DBG(profiledbgfile,"Disconnection complete.\n" );
      }
      
      profileApplContext.deviceState = APPL_INIT_DONE;
      
      break;
      
    case CP_COMMAND:
    
      params_len = cmd[UARTHEADERSIZE-1];
      
      if (params_len > 0) {
        if(HAL_UART_Receive(&UartHandle, (uint8_t *)aRxBuffer, params_len, 300) != HAL_OK) {       
          _Error_Handler(__FILE__, __LINE__);
        }
        
        profileApplContext.cpFunc(aRxBuffer, params_len);
      }
      
      break;
      
    default:
      APPL_MESG_DBG(profiledbgfile,"Command unsupported.\n" );
    }
    
#if 0
      len += 2; // Status and Command code
      PACK_2_BYTE_PARAMETER(response + RESP_LEN_OFFSET_LSB, len);
      len += RESP_CMDCODE_OFFSET;
      
      if(HAL_UART_Transmit(&UartHandle, (uint8_t*)response, len, 300) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
      }
      
      if(response[RESP_STRING_LEN_OFFSET] != 0) {
        if(HAL_UART_Transmit(&UartHandle, payloadStr, response[RESP_STRING_LEN_OFFSET], 300) != HAL_OK) {
          _Error_Handler(__FILE__, __LINE__);
        }
      }
#endif /* 0 */
    
  }

}

void notify_uart(uint8_t* payloadString, int n, uint8_t cmdCode)
{
  uint8_t len = 0;
  uint8_t response[PROFILE_CENTRAL_HEADER_SIZE];
  
  if(payloadString != NULL) {
    response[PKT_TYPE_OFFSET] = VENDOR_PKT;
    response[RESP_VENDOR_CODE_OFFSET] = RESPONSE;
    response[RESP_CMDCODE_OFFSET] = cmdCode;

    response[RESP_STRING_LEN_OFFSET] = n;
    response[RESP_STATUS_OFFSET] = 0;  

    len += 2; // Status and Command code
    PACK_2_BYTE_PARAMETER(response + RESP_LEN_OFFSET_LSB, len);
    len += RESP_CMDCODE_OFFSET;

    /*
    if(cmdCode == SERVICE_UUID) {
      for (uint8_t i=0; i<n; i++)
        HR_RESERVED_MSG(profiledbgfile,"%02x", payloadString[i]);
      
      HR_RESERVED_MSG(profiledbgfile,"\n");
    }
    */
    if(HAL_UART_Transmit(&UartHandle, (uint8_t*)response, len, 300) != HAL_OK) {
      _Error_Handler(__FILE__, __LINE__);
    }

    if(HAL_UART_Transmit(&UartHandle, (uint8_t*)payloadString, n, 300) != HAL_OK) {
      _Error_Handler(__FILE__, __LINE__);
    }
  }
}

/**
 * @brief  Set current BlueNRG profile
 * @param  None
 * @retval None
 */
void BNRG_Set_Current_profile(void)
{
#if (BLE_CURRENT_PROFILE_ROLES & HEART_RATE_COLLECTOR)
  HR_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & HEALTH_THERMOMETER_COLLECTOR)
  HT_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & GLUCOSE_COLLECTOR)
  GC_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & BLOOD_PRESSURE_COLLECTOR)
  BP_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & TIME_CLIENT)
  TC_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & FIND_ME_LOCATOR)
  FML_Appl_Context_Init();
#endif

#if (BLE_CURRENT_PROFILE_ROLES & ALERT_NOTIFICATION_CLIENT)
  ANC_Appl_Context_Init();
#endif
}

static void wait(void)
{
  volatile uint32_t i;
  for (i=0; i<200000; i++) __NOP();
}

static void blink_led(void)
{
  BSP_LED_Toggle(LED2);
  wait();
  BSP_LED_Toggle(LED2); 
  wait();
}

/**
  * @brief  UART error callbacks
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @param  UART handle pointer  
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
  blink_led();
  PRINTF("HAL_UART_ErrorCallback\n");
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
