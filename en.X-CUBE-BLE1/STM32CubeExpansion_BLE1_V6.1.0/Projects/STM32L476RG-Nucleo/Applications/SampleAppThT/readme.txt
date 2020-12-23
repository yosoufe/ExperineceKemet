/**
  @page BlueNRG Expansion Board for STM32 Nucleo Boards Sample Application
  
  @verbatim
  ******************** (C) COPYRIGHT 2014 STMicroelectronics *******************
  * @file    readme.txt 
  * @author  CL/AST  
  * @version V0.0.1
  * @date    19-May-2014
  * @brief   Description of the BlueNRG sample application.
  ******************************************************************************
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
  @endverbatim

@par Example Description 

This sample application shows how to use BlueNRG Bluetooth Low Energy 
stack and to measure the application throughput.
To test this application you need two STM32 Nucleo boards with their
respective BlueNRG STM32 expansion boards. One board needs to be configured
as Server-Peripheral role, while the other needs to be configured as Client-Central
role.
Before flashing the boards, please make sure to use the right configuration by selecting
it from the menu options of the toolchain.

Selecting the "SampleApp_*" configurations allows to target the SampleApp.
 - Program the CLIENT on one STM32 Nucleo board, with BlueNRG STM32 expansion board,
   and reset it (configuration: SampleApp_CLIENT). 
 - Program the SERVER on a second STM32 Nucleo board, with BlueNRG STM32 expansion
   board, and reset it (configuration: SampleApp_SERVER). 
 - After establishing the connection between the two boards (when the LED2 on the
   CLIENT turns off),
   by pressing the USER button on one board, the LD2 LED on the other one gets toggled
   and viceversa.
   No serial terminal emulator must be open since no throughput test is performed.
 - If you have only one STM32 Nucleo board, you can program it as SERVER and use as CLIENT
   the BLE IOT app for Android devices available on the Play Store at
   https://play.google.com/store/apps/details?id=com.stmicro.bleiot 

Selecting the "ThroughputTest_*" configurations allows to perform a throughput test.
 - Program the CLIENT on one STM32 Nucleo board, with BlueNRG STM32 expansion board,
   and reset it (configuration: ThroughputTest_CLIENT).
   The platform will be seen on the PC as a virtual COM port. Open the port in a serial terminal emulator
   (Word Length = 8 Bits, Stop Bit = One Stop bit, Parity = ODD parity, BaudRate = 115200 baud,
   Hardware flow control disabled (RTS and CTS signals)).
 - Program the SERVER on a second STM32 Nucleo board, with BlueNRG STM32 expansion
   board, and reset it (configuration: ThroughputTest_SERVER).
   The two boards will try to establish a connection.
 - During the connection and characteristics discovery phase the LED2 on the CLIENT
   is ON.
   Once the connection between the two boards is established, the LED2 on the CLIENT turns OFF.
 - The throughput test can be started by pressing the USER button on the the Server-Peripheral board.
   The Server-Peripheral will continuously send notifications of 20 bytes to the Client-Central.
 - After every 500 packets, the measured application throughput will be displayed on the terminal emulator.  
 - IMPORTANT NOTE: To avoid issues with USB connection (mandatory if you have USB 3.0), it is   
   suggested to update the ST-Link/V2 firmware for STM32 Nucleo boards to the latest version.
   Please refer to the readme.txt file in the Applications directory for details.
 - WARNING: When using the NUCLEO-L053R8 boards the Throughput test may fails. No problem occurs
   when using NUCLEO-F401RE or NUCLEO-L476RG boards.
 
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
