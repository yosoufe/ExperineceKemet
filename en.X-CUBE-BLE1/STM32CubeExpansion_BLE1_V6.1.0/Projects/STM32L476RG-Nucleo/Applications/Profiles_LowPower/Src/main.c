/**
******************************************************************************
* @file    main.c 
* @author  AAS / CL
* @version V1.0.0
* @date    04-July-2014
* @brief   This sample code shows how to use the BlueNRG Bluetooth Low Energy 
*          Expansion Board to exchange data between two BLE devices.
*          The communication is done using 1 Nucleo boards and one Smartphone
*          with BLE chip and Android >= v4.3.
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

/* USER CODE BEGIN Includes */
#include "hci_tl.h"
#include "profile_application.h"
#include "debug.h"
#include "stm32xx_timerserver.h" 
#include "stm32xx_hal_app_rtc.h"
/* USER CODE END Includes */

#include "stm32xx_lpm.h"
#include "low_power_conf.h"
#include "bluenrg_utils.h"
#include "hci_tl_interface.h"
#include "bluenrg_conf.h"

#include <stdio.h>

/** @addtogroup Applications
 *  @{
 */

/** @defgroup Profiles_LowPower
 *  @brief Sample application for X-NUCLEO-IDB04A1/X-NUCLEO-IDB05A1 STM32 expansion boards.
 *  @{
 */

/** @defgroup MAIN
 *  @{
 */

/* USER CODE BEGIN PV */
/** @defgroup MAIN_Private_Defines
 *  @{
 */
/* Private defines -----------------------------------------------------------*/
/**
* @brief Enable/Disable the low power mode (0: disabled, 1:enabled)
*        For debugging keep the low power mode disabled.
*        For CubeMX generated projects keep the low power mode disabled.
*/
#define LOW_POWER_ENABLED 1
/**
 * @}
 */

/* Private macros ------------------------------------------------------------*/

/** @defgroup MAIN_Global_Variables
 *  @{
 */
/* Global variables ---------------------------------------------------------*/
tProfileApplContext profileApplContext; /* Profile Application Context */
/**
 * @}
 */

/** @defgroup MAIN_Private_Variables
 *  @{
 */
/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;  /* RTC handler declaration */
extern volatile uint8_t send_measurement;
#if (BLE_CURRENT_PROFILE_ROLES & HID_DEVICE)
extern volatile uint8_t skip_hid_tick_inc;
extern volatile uint8_t hid_tick;
#endif

/**
 * @}
 */
/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/** @defgroup MAIN_Private_Function_Prototypes
 *  @{
 */
/* Private function prototypes -----------------------------------------------*/
static void Init_RTC(void);
/**
 * @}
 */
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  Main function to show how to use BlueNRG Bluetooth Low Energy 
 *         stack.
 *         To test this application you need:
 *         - an STM32 Nucleo board with its BlueNRG STM32 expansion board
 *         - a Smartphone with Bluetooth Low Energy (BLE) chip and Android
 *           OS >= v4.3.
 *         On the Android smartphone the STM32_BLE_Toolbox App must be installed
 *         and running.
 *         The STM32_BLE_Toolbox App can be found in this package at:
 *         $PATH_TO_THIS_PACKAGE$\Utilities\Android_Software\Profiles_Central
 *         This sample application configures the board as Server-Peripheral,
 *         while the smartphone plays the Client-Central role.
 *         To set/change the BLE Profile to test, change the value of the macro
 *         BLE_CURRENT_PROFILE_ROLES (in the "active profile" section) in file:
 *         $PATH_TO_THIS_PACKAGE$\Middlewares\ST\STM32_BlueNRG\Profile_Framework
 *         \includes\host_config.h
 *         For example, if the HEART_RATE profile is set, after the connection
 *         between the board and the smartphone has been established, the
 *         STM32_BLE_Toolbox App will show the Heart Rate values in bpm (beats
 *         per minute) coming from the STM32 Nucleo board.
 *         The communication is done using a vendor specific profile.
 *
 * @param  None
 * @retval None
 */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t low_power_enabled;
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
#if LOW_POWER_ENABLED
  low_power_enabled = TRUE;
#else
  low_power_enabled = FALSE;
