[#ftl]
[#assign moduleName = "x-cube-ble1"]
[#if ModuleName??]
    [#assign moduleName = ModuleName]
[/#if]
/**
  ******************************************************************************
  * File Name          : app_${moduleName?lower_case}.c
  * Description        : Source file
  *
  ******************************************************************************
  *
  * COPYRIGHT ${year} STMicroelectronics
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
  ******************************************************************************
  */#n

/* Includes ------------------------------------------------------------------*/
#include "app_${moduleName?lower_case}.h"
[#if includes??]
[#list includes as include]
#include "${include}"
[/#list]
[/#if]

[#assign useSENSORDEMO = false]
[#assign useSENSORDEMO_BS = false]
[#assign useSAMPLEAPP = false]
[#assign useBEACON = false]
[#assign useVIRTUALCOMPORT = false]
[#assign useCENTRALPROFILES = false]
[#assign usePERIPHERALPROFILES = false]

[#if RTEdatas??]
[#list RTEdatas as define]

[#if define?ends_with("SENSOR_DEMO")]
[#assign useSENSORDEMO = true]
[/#if]
[#if define?contains("SENSOR_DEMO_BS")]
[#assign useSENSORDEMO_BS = true]
[/#if]
[#if define?contains("SAMPLE_APP")]
[#assign useSAMPLEAPP = true]
[/#if]
[#if define?contains("BEACON")]
[#assign useBEACON = true]
[/#if]
[#if define?contains("VIRTUAL_COM_PORT")]
[#assign useVIRTUALCOMPORT = true]
[/#if]
[#if define?contains("PERIPHERAL_PROFILES")]
[#assign usePERIPHERALPROFILES = true]
[/#if]
[#if define?contains("CENTRAL_PROFILES")]
[#assign useCENTRALPROFILES = true]
[/#if]

[/#list]
[/#if]

[#if useSENSORDEMO]
[@common.optinclude name=mxTmpFolder + "/sensor_demo_gv.tmp"/]
[/#if]
[#if useSENSORDEMO_BS]
[@common.optinclude name=mxTmpFolder + "/sensor_demo_bs_gv.tmp"/]
[/#if]
[#if useSAMPLEAPP]
[@common.optinclude name=mxTmpFolder + "/sample_app_gv.tmp"/]
[/#if]
[#if useBEACON]
[@common.optinclude name=mxTmpFolder + "/beacon_gv.tmp"/]
[/#if]
[#if useVIRTUALCOMPORT]
[@common.optinclude name=mxTmpFolder + "/virtual_com_port_gv.tmp"/]
[/#if]
[#if usePERIPHERALPROFILES]
[@common.optinclude name=mxTmpFolder + "/peripheral_profiles_gv.tmp"/]
[/#if]
[#if useCENTRALPROFILES]
[@common.optinclude name=mxTmpFolder + "/central_profiles_gv.tmp"/]
[/#if]
[#if (!useSENSORDEMO) && (!useSENSORDEMO_BS) && (!useSAMPLEAPP) && (!useBEACON) && (!useVIRTUALCOMPORT)]
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private Variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private Function Prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */
[/#if]


#if PRINT_CSV_FORMAT
extern volatile uint32_t ms_counter;
/**
 * @brief  This function is a utility to print the log time
 *         in the format HH:MM:SS:MSS (DK GUI time format)
 * @param  None
 * @retval None
 */
void print_csv_time(void){
  uint32_t ms = HAL_GetTick();
  PRINT_CSV("%02ld:%02ld:%02ld.%03ld", ms/(60*60*1000)%24, ms/(60*1000)%60, (ms/1000)%60, ms%1000);
}
#endif

void ${fctName}(void)
{
  /* USER CODE BEGIN SV */ 

  /* USER CODE END SV */
  
  /* USER CODE BEGIN ${fctName?replace("MX_","")}_PreTreatment */
  
  /* USER CODE END ${fctName?replace("MX_","")}_PreTreatment */

[#if useSENSORDEMO | useSENSORDEMO_BS | useSAMPLEAPP | useBEACON | useVIRTUALCOMPORT]
  /* Initialize the peripherals and the BLE Stack */
[/#if]
[#if useSENSORDEMO]
[@common.optinclude name=mxTmpFolder + "/sensor_demo_init.tmp"/]
[/#if]
[#if useSENSORDEMO_BS]
[@common.optinclude name=mxTmpFolder + "/sensor_demo_bs_init.tmp"/]
[/#if]
[#if useSAMPLEAPP]
[@common.optinclude name=mxTmpFolder + "/sample_app_init.tmp"/]
[/#if]
[#if useBEACON]
[@common.optinclude name=mxTmpFolder + "/beacon_init.tmp"/]
[/#if]
[#if useVIRTUALCOMPORT]
[@common.optinclude name=mxTmpFolder + "/virtual_com_port_init.tmp"/]
[/#if]
[#if usePERIPHERALPROFILES]
[@common.optinclude name=mxTmpFolder + "/peripheral_profiles_init.tmp"/]
[/#if]
[#if useCENTRALPROFILES]
[@common.optinclude name=mxTmpFolder + "/central_profiles_init.tmp"/]
[/#if]
  
  /* USER CODE BEGIN ${fctName?replace("MX_","")}_PostTreatment */
  
  /* USER CODE END ${fctName?replace("MX_","")}_PostTreatment */
}

/*
 * BlueNRG-MS background task
 */
void ${fctProcessName}(void)
{
  /* USER CODE BEGIN ${fctProcessName?replace("MX_","")}_PreTreatment */
  
  /* USER CODE END ${fctProcessName?replace("MX_","")}_PreTreatment */
  
[#if useSENSORDEMO]  
  User_Process(&axes_data);  
  hci_user_evt_proc();
[/#if]   
[#if useSENSORDEMO_BS | useSAMPLEAPP | useBEACON | useCENTRALPROFILES]
  User_Process();  
  hci_user_evt_proc();
[/#if]
[#if useVIRTUALCOMPORT]
  User_Process();  
[/#if]
[#if usePERIPHERALPROFILES]
  hci_user_evt_proc();
  User_Process();
[/#if]

  /* USER CODE BEGIN ${fctProcessName?replace("MX_","")}_PostTreatment */
  
  /* USER CODE END ${fctProcessName?replace("MX_","")}_PostTreatment */
}

[#if useSENSORDEMO]
[#-- SensorDemo Code Begin--]
[@common.optinclude name=mxTmpFolder + "/sensor_demo.tmp"/]
[#-- SensorDemo Code End--]
[/#if]
[#if useSENSORDEMO_BS]
[#-- SensorDemo_BLESEnsor-App Code Begin--]
[@common.optinclude name=mxTmpFolder + "/sensor_demo_bs.tmp"/]
[#-- SensorDemo_BLESensor-App Code End--]
[/#if]
[#if useSAMPLEAPP]
[#-- SampleApp Code Begin--]
[@common.optinclude name=mxTmpFolder + "/sample_app.tmp"/]
[#-- SampleApp Code End--]
[/#if]
[#if useBEACON]
[#-- Beacon Code Begin--]
[@common.optinclude name=mxTmpFolder + "/beacon.tmp"/]
[#-- Beacon Code End--]
[/#if]
[#if useVIRTUALCOMPORT]
[#-- Virtual_COM_Port Code Begin--]
[@common.optinclude name=mxTmpFolder + "/virtual_com_port.tmp"/]
[#-- Virtual_COM_Port Code End--]
[/#if]
[#if usePERIPHERALPROFILES]
[#-- PeripheralProfiles Code Begin--]
[@common.optinclude name=mxTmpFolder + "/peripheral_profiles.tmp"/]
[#-- PeripheralProfiles Code End--]
[/#if]
[#if useCENTRALPROFILES]
[#-- CentralProfiles Code Begin--]
[@common.optinclude name=mxTmpFolder + "/central_profiles.tmp"/]
[#-- CentralProfiles Code End--]
[/#if]
[#if (!useSENSORDEMO) && (!useSENSORDEMO_BS) && (!useSAMPLEAPP) && (!useBEACON) && (!useVIRTUALCOMPORT)]
/* USER CODE BEGIN Function Implementation */

/* USER CODE END Function Implementation */
[/#if]


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
