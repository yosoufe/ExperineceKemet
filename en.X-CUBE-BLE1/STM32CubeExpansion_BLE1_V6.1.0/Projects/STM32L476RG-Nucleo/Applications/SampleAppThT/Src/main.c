/**
******************************************************************************
* @file    main.c 
* @author  CL
* @version V1.0.0
* @date    04-July-2014
* @brief   This sample code shows how to use the BlueNRG STM32 expansion board
*          to exchange data between two devices.
*          It provides also the feature for the throughput estimation.
*          The communication is done using two STM32 Nucleo boards.
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
#include "main.h"
#include "cube_hal.h"

#include "uart_conf.h"

/* USER CODE BEGIN Includes */
#include "sample_service.h"
#include "role_type.h"
#include "bluenrg_conf.h"
#include "stm32_bluenrg_ble.h"
#include "bluenrg_utils.h"
/* USER CODE END Includes */

#include <stdio.h>

/** @addtogroup Applications
 *  @{
 */

/** @defgroup SampleAppThT
 *  @{
 */

/** @defgroup MAIN 
 * @{
 */

/* USER CODE BEGIN PV */
/** @defgroup MAIN_Private_Defines 
 * @{
 */ 
/**
 * Define the role here only if it is not already defined in the project options
 * For the CLIENT_ROLE comment the line below 
 * For the SERVER_ROLE uncomment the line below
 */
//#define SERVER_ROLE

/**
 * Define (enable) the throughput test here only if it is not already defined 
 * (enabled) in the project options
 * To enable the THROUGHPUT_TEST uncomment the line below 
 * To disable the THROUGHPUT_TEST comment the line below
 */
//#define THROUGHPUT_TEST

#define BDADDR_SIZE 6

/**
 * @}
 */

/* Private macros ------------------------------------------------------------*/

/** @defgroup MAIN_Private_Variables
 * @{
 */ 
/* Private variables ---------------------------------------------------------*/
uint8_t bnrg_expansion_board = IDB04A1; /* at startup, suppose the X-NUCLEO-IDB04A1 is used */   

#ifdef SERVER_ROLE
  BLE_RoleTypeDef BLE_Role = SERVER;
#else
  BLE_RoleTypeDef BLE_Role = CLIENT;
#endif

#ifdef THROUGHPUT_TEST
  uint8_t throughput_test = 1; /* enable the test for the estimation of the throughput */  
#else
  uint8_t throughput_test = 0; /* disable the test for the estimation of the throughput */
#endif

UART_HandleTypeDef UartHandle;

extern volatile uint8_t set_connectable;
extern volatile int connected;
extern volatile uint8_t notification_enabled;

extern volatile uint8_t end_read_tx_char_handle;
extern volatile uint8_t end_read_rx_char_handle;

/**
 * @}
 */
/* USER CODE END PV */

/** @defgroup MAIN_Private_Function_Prototypes
 * @{
 */ 
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void User_Process(void);

/* USER CODE END PFP */
/**
 * @}
 */

/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for UART interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();
  /* Enable USART1 clock */
  USARTx_CLK_ENABLE(); 
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = USARTx_TX_AF;
  
  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);
    
  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = USARTx_RX_AF;
    
  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
}

/** 
* @brief  Init the VCOM.
* @param  None
* @return None
*/
void vcom_init(void)
{
  /*## Configure the UART peripheral ######################################*/
  /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
  /* UART1 configured as follow:
      - Word Length = 8 Bits
      - Stop Bit = One Stop bit
      - Parity = ODD parity
      - BaudRate = 115200 baud
      - Hardware flow control disabled (RTS and CTS signals) */
  UartHandle.Instance        = USARTx;
  
  UartHandle.Init.BaudRate   = 115200;
  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits   = UART_STOPBITS_1;
  UartHandle.Init.Parity     = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode       = UART_MODE_TX_RX;
  
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    /* Initialization Error */
    _Error_Handler(__FILE__, __LINE__); 
  }
}

/**
 * @brief  Main function to show how to use the BlueNRG STM32 expansion board 
 *         to exchange data between two STM32 Nucleo boards with their
 *         respective BlueNRG STM32 expansion boards.
 *         One board will act as Server-Peripheral and the other as 
 *         Client-Central.
 *         After connection has been established, by pressing the USER button
 *         on one board, the LD2 LED on the other one gets toggled and
 *         viceversa.
 *         The communication is done using a vendor specific profile.
 * @param  None
 * @retval None
 */
