/**
  @page BlueNRG Expansion Board for STM32 Nucleo Boards BLE Profiles Application
   
  @verbatim
  ******************** (C) COPYRIGHT 2015 STMicroelectronics *******************
  * @file    readme.txt
  * @author  CL/AST
  * @version V1.0.0
  * @date    06-Mar-2015
  * @brief   BLE Profiles Sample Application
  ******************************************************************************
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
  @endverbatim

@par Example Description 

This sample application shows how to use BlueNRG Bluetooth Low Energy 
stack.
It uses the STM32 Cube Low Level low power optimizations along with the DMA module.
Current consumption could be monitored through an amperemeter connected to JP6.

To test this application you need:
- an STM32 Nucleo board with its BlueNRG STM32 expansion board
- a Smartphone with Bluetooth Low Energy (BLE) chip and Android
  OS ≥ v4.3 or iOS ≥ v8.0.
The ST BLE Profile app is available:
- on Google Play Store at https://play.google.com/store/apps/details?id=com.stm.bluetoothlevalidation&hl=it
- on Apple App Store at https://apps.apple.com/it/app/stm32-ble-toolbox/id1081331769

This sample application configures the board as Server-Peripheral, while the
smartphone plays the Client-Central role.

To set/change the BLE Profile to test, change the value of the macro BLE_CURRENT_PROFILE_ROLES
(in the "active profile" section) in file:
$PATH_TO_THIS_PACKAGE$\Middlewares\ST\BlueNRG-MS\profiles\Peripheral\Inc\host_config.h

For example, if the HEART_RATE profile is set, after the connection between the
board and the smartphone has been established, the ST BLE Profile App will
show the Heart Rate values in bpm (beats per minute) coming from the STM32 Nucleo
board.

@par Hardware and Software environment

  - This example runs on STM32 Nucleo boards with X-NUCLEO-IDB05A2 STM32 expansion board
    (the X-NUCLEO-IDB05A1 expansion board can be also used)
  - This example has been tested with STMicroelectronics:
    - NUCLEO-L476RG RevC board
    and can be easily tailored to any other supported device and development board.
    
@par How to use it? 

In order to make the program work, you must do the following:
 - WARNING: before opening the project with any toolchain be sure your folder
   installation path is not too in-depth since the toolchain may report errors
   after building.
 - WARNING: because the application is configured in low power mode, the
   debug feature of your toolchain cannot be used.
   For using debugging you must disable the low power configuration
   (set macro LOW_POWER_MODE to 0 in $PATH_TO_THIS_APPLICATION\Inc\main.h).
 - Open STM32CubeIDE (this firmware has been successfully tested with Version 1.4.1).
   Alternatively you can use the Keil uVision toolchain (this firmware
   has been successfully tested with V5.31.0) or the IAR toolchain (this firmware has 
   been successfully tested with Embedded Workbench V8.50.5).
 - Rebuild all files and load your image into target memory.
 - Run the example.
 - Alternatively, you can download the pre-built binaries in "Binary" 
   folder included in the distributed package. 
   
 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
