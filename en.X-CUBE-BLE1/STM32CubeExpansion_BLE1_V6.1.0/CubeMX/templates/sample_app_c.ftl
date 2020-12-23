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
 * @brief  Configure the device as Client or Server and manage the communication
 *         between a client and a server.
 *
 * @param  None
 * @retval None
 */
static void User_Process(void)
{
  if (set_connectable) 
  {
    /* Establish connection with remote device */
    Make_Connection();
    set_connectable = FALSE;
    user_button_init_state = BSP_PB_GetState(BUTTON_KEY);
  }
  
  if (BLE_Role == CLIENT) 
  {
    /* Start TX handle Characteristic dynamic discovery if not yet done */
    if (connected && !end_read_tx_char_handle){
      startReadTXCharHandle();
    }
    /* Start RX handle Characteristic dynamic discovery if not yet done */
    else if (connected && !end_read_rx_char_handle){      
      startReadRXCharHandle();
    }
    
    if (connected && end_read_tx_char_handle && end_read_rx_char_handle && !notification_enabled) 
    {
      BSP_LED_Off(LED2); //end of the connection and chars discovery phase
      enableNotification();
    }
  }  

  /* Check if the User Button has been pushed */
  if (user_button_pressed) 
  {
    /* Debouncing */
    HAL_Delay(50);
    
    /* Wait until the User Button is released */
    while (BSP_PB_GetState(BUTTON_KEY) == !user_button_init_state);
    
    /* Debouncing */
    HAL_Delay(50);
    
    if (connected && notification_enabled)
    {
      /* Send a toggle command to the remote device */
      uint8_t data[20] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J'};
      sendData(data, sizeof(data));
      
      //BSP_LED_Toggle(LED2);  /* Toggle the LED2 locally. */
                               /* If uncommented be sure the BSP_LED_Init(LED2)
                                * is called in main().
                                * E.g. it can be enabled for debugging. */
    }
    
    /* Reset the User Button flag */
    user_button_pressed = 0;
  } 
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