int main(void)
{ 
  /* USER CODE BEGIN 1 */
  uint8_t CLIENT_BDADDR[] = {0xbb, 0x00, 0x00, 0xE1, 0x80, 0x02};
  uint8_t SERVER_BDADDR[] = {0xaa, 0x00, 0x00, 0xE1, 0x80, 0x02};
  uint8_t bdaddr[BDADDR_SIZE];
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  
  uint8_t  hwVersion;
  uint16_t fwVersion;
  
  int ret;
  /* USER CODE END 1 */
  
  /* STM32Cube HAL library initialization:
   *  - Configure the Flash prefetch, Flash preread and Buffer caches
   *  - Systick timer is configured by default as source of time base, but user 
   *    can eventually implement his proper time base source (a general purpose 
   *    timer for example or other time source), keeping in mind that Time base 
   *    duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
   *    handled in milliseconds basis.
   *  - Low Level Initialization
   */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* Configure LED2 */
  BSP_LED_Init(LED2);
  
  /* Configure the User Button in GPIO Mode */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
  /* USER CODE END Init */
  
  /* Configure the system clock */
  SystemClock_Config();
  
  /* Initialize the VCOM interface */
  vcom_init();
  
  /* Initialize the BlueNRG SPI driver */
  BNRG_SPI_Init();

  /* Initialize the BlueNRG HCI */
  HCI_Init();
  
  /* Reset BlueNRG hardware */
  BlueNRG_RST();
  
  /* USER CODE BEGIN 2 */
  /* get the BlueNRG HW and FW versions */
  getBlueNRGVersion(&hwVersion, &fwVersion);

  /* 
   * Reset BlueNRG again otherwise we won't
   * be able to change its MAC address.
   * aci_hal_write_config_data() must be the first
   * command after reset otherwise it will fail.
   */
  BlueNRG_RST();
  
  PRINTF("HWver %d, FWver %d\n", hwVersion, fwVersion);
  
  if (hwVersion > 0x30) { /* X-NUCLEO-IDB05A1 expansion board is used */
    bnrg_expansion_board = IDB05A1; 
  }
  
  if(BLE_Role == CLIENT) {
    BLUENRG_memcpy(bdaddr, CLIENT_BDADDR, sizeof(CLIENT_BDADDR));
  } else {
    BLUENRG_memcpy(bdaddr, SERVER_BDADDR, sizeof(SERVER_BDADDR));
  }
  
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                                  CONFIG_DATA_PUBADDR_LEN,
                                  bdaddr);
  if(ret){
    PRINTF("Setting BD_ADDR failed 0x%02x.\n", ret);
  }
  
  ret = aci_gatt_init();    
  if(ret){
    PRINTF("GATT_Init failed.\n");
  }
  
  if(BLE_Role == SERVER) {
    if (bnrg_expansion_board == IDB05A1) {
      ret = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);
    }
    else {
      ret = aci_gap_init_IDB04A1(GAP_PERIPHERAL_ROLE_IDB04A1, &service_handle, &dev_name_char_handle, &appearance_char_handle);
    }
  }
  else {
    if (bnrg_expansion_board == IDB05A1) {
      ret = aci_gap_init_IDB05A1(GAP_CENTRAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);
    }
    else {
      ret = aci_gap_init_IDB04A1(GAP_CENTRAL_ROLE_IDB04A1, &service_handle, &dev_name_char_handle, &appearance_char_handle);
    }
  }
  
  if(ret != BLE_STATUS_SUCCESS){
    PRINTF("GAP_Init failed.\n");
  }
    
  ret = aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
                                     OOB_AUTH_DATA_ABSENT,
                                     NULL,
                                     7,
                                     16,
                                     USE_FIXED_PIN_FOR_PAIRING,
                                     123456,
                                     BONDING);
  if (ret == BLE_STATUS_SUCCESS) {
    PRINTF("BLE Stack Initialized.\n");
  }
  
  if(BLE_Role == SERVER) {
    PRINTF("SERVER: BLE Stack Initialized\n");
    ret = Add_Sample_Service();
    
    if(ret == BLE_STATUS_SUCCESS)
      PRINTF("Service added successfully.\n");
    else
      PRINTF("Error while adding service.\n");
    
  } else {
    PRINTF("CLIENT: BLE Stack Initialized\n");
  }
  
  /* Set output power level */
  ret = aci_hal_set_tx_power_level(1,4);
  /* USER CODE END 2 */
  
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
    HCI_Process();
    User_Process();
  }
  /* USER CODE END 3 */
  
}

/* USER CODE BEGIN 4 */
/**
 * @brief  Process user input (i.e. pressing the USER button on Nucleo board)
 *         and send a LED toggle command to the remote board.
 * @param  None
 * @retval None
 */
void User_Process(void)
{
  if(set_connectable){
    /* Establish connection with remote device */
    Make_Connection();
    set_connectable = FALSE;
  }
  
  if(BLE_Role == CLIENT) {
    /* Start TX handle Characteristic dynamic discovery if not yet done */
    if (connected && !end_read_tx_char_handle){
      startReadTXCharHandle();
    }
    /* Start RX handle Characteristic dynamic discovery if not yet done */
    else if (connected && !end_read_rx_char_handle){      
      startReadRXCharHandle();
    }
    
    if(connected && end_read_tx_char_handle && end_read_rx_char_handle && !notification_enabled) {
      BSP_LED_Off(LED2); //end of the connection and chars discovery phase
      enableNotification();
    }
  }  

  /* Check if the user has pushed the button */
  if(BSP_PB_GetState(BUTTON_KEY) == RESET)
  {
    while (BSP_PB_GetState(BUTTON_KEY) == RESET);
    
    if(connected && notification_enabled){
      /* Send a toggle command to the remote device */
      uint8_t data[20] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J'};
      sendData(data, sizeof(data));
      
      //BSP_LED_Toggle(LED2);  // toggle the LED2 locally.
                               // If uncommented be sure BSP_LED_Init(LED2) is
                               // called in main().
                               // E.g. it can be enabled for debugging.
    }
  }
}
/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {}
}
#endif

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  /* Blink LED2 */
  while(1) 
  {
    BSP_LED_Toggle(LED2);
    HAL_Delay(100);
  }
  /* USER CODE END Error_Handler_Debug */ 
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