#endif
  /* USER CODE END Init */
  
  /* Configure the system clock */
  SystemClock_Config();
    
  /* USER CODE BEGIN SysInit */  
  /** 
   * Configure the system in Low Power Mode  
   * - Unused GPIO in Analog mode
   * - Fast Wakeup
   * - Verefint disabled in low power mode
   * - FLASH in low power mode
   */
  if (low_power_enabled) {
    SystemPower_Config();
  }
  
  /* Initialize the Profile Application Context's Data Structures */
  BLUENRG_memset(&profileApplContext,0,sizeof(tProfileApplContext));
  /* USER CODE END SysInit */

  /* Initialize the BlueNRG SPI driver */
  BNRG_SPI_Init();
  
  /* USER CODE BEGIN 2 */ 
  /* Configure the RTC */
  Init_RTC();  
  TIMER_Init(&hrtc);
  TIMER_Create(eTimerModuleID_BlueNRG_Profile_App, &(profileApplContext.profileTimer_Id), eTimerMode_Repeated, 0);
 
  /* Set current BlueNRG profile (HRM, HTM, GS, ...) */
  BNRG_Set_Current_profile();
  
  /* Initialize the BlueNRG Profile */
  /* set tx power and security parameters (common to all profiles) */
  BNRG_Profiles_Init(); 
  /* low level profile initialization (profile specific) */
  profileApplContext.initProfileFunc();
  
  BLE_Profile_Write_DeviceState(APPL_UNINITIALIZED);
  /* USER CODE END 2 */
  
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /* Start the main processes */  
  while(1)
  {
  /* USER CODE END WHILE */
    
  /* USER CODE BEGIN 3 */
    HCI_Process();
    profileApplContext.profileStateMachineFunc();
    if (Profile_Process_Q() == 0x00)
    {
      if (low_power_enabled)
      {
        if (profileApplContext.profileApplicationProcessFunc() != 0) {
          LPM_Mode_Request(eLPM_MAIN_LOOP_PROCESSES, eLPM_Mode_RUN);
        }else{
          LPM_Mode_Request(eLPM_MAIN_LOOP_PROCESSES, eLPM_Mode_LP_Stop);
        }
      }
      else
      {
        profileApplContext.profileApplicationProcessFunc();
      }
    }
    LPM_Enter_Mode();
    
  }
  /* USER CODE END 3 */
    
}

/* USER CODE BEGIN 4 */
/**
  * @brief  Initialize RTC block
  *
  * @note
  * @param  None
  * @retval None
  */
static void Init_RTC(void)
{
  /* Initialize the HW - 37Khz LSI being used */
  /* Enable the LSI clock */
  __HAL_RCC_LSI_ENABLE();
  
  /* Enable power module clock */
  __PWR_CLK_ENABLE();
  
  /* Enable acces to the RTC registers */
  HAL_PWR_EnableBkUpAccess();
  
  /**
   *  Write twice the value to flush the APB-AHB bridge
   *  This bit shall be written in the register before writing the next one
   */
  HAL_PWR_EnableBkUpAccess();

  /* Select LSI as RTC Input */
  __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSI);
  
  /* Enable RTC */
  __HAL_RCC_RTC_ENABLE();
  
  hrtc.Instance = RTC;                  /**< Define instance */
  hrtc.Lock = HAL_UNLOCKED;             /**< Initialize lock */
  hrtc.State = HAL_RTC_STATE_READY;     /**< Initialize state */
  
  /**
  * Bypass the shadow register
  */
  HAL_RTCEx_EnableBypassShadow(&hrtc);
  
  /**
  * Set the Asynchronous prescaler
  */
  hrtc.Init.AsynchPrediv = RTC_ASYNCH_PRESCALER;
  hrtc.Init.SynchPrediv = RTC_SYNCH_PRESCALER;
  HAL_RTC_Init(&hrtc);
  
  /* Disable Write Protection */
  __HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc);
  
  HAL_APP_RTC_Set_Wucksel(&hrtc, WUCKSEL_DIVIDER);  /**< Tick timer is 55us */
  
  /* Wait for LSI to be stable */
  while(__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) == 0);
  
  return;
}

void TIMER_Notification(eTimerProcessID_t eTimerProcessID, uint8_t ubTimerID, pf_TIMER_TimerCallBack_t pfTimerCallBack)
{
  uint32_t uwPRIMASK_Bit = __get_PRIMASK();	/**< backup PRIMASK bit */;
  
  switch (eTimerProcessID)
  {
  case eTimerModuleID_BlueNRG_Profile_App:
#if !((BLE_CURRENT_PROFILE_ROLES & ALERT_NOTIFICATION_SERVER) || (BLE_CURRENT_PROFILE_ROLES & HID_DEVICE))
    /* Clear Wake Up Flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);    
    __disable_irq();  /**< Disable all interrupts by setting PRIMASK bit on Cortex*/

    send_measurement++;
    __set_PRIMASK(uwPRIMASK_Bit);  /**< Restore PRIMASK bit*/
#endif
#if (BLE_CURRENT_PROFILE_ROLES & HID_DEVICE)
    if(skip_hid_tick_inc == 0) {
      /* Clear Wake Up Flag */
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
      __disable_irq();  /**< Disable all interrupts by setting PRIMASK bit on Cortex*/

      hid_tick++;
      __set_PRIMASK(uwPRIMASK_Bit);  /**< Restore PRIMASK bit*/
    }
#endif
    break;
  default:
    if (pfTimerCallBack != 0)
    {
      pfTimerCallBack();
    }
    break;
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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: PRINTF("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
  /* USER CODE END 6 */
  
}
#endif

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
 
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
