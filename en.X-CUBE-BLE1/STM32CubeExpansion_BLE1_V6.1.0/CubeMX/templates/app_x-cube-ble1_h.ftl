[#ftl]
[#assign moduleName = "x-cube-ble1"]
[#if ModuleName??]
	[#assign moduleName = ModuleName]
[/#if]
/**
  ******************************************************************************
  * File Name          : app_${moduleName?lower_case}.h
  * Description        : Header file
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_${moduleName?upper_case?replace("-","_")}_H
#define APP_${moduleName?upper_case?replace("-","_")}_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
[#if includes??]
[#list includes as include]
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
#include "${BoardName}.h"
[/#if]
[#if define?contains("BEACON")]
[#assign useBEACON = true]
[/#if]
[#if define?contains("VIRTUAL_COM_PORT")]
[#assign useVIRTUALCOMPORT = true]
[/#if]
[#if define?contains("CENTRAL_PROFILES")]
[#assign useCENTRALPROFILES = true]
[/#if]
[#if define?contains("PERIPHERAL_PROFILES")]
[#assign usePERIPHERALPROFILES = true]
[/#if]
[/#list]
[/#if]

[#if (!useSENSORDEMO) && (!useSENSORDEMO_BS) && (!useSAMPLEAPP) && (!useBEACON) && (!useVIRTUALCOMPORT)]
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported Variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */
[/#if]

/* Exported Functions --------------------------------------------------------*/
void ${fctName}(void);
void ${fctProcessName}(void);
[#if (!useSENSORDEMO) && (!useSENSORDEMO_BS) && (!useSAMPLEAPP) && (!useBEACON) && (!useVIRTUALCOMPORT)]

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */
[/#if]

#ifdef __cplusplus
}
#endif
#endif /* APP_${moduleName?upper_case?replace("-","_")}_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
