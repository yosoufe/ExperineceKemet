[#ftl]
#include "hci.h"
#include "hci_tl.h"
#include "${BoardName}.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* UART timeout values */
#define BLE_UART_SHORT_TIMEOUT      30
#define BLE_UART_LONG_TIMEOUT       300

/* HCI Packet types */
#define HCI_COMMAND_PKT             0x01
#define HCI_ACLDATA_PKT             0x02
#define HCI_SCODATA_PKT             0x03
#define HCI_EVENT_PKT               0x04
#define HCI_VENDOR_PKT              0xff

#define HCI_TYPE_OFFSET             0
#define HCI_VENDOR_CMDCODE_OFFSET   1
#define HCI_VENDOR_LEN_OFFSET       2
#define HCI_VENDOR_PARAM_OFFSET     4

#define FW_VERSION_MAJOR            1
#define FW_VERSION_MINOR            9

/* Commands */
#define VERSION                     0x01
#define EEPROM_READ                 0x02
#define EEPROM_WRITE                0x03
#define BLUENRG_RESET               0x04
#define HW_BOOTLOADER               0x05

#define MAX_RESP_SIZE               255

#define RESP_VENDOR_CODE_OFFSET     1
#define RESP_LEN_OFFSET_LSB         2
#define RESP_LEN_OFFSET_MSB         3
#define RESP_CMDCODE_OFFSET         4
#define RESP_STATUS_OFFSET          5
#define RESP_PARAM_OFFSET           6

/* Types of vendor codes */
#define ERROR                       0
/* Error codes */
#define UNKNOWN_COMMAND	            0x01
#define INVALID_PARAMETERS          0x12

#define RESPONSE                    1
/* end of vendor codes */

/* Size of Reception buffer */
#define UARTHEADERSIZE              4
#define RXBUFFERSIZE                255

/* Private macros ------------------------------------------------------------*/
#define PACK_2_BYTE_PARAMETER(ptr, param)  do{\
                *((uint8_t *)ptr) = (uint8_t)(param);   \
                *((uint8_t *)ptr+1) = (uint8_t)(param)>>8; \
                }while(0)
 
/* Private variables ---------------------------------------------------------*/
extern UART_HandleTypeDef hcom_uart[COMn];
/* Buffer used for reception */
uint8_t uart_header[UARTHEADERSIZE];
uint8_t aRxBuffer[RXBUFFERSIZE];

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void Hal_Write_Serial(const void* data1, const void* data2, int32_t n_bytes1,
                      int32_t n_bytes2);
void BlueNRG_HW_Bootloader(void);
void handle_vendor_command(uint8_t* cmd, uint8_t cmd_len);
static void User_Init(void);
static void User_Process(void);

static void Enable_SPI_IRQ(void);
static void Disable_SPI_IRQ(void);
static void set_irq_as_input(void);
static void set_irq_as_output(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */
