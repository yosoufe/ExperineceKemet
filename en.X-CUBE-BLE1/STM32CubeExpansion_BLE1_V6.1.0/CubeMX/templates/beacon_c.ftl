[#ftl]
/**
 * @brief  Initialize User process.
 *
 * @param  None
 * @retval None
 */
static void User_Init(void)
{  
  BSP_COM_Init(COM1); 
}

/**
 * @brief  User application process
 * @param  None
 * @retval None
 */
static void User_Process(void)
{
  EnterStopMode(); 
}

/**
 * @brief  Enter the STOP mode
 * @param  None
 * @retval None
 */
void EnterStopMode(void)
{
[#if FamilyName?contains("STM32G0")]
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF);
[#else]  
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
[/#if]
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}
