/*
 * errorHandling.c
 *
 *  Created on: Jan 10, 2021
 *      Author: yousof
 */

#include "errorHandling.h"
#include "stm32h7xx_nucleo.h"

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    /* Toggle LED3 */
    HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
    HAL_Delay(500);
  }
  /* USER CODE END Error_Handler_Debug */
}
