[#ftl]
/**
 * @brief  Initialize User process.
 *
 * @param  None
 * @retval None
 */
static void User_Init(void)
{
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
  BSP_LED_Init(LED2);
  
  BSP_COM_Init(COM1);
}

/**
 * @brief  Process user input (i.e. pressing the USER button on Nucleo board)
 *         and send the updated acceleration data to the remote client.
 *
 * @param  None
 * @retval None
 */
static void User_Process(void)
{
  float data_t;
  float data_p;
  static uint32_t counter = 0;
  
  if (set_connectable)
  {
    Set_DeviceConnectable();
    set_connectable = FALSE;
  }  

#if USE_BUTTON  
  /* Check if the user has pushed the button */    
  if (user_button_pressed) 
  {
    /* Debouncing */
    HAL_Delay(50);
    
    /* Wait until the User Button is released */
    while (BSP_PB_GetState(BUTTON_KEY) == !user_button_init_state);
    
    /* Debouncing */
    HAL_Delay(50);
#endif
    BSP_LED_Toggle(LED2);
    
    if (connected)
    {
      /* Set a random seed */
      srand(HAL_GetTick());
	  
      /* Update emulated Environmental data */
      Set_Random_Environmental_Values(&data_t, &data_p);
      BlueMS_Environmental_Update((int32_t)(data_p *100), (int16_t)(data_t * 10));
	              
      /* Update emulated Acceleration, Gyroscope and Sensor Fusion data */
      Set_Random_Motion_Values(counter);
      Acc_Update(&x_axes, &g_axes, &m_axes);      
      Quat_Update(&q_axes);
      
      counter ++;
      if (counter == 40) {
        counter = 0;
        Reset_Motion_Values();
      }
#if !USE_BUTTON      
      HAL_Delay(1000); /* wait 1 sec before sending new data */
#endif
    }
#if USE_BUTTON
    /* Reset the User Button flag */
    user_button_pressed = 0;
  }
#endif  
}

/**
 * @brief  Set random values for all environmental sensor data
 * @param  float pointer to temperature data
 * @param  float pointer to pressure data
 * @retval None
 */
static void Set_Random_Environmental_Values(float *data_t, float *data_p)
{ 
  *data_t = 27.0 + ((uint64_t)rand()*5)/RAND_MAX;     /* T sensor emulation */
  *data_p = 1000.0 + ((uint64_t)rand()*80)/RAND_MAX; /* P sensor emulation */
}

/**
 * @brief  Set random values for all motion sensor data
 * @param  uint32_t counter for changing the rotation direction
 * @retval None
 */
static void Set_Random_Motion_Values(uint32_t cnt)
{ 
  /* Update Acceleration, Gyroscope and Sensor Fusion data */
  if (cnt < 20) {
    x_axes.AXIS_X +=  (10  + ((uint64_t)rand()*3*cnt)/RAND_MAX);
    x_axes.AXIS_Y += -(10  + ((uint64_t)rand()*5*cnt)/RAND_MAX);
    x_axes.AXIS_Z +=  (10  + ((uint64_t)rand()*7*cnt)/RAND_MAX);
    g_axes.AXIS_X +=  (100 + ((uint64_t)rand()*2*cnt)/RAND_MAX);
    g_axes.AXIS_Y += -(100 + ((uint64_t)rand()*4*cnt)/RAND_MAX);
    g_axes.AXIS_Z +=  (100 + ((uint64_t)rand()*6*cnt)/RAND_MAX);
    m_axes.AXIS_X +=  (3  + ((uint64_t)rand()*3*cnt)/RAND_MAX);
    m_axes.AXIS_Y += -(3  + ((uint64_t)rand()*4*cnt)/RAND_MAX);
    m_axes.AXIS_Z +=  (3  + ((uint64_t)rand()*5*cnt)/RAND_MAX);
    
    q_axes.AXIS_X -= (100  + ((uint64_t)rand()*3*cnt)/RAND_MAX);
    q_axes.AXIS_Y += (100  + ((uint64_t)rand()*5*cnt)/RAND_MAX);
    q_axes.AXIS_Z -= (100  + ((uint64_t)rand()*7*cnt)/RAND_MAX);
  }
  else {
    x_axes.AXIS_X += -(10  + ((uint64_t)rand()*3*cnt)/RAND_MAX);
    x_axes.AXIS_Y +=  (10  + ((uint64_t)rand()*5*cnt)/RAND_MAX);
    x_axes.AXIS_Z += -(10  + ((uint64_t)rand()*7*cnt)/RAND_MAX);
    g_axes.AXIS_X += -(100 + ((uint64_t)rand()*2*cnt)/RAND_MAX);
    g_axes.AXIS_Y +=  (100 + ((uint64_t)rand()*4*cnt)/RAND_MAX);
    g_axes.AXIS_Z += -(100 + ((uint64_t)rand()*6*cnt)/RAND_MAX);
    m_axes.AXIS_X += -(3  + ((uint64_t)rand()*7*cnt)/RAND_MAX);
    m_axes.AXIS_Y +=  (3  + ((uint64_t)rand()*9*cnt)/RAND_MAX);
    m_axes.AXIS_Z += -(3  + ((uint64_t)rand()*3*cnt)/RAND_MAX);
    
    q_axes.AXIS_X += (200 + ((uint64_t)rand()*7*cnt)/RAND_MAX);
    q_axes.AXIS_Y -= (150 + ((uint64_t)rand()*3*cnt)/RAND_MAX);
    q_axes.AXIS_Z += (10  + ((uint64_t)rand()*5*cnt)/RAND_MAX);
  }
  
}

/**
 * @brief  Reset values for all motion sensor data
 * @param  None
 * @retval None
 */
static void Reset_Motion_Values(void)
{
  x_axes.AXIS_X = (x_axes.AXIS_X)%2000 == 0 ? -x_axes.AXIS_X : 10;
  x_axes.AXIS_Y = (x_axes.AXIS_Y)%2000 == 0 ? -x_axes.AXIS_Y : -10;
  x_axes.AXIS_Z = (x_axes.AXIS_Z)%2000 == 0 ? -x_axes.AXIS_Z : 10;
  g_axes.AXIS_X = (g_axes.AXIS_X)%2000 == 0 ? -g_axes.AXIS_X : 100;
  g_axes.AXIS_Y = (g_axes.AXIS_Y)%2000 == 0 ? -g_axes.AXIS_Y : -100;
  g_axes.AXIS_Z = (g_axes.AXIS_Z)%2000 == 0 ? -g_axes.AXIS_Z : 100;
  m_axes.AXIS_X = (g_axes.AXIS_X)%2000 == 0 ? -m_axes.AXIS_X : 3;
  m_axes.AXIS_Y = (g_axes.AXIS_Y)%2000 == 0 ? -m_axes.AXIS_Y : -3;
  m_axes.AXIS_Z = (g_axes.AXIS_Z)%2000 == 0 ? -m_axes.AXIS_Z : 3;
  q_axes.AXIS_X = -q_axes.AXIS_X;
  q_axes.AXIS_Y = -q_axes.AXIS_Y;
  q_axes.AXIS_Z = -q_axes.AXIS_Z;
}

/**
  * @brief  BSP Push Button callback
  * @param  Button Specifies the pin connected EXTI line
  * @retval None.
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
  /* Set the User Button flag */
  user_button_pressed = 1;
}
