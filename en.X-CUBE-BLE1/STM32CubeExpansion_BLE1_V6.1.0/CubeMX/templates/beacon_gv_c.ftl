[#ftl]
#include <string.h>
#include "hci_le.h"
#include "hci_tl.h"
#include "sensor_service.h"
#include "eddystone_beacon.h"
#include "bluenrg_utils.h"
#include "${BoardName}.h"
#include "bluenrg_gap_aci.h"
#include "bluenrg_gatt_aci.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/**
 * Define the beacon type here only if it is not already defined in the project 
 * options.
 * For the URL beacon type uncomment the line below.
 * Otherwise, if it is not already defined in the project options, the UID 
 * beacon type is set.
 */
//#define EDDYSTONE_BEACON_TYPE EDDYSTONE_URL_BEACON_TYPE

#ifndef EDDYSTONE_BEACON_TYPE
  #define EDDYSTONE_BEACON_TYPE EDDYSTONE_UID_BEACON_TYPE
#endif

#define IDB04A1 0
#define IDB05A1 1

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void EnterStopMode(void);
static void User_Init(void);
static void User_Process(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */
