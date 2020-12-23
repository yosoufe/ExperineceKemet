/**
  ******************************************************************************
  * @file    stm32xx_hal_timerserver_exti.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    01-October-2014
  * @brief   Header for stm32xx_timerserver.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
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
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32XX_HAL_TIMERSERVER_EXTI_H
#define __STM32XX_HAL_TIMERSERVER_EXTI_H

#ifdef __cplusplus
 extern "C" {
#endif

/** @addtogroup Middlewares
 *  @{
 */

/** @addtogroup ST
 *  @{
 */
 
/** @addtogroup TimerServer
 *  @{
 */

/** @defgroup STM32XX_HAL_TIMERSERVER_EXTI
 *  @{
 */
 
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/** @defgroup STM32XX_HAL_TIMERSERVER_EXTI_Exported_Macros
 *  @{
 */
/* Exported macros -----------------------------------------------------------*/
 /**
   * @brief  Set Rising edge in EXTI for Wakeup Timer.
   * @param  None
   * @retval None
   */
#if defined (STM32F030x6) || defined (STM32F030x8) || defined (STM32F030xC) || defined (STM32F031x6) ||\
    defined (STM32F038xx) || defined (STM32F042x6) || defined (STM32F048xx) || defined (STM32F051x8) ||\
    defined (STM32F058xx) || defined (STM32F070x6) || defined (STM32F070xB) || defined (STM32F071xB) ||\
    defined (STM32F072xB) || defined (STM32F078xx) || defined (STM32F091xC) || defined (STM32F098xx) 
  #define __HAL_TIMERSERVER_EXTI_RTC_SET_WAKEUPTIMER_RISING_EDGE()  (EXTI->RTSR |= (RTC_EXTI_LINE_WAKEUPTIMER_EVENT))
#endif
      
#if defined (STM32F100xB) || defined (STM32F100xE) || defined (STM32F101x6) || defined (STM32F101xB) ||\
    defined (STM32F101xE) || defined (STM32F101xG) || defined (STM32F102x6) || defined (STM32F102xB) ||\
    defined (STM32F103x6) || defined (STM32F103xB) || defined (STM32F103xE) || defined (STM32F103xG) ||\
    defined (STM32F105xC) || defined (STM32F107xC)
  #define __HAL_TIMERSERVER_EXTI_RTC_SET_WAKEUPTIMER_RISING_EDGE()  (EXTI->RTSR |= (RTC_EXTI_LINE_WAKEUPTIMER_EVENT))
#endif

#if defined (STM32F205xx) || defined (STM32F215xx) || defined (STM32F207xx) || defined (STM32F217xx)
    #define __HAL_TIMERSERVER_EXTI_RTC_SET_WAKEUPTIMER_RISING_EDGE()  (EXTI->RTSR |= (RTC_EXTI_LINE_WAKEUPTIMER_EVENT))
#endif

#if defined (STM32F301x8) || defined (STM32F302x8) || defined (STM32F302xC) || defined (STM32F302xE) ||\
    defined (STM32F303x8) || defined (STM32F303xC) || defined (STM32F303xE) || defined (STM32F318xx) ||\
    defined (STM32F328xx) || defined (STM32F334x8) || defined (STM32F358xx) || defined (STM32F373xC) ||\
    defined (STM32F378xx) || defined (STM32F398xx)     
  #define __HAL_TIMERSERVER_EXTI_RTC_SET_WAKEUPTIMER_RISING_EDGE()  (EXTI->RTSR |= (RTC_EXTI_LINE_WAKEUPTIMER_EVENT))
#endif
      
#if defined (STM32F401xC) || defined (STM32F401xE) || defined (STM32F405xx) || defined (STM32F407xx) ||\
    defined (STM32F410Cx) || defined (STM32F410Rx) || defined (STM32F410Tx) || defined (STM32F411xE) ||\
    defined (STM32F412Cx) || defined (STM32F412Rx) || defined (STM32F412Vx) || defined (STM32F412Zx) ||\
    defined (STM32F413xx) || defined (STM32F415xx) || defined (STM32F417xx) || defined (STM32F423xx) ||\
    defined (STM32F427xx) || defined (STM32F429xx) || defined (STM32F437xx) || defined (STM32F439xx) ||\
    defined (STM32F446xx) || defined (STM32F469xx) || defined (STM32F479xx) 
  #define __HAL_TIMERSERVER_EXTI_RTC_SET_WAKEUPTIMER_RISING_EDGE()  (EXTI->RTSR |= (RTC_EXTI_LINE_WAKEUPTIMER_EVENT))
#endif

#if defined (STM32F756xx) || defined (STM32F746xx) || defined (STM32F745xx) || defined (STM32F767xx) ||\
    defined (STM32F769xx) || defined (STM32F777xx) || defined (STM32F779xx) || defined (STM32F722xx) ||\
	defined (STM32F723xx) || defined (STM32F732xx) || defined (STM32F733xx)
  #define __HAL_TIMERSERVER_EXTI_RTC_SET_WAKEUPTIMER_RISING_EDGE()  (EXTI->RTSR |= (RTC_EXTI_LINE_WAKEUPTIMER_EVENT))
#endif
      
#if defined (STM32L011xx) || defined (STM32L021xx) || defined (STM32L031xx) || defined (STM32L041xx) ||\
    defined (STM32L051xx) || defined (STM32L052xx) || defined (STM32L053xx) || defined (STM32L061xx) ||\
    defined (STM32L062xx) || defined (STM32L063xx) || defined (STM32L071xx) || defined (STM32L072xx) ||\
    defined (STM32L073xx) || defined (STM32L081xx) || defined (STM32L082xx) || defined (STM32L083xx)
 #define __HAL_TIMERSERVER_EXTI_RTC_SET_WAKEUPTIMER_RISING_EDGE()  (EXTI->RTSR |= (RTC_EXTI_LINE_WAKEUPTIMER_EVENT))
#endif

#if defined (STM32L100xB)  || defined (STM32L100xBA) || defined (STM32L100xC)  || defined (STM32L151xB)  ||\
    defined (STM32L151xBA) || defined (STM32L151xC)  || defined (STM32L151xCA) || defined (STM32L151xD)  ||\
    defined (STM32L151xDX) || defined (STM32L151xE)  || defined (STM32L152xB)  || defined (STM32L152xBA) ||\
    defined (STM32L152xC)  || defined (STM32L152xCA) || defined (STM32L152xD)  || defined (STM32L152xDX) ||\
    defined (STM32L152xE)  || defined (STM32L162xC)  || defined (STM32L162xCA) || defined (STM32L162xD)  ||\
    defined (STM32L162xDX) || defined (STM32L162xE)
 #define __HAL_TIMERSERVER_EXTI_RTC_SET_WAKEUPTIMER_RISING_EDGE()  (EXTI->RTSR |= (RTC_EXTI_LINE_WAKEUPTIMER_EVENT))
#endif
     
#if defined (STM32L431xx) || defined (STM32L432xx) || defined (STM32L433xx) || defined (STM32L442xx) ||\
    defined (STM32L443xx) || defined (STM32L451xx) || defined (STM32L452xx) || defined (STM32L462xx) ||\
    defined (STM32L471xx) || defined (STM32L475xx) || defined (STM32L476xx) || defined (STM32L485xx) ||\
    defined (STM32L486xx)
 #define __HAL_TIMERSERVER_EXTI_RTC_SET_WAKEUPTIMER_RISING_EDGE()  (EXTI->RTSR1 |= (RTC_EXTI_LINE_WAKEUPTIMER_EVENT))
#endif      
   
/**
 * @}
 */
 
/* Exported functions ------------------------------------------------------- */

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

#ifdef __cplusplus
}
#endif

#endif /*__STM32XX_HAL_TIMERSERVER_EXTI_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
