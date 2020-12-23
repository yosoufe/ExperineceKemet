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
#include "cube_hal.h"
#include "bluenrg_types.h"
#include "hci_const.h"
#include "low_power_conf.h"
#include "profile_application.h"
#include "debug.h"

#include "hci_tl_interface.h"
#include "bluenrg_hal_aci.h"
#include "ble_profile.h"

#include "stm32xx_timerserver.h"
#include "stm32xx_hal_app_rtc.h"

#include <stdio.h>

/** @defgroup Applications
 *  @{
 */

/** @defgroup Profiles_DMA_LowPower
 *  @brief Sample application for X-NUCLEO-IDB04A1/X-NUCLEO-IDB05A1 STM32 expansion boards.
 *  @{
 */

/** @defgroup MAIN
 *  @{
 */

/** @defgroup MAIN_Global_Variables
 *  @{
 */
/* Global variables ---------------------------------------------------------*/
tProfileApplContext profileApplContext; /* Profile Application Context */
/**
 * @}
 */
 
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/

/** @defgroup MAIN_Private_Variables
 *  @{
 */
/* Private variables ---------------------------------------------------------*/
static RTC_HandleTypeDef hrtc;  /* RTC handler declaration */
static uint32_t TaskExecutionRequested = 0;
static uint8_t Hci_Cmd_Lock = FALSE;
static pf_TIMER_TimerCallBack_t BlueNRG_Profile_Event_Callback;

#if (BLE_CURRENT_PROFILE_ROLES & HID_DEVICE)
extern volatile uint8_t skip_hid_tick_inc;
extern volatile uint8_t hid_tick;
#endif
/**
 * @}
 */

/** @defgroup MAIN_Private_Function_Prototypes
 *  @{
 */
/* Private function prototypes -----------------------------------------------*/
void Background (void);
static void Init_RTC(void);
/**
 * @}
 */

/**
  * @brief  This function is empty to avoid starting the SysTick Timer
  * @param  None
  * @retval None
  */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  return (HAL_OK);
}

/**
  * @brief This function provides accurate delay (in milliseconds) based
  *        on variable incremented.
  * @note In the default implementation , SysTick timer is the source of time base.
  *       It is used to generate interrupts at regular time intervals where uwTick
  *       is incremented.
  * @note ThiS function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @note NOT USED IN THIS EXAMPLE
  * @param Delay: specifies the delay time length, in milliseconds.
  * @retval None
  */
void HAL_Delay(__IO uint32_t Delay)
{
  return;
}

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
#if (JTAG_SUPPORTED == 1) 
  /*
  * Keep debugger enabled while in any low power mode
  */
  HAL_DBGMCU_EnableDBGSleepMode();
  HAL_DBGMCU_EnableDBGStopMode();
  HAL_DBGMCU_EnableDBGStandbyMode();

#endif /* (JTAG_SUPPORTED == 1) */
 
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
  
  /* Configure the system clock */
  SystemClock_Config();
  
  /* Configure the system Power */
  SystemPower_Config();
    
  /* Initialize the Profile Application Context's Data Structures */
  BLUENRG_memset(&profileApplContext,0,sizeof(tProfileApplContext));
  
  /* Configure the RTC */
  Init_RTC();
  TIMER_Init(&hrtc);
  TIMER_Create(eTimerModuleID_BlueNRG_Profile_App, &(profileApplContext.profileTimer_Id), eTimerMode_Repeated, 0);
  
  LPM_Mode_Request(eLPM_MAIN_LOOP_PROCESSES, eLPM_Mode_LP_Stop);  
  
  /* Initialize the BlueNRG SPI driver */
  BNRG_SPI_Init();
  
  /* Set current BlueNRG profile (HRM, HTM, GS, ...) */
  BNRG_Set_Current_profile();
  
  /* Initialize the BlueNRG Profile */
  /* set tx power and security parameters (common to all profiles) */
  BNRG_Profiles_Init();  
  /* low level profile initialization (profile specific) */
  profileApplContext.initProfileFunc();
  
  /* Start the main processes */
  while(1)
  {
    Background();
    
  } /* end while(1) */  
}
    
/**
  * @brief  Request action to the scheduler in background
  *
  * @note
  * @retval None
  */
void TaskExecutionRequest(eMAIN_Background_Task_Id_t eMAIN_Background_Task_Id)
{
  __disable_irq();
  TaskExecutionRequested |= (1 << eMAIN_Background_Task_Id);
  __enable_irq();
  
  return;
}

/**
  * @brief  Notify the action in background has been completed
  *
  * @note
  * @param  eMAIN_Background_Task_Id: Id of the request
  * @retval None
  */
void TaskExecuted(eMAIN_Background_Task_Id_t eMAIN_Background_Task_Id)
{
  __disable_irq();
  TaskExecutionRequested &= ~(1 << eMAIN_Background_Task_Id);
  __enable_irq();
  
  return;
}

void HCI_Process_Notification_Request()
{
  TaskExecutionRequest(eMAIN_HCI_Process_Request_Id);
}

void Profile_Process_Notification_Request()
{
  TaskExecutionRequest(eMAIN_Profile_Process_Request_Id);
}

/**
  * @brief  Background task
  *
  * @note
  * @param  None
  * @retval None
  */
void Background (void)
{
  if(TaskExecutionRequested & (1<< eMAIN_Profile_Event_Id))
  {
    if(Hci_Cmd_Lock == FALSE)
    {
      TaskExecuted(eMAIN_Profile_Event_Id);
      BlueNRG_Profile_Event_Callback();
    }
  }
  if(TaskExecutionRequested & (1<< eMAIN_Profile_Measurement_update_Id))
  {
    if(Hci_Cmd_Lock == FALSE)
    {
      TaskExecuted(eMAIN_Profile_Measurement_update_Id);
      profileApplContext.profileProcessMeasurementFunc();
    }
  }
    
  if(TaskExecutionRequested & (1<< eMAIN_HCI_Process_Request_Id))
  {
    /**
     * It shall be controlled in the application that no HCI command are sent
     * if one is already pending
     */
    if(Hci_Cmd_Lock == FALSE)
    {
      TaskExecuted(eMAIN_HCI_Process_Request_Id);
      HCI_Process();
    }
  }
  
  if(TaskExecutionRequested & (1<< eMAIN_Profile_Process_Request_Id))
  {
    if(Hci_Cmd_Lock == FALSE)
    {
      TaskExecuted(eMAIN_Profile_Process_Request_Id);
      Profile_Process_Q();
    }
  }
  
  if(TaskExecutionRequested & (1<< eMAIN_Profile_StateMachine_update_Id))
  {
    if(Hci_Cmd_Lock == FALSE)
    {
      TaskExecuted(eMAIN_Profile_StateMachine_update_Id);
      profileApplContext.profileStateMachineFunc();
    }
  }
  
  if(TaskExecutionRequested & (1<< eMAIN_Profile_App_DeviceState_update_Id))
  {
    if(Hci_Cmd_Lock == FALSE)
    {
      TaskExecuted(eMAIN_Profile_App_DeviceState_update_Id);
      profileApplContext.profileApplicationProcessFunc();
    }      
  }
    
  /**
  * Power management
  */
#if LOW_POWER_MODE
  __disable_irq();
  if((TaskExecutionRequested == 0) || (((TaskExecutionRequested & EVENT_NOT_REQUIRING_SENDING_HCI_COMMAND) == 0) && (Hci_Cmd_Lock == TRUE)))
  {
    LPM_Enter_Mode();
  }
  __enable_irq();
#endif /* LOW_POWER_MODE */
  
  return;
}

void HCI_Wait_For_Response(void)
{
  Background();
  
  return;
}

/**
  * @brief  Initialize RTC block
  *
  * @note
  * @param  None
  * @retval None
  */
static void Init_RTC(void)
{
  
  /* Initialize the HW - 37Khz LSI being used*/
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
  __HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc) ;
  
  HAL_APP_RTC_Set_Wucksel(&hrtc, WUCKSEL_DIVIDER);  /**< Tick timer is 55us */
  
  /* Wait for LSI to be stable */
  while(__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) == 0);
  
  return;
}
    
void TIMER_Notification(eTimerProcessID_t eTimerProcessID, uint8_t ubTimerID, pf_TIMER_TimerCallBack_t pfTimerCallBack)
{
  switch (eTimerProcessID)
  {
  case eTimerModuleID_BlueNRG_Profile_App:
    /**
    * The code shall be executed in the background as aci command may be sent
    * The background is the only place where the application can make sure a new aci command
    * is not sent if there is a pending one
    */  
#if (BLE_CURRENT_PROFILE_ROLES & HID_DEVICE)
    if(skip_hid_tick_inc == 0) {
      hid_tick++;
    }
#endif
    TaskExecutionRequest(eMAIN_Profile_Measurement_update_Id);
    break;
   
  case eTimerModuleID_BlueNRG_Profile:
    /**
    * The code shall be executed in the background as aci command may be sent
    * The background is the only place where the application can make sure a new aci command
    * is not sent if there is a pending one
    */
    BlueNRG_Profile_Event_Callback = pfTimerCallBack;
    TaskExecutionRequest(eMAIN_Profile_Event_Id);
    break;
    
  case eTimerModuleID_BlueNRG_HCI:
    /**
    * The code is very short and there is no aci command to be send so it is better to call it in the interrupt handler
    *
    */
    pfTimerCallBack();
    break;
    
  case eTimerModuleID_Interrupt:
    /*
    * This is expected to be called in the Timer Interrupt Handler to limit latency
    * Calling the callback in the background task may impact power performance
    */
    pfTimerCallBack();
    break;

  default:
    if (pfTimerCallBack != 0)
    {
      pfTimerCallBack();
    }
    break;
  }
}

void HCI_Cmd_Status(HCI_CMD_STATUS_t Hci_Cmd_Status)
{
  if(Hci_Cmd_Status == BUSY)
  {
    Hci_Cmd_Lock = TRUE;
  }
  else
  {
    Hci_Cmd_Lock = FALSE;
  }
  return;
}

/**
 * When the critical section is used #define TIMER_USE_PRIMASK_AS_CRITICAL_SECTION in stm32xx_timerserver.h
 * the BNRG_Timer_Start_Allowed() may be called here
 * otherwise it shall be call from the background as BNRG_Request_Timer_Start() is called from DMA interrupt
 * and may result in nested call of the timer interface (which is not supported
 */
void BNRG_Request_Timer_Start(void)
{
  BNRG_Timer_Start_Allowed();
  return;
}

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
     ex: PRINTF("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
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
