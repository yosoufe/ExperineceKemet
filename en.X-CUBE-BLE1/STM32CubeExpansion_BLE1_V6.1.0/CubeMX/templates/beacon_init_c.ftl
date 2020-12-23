[#ftl]
  tBleStatus ret = BLE_STATUS_SUCCESS;

  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  uint8_t SERVER_BDADDR[] = { MAC_ADDRESS };

  uint8_t  hwVersion;
  uint16_t fwVersion;
  uint8_t bnrg_expansion_board = IDB04A1; /* at startup, suppose the X-NUCLEO-IDB04A1 is used */

  User_Init();
    
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

  if (hwVersion > 0x30) { /* X-NUCLEO-IDB05A1 expansion board is used */
    bnrg_expansion_board = IDB05A1; 
  }
  
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                                  CONFIG_DATA_PUBADDR_LEN,
                                  SERVER_BDADDR);

  if (ret != BLE_STATUS_SUCCESS)
  {
    __asm("nop");
  }

  ret = aci_gatt_init();

  if (ret != BLE_STATUS_SUCCESS)
  {
    __asm("nop");
  }

  if (bnrg_expansion_board == IDB05A1) {
    ret = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);
  }
  else {
    ret = aci_gap_init_IDB04A1(GAP_PERIPHERAL_ROLE_IDB04A1, &service_handle, &dev_name_char_handle, &appearance_char_handle);
  }
  
  if (ret != BLE_STATUS_SUCCESS)
  {
    __asm("nop");
  }

  ret = aci_hal_set_tx_power_level(1,4);

  if (ret != BLE_STATUS_SUCCESS)
  {
    __asm("nop");
  }

  /* Initialize beacon services */
  if (EDDYSTONE_BEACON_TYPE & EDDYSTONE_UID_BEACON_TYPE)
  {
    EddystoneUID_Start();
  }
  if (EDDYSTONE_BEACON_TYPE & EDDYSTONE_URL_BEACON_TYPE)
  {
    EddystoneURL_Start();
  }

