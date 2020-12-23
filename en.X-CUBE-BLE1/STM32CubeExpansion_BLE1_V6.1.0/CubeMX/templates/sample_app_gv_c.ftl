[#ftl]
#include "hci_tl.h"
#include "sample_service.h"
#include "role_type.h"
#include "bluenrg_utils.h"
#include "bluenrg_gatt_server.h"
#include "bluenrg_gap_aci.h"
#include "bluenrg_gatt_aci.h"
#include "bluenrg_hal_aci.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/**
 * Define the role here only if it is not already defined in the project options
 * For the CLIENT_ROLE comment the line below 
 * For the SERVER_ROLE uncomment the line below
 */
#define SERVER_ROLE

#define BDADDR_SIZE 6
 
/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
uint8_t bnrg_expansion_board = IDB04A1; /* at startup, suppose the X-NUCLEO-IDB04A1 is used */  
static volatile uint8_t user_button_init_state = 1;
static volatile uint8_t user_button_pressed = 0;

#ifdef SERVER_ROLE
  BLE_RoleTypeDef BLE_Role = SERVER;
#else
  BLE_RoleTypeDef BLE_Role = CLIENT;
#endif

extern volatile uint8_t set_connectable;
extern volatile int     connected;
extern volatile uint8_t notification_enabled;

extern volatile uint8_t end_read_tx_char_handle;
extern volatile uint8_t end_read_rx_char_handle;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void User_Process(void);
static void User_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */
