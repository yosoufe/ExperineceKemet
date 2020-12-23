/**
  @page ProfileCentral Tool for BLE Profiles_Central sample application
  
  @verbatim
  ******************** (C) COPYRIGHT 2015 STMicroelectronics *******************
  * @file    readme.txt  
  * @author  CL/AST  
  * @version V3.1.0
  * @date    15-May-2015
  * @brief   This application provides the user with a GUI to
  *          control the Profile Central Role running on the STM32 Nucleo board
  *          connected with a BlueNRG/BlueNRG-MS expansion board.
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

@par Tool Description 

	- ProfileCentral is a Java based graphical tool allowing the user to control the 
	Profile Central Role running on the STM32 Nucleo board
	connected with a BlueNRG/BlueNRG-MS expansion board.
	- ProfileCentral is platform independent and supports Windows, Mac OS X and Linux
	- In the current version ProfileCentral can be used for the following Central Roles:
		- Heart Rate (HR)
		- Time Client (TC)
		- Find Me Locator (FML)
		- Blood Pressure (BP)
		- Health Thermometer (HT)
		- Alert Notification Client (ANC)
		- Glucose (GL)		

        
@par Hardware and Software environment

	- ProfileCentral uses java-simple-serial-connector (jSSC-2.6.0), a library for working with serial ports from Java (http://code.google.com/p/java-simple-serial-connector/)
	- ProfileCentral requires JRE (Java Runtime) 6+
	- ProfileCentral requires an STM32 Nucleo board connected with a BlueNRG/BlueNRG-MS expansion board.
	It also requires the STM32 Nucleo is plugged to the PC via a USB cable.
	- The Profiles_Central application (enabling the HR, TC, FML, BP, HT, NAC, or GL role) should run on the STM32 Nucleo board.
	- The tool has been tested with NUCLEO-L053R8 RevC, NUCLEO-L476RG RevC, NUCLEO-F401RE RevC and NUCLEO-F411RE RevC.
	- WARNING:
		- UART limitations on NUCLEO-L053R8 could prevent a correct usage of ProfileCentral tool.

@par How to use it ? 

In order to make the program work, you must do the following:
	- Windows/Mac: The tool can be launched by double-clicking the profileCentral.jar
	- Linux: Due to serial port access permission, the "user" should be added to "dialout" group before launching the tool.
	The tool can be launched by typing the following command line:
	$ java -jar profileCentral.jar	
	- The user should select the relevant serial port and then push the "Open" button.
		- NOTE: On Windows, the active serial port is typically the one highest numbered.
	- Once the serial port is open, the Central device is initialized and the discovery phase is started.
	- The discovered peripheral devices (if found) are exposed to the user in a combo box.
	- The user can re-issue the discovery procedure by pushing the "Scan" button.
	- The user can start the connection procedure (with the selected peripheral) by pushing the "Connect" toggle button.
	- Once the connection is successfully established, all the supported Services and Characteristics can be automatically discovered.
	- The measurement notifications/indications are automatically enabled.
	- The user can view the discovered Services and Characteristics (along with the notifications) in a text area.
	- The user can disconnect from the selected peripheral by pushing the "Disconnect" toggle button.
	- By closing the serial port, the disconnection is automatically performed and the Central device is de-initialized.

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
