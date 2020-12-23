/**
  ******************************************************************************
  * @file    bluenrg_interface.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    04-July-2014
  * @brief   This file provides code for the BlueNRG STM32 expansion board driver
  *          based on STM32Cube HAL for STM32 Nucleo boards.
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
#include "hci_tl_interface.h"

#include "bluenrg_conf.h"
#include "hci_const.h"
#include "cube_hal.h"

extern SPI_Context_t SPI_Context;

uint8_t *HCI_read_packet;

/**
 * @brief  Writes data to a serial interface.
 * @param  data1   :  1st buffer
 * @param  data2   :  2nd buffer
 * @param  n_bytes1: number of bytes in 1st buffer
 * @param  n_bytes2: number of bytes in 2nd buffer
 * @retval None
 */
void HCI_TL_SPI_Send(const void* data1, const void* data2, int32_t n_bytes1,
                      int32_t n_bytes2)
{
  /* New implementation (with DMA write) */
  BlueNRG_SPI_Write((uint8_t *)data1,(uint8_t *)data2, n_bytes1, n_bytes2);
}

/**
 * @brief  Init request for an event
 * @param  None
 * @retval None
 */
void Hal_Init_Event_Request(void)
{  
  Event_Request();
  
  return;
}

/**
 * @brief  Request for an event
 * @param  uint8_t*: the event packet
 * @param  uint8_t : the event packet size
 * @retval None
 */
void Hal_Event_Request(uint8_t *buffer, uint8_t buff_size)
{
  HCI_read_packet = buffer;
  
  BlueNRG_SPI_Request_Events(((tHciDataPacket*)buffer)->dataBuff, buff_size);
}

/**
 * @brief  EXTI line detection callback.
 * @param  uint16_t GPIO_Pin Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == BNRG_SPI_EXTI_PIN) {    
    BlueNRG_SPI_IRQ_Callback(); 
  }
  else {
    if(GPIO_Pin == KEY_BUTTON_PIN) {     
      LPM_Mode_Request(eLPM_MAIN_LOOP_PROCESSES, eLPM_Mode_RUN);
    }
  }  
}

/**
  * @brief HCI Transport Layer Low Level Interrupt Service Routine
  *
  * @param  None
  * @retval None
  */
void hci_tl_lowlevel_isr(void)
{      
  hci_notify_asynch_evt(HCI_read_packet, SPI_Context.SPI_Receive_Context.payload_len);

  /* USER CODE BEGIN hci_tl_lowlevel_isr */

  /* USER CODE END hci_tl_lowlevel_isr */ 
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
