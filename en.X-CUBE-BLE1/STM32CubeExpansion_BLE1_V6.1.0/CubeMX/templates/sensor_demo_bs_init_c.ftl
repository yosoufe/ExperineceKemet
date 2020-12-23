[#ftl]
  const char *name = "BlueNRG";    
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  
  uint8_t  bdaddr_len_out;
  uint8_t  hwVersion;
  uint16_t fwVersion;
  int ret;  
  
  User_Init();
  
  /* Get the User Button initial state */
  user_button_init_state = BSP_PB_GetState(BUTTON_KEY);
    
  hci_init(user_notify, NULL);
  
  /* get the BlueNRG HW and FW versions */
  getBlueNRGVersion(&hwVersion, &fwVersion);
  	
  /* 
   * Reset BlueNRG again otherwise we won't
   * be able to change its MAC address.
   * aci_hal_write_config_data() must be the first
   * command after reset otherwise it will fail.
   */
  hci_reset(); 
  HAL_Delay(100);
 
  PRINTF("HWver %d\nFWver %d\n", hwVersion, fwVersion);
  if (hwVersion > 0x30) { /* X-NUCLEO-IDB05A1 expansion board is used */
    bnrg_expansion_board = IDB05A1;    
  }
  
  ret = aci_hal_read_config_data(CONFIG_DATA_RANDOM_ADDRESS, BDADDR_SIZE, &bdaddr_len_out, bdaddr);
  
  if (ret) {
    PRINTF("Read Static Random address failed.\n");
  }

  if ((bdaddr[5] & 0xC0) != 0xC0) {
    PRINTF("Static Random address not well formed.\n");
    while(1);
  }
  
  /* GATT Init */
  ret = aci_gatt_init();    
  if(ret){
    PRINTF("GATT_Init failed.\n");
  }

  /* GAP Init */
  if (bnrg_expansion_board == IDB05A1) {
    ret = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);
  }
  else {
    ret = aci_gap_init_IDB04A1(GAP_PERIPHERAL_ROLE_IDB04A1, &service_handle, &dev_name_char_handle, &appearance_char_handle);
  }
  if (ret != BLE_STATUS_SUCCESS) {
    PRINTF("GAP_Init failed.\n");
  }

  /* Update device name */
  ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0,
                                   strlen(name), (uint8_t *)name);
  if (ret) {
    PRINTF("aci_gatt_update_char_value failed.\n");            
    while(1);
  }
  
  ret = aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
                                     OOB_AUTH_DATA_ABSENT,
                                     NULL,
                                     7,
                                     16,
                                     USE_FIXED_PIN_FOR_PAIRING,
                                     123456,
                                     BONDING);
  if (ret) {
    PRINTF("aci_gap_set_authentication_requirement failed.\n");
    while(1);
  }
  
  PRINTF("BLE Stack Initialized\n");
  
  ret = Add_HWServW2ST_Service();
  if(ret == BLE_STATUS_SUCCESS) {
    PRINTF("BlueMS HW service added successfully.\n");
  } else {
    PRINTF("Error while adding BlueMS HW service: 0x%02x\r\n", ret);
    while(1);
  }
  
  ret = Add_SWServW2ST_Service();
  if(ret == BLE_STATUS_SUCCESS) {
     PRINTF("BlueMS SW service added successfully.\n");
  } else {
     PRINTF("Error while adding BlueMS HW service: 0x%02x\r\n", ret);
     while(1);
  }

  /* Set output power level */
  ret = aci_hal_set_tx_power_level(1,4);
