[#ftl]
/**
  ******************************************************************************
  * @file  : hci_tl_interface.c
  * @brief : This file provides the implementation for all functions prototypes 
  *          for the STM32 BlueNRG HCI Transport Layer interface
  ******************************************************************************
  *
  * COPYRIGHT ${year} STMicroelectronics
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  ******************************************************************************
  */#n
[#assign IpInstance = ""]
[#assign UsartInstance = ""]
[#assign SpiInstance = ""]
[#assign IpName = ""]
[#assign ExtiLine = ""]
[#assign IrqNumber = ""]
[#assign BNRG_EXTI_LINE = ""]
[#assign BNRG_EXTI_PORT = ""]
[#assign BNRG_EXTI_PIN = ""]
[#assign BNRG_EXTI_IRQn = ""]

[#list BspIpDatas as SWIP] 
	[#if SWIP.variables??]	
		[#list SWIP.variables as variables]
			[#if variables.name?contains("IpName")]
				[#assign IpName = variables.value]
			[/#if]
			[#if variables.name?contains("IpInstance")]
				[#assign IpInstance = variables.value]
			[/#if]
			[#if variables.name?contains("GPIO_INT_NUM")]
				[#assign IrqNumber = variables.value]
			[/#if]			
			[#if variables.value?contains("BUS IO driver")]
				[#assign SpiInstance = IpInstance]
			[/#if]
			[#if variables.name?contains("EXTI_LINE_NUMBER")]
				[#assign ExtiLine = variables.value]
			[/#if]
			[#if variables.value?contains("Exti Line")]				
				[#assign BNRG_EXTI_LINE = ExtiLine]
				[#assign BNRG_EXTI_PORT = IpName]
				[#assign BNRG_EXTI_PIN = IpInstance]
				[#assign BNRG_EXTI_IRQn = IrqNumber]				
			[/#if]			
		[/#list]
	[/#if]
[/#list] 
  
[#assign useSENSORDEMO = false]
[#assign useSAMPLEAPP = false]
[#assign useBEACON = false]
[#assign useVIRTUALCOMPORT = false]
[#assign useHCI_TL = false]

[#if RTEdatas??]
[#list RTEdatas as define]

[#if define?contains("SENSOR_DEMO")]
[#assign useSENSORDEMO = true]
[/#if]
[#if define?contains("SAMPLE_APP")]
[#assign useSAMPLEAPP = true]
[/#if]
[#if define?contains("BEACON")]
[#assign useBEACON = true]
[/#if]
[#if define?contains("VIRTUAL_COM_PORT")]
[#assign useVIRTUALCOMPORT = true]
[/#if]
[#if define?ends_with("HCI_TL")]
[#assign useHCI_TL = true]
[/#if]

[/#list]
[/#if]

/* Includes ------------------------------------------------------------------*/
#include "RTE_Components.h"

[#if useHCI_TL]
#include "hci_tl.h"
[#else]
#include "hci_tl_interface.h"
#include "${FamilyName?lower_case}xx_hal_exti.h"
[/#if]

/* Defines -------------------------------------------------------------------*/

#define HEADER_SIZE       5U
#define MAX_BUFFER_SIZE   255U
#define TIMEOUT_DURATION  15U

/* Private variables ---------------------------------------------------------*/
EXTI_HandleTypeDef hexti${BNRG_EXTI_LINE};

/******************** IO Operation and BUS services ***************************/

/**
 * @brief  Initializes the peripherals communication with the BlueNRG
 *         Expansion Board (via SPI, I2C, USART, ...)
 *
 * @param  void* Pointer to configuration struct 
 * @retval int32_t Status
 */
int32_t HCI_TL_SPI_Init(void* pConf)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  /* Configure EXTI Line */
  GPIO_InitStruct.Pin = HCI_TL_SPI_EXTI_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(HCI_TL_SPI_EXTI_PORT, &GPIO_InitStruct);
   
  /* Configure RESET Line */
  GPIO_InitStruct.Pin =  HCI_TL_RST_PIN ;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(HCI_TL_RST_PORT, &GPIO_InitStruct);
  
  /* Configure CS */
  GPIO_InitStruct.Pin = HCI_TL_SPI_CS_PIN ;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(HCI_TL_SPI_CS_PORT, &GPIO_InitStruct); 
    
  return BSP_${SpiInstance}_Init();
}

/**
 * @brief  DeInitializes the peripherals communication with the BlueNRG
 *         Expansion Board (via SPI, I2C, USART, ...)
 *
 * @param  None
 * @retval int32_t 0
 */
int32_t HCI_TL_SPI_DeInit(void)
{
  HAL_GPIO_DeInit(HCI_TL_SPI_EXTI_PORT, HCI_TL_SPI_EXTI_PIN); 
  HAL_GPIO_DeInit(HCI_TL_SPI_CS_PORT, HCI_TL_SPI_CS_PIN); 
  HAL_GPIO_DeInit(HCI_TL_RST_PORT, HCI_TL_RST_PIN);   
  return 0;
}

/**
 * @brief Reset BlueNRG module.
 *
 * @param  None
 * @retval int32_t 0
 */
int32_t HCI_TL_SPI_Reset(void)
{
  // Deselect CS PIN for BlueNRG to avoid spurious commands
  HAL_GPIO_WritePin(HCI_TL_SPI_CS_PORT, HCI_TL_SPI_CS_PIN, GPIO_PIN_SET);

  HAL_GPIO_WritePin(HCI_TL_RST_PORT, HCI_TL_RST_PIN, GPIO_PIN_RESET);
  HAL_Delay(5);
  HAL_GPIO_WritePin(HCI_TL_RST_PORT, HCI_TL_RST_PIN, GPIO_PIN_SET);
  HAL_Delay(5);    
  return 0;
}  

/**
 * @brief  Reads from BlueNRG SPI buffer and store data into local buffer.
 *
 * @param  buffer : Buffer where data from SPI are stored
 * @param  size   : Buffer size
 * @retval int32_t: Number of read bytes
 */
int32_t HCI_TL_SPI_Receive(uint8_t* buffer, uint16_t size)
{
  uint16_t byte_count;
  uint8_t len = 0;
  uint8_t char_ff = 0xff;
  volatile uint8_t read_char;

  uint8_t header_master[HEADER_SIZE] = {0x0b, 0x00, 0x00, 0x00, 0x00};
  uint8_t header_slave[HEADER_SIZE];

  /* CS reset */
  HAL_GPIO_WritePin(HCI_TL_SPI_CS_PORT, HCI_TL_SPI_CS_PIN, GPIO_PIN_RESET);

  /* Read the header */
  BSP_${SpiInstance}_SendRecv(header_master, header_slave, HEADER_SIZE);

  if(header_slave[0] == 0x02) 
  {
    /* device is ready */
    byte_count = (header_slave[4] << 8)| header_slave[3];
  
    if(byte_count > 0) 
    {
      /* avoid to read more data than the size of the buffer */
      if (byte_count > size){
        byte_count = size;
      }        
  
      for(len = 0; len < byte_count; len++)
      {                                               
        BSP_${SpiInstance}_SendRecv(&char_ff, (uint8_t*)&read_char, 1);
        buffer[len] = read_char;
      }
    }
  }
  /* Release CS line */
  HAL_GPIO_WritePin(HCI_TL_SPI_CS_PORT, HCI_TL_SPI_CS_PIN, GPIO_PIN_SET);
  
#if PRINT_CSV_FORMAT
  if (len > 0) {
    print_csv_time();
    for (int i=0; i<len; i++) {
      PRINT_CSV(" %02x", buffer[i]);
    }
    PRINT_CSV("\n");
  }
#endif
  
  return len;  
}

/**
 * @brief  Writes data from local buffer to SPI.
 *
 * @param  buffer : data buffer to be written
 * @param  size   : size of first data buffer to be written
 * @retval int32_t: Number of read bytes
 */
int32_t HCI_TL_SPI_Send(uint8_t* buffer, uint16_t size)
{  
  int32_t result;
  
  uint8_t header_master[HEADER_SIZE] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  uint8_t header_slave[HEADER_SIZE];
  
  static uint8_t read_char_buf[MAX_BUFFER_SIZE];
  uint32_t tickstart = HAL_GetTick();
  
  do
  {
    result = 0;
    
    /* CS reset */
    HAL_GPIO_WritePin(HCI_TL_SPI_CS_PORT, HCI_TL_SPI_CS_PIN, GPIO_PIN_RESET);
    
    /* Read header */  
    BSP_${SpiInstance}_SendRecv(header_master, header_slave, HEADER_SIZE);
    
    if(header_slave[0] == 0x02) 
    {
      /* SPI is ready */
      if(header_slave[1] >= size) 
      {
        BSP_${SpiInstance}_SendRecv(buffer, read_char_buf, size);
      } 
      else 
      {
        /* Buffer is too small */
        result = -2;
      }
    } else {
      /* SPI is not ready */
      result = -1;
    }
    
    /* Release CS line */
    HAL_GPIO_WritePin(HCI_TL_SPI_CS_PORT, HCI_TL_SPI_CS_PIN, GPIO_PIN_SET);
    
    if((HAL_GetTick() - tickstart) > TIMEOUT_DURATION)
    {
      result = -3;
      break;
    }
  } while(result < 0);

  return result;
}

[#if useHCI_TL]
/**
 * @brief  Reports if the BlueNRG has data for the host micro.
 *
 * @param  None
 * @retval int32_t: 1 if data are present, 0 otherwise
 */
static int32_t IsDataAvailable(void)
{
  return (HAL_GPIO_ReadPin(HCI_TL_SPI_EXTI_PORT, HCI_TL_SPI_EXTI_PIN) == GPIO_PIN_SET);
} 
[/#if]

/***************************** hci_tl_interface main functions *****************************/
/**
 * @brief  Register hci_tl_interface IO bus services
 *
 * @param  None
 * @retval None
 */ 
void hci_tl_lowlevel_init(void)
{
  /* USER CODE BEGIN hci_tl_lowlevel_init 1 */
  
  /* USER CODE END hci_tl_lowlevel_init 1 */
[#if useHCI_TL]
  tHciIO fops;  
  
  /* Register IO bus services */
  fops.Init    = HCI_TL_SPI_Init;
  fops.DeInit  = HCI_TL_SPI_DeInit;
  fops.Send    = HCI_TL_SPI_Send;
  fops.Receive = HCI_TL_SPI_Receive;
  fops.Reset   = HCI_TL_SPI_Reset;
  fops.GetTick = BSP_GetTick;
  
  hci_register_io_bus (&fops);
  
  /* USER CODE BEGIN hci_tl_lowlevel_init 2 */
  
  /* USER CODE END hci_tl_lowlevel_init 2 */
  
  /* Register event irq handler */
  HAL_EXTI_GetHandle(&hexti${BNRG_EXTI_LINE}, EXTI_LINE_${BNRG_EXTI_LINE});
[#if useVIRTUALCOMPORT]
  HAL_EXTI_RegisterCallback(&hexti${BNRG_EXTI_LINE}, HAL_EXTI_COMMON_CB_ID, NULL);
[#else]
  HAL_EXTI_RegisterCallback(&hexti${BNRG_EXTI_LINE}, HAL_EXTI_COMMON_CB_ID, hci_tl_lowlevel_isr);
[/#if]
  HAL_NVIC_SetPriority(${BNRG_EXTI_IRQn}, 0, 0);
  HAL_NVIC_EnableIRQ(${BNRG_EXTI_IRQn});
[/#if]

  /* USER CODE BEGIN hci_tl_lowlevel_init 3 */
  
  /* USER CODE END hci_tl_lowlevel_init 3 */

}

/**
  * @brief HCI Transport Layer Low Level Interrupt Service Routine
  *
  * @param  None
  * @retval None
  */
void hci_tl_lowlevel_isr(void)
{
  /* Call hci_notify_asynch_evt() */
[#if useHCI_TL]
  while(IsDataAvailable())
  {        
    if (hci_notify_asynch_evt(NULL))
    {
      return;
    }
  }
[/#if]

  /* USER CODE BEGIN hci_tl_lowlevel_isr */

  /* USER CODE END hci_tl_lowlevel_isr */ 
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
