/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2018     SEGGER Microcontroller GmbH              *
*                                                                    *
*       www.segger.com     Support: support_emusb@segger.com         *
*                                                                    *
**********************************************************************
*                                                                    *
*       emUSB-Host * USB Host stack for embedded applications        *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product.                          *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       emUSB-Host version: V2.15-r13960                             *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File        : USBH_Int.h
Purpose     : Internals used across different layers
              of the emUSB-Host stack
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USBH_INT_H // Avoid multiple/recursive inclusion
#define USBH_INT_H

#include <string.h>
#include <stdio.h>

#include "SEGGER.h"
#include "USBH_ConfDefaults.h"
#include "USBH.h"
#include "USBH_Util.h"

#if defined(__cplusplus)
  extern "C" { // Make sure we have C-declarations in C++ programs
#endif


/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/
#define USBH_NUM_DEFAULT_EP0               3         // 3 Default EP0 for enumeration of a device when no USB address was assigned to the device.
                                                     // 3 because of different USB speeds: low, full and high
#define USBH_MAX_PACKETSIZE              512
#define USBH_DEFAULT_STATE_EP0_SIZE        8

#define DEFAULT_MAX_TRANSFER_SIZE     0x4000         // Default maximal transfer size supported by emUSB-Host drivers (limited by hardware).

#define USBH_EP_STOP_DELAY_TIME            2         // Delay time in ms necessary until a stopped endpoint list is no longer processed by the HC

#if USBH_SUPPORT_VIRTUALMEM
  #define USBH_V2P(p)                 USBH_v2p(p)
#else
  #define USBH_V2P(p)                 SEGGER_PTR2ADDR(p)    // lint D:103[a]
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
//
// Internal debug switches.
//
#ifndef USBH_MEM_DEBUG
  #define USBH_MEM_DEBUG 0
#endif
#ifndef USBH_OHCI_DEBUG_TRANSFERS
  #define USBH_OHCI_DEBUG_TRANSFERS 0
#endif

/*********************************************************************
*
*       Basic types
*
**********************************************************************
*/
typedef     unsigned char  USBH_BOOL;

#ifndef     FALSE
  #define   FALSE        ((USBH_BOOL)0)
#endif
#ifndef     TRUE
  #define   TRUE         ((USBH_BOOL)1)         //lint !e9046  N:100
#endif

/*********************************************************************
*
*       Macros
*
**********************************************************************
*/
#define USBH_ZERO_MEMORY(mem,count)     USBH_MEMSET((mem), 0, (count))
#define USBH_ZERO_STRUCT(s)             USBH_ZERO_MEMORY(&(s),sizeof(s))

// Helper macro, used to convert enum constants to string values
#define USBH_ENUM_TO_STR(e) (x==(e)) ? #e       //lint !e773 !e9024  N:102

// Allow context pointer to be converted back to structure pointer.
#define USBH_CTX2PTR(Type, Ctx)  (/*lint -e(9087) -e(9076) -e(9079) D:100[a] D:101 */((Type*)(Ctx)))

// Allow handle to be converted back to structure pointer.
#define USBH_HDL2PTR(Type, Hdl)  (/*lint -e(9087) -e(9079) D:100[b] */((Type*)(Hdl)))

// Allow any structure pointer to be converted to a context (void) pointer.
#define USBH_PTR2CTX(Ctx)        (/*lint -e(9087) -e(9076) D:100[a] D:101 */((void*)(Ctx)))

// Convert universal I/O pointer (void) to (U8*)
#define USBH_U8PTR(p)            (/*lint -e(9079) D:100[e] */((U8 *)(p)))

// Parameters of functions can't be made 'const', if pointer to function is used as callback or in API structure.
#define USBH_CALLBACK_USE   /*lint -e{818}  D:110 */
#define USBH_API_USE        /*lint -e{818}  D:110 */

/*********************************************************************
*
*       Log/Warn functions
*
**********************************************************************
*/
#if USBH_SUPPORT_LOG
  #define USBH_LOG(p) USBH_Logf p
#else
  #ifdef _lint
    //lint -emacro((717), USBH_LOG)  N:102
    #define USBH_LOG(p)  /*lint -save -e9036 N:102 */do {} while(0) /*lint -restore*/
  #else
    #define USBH_LOG(p)
  #endif
#endif

#if USBH_SUPPORT_WARN
  #define USBH_WARN(p) USBH_Warnf p
#else
  #ifdef _lint
    //lint -emacro({717}, USBH_WARN)  N:102
    #define USBH_WARN(p)  /*lint -save -e9036 N:102 */do {} while(0) /*lint -restore*/
  #else
    #define USBH_WARN(p)
  #endif
#endif

void USBH_Log          (const char * s);
void USBH_Warn         (const char * s);
void USBH_Logf         (U32 Type, const char * sFormat, ...);
void USBH_Warnf        (U32 Type, const char * sFormat, ...);
void USBH_Panic        (const char * sError);

/*********************************************************************
*
*       USBH_OS_ functions.
*
**********************************************************************
*/
void USBH_OS_DisableInterrupt(void);
void USBH_OS_EnableInterrupt (void);
void USBH_OS_Init            (void);
void USBH_OS_DeInit          (void);
//
// Wait and signal for USBH Main Task
//
void USBH_OS_WaitNetEvent    (unsigned ms);
void USBH_OS_SignalNetEvent  (void);
//
// Wait and signal for USBH ISR Task
//
U32  USBH_OS_WaitISR         (void);
void USBH_OS_SignalISREx     (U32 DevIndex);
//
// Locking
//
void USBH_OS_Lock            (unsigned Idx);
void USBH_OS_Unlock          (unsigned Idx);
//
// To avoid deadlocks the mutex hierarchy must be observed:
// While a task has a lock on a mutex, it must not try to lock another mutex with higher index.
//
#define USBH_MUTEX_TIMER     0      // Timer management
#define USBH_MUTEX_DEVICE    0      // Device management (DevList, RefCnt)
#define USBH_MUTEX_MEM       0      // Memory allocation
#define USBH_MUTEX_CDC       1      // CDC class usage
#define USBH_MUTEX_BULK      1      // BULK class usage
#define USBH_MUTEX_HID       1      // HID class usage
#define USBH_MUTEX_FT232     1      // FT232 class usage
#define USBH_MUTEX_MTP       1      // MTP class usage
#define USBH_MUTEX_PRINTER   1      // Printer class usage
#define USBH_MUTEX_RNDIS     1      // RNDIS class usage
#define USBH_MUTEX_DRIVER    2      // Driver mutex
#define USBH_MUTEX_MSD       3      // MSD class usage
#define USBH_MUTEX_NET       3      // NET driver usage

#define USBH_MUTEX_COUNT     4      // Total number of mutexes

//
// Event objects
//
#define USBH_OS_EVENT_SIGNALED 0
#define USBH_OS_EVENT_TIMEOUT  1

typedef struct     _USBH_OS_EVENT_OBJ     USBH_OS_EVENT_OBJ;
USBH_OS_EVENT_OBJ * USBH_OS_AllocEvent    (void);                       // Allocates and returns an event object.
void                USBH_OS_FreeEvent     (USBH_OS_EVENT_OBJ * pEvent); // Releases an object event.
void                USBH_OS_SetEvent      (USBH_OS_EVENT_OBJ * pEvent); // Sets the state of the specified event object to signaled.
void                USBH_OS_ResetEvent    (USBH_OS_EVENT_OBJ * pEvent); // Sets the state of the specified event object to none-signaled.
void                USBH_OS_WaitEvent     (USBH_OS_EVENT_OBJ * pEvent);
int                 USBH_OS_WaitEventTimed(USBH_OS_EVENT_OBJ * pEvent, U32 milliSeconds);

//lint -esym(9058, _USBH_OS_EVENT_OBJ)  N:100

/*********************************************************************
*
*       USB protocol related definitions
*
**********************************************************************
*/
// USB descriptor types
#define USB_DEVICE_DESCRIPTOR_TYPE                    0x01u
#define USB_CONFIGURATION_DESCRIPTOR_TYPE             0x02u
#define USB_STRING_DESCRIPTOR_TYPE                    0x03u
#define USB_INTERFACE_DESCRIPTOR_TYPE                 0x04u
#define USB_ENDPOINT_DESCRIPTOR_TYPE                  0x05u
#define USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE          0x06u
#define USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR_TYPE 0x07u
#define USB_INTERFACE_ASSOCIATION_TYPE                0x0Bu
#define USB_HID_DESCRIPTOR_TYPE                       0x21u
#define USB_HID_DESCRIPTOR_TYPE_REPORT                0x22u

// Defines for Standard Configruation pDescriptor
// bmAttributes
#define USB_CONF_BUSPWR                               0x80 // Config. attribute: Bus powered
#define USB_CONF_SELFPWR                              0x40 // Config. attribute: Self powered
#define USB_CONF_REMOTE_WAKEUP                        0x20 // Config. attribute: Remote Wakeup

// USB classes
#define USB_DEVICE_CLASS_RESERVED                     0x00u
#define USB_DEVICE_CLASS_AUDIO                        0x01u
#define USB_DEVICE_CLASS_COMMUNICATIONS               0x02u
#define USB_DEVICE_CLASS_HUMAN_INTERFACE              0x03u
#define USB_DEVICE_CLASS_MONITOR                      0x04u
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE           0x05u
#define USB_DEVICE_CLASS_IMAGE                        0x06u
#define USB_DEVICE_CLASS_PRINTER                      0x07u
#define USB_DEVICE_CLASS_STORAGE                      0x08u
#define USB_DEVICE_CLASS_HUB                          0x09u
#define USB_DEVICE_CLASS_DATA                         0x0Au
#define USB_DEVICE_CLASS_SMART_CARD                   0x0Bu
#define USB_DEVICE_CLASS_WIRELESS                     0xE0u
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC              0xFFu

// HID protocol and subclass definitions
#define HID_DEVICE_BOOT_INTERFACE_SUBCLASS            0x01u
#define HID_DEVICE_KEYBOARD_PROTOCOL                  0x01u
#define HID_DEVICE_MOUSE_PROTOCOL                     0x02u

// IMAGE protocol and subclass definitions
#define IMAGE_DEVICE_INTERFACE_SUBCLASS               0x01u
#define IMAGE_DEVICE_INTERFACE_PROTOCOL               0x01u

// WIRELESS protocol and subclass definitions
#define WIRELESS_DEVICE_INTERFACE_SUBCLASS            0x01u
#define WIRELESS_USB_INTERFACE_SUBCLASS               0x02u
#define WIRELESS_RNDIS_INTERFACE_PROTOCOL             0x03u

// CDC protocol and subclass definitions
#define USBH_CDC_SUBCLASS_ETHERNET_CONTROL_MODEL      0x06u
#define USBH_CDC_SUBCLASS_NONE                        0x00u
#define USBH_CDC_PROTOCOL_NONE                        0x00u

// USB endpoint types
#define USB_ENDPOINT_TYPE_CONTROL                     0x00u
#define USB_ENDPOINT_TYPE_ISOCHRONOUS                 0x01u
#define USB_ENDPOINT_TYPE_BULK                        0x02u
#define USB_ENDPOINT_TYPE_INTERRUPT                   0x03u

// bRequest in USB Device Request
// Standard Request Codes
#define USB_REQ_GET_STATUS                            0x00
#define USB_REQ_CLEAR_FEATURE                         0x01
#define USB_REQ_SET_FEATURE                           0x03
#define USB_REQ_SET_ADDRESS                           0x05
#define USB_REQ_GET_DESCRIPTOR                        0x06
#define USB_REQ_SET_DESCRIPTOR                        0x07
#define USB_REQ_GET_CONFIGURATION                     0x08
#define USB_REQ_SET_CONFIGURATION                     0x09
#define USB_REQ_GET_INTERFACE                         0x0A
#define USB_REQ_SET_INTERFACE                         0x0B
#define USB_REQ_SYNCH_FRAME                           0x0C
// Hub class requests
#define USB_REQ_CLEAR_TT_BUFFER                       0x08

// GetStatus Requests Recipients and STATUS Codes
#define USB_STATUS_DEVICE                             0x80 // Get Status: Device
#define USB_STATUS_INTERFACE                          0x81 // Get Status: Interface
#define USB_STATUS_ENDPOINT                           0x82 // Get Status: End Point
#define USB_STATUS_SELF_POWERED                       0x01
#define USB_STATUS_REMOTE_WAKEUP                      0x02
#define USB_STATUS_ENDPOINT_HALT                      0x01
#define USB_STATUS_LENGTH                             2 // 2 byte

// Standard Feature Selectors
#define USB_FEATURE_REMOTE_WAKEUP                     0x01
#define USB_FEATURE_STALL                             0x00
#define USB_FEATURE_TEST_MODE                         0x02

// Common descriptor indexes
#define USB_DESC_LENGTH_INDEX                         0
#define USB_DESC_TYPE_INDEX                           1


#define USBH_CONFIG_DESCRIPTOR_OFF_BMATTRIBUTES       (0x07)
#define USBH_CONFIG_DESCRIPTOR_OFF_MAXPOWER           (0x08)

#define USB_DEVICE_DESCRIPTOR_LENGTH                  18u
#define USB_DEVICE_DESCRIPTOR_EP0_FIFO_SIZE_OFS        7u

typedef struct { // Configuration descriptor
  U8  bLength;
  U8  bDescriptorType;
  U16 wTotalLength;
  U8  bNumInterfaces;
  U8  bConfigurationValue;
  U8  iConfiguration;
  U8  bmAttributes;
  U8  MaxPower;
} USB_CONFIGURATION_DESCRIPTOR;

#define USB_CONFIGURATION_DESCRIPTOR_LENGTH             9u
#define USB_CONFIGURATION_DESCRIPTOR_BMATTRIBUTES_INDEX (7)
#define USB_CONFIGURATION_DESCRIPTOR_WTOTALLENGTH_INDEX (2)
#define USB_CONFIGURATION_DESCRIPTOR_POWER_INDEX        (8)

//
// Interface descriptor
//
#define USB_INTERFACE_DESCRIPTOR_LENGTH               9
#define USB_INTERFACE_DESC_NUMBER_OFS                 2
#define USB_INTERFACE_DESC_ALTSETTING_OFS             3
#define USB_INTERFACE_DESC_NUM_EPS_OFS                4
#define USB_INTERFACE_DESC_CLASS_OFS                  5
#define USB_INTERFACE_DESC_SUBCLASS_OFS               6
#define USB_INTERFACE_DESC_PROTOCOL_OFS               7

//
// Endpoint descriptor
//
#define USB_ENDPOINT_DESCRIPTOR_LENGTH                7u
#define USB_EP_DESC_ADDRESS_OFS                       2u
#define USB_EP_DESC_ATTRIB_OFS                        3u
#define USB_EP_DESC_PACKET_SIZE_OFS                   4u
#define USB_EP_DESC_INTERVAL_OFS                      6u
#define USB_EP_DESC_ATTRIB_MASK                       0x03u
#define USB_EP_DESC_DIR_MASK                          0x80u

#define USB_LANGUAGE_ID                               (0x0409)

#define USB_SETUP_PACKET_LEN                          8
#define USB_SETUP_TYPE_INDEX                          0
#define USB_SETUP_LENGTH_INDEX_LSB                    6
#define USB_SETUP_LENGTH_INDEX_MSB                    7

/*********************************************************************
*
*       HUB status
*
**********************************************************************
*/
// pHub class descriptor
#define HDC_MAX_HUB_DESCRIPTOR_LENGTH                 71u
#define USB_HUB_DESCRIPTOR_TYPE                       0x29u

// Class specific hub  descriptor
#define HDC_DESC_PORT_NUMBER_OFS                      2
#define HDC_DESC_CHARACTERISTICS_LOW_OFS              3
#define HDC_DESC_CHARACTERISTICS_HIGH_OFS             4
#define HDC_DESC_POWER_GOOD_TIME_OFS                  5
#define HDC_DESC_MAX_CUURENT_OFS                      6
#define HDC_DESC_DEVICE_REMOVABLE_OFS                 7
#define HDC_DESC_POWER_SWITCH_MASK                    0x3
#define HDC_DESC_ALL_POWER_SWITCH_VALUE               0x0
#define HDC_DESC_SINGLE_POWER_SWITCH_VALUE            0x1
#define HDC_DESC_COMPOUND_DEVICE_MASK                 0x4
#define HDC_DESC_OVERCURRENT_MASK                     0x18
#define HDC_DESC_OVERCURRENT_GLOBAL_VAL               0x0
#define HDC_DESC_OVERCURRENT_SELECTIVE_VAL            0x08
#define HDC_DESC_NO_OVERCURRENT_MASK                  0x10
#define HDC_DESC_SUPPORT_INIDCATOR_MASK               0x80

// pHub status request length
#define HCD_GET_STATUS_LENGTH                         4
#define HDC_DESC_MIN_LENGTH                           8u

// bRequest in USB Class Request
// HDC Standard Request Codes
#define HDC_REQTYPE_GET_STATUS                        0
#define HDC_REQTYPE_CLEAR_FEATRUE                     1
// RESERVED (used in previous specifications for GET_STATE)
#define HDC_REQTYPE_GET_STATUS_OLD                    2
#define HDC_REQTYPE_SET_FEATRUE                       3
// RESERVED 4 and 5
#define HDC_REQTYPE_GET_DESCRIPTOR                    6
#define HDC_REQTYPE_SET_DESCRIPTOR                    7
#define HDC_REQTYPE_CLEAR_TT_BUFFER                   8
#define HDC_REQTYPE_RESET_TT                          9
#define HDC_REQTYPE_GET_TT_STATE                      10
#define HDC_REQTYPE_STOP_TT                           11

// pHub class hub feature selectors
// pHub change bits
#define HDC_SELECTOR_C_HUB_LOCAL_POWER                0
#define HDC_SELECTOR_C_HUB_OVER_CURRENT               1

// pHub class port feature selectors
// Port Selectors
#define HDC_SELECTOR_PORT_CONNECTION                  0
#define HDC_SELECTOR_PORT_ENABLE                      1
#define HDC_SELECTOR_PORT_SUSPEND                     2
#define HDC_SELECTOR_PORT_OVER_CURREWNT               3
#define HDC_SELECTOR_PORT_RESET                       4
#define HDC_SELECTOR_PORT_POWER                       8
#define HDC_SELECTOR_PORT_LOW_SPEED                   9

// Port change bits
#define HDC_SELECTOR_C_PORT_CONNECTION                16
#define HDC_SELECTOR_C_PORT_ENABLE                    17
#define HDC_SELECTOR_C_PORT_SUSPEND                   18
#define HDC_SELECTOR_C_PORT_OVER_CURRENT              19
#define HDC_SELECTOR_C_PORT_RESET                     20

// Port Selectors
#define HDC_SELECTOR_PORT_TEST                        21
#define HDC_SELECTOR_PORT_INDICATOR                   22

// Port status bits
#define PORT_STATUS_CONNECT                           0x00000001UL
#define PORT_STATUS_ENABLED                           0x00000002UL
#define PORT_STATUS_SUSPEND                           0x00000004UL
#define PORT_STATUS_OVER_CURRENT                      0x00000008UL
#define PORT_STATUS_RESET                             0x00000010UL
#define PORT_STATUS_POWER                             0x00000100UL
#define PORT_STATUS_LOW_SPEED                         0x00000200UL
#define PORT_STATUS_HIGH_SPEED                        0x00000400UL
#define PORT_C_ALL_MASK                               0x001F0000UL
#define PORT_C_STATUS_CONNECT                         0x00010000UL
#define PORT_C_STATUS_ENABLE                          0x00020000UL
#define PORT_C_STATUS_SUSPEND                         0x00040000UL
#define PORT_C_STATUS_OVER_CURRENT                    0x00080000UL
#define PORT_C_STATUS_RESET                           0x00100000UL

// Hub status bits
#define HUB_STATUS_LOCAL_POWER                        0x00000001UL
#define HUB_STATUS_OVER_CURRENT                       0x00000002UL
#define HUB_STATUS_C_LOCAL_POWER                      0x00010000UL
#define HUB_STATUS_C_OVER_CURRENT                     0x00020000UL

/*********************************************************************
*
*       Driver interface
*
**********************************************************************
*/
typedef void * USBH_HC_HANDLE;          // Context for the host controller driver
typedef void * USBH_HC_EP_HANDLE;       // Handle to an endpoint

typedef enum {
  USBH_HOST_RESET,                      // Do nothing on the ports, power off
  USBH_HOST_RUNNING,                    // Turn on generation of SOF
  USBH_HOST_SUSPEND                     // Stop processing of all queues, stop SOF's
} USBH_HOST_STATE;

typedef enum {
  USBH_PORT_POWER_RUNNING,
  USBH_PORT_POWER_SUSPEND
} USBH_PORT_POWER_STATE;

typedef void        USBH_ROOT_HUB_NOTIFICATION_FUNC(void * pContext, U32 Notification);
typedef USBH_STATUS USBH_HOST_INIT_FUNC            (USBH_HC_HANDLE hHostController, USBH_ROOT_HUB_NOTIFICATION_FUNC * pfUbdRootHubNotification, void * pRootHubNotificationContext);
typedef USBH_STATUS USBH_HOST_EXIT_FUNC            (USBH_HC_HANDLE hHostController);
typedef USBH_STATUS USBH_SET_HC_STATE_FUNC         (USBH_HC_HANDLE hHostController, USBH_HOST_STATE HostState);
typedef U32         USBH_GET_HC_FRAME_NUMBER_FUNC  (USBH_HC_HANDLE hHostController);
typedef USBH_HC_EP_HANDLE USBH_ADD_ENDPOINT_FUNC   (USBH_HC_HANDLE hHostController, U8 EndpointType, U8 DeviceAddress, U8 EndpointAddress, U16 MaxFifoSize, U16 IntervalTime, USBH_SPEED Speed);
typedef void        USBH_RELEASE_EP_COMPLETION_FUNC(void * pContext);
typedef void        USBH_RELEASE_ENDPOINT_FUNC     (USBH_HC_EP_HANDLE hEndPoint, USBH_RELEASE_EP_COMPLETION_FUNC * pfReleaseEpCompletion, void * pContext);
typedef USBH_STATUS USBH_ABORT_ENDPOINT_FUNC       (USBH_HC_EP_HANDLE hEndPoint);
typedef USBH_STATUS USBH_RESET_ENDPOINT_FUNC       (USBH_HC_EP_HANDLE hEndPoint);
typedef USBH_STATUS USBH_SUBMIT_REQUEST_FUNC       (USBH_HC_EP_HANDLE hEndPoint, USBH_URB * pUrb);
typedef USBH_STATUS USBH_ISO_DATA_FUNC             (USBH_HC_EP_HANDLE hEndPoint, USBH_ISO_DATA_CTRL *pIsoData);

// IO control functions
#define USBH_IOCTL_FUNC_GET_MAX_TRANSFER_SIZE      1
#define USBH_IOCTL_FUNC_CONF_MAX_XFER_BUFF_SIZE    2
#define USBH_IOCTL_FUNC_CONF_POWER_PIN_ON_LEVEL    3

//
typedef struct {
  union {
    struct {
      USBH_HC_EP_HANDLE   hEndPoint;               // used for USBH_IOCTL_FUNC_GET_MAX_TRANSFER_SIZE
      U32                 Size;                    // used for USBH_IOCTL_FUNC_GET_MAX_TRANSFER_SIZE and USBH_IOCTL_FUNC_CONF_MAX_XFER_BUFF_SIZE
    } MaxTransferSize;
    U8 SetHighIsPowerOn;
  } u;
} USBH_IOCTL_PARA;

typedef USBH_STATUS  USBH_IOCTL_FUNC            (USBH_HC_HANDLE hHostController, unsigned Func, USBH_IOCTL_PARA *pParam);
typedef unsigned int USBH_GET_PORT_COUNT_FUNC   (USBH_HC_HANDLE hHostController); // Returns the number of root hub ports. An zero value is returned on an error.
typedef U32          USBH_GET_HUB_STATUS_FUNC   (USBH_HC_HANDLE hHostController); // Returns the HUB status as defined in the USB specification 11.24.2.6
typedef U32          USBH_GET_PORT_STATUS_FUNC  (USBH_HC_HANDLE hHostController, U8  Port); // One based index of the port / return the port status as defined in the USB specification 11.24.2.7
typedef void         USBH_SET_PORT_POWER_FUNC   (USBH_HC_HANDLE hHostController, U8  Port, U8 PowerOn); // one based index of the port / 1 to turn the power on or 0 for off
typedef void         USBH_RESET_PORT_FUNC       (USBH_HC_HANDLE hHostController, U8  Port); // One based index of the port
typedef void         USBH_DISABLE_PORT_FUNC     (USBH_HC_HANDLE hHostController, U8  Port); // One based index of the port// Disable the port, no requests and SOF's are issued on this port
typedef void         USBH_SET_PORT_SUSPEND_FUNC (USBH_HC_HANDLE hHostController, U8  Port, USBH_PORT_POWER_STATE State); // One based index of the port / Switch the port power between running and suspend
typedef int          USBH_CHECK_INTERRUPT_FUNC  (USBH_HC_HANDLE hHostController);
typedef void         USBH_ISR_FUNC              (USBH_HC_HANDLE hHostController);

typedef struct {
  // Global driver functions
  USBH_HOST_INIT_FUNC           * pfHostInit;
  USBH_HOST_EXIT_FUNC           * pfHostExit;
  USBH_SET_HC_STATE_FUNC        * pfSetHcState;
  USBH_GET_HC_FRAME_NUMBER_FUNC * pfGetFrameNumber;
  // Endpoint functions
  USBH_ADD_ENDPOINT_FUNC        * pfAddEndpoint;
  USBH_RELEASE_ENDPOINT_FUNC    * pfReleaseEndpoint;
  USBH_ABORT_ENDPOINT_FUNC      * pfAbortEndpoint;
  USBH_RESET_ENDPOINT_FUNC      * pfResetEndpoint;
  USBH_SUBMIT_REQUEST_FUNC      * pfSubmitRequest;
  // Root pHub functions
  USBH_GET_PORT_COUNT_FUNC      * pfGetPortCount;
  USBH_GET_HUB_STATUS_FUNC      * pfGetHubStatus;
  USBH_GET_PORT_STATUS_FUNC     * pfGetPortStatus;
  USBH_SET_PORT_POWER_FUNC      * pfSetPortPower;
  USBH_RESET_PORT_FUNC          * pfResetPort;
  USBH_DISABLE_PORT_FUNC        * pfDisablePort;
  USBH_SET_PORT_SUSPEND_FUNC    * pfSetPortSuspend;
  USBH_CHECK_INTERRUPT_FUNC     * pfCheckIsr;
  USBH_ISR_FUNC                 * pfIsr;
  USBH_IOCTL_FUNC               * pfIoctl;
  USBH_ISO_DATA_FUNC            * pfIsoData;
} USBH_HOST_DRIVER;

/*********************************************************************
*
*       Types used by some drivers
*
**********************************************************************
*/
// URBs HcFlags allowed values
#define URB_CANCEL_PENDING_MASK                       0x01u // Pending URB must be canceled
//
// Control endpoint states
//
typedef enum {
  ES_IDLE   = 0,
  ES_SETUP,
  ES_DATA,
  ES_COPY_DATA,
  ES_PROVIDE_HANDSHAKE,
  ES_HANDSHAKE,
  ES_ERROR
} USBH_EP0_PHASE;
//
// Endpoint states (used by some drivers)
//
typedef enum {
  USBH_EP_STATE_IDLE,           // The endpoint is not linked
  USBH_EP_STATE_UNLINK,         // If the timer routine runs then the endpoint is removed and deleted
  USBH_EP_STATE_LINKED          // The endpoint is linked
} USBH_EP_STATE;

/*********************************************************************
*
*       Timer
*
**********************************************************************
*/
typedef void   USBH_TIMER_FUNC(void * pContext);                                                 // Typedef callback function which is called on a timer Timeout

typedef struct {
  USBH_DLIST           List;              // Must be first element in structure.
#if (USBH_DEBUG > 1)
  U32                  Magic;
#endif
  USBH_TIMER_FUNC    * pfHandler;
  void               * pContext;
  I32                  TimeOfExpiration;
  I8                   IsActive;
  U32                  Timeout;
} USBH_TIMER;

typedef USBH_TIMER * USBH_TIMER_HANDLE;                                                           // Handle to a OS timer object

void              USBH_InitTimer    (USBH_TIMER * pTimer, USBH_TIMER_FUNC * pfHandler, void * pContext);  // Initializes a timer object.
void              USBH_ReleaseTimer (USBH_TIMER * pTimer);                                                // Release a timer object.
void              USBH_StartTimer   (USBH_TIMER * pTimer, U32 ms);                                        // Starts a timer. The timer is restarted again if it is running.
void              USBH_CancelTimer  (USBH_TIMER * pTimer);                                                // Cancels an timer if running, the completion routine is not called.
int               USBH_IsTimerActive(const USBH_TIMER * pTimer);
USBH_TIMER_HANDLE USBH_AllocTimer   (USBH_TIMER_FUNC * pfHandler, void * pContext);
void              USBH_FreeTimer    (USBH_TIMER_HANDLE hTimer);

//lint -sem(USBH_IsTimerActive, pure)  N:100

/*********************************************************************
*
*       Forward declarations
*
**********************************************************************
*/
typedef struct _USBH_HOST_CONTROLLER  USBH_HOST_CONTROLLER;
typedef struct _USBH_HUB_PORT         USBH_HUB_PORT;
typedef struct _USBH_HUB              USBH_HUB;
typedef struct _USB_INTERFACE         USB_INTERFACE;

//lint -esym(9058, _USBH_HOST_CONTROLLER, _USBH_HUB_PORT, _USBH_HUB, _USB_INTERFACE)  N:100

/*********************************************************************
*
*       SubState helper functions
*
**********************************************************************
*/
typedef enum {
  USBH_SUBSTATE_IDLE,                                     // Idle, if an URB is completed or if no URb is submitted and an Timeout occurrs!
  USBH_SUBSTATE_TIMER,                                    // USBH_URB_SubStateWait on success
  USBH_SUBSTATE_TIMERURB,                                 // USBH_URB_SubStateSubmitRequest on success
  USBH_SUBSTATE_TIMEOUT_PENDING_URB                       // On Timeout and pending URB
} USBH_SUBSTATE_STATE;

typedef void USBH_SUBSTATE_FUNC(void * pContext);         // Callback function for a SubState.

typedef struct {
  USBH_BOOL                    TimerCancelFlag; // Timer to for detecting an Timeout
  USBH_SUBSTATE_STATE          State;
  USBH_URB                   * pUrb;
  USBH_TIMER                   Timer;
  // Additional pointer for faster accesses
  USBH_HOST_CONTROLLER       * pHostController;
  USBH_HC_EP_HANDLE          * phEP;
  USBH_SUBMIT_REQUEST_FUNC   * pfSubmitRequest;
  USBH_ABORT_ENDPOINT_FUNC   * pfAbortEndpoint;
  USB_DEVICE                 * pDevRefCnt;
  USBH_SUBSTATE_FUNC         * pfCallback; // This callback routine is called if an URB is complete or on an timer Timeout
                                           // started with USBH_URB_SubStateWait. If the timer routine runs and an pending pUrb exist
                                           // then the pUrb is aborted and the CallbackRoutine is not called.
  void                       * pContext;
} URB_SUB_STATE;

void            USBH_URB_SubStateInit         (URB_SUB_STATE * pSubState, USBH_HOST_CONTROLLER * pHostController, USBH_HC_EP_HANDLE * phEP, USBH_SUBSTATE_FUNC * pfRoutine, void * pContext);
USBH_STATUS     USBH_URB_SubStateSubmitRequest(URB_SUB_STATE * pSubState, USBH_URB * pUrb, U32 Timeout, USB_DEVICE * pDevRefCnt);
void            USBH_URB_SubStateWait         (URB_SUB_STATE * pSubState, U32 Timeout, USB_DEVICE * pDevRefCnt);
void            USBH_URB_SubStateExit         (URB_SUB_STATE * pSubState);

/*********************************************************************
*
*       HUB (root + external)
*
**********************************************************************
*/
#ifndef USBH_HUB_MAX_INT_PACKET_SIZE
  #define USBH_HUB_MAX_INT_PACKET_SIZE    4     // max. packet size of an interrupt transfer from an external HUB
#endif

//
// Port reset states. Do not change the order of the enum definitions!
// The RootHub and Hub modules depend on this order.
//
typedef enum {
  USBH_HUB_PORTRESET_IDLE,
  USBH_HUB_PORTRESET_START,
  USBH_HUB_PORTRESET_RESTART,
  USBH_HUB_PORTRESET_WAIT_RESTART,
  USBH_HUB_PORTRESET_WAIT_RESET_0,
  USBH_HUB_PORTRESET_IS_ENABLED_0,
  USBH_HUB_PORTRESET_GET_DEV_DESC,
  USBH_HUB_PORTRESET_WAIT_RESET_1,
  USBH_HUB_PORTRESET_IS_ENABLED_1,
  USBH_HUB_PORTRESET_SET_ADDRESS,
  USBH_HUB_PORTRESET_START_DEVICE_ENUM
} USBH_HUB_PORTRESET_STATE;

typedef enum { // Hub initialization state machine
  USBH_HUB_ENUM_IDLE,                  // Idle
  USBH_HUB_ENUM_START,                 // Start the state machine
//  USBH_HUB_ENUM_GET_STATUS,            // Get the device status
  USBH_HUB_ENUM_HUB_DESC               // Check the hub descriptor
} USBH_HUB_ENUM_STATE;

typedef struct {
#if (USBH_DEBUG > 1)
  U32                             Magic;
#endif
  USBH_HOST_CONTROLLER          * pHostController;      // Backward pointer to the host controller
  unsigned int                    PortCount;            // Number of ports
  USBH_HUB_PORT                 * pPortList;
  URB_SUB_STATE                   SubState;             // Sub state machine for device reset and set address,  easier handling if both an timer and URB is started!
  USBH_HUB_PORTRESET_STATE        PortResetEnumState;
  USBH_HUB_PORT                 * pEnumPort;
  USB_DEVICE                    * pEnumDevice;
  USBH_URB                        EnumUrb;              // Embedded URB
  USBH_HC_EP_HANDLE               hEnumEP;
} ROOT_HUB;

void            USBH_ROOTHUB_Init             (USBH_HOST_CONTROLLER * pHostController);
void            USBH_ROOTHUB_OnNotification   (void     * pRootHubContext, U32 Notification);
void            USBH_ROOTHUB_Release          (ROOT_HUB * pRootHub);
void            USBH_ROOTHUB_InitPorts        (ROOT_HUB * pRootHub);
void            USBH_ROOTHUB_ServicePorts     (ROOT_HUB * pRootHub);

#define USBHUB_DEFAULT_INTERFACE          0

#define USBH_PORT_DO_UPDATE_STATUS    (1u << 0)
#define USBH_PORT_DO_POWER_UP         (1u << 1)
#define USBH_PORT_DO_DELAY            (1u << 2)
#define USBH_PORT_DO_DISABLE          (1u << 3)
#define USBH_PORT_DO_RESET            (1u << 4)
#define USBH_PORT_DO_POWER_DOWN       (1u << 5)

struct _USBH_HUB_PORT {
#if (USBH_DEBUG > 1)
  U32                  Magic;
#endif
  ROOT_HUB           * pRootHub;           // Null if no root hub port
  USBH_HUB           * pExtHub;            // Null if no external hub
  U32                  PortStatus;         // A copy of the port status returned from the HUB
  I32                  DelayUntil;         // Delay next port action until this system time is reached.
  U8                   ToDo;               // Bit mask of USBH_PORT_DO_... macros
  USBH_SPEED           PortSpeed;          // The current speed of the device
  U8                   HubPortNumber;      // The one based index of the hub port
  USB_DEVICE         * pDevice;            // Device connected to this port, for tree operation
  unsigned int         RetryCounter;       // Counts the number of retries
#if USBH_SUPPORT_HUB_CLEAR_TT_BUFFER
  U16                  ClearTTQueue[4];    // Queued 'Clear TT Buffer' commands
#endif
};

typedef enum {
  USBH_HUB_ACT_IDLE,
  USBH_HUB_ACT_GET_PORT_STATUS,
  USBH_HUB_ACT_POWER_UP,
  USBH_HUB_ACT_POWER_DOWN,
  USBH_HUB_ACT_CLR_CHANGE,
  USBH_HUB_ACT_DISABLE,
  USBH_HUB_ACT_RESET,
  USBH_HUB_ACT_GET_DESC,
  USBH_HUB_ACT_SET_ADDRESS,
  USBH_HUB_ACT_CLEAR_TT
} USBH_HUB_ACTION;

struct _USBH_HUB { // USB HUB object
#if USBH_DEBUG > 1
  U32                   Magic;
#endif
  USB_DEVICE          * pHubDevice;                  // Backward pointer to the USB hub device
  unsigned int          PowerGoodTime;               // Power on to power good time in ms
  unsigned int          Characteristics;
  unsigned int          PortCount;                   // Number of ports
  USBH_HUB_PORT       * pPortList;                   // Array of ports

  // Main hub processing state machine.
  USBH_TIMER            ProcessPorts;
  URB_SUB_STATE         PortsSubState;               // sub state machine (submitting and aborting of URBs)
  USBH_URB              PortsUrb;
  USBH_HUB_PORT       * pPendingActionPort;
  U8                    CtrlRetryCounter;            // Retry counter for control transfers to the HUB
  I8                    Suspend;
  USBH_HUB_ACTION       PendingAction;

  // For enumeration of devices connected to HUBs ports
  USBH_HUB_PORT       * pEnumPort;                   // Active port for 'reset' and 'set address'
  USB_DEVICE          * pEnumDevice;                 // Active device for 'reset' and 'set address'
  USBH_HUB_PORTRESET_STATE   PortResetEnumState;     // Current state of the port reset state machine

  // For enumeration of the HUB itself
  USBH_URB              EnumUrb;                     // URBs for 'get descriptor', etc.
  USBH_HUB_ENUM_STATE   EnumState;                   // State of the Hubs initialization process
  URB_SUB_STATE         EnumSubState;                // Sub state for hub enumeration

  // Helper sub state machines
  URB_SUB_STATE         PortResetControlUrbSubState;
  USBH_HC_EP_HANDLE     PortResetEp0Handle;

  // To get hub and port notifications
  USB_ENDPOINT        * pInterruptEp;
  USBH_URB              InterruptUrb;
  U8                    InterruptTransferBuffer[USBH_HUB_MAX_INT_PACKET_SIZE];
  unsigned int          InterruptTransferBufferSize;
  USBH_STATUS           InterruptUrbStatus;
  U8                    IntRetryCounter;             // Retry counter for interrupt transfers from the HUB
  I32                   IntLastErrorTime;
  USBH_INTERFACE_ID     InterfaceId;                 // ID of the interface for port notifications
};

typedef struct {
  void      (*pfStartHub)                          (USB_DEVICE * pEnumDev);
  void      (*pfReStartHubPort)                    (USBH_HOST_CONTROLLER * pHostController);
  void      (*pfDisablePort)                       (USBH_HUB_PORT * pPort);
  void      (*pfDeleteHub)                         (USBH_HUB * pUsbHub);
  void      (*pfMarkChildDevicesAsRemoved)         (USBH_HOST_CONTROLLER * pHostController);
#if USBH_SUPPORT_HUB_CLEAR_TT_BUFFER
  void      (*pfClearTTBuffer)                     (USBH_HUB_PORT * pPort, U8 EndpointAddress, U8 DeviceAddress, U8 EPType);
#endif
  void      (*pfServiceAll)                        (USBH_HOST_CONTROLLER * pHostController);
} USBH_EXT_HUB_API;

/*********************************************************************
*
*       Endpoints
*
**********************************************************************
*/
typedef struct {
#if (USBH_DEBUG > 1)
  U32                  Magic;
#endif
  USB_DEVICE         * pUsbDevice; // Pointer to the owning host controller
  USBH_HC_EP_HANDLE    hEP;  // Endpoint handle must be used to submit an URB
  unsigned int         UrbCount;
} USBH_DEFAULT_EP;

struct _USB_ENDPOINT {
#if (USBH_DEBUG > 1)
  U32                 Magic;
#endif
  USB_ENDPOINT      * pNext;                // Points to next endpoint in interface.
  USB_INTERFACE     * pUsbInterface;        // Backward pointer
  const U8          * pEndpointDescriptor;  // Descriptor
  USBH_HC_EP_HANDLE   hEP;                  // Endpoint handle must be used to submit an URB
  I8                  ActiveUrb;            // Urb active for this endpoint
  U8                  EPAddr;               // USB address (from descriptor)
  U8                  EPType;               // Type of endpoint
  U8                  MultiPktCount;        // For high bandwidth ISO EPs
  U16                 MaxPacketSize;
  U16                 IntervalTime;         // in uFrames
};

/*********************************************************************
*
*       Interfaces
*
**********************************************************************
*/

//lint -esym(9045, struct _USB_INTERFACE)  is hidden to the user API  N:100
struct _USB_INTERFACE {
#if (USBH_DEBUG > 1)
  U32                 Magic;
#endif
  USBH_DLIST          ListEntry;                 // To store this object in the device object
  USB_DEVICE        * pDevice;                   // Backward pointer
  USB_ENDPOINT      * pEndpointList;             // List of endpoints
  U8                  ExclusiveUsed;
  U8                  CurrentAlternateSetting;
  U8                  NewAlternateSetting;
  const U8          * pInterfaceDescriptor;      // whole interface descriptor including all alternate settings.
  unsigned            InterfaceDescriptorSize;
  unsigned int        OpenCount;
  USBH_INTERFACE_ID   InterfaceId;               // ID of this interface
};

/*********************************************************************
*
*       USB devices
*
**********************************************************************
*/
// State for device enumeration
typedef enum {
  DEV_ENUM_IDLE,                 // No enumeration running
  DEV_ENUM_START,                // First state
  DEV_ENUM_GET_DEVICE_DESC,      // Get the complete device descriptor
  DEV_ENUM_GET_CONFIG_DESC_PART, // Get the first part of the configuration descriptor
  DEV_ENUM_GET_CONFIG_DESC,      // Get the complete configuration descriptor
  DEV_ENUM_GET_LANG_ID,          // Get the language ID's
  DEV_ENUM_GET_SERIAL_DESC,      // Get the serial number
  DEV_ENUM_PREP_SET_CONFIG,      // Prepare 'Set configuration'
  DEV_ENUM_SET_CONFIGURATION,    // Set the configuration
  DEV_ENUM_INIT_HUB              // The device is an hub and is  initialized
} DEV_ENUM_STATE;

typedef enum { // Do not modify the sequence
  DEV_STATE_UNKNOWN = 0,
  DEV_STATE_REMOVED,
  DEV_STATE_ENUMERATE,
  DEV_STATE_WORKING
} USB_DEV_STATE;

struct _USB_DEVICE {
#if (USBH_DEBUG > 1)
  U32                          Magic;
  U32                          UniqueID;
#endif
  USBH_DLIST                   ListEntry;               // To store this object in the device list of the host controller
  int                          RefCount;
  USBH_HOST_CONTROLLER       * pHostController;         // Pointer to the owning host controller
  USBH_DLIST                   UsbInterfaceList;        // List for interfaces
  unsigned int                 InterfaceCount;
  USBH_HUB_PORT              * pParentPort;             // This is the hub port where this device is connected to
  U8                           UsbAddress;              // This is the USB address of the device
  USBH_SPEED                   DeviceSpeed;             // pSpeed of the device connection
  U8                           MaxFifoSize;             // The FIFO size
  U8                           ConfigurationIndex;      // The index of the current configuration
  U8                           NumConfigurations;       // The index of the current configuration
  // Descriptors
  USBH_DEVICE_DESCRIPTOR       DeviceDescriptor;        // A typed copy
  U8                        ** ppConfigDesc;            // Table of all configuration descriptors.
  U16                        * paConfigSize;            // Table of sizes for all configuration descriptors.
  U8                         * pConfigDesc;             // ppConfigDesc points to pConfigDesc,
                                                        // if device has only one configuration descriptor (otherwise memory is allocated).
  U16                          ConfigSize;              // paConfigSize points to ConfigSize,
                                                        // if device has only one configuration descriptor (otherwise memory is allocated).
  U8                         * pConfigDescriptor;       // Points to the current configuration descriptor
  U16                          ConfigDescriptorSize;    // Size of the current configuration descriptor
  U16                          LanguageId;              // First language ID
  U8                         * pSerialNumber;           // Serial number without header, UNICODE
  unsigned int                 SerialNumberSize;
  USBH_DEFAULT_EP              DefaultEp;               // Embedded default endpoint
  USBH_HUB                   * pUsbHub;                 // This pointer is valid if the device is a hub
  USB_DEVICE                 * pHubDevice;              // If connected to a HUB: Points to the HUB device.
  USB_DEV_STATE                State;                   // Current device state
  U8                         * pCtrlTransferBuffer;     // Used from reset and enumeration state machines.
  unsigned int                 CtrlTransferBufferSize;
  // State variables for device enumeration
  DEV_ENUM_STATE               EnumState;               // Enumeration state
  URB_SUB_STATE                SubState;
  USBH_URB                     EnumUrb;                 // Embedded URB
  USBH_DEVICE_ID               DeviceId;                // Device ID for this device
};

/*********************************************************************
*
*       HostController
*
**********************************************************************
*/
typedef enum {
  HC_UNKNOWN,
  HC_REMOVED,
  HC_WORKING,
  HC_SUSPEND
} HOST_CONTROLLER_STATE;

struct _USBH_HOST_CONTROLLER {                        // Global driver object
  USBH_DLIST                    ListEntry;            // List entry for USBH_HOST_CONTROLLER
  int                           RefCount;             // Ref pCount
  HOST_CONTROLLER_STATE         State;                // The state of the HC
  USBH_DLIST                    DeviceList;           // List of USB devices
  U32                           DeviceListLckCnt;     // Lock count for loop through all devices.
  const USBH_HOST_DRIVER      * pDriver;              // Host controller entry
  void                        * pPrvData;             // Pointer to private driver data structure. It is passed to each function.
  U32                           UsbAddressUsed[4];    // Bit field to mark usage of up to 128 addresses.
  U8                            MaxAddress;
  U8                            Index;
  U8                            NextFreeAddress;
  U8                            ActivePortReset;      // Bit 0: Port reset active, Bits 1-7: Enumerations active.
#if USBH_DELAY_BETWEEN_ENUMERATIONS > 0u
  U32                           LastActiveEnum;       // Time stamp of end of last enumeration process.
#endif
  ROOT_HUB                      RootHub;              // Embedded root hub
  USBH_HC_EP_HANDLE             LowSpeedEndpoint;
  USBH_HC_EP_HANDLE             FullSpeedEndpoint;
  USBH_HC_EP_HANDLE             HighSpeedEndpoint;
#if (USBH_DEBUG > 1)
  U32                           Magic;
#endif
  // PortResetActive points to a port where the port reset state machine is started or is active. At the end of the
  // set address state of a port reset or if the device where the port is located is removed this pointer is set to NULL!
  USBH_HUB_PORT               * pActivePortReset;
  USBH_TIMER                    PortServiceTimer;
};

/*********************************************************************
*
*       USBH_GLOBAL
*
**********************************************************************
*/
typedef struct {
  USBH_DLIST                      HostControllerList;
  // Registered PNP notifications
  USBH_DLIST                      NotificationList;
  // Delayed Pnp notifications, called in an timer routine
  USBH_DLIST                      DelayedPnPNotificationList;
  USBH_TIMER                      DelayedPnPNotifyTimer;
  USBH_DLIST                      EnumErrorNotificationList;
  USBH_DLIST                      DeviceRemovalNotificationList;
  // Next free ID's for a new enumerated device
  USBH_INTERFACE_ID               NextInterfaceId;
  USBH_DEVICE_ID                  NextDeviceId;
//  USBH_ON_SETCONFIGURATION_FUNC * pfOnSetConfiguration;
  USBH_SET_CONF_HOOK            * pFirstOnSetConfHook;
  USBH_ON_SETPORTPOWER_FUNC     * pfOnSetPortPower;
//  void                          * pOnSetConfigContext;
  USBH_EXT_HUB_API              * pExtHubApi;
  struct {
    U8  RootHubPortsAlwaysPowered;
    U8  RootHubPerPortPowered;
    U8  RootHubSupportOvercurrent;
    U32 DefaultPowerGoodTime;
  } Config;
  volatile I8 IsRunning;
  volatile I8 TimerTaskIsRunning;
  volatile I8 ISRTaskIsRunning;
  U8          ConfigCompleted;
  U8          HostControllerCount;
#if USBH_SUPPORT_VIRTUALMEM
  USBH_V2P_FUNC           * pfV2P;
#endif
#if (USBH_DEBUG > 1)
  U32 DevUniqueID;
#endif
#if (USBH_SUPPORT_TRACE != 0)
  struct {
    U32                   APIOffset;
    const USBH_TRACE_API* pAPI;
  } Trace;
#endif
  const char * sCopyright;
} USBH_GLOBAL;

extern USBH_GLOBAL USBH_Global;
#ifdef USBHCORE_C
       USBH_GLOBAL USBH_Global;
#endif

extern SEGGER_CACHE_CONFIG USBH_CacheConfig;

/*********************************************************************
*
*       Notification types
*
**********************************************************************
*/
typedef void USBH_ON_DRIVER_DEVICE_REMOVED_FUNC(void * pContext);

typedef struct {
  USBH_ON_DRIVER_DEVICE_REMOVED_FUNC  * pDevRemNotification; // The notification function, registered by USBH_RegisterDeviceRemovalNotification
} USBH_DEV_REM_NOTIFICATION;

//lint -esym(9045, struct _ENUM_ERROR_NOTIFICATION)  is hidden to the user API  N:100
typedef struct _ENUM_ERROR_NOTIFICATION {
#if (USBH_DEBUG > 1)
  U32                       Magic;
#endif
  USBH_DLIST                ListEntry;         // To build a list of all error notification callbacks.
  void                    * pContext;          // User context / A copy of the parameter passed to pfOnEnumError()
  USBH_ON_ENUM_ERROR_FUNC * pfOnEnumError;
} ENUM_ERROR_NOTIFICATION;

//lint -esym(9045, struct _USBH_NOTIFICATION)  is hidden to the user API  N:100
typedef struct _USBH_NOTIFICATION {
#if (USBH_DEBUG > 1)
  U32                   Magic;
#endif
  USBH_DLIST            ListEntry;
  union {
    USBH_PNP_NOTIFICATION       PNP;      // The notification passed to USBH_RegisterPnPNotification
    USBH_DEV_REM_NOTIFICATION   DevRem;   // The notification passed to USBH_RegisterDeviceRemovalNotification
  } Notification;
} USBH_NOTIFICATION;

// Used for indirect calling of the user notification routine
typedef struct {
#if (USBH_DEBUG > 1)
  U32 Magic;
#endif
  // To store this object in the BUS_DRIVER object
  USBH_DLIST               ListEntry;
  void                   * pContext;
  USBH_PNP_EVENT           Event;
  USBH_ON_PNP_EVENT_FUNC * pfNotifyCallback;
  USBH_INTERFACE_ID        Id;
} DELAYED_PNP_NOTIFY_CONTEXT;

/*********************************************************************
*
*       Macros for list handling, magic and asserts
*
**********************************************************************
*/
//
// Calculate the pointer to the base of the structure given its type and a pointer to a field within the structure.
//
//lint --emacro((413,923,946,947,9033,9078),STRUCT_BASE_POINTER)  D:103[d] D:106
#define STRUCT_BASE_POINTER(fieldptr, type, field)            ((type *)(((char *)(fieldptr)) - ((char *)(&(((type *)0)->field)))))

// Needs the struct and the name of the list entry inside the struct
#define GET_ENUM_ERROR_NOTIFICATION_FROM_ENTRY(pListEntry)    STRUCT_BASE_POINTER(pListEntry, ENUM_ERROR_NOTIFICATION,    ListEntry)
#define GET_NOTIFICATION_FROM_ENTRY(pListEntry)               STRUCT_BASE_POINTER(pListEntry, USBH_NOTIFICATION,          ListEntry)
#define GET_DELAYED_PNP_NOTIFY_CONTEXT_FROM_ENTRY(pListEntry) STRUCT_BASE_POINTER(pListEntry, DELAYED_PNP_NOTIFY_CONTEXT, ListEntry)
#define GET_USB_DEVICE_FROM_ENTRY(pListEntry)                 STRUCT_BASE_POINTER(pListEntry, USB_DEVICE,                 ListEntry)
#define GET_HOST_CONTROLLER_FROM_ENTRY(pListEntry)            STRUCT_BASE_POINTER(pListEntry, USBH_HOST_CONTROLLER,       ListEntry)
#define GET_USB_INTERFACE_FROM_ENTRY(pListEntry)              STRUCT_BASE_POINTER(pListEntry, USB_INTERFACE,              ListEntry)
#define GET_TIMER_FROM_ENTRY(pListEntry)                      STRUCT_BASE_POINTER(pListEntry, USBH_TIMER,                 List)

#define FOUR_CHAR_ULONG(c1,c2,c3,c4)    (((U32)(c1)) | (((U32)(c2)) << 8) | (((U32)(c3)) << 16) | (((U32)(c4)) << 24)) // Generates a magic ulong (four char code)

#if (USBH_DEBUG > 1)
  #define USBH_ASSERT(condition)           if (!(condition)) { USBH_WARN((USBH_MTYPE_CORE, "\nASSERTION FAILED: %s @%s(%d)\n", #condition, __FILE__, __LINE__)); USBH_PANIC("Assert"); }
  #define USBH_ASSERT_PTR(Ptr)             USBH_ASSERT(Ptr != NULL)
  #define USBH_ASSERT_MAGIC(p,type)        USBH_ASSERT(USBH_IS_PTR_VALID((p),type))
  #define USBH_ASSERT0                     { USBH_WARN((USBH_MTYPE_CORE, "\nASSERT0: %s(%d)\n", __FILE__, __LINE__)); USBH_PANIC("Assert"); }
  #define USBH_IS_PTR_VALID(p,type)        ((p)!=NULL && (p)->Magic==type##_MAGIC) // Takes a pointer and its type and compares the Magic field with a constant
#else
  #define USBH_ASSERT(condition)
  #define USBH_ASSERT_PTR(Ptr)
  #define USBH_ASSERT_MAGIC(p, type)
  #define USBH_ASSERT0
  #define USBH_IS_PTR_VALID(p,type)
#endif
#define USBH_IS_ALIGNED(val,size) (((val) & ((size)-1u)) == 0u)         // Returns true if the given value is aligned to a 'size' boundary

#if (USBH_DEBUG > 1)                                                  // Handy macro to enable code in debug builds only
  #define USBH_IFDBG(x) { x; }
#else
  #define USBH_IFDBG(x)
#endif

#define ENUM_ERROR_NOTIFICATION_MAGIC           FOUR_CHAR_ULONG('E','N','O','T')
#define USBH_PNP_NOTIFICATION_MAGIC             FOUR_CHAR_ULONG('P','N','P','N')
#define USBH_DEV_REM_NOTIFICATION_MAGIC         FOUR_CHAR_ULONG('D','R','E','M')
#define DELAYED_PNP_NOTIFY_CONTEXT_MAGIC        FOUR_CHAR_ULONG('P','N','P','D')
#define INTERFACE_LIST_MAGIC                    FOUR_CHAR_ULONG('I','F','A','E')
#define USBH_DEFAULT_EP_MAGIC                   FOUR_CHAR_ULONG('E','P','0',' ')
#define USB_ENDPOINT_MAGIC                      FOUR_CHAR_ULONG('E','N','D','P')
#define USB_INTERFACE_MAGIC                     FOUR_CHAR_ULONG('I','N','T','F')
#define USB_DEVICE_MAGIC                        FOUR_CHAR_ULONG('U','D','E','V')
#define ROOT_HUB_MAGIC                          FOUR_CHAR_ULONG('R','H','U','B')
#define USBH_HUB_MAGIC                          FOUR_CHAR_ULONG('U','H','U','B')
#define USBH_HUB_PORT_MAGIC                     FOUR_CHAR_ULONG('P','O','R','T')
#define USBH_HOST_CONTROLLER_MAGIC              FOUR_CHAR_ULONG('H','O','S','T')
#define USBH_TIMER_MAGIC                        FOUR_CHAR_ULONG('T','I','M','R')
#define USBH_MSD_UNIT_MAGIC                     FOUR_CHAR_ULONG('U','N','I','T')
#define USBH_MSD_INST_MAGIC                     FOUR_CHAR_ULONG('M','D','E','V')
#define USBH_CCID_INST_MAGIC                    FOUR_CHAR_ULONG('C','C','I','D')

/*********************************************************************
*
*       HostController functions.
*
**********************************************************************
*/
USBH_HOST_CONTROLLER * USBH_AddHostController       (const USBH_HOST_DRIVER * pDriver, void *pPrvData, U8 MaxUsbAddress, U32 *pIndex);
void                   USBH_RemoveHostController    (USBH_HOST_CONTROLLER * pHostController);
void                   USBH_EnumerateDevices        (USBH_HOST_CONTROLLER *pHostController);
USBH_HOST_CONTROLLER * USBH_CreateHostController    (const USBH_HOST_DRIVER     * pDriver, void *pPrvData);
void                   USBH_DeleteHostController    (USBH_HOST_CONTROLLER * pHostController);
USBH_HOST_CONTROLLER * USBH_HCIndex2Inst            (U32 HostControllerIndex);
unsigned               USBH_ClaimActivePortReset    (USBH_HOST_CONTROLLER * pHost);
void                   USBH_ReleaseActivePortReset  (USBH_HOST_CONTROLLER * pHost);
void                   USBH_ClaimActiveEnumeration  (USBH_HOST_CONTROLLER * pHost);
void                   USBH_ReleaseActiveEnumeration(USBH_HOST_CONTROLLER * pHost);
void                   USBH_BD_FreeUsbAddress       (USBH_HOST_CONTROLLER * pHostController, U8 Address);
void                   USBH_HC_ServicePorts         (USBH_HOST_CONTROLLER * pHostController);
U8                     USBH_BD_GetUsbAddress        (USBH_HOST_CONTROLLER * pHostController);
//
// Reference counting macros to the USBH_HOST_CONTROLLER object
//
#if (USBH_DEBUG > 1)
  void USBH_HcIncRef(USBH_HOST_CONTROLLER *pHostController, const char *sFile, unsigned Line);
  void USBH_HcDecRef(USBH_HOST_CONTROLLER *pHostController, const char *sFile, unsigned Line);
  #define USBH_HC_INC_REF(pHostController)   USBH_HcIncRef(pHostController, __FILE__, __LINE__)
  #define USBH_HC_DEC_REF(pHostController)   USBH_HcDecRef(pHostController, __FILE__, __LINE__)
#else
  void USBH_HcIncRef(USBH_HOST_CONTROLLER *pHostController);
  void USBH_HcDecRef(USBH_HOST_CONTROLLER *pHostController);
  #define USBH_HC_INC_REF                    USBH_HcIncRef
  #define USBH_HC_DEC_REF                    USBH_HcDecRef
#endif

/*********************************************************************
*
*       Device functions.
*
**********************************************************************
*/
USB_DEVICE * USBH_CreateNewUsbDevice                (USBH_HOST_CONTROLLER * pHostController);
void         USBH_StartEnumeration                  (USB_DEVICE * pDev);
void         USBH_DeleteDevice                      (USB_DEVICE * pDev);
void         USBH_DeleteInterfaces                  (const USB_DEVICE * pDev);
void         USBH_MarkDeviceAsRemoved               (USB_DEVICE * pDev);
void         USBH_MarkParentAndChildDevicesAsRemoved(USB_DEVICE * pUsbDevice);
int          USBH_CheckCtrlTransferBuffer           (USB_DEVICE * pDev, unsigned    RequestLength);
void         USBH_ProcessEnumError                  (USB_DEVICE * pDev, USBH_STATUS Status,     USBH_BOOL bRetry);
void         USBH_LockDeviceList                    (USBH_HOST_CONTROLLER *pHost);
void         USBH_UnlockDeviceList                  (USBH_HOST_CONTROLLER *pHost);
void         USBH_CleanupDeviceList                 (void);
void         USBH_AddUsbDevice                      (USB_DEVICE      * pDevice);
//
// Reference counting macros to the USB_DEVICE object
//
#if (USBH_DEBUG > 1)
  USBH_STATUS USBH_IncRef         (USB_DEVICE * pDevice, const char *pFile, int Line);
  void        USBH_DecRef         (USB_DEVICE * pDevice, const char *pFile, int Line);
  #define USBH_INC_REF(pDevice)   USBH_IncRef(pDevice, __FILE__, __LINE__)
  #define USBH_DEC_REF(pDevice)   USBH_DecRef(pDevice, __FILE__, __LINE__)
#else
  USBH_STATUS USBH_IncRef         (USB_DEVICE * pDevice);
  void        USBH_DecRef         (USB_DEVICE * pDevice);
  #define USBH_INC_REF            USBH_IncRef
  #define USBH_DEC_REF            USBH_DecRef
#endif

/*********************************************************************
*
*       Interface functions.
*
**********************************************************************
*/
USBH_STATUS     USBH_CreateInterfaces                  (USB_DEVICE * pDev);
USBH_STATUS     USBH_SearchUsbInterface                (const USB_DEVICE * pDev, const USBH_INTERFACE_MASK * pInterfaceMask, USB_INTERFACE * * ppUsbInterface);
USBH_STATUS     USBH_EpSubmitUrb                       (USB_ENDPOINT  * pUsbEndpoint, USBH_URB * pUrb);
USB_INTERFACE * USBH_BD_NewUsbInterface                (USB_DEVICE    * pDevice);
USBH_STATUS     USBH_BD_CompareUsbInterface            (const USB_INTERFACE * pInterface, const USBH_INTERFACE_MASK * pInterfaceMask, USBH_BOOL EnableHubInterfaces);
USB_ENDPOINT  * USBH_BD_SearchUsbEndpointInInterface   (const USB_INTERFACE * pInterface, const USBH_EP_MASK * pMask);
USBH_STATUS     USBH_ResetEndpoint                     (USBH_INTERFACE_HANDLE hIface, USBH_URB * pUrb, U8 Endpoint, USBH_ON_COMPLETION_FUNC * pfCompletion, void * pContext);
void            USBH_DefaultReleaseEpCompletion        (void * pContext);
void            USBH_FindAltInterfaceDesc              (const USB_INTERFACE * pInterface, unsigned AlternateSetting, const U8 ** ppDesc, unsigned * pDescLen);
const U8      * USBH_FindNextEndpointDesc              (const U8 ** ppDesc, unsigned * pDescLen);
USBH_STATUS     USBH_GetDescriptorPtr                  (USBH_INTERFACE_HANDLE hInterface, U8 AlternateSetting, U8 Type, const U8 ** ppDesc);
USBH_STATUS     USBH_GetCurrentConfDescriptorPtr       (USBH_INTERFACE_HANDLE hInterface, const U8 ** pDesc, unsigned * pDescLen);
USBH_STATUS     USBH_GetInterfaceDescriptorPtr         (USBH_INTERFACE_HANDLE hInterface, U8 AlternateSetting, const U8 ** ppDesc, unsigned * pDescLen);

/*********************************************************************
*
*       Notification functions.
*
**********************************************************************
*/
USBH_NOTIFICATION_HANDLE USBH_RegisterDeviceRemovalNotification  (const USBH_DEV_REM_NOTIFICATION  * pDevRemNotification);
void                     USBH_UnegisterDeviceRemovalNotification (USBH_NOTIFICATION_HANDLE     Handle);
void                     USBH_PNP_ProcessDeviceNotifications     (USBH_NOTIFICATION * pPnpNotification, const USB_DEVICE * pDev, USBH_PNP_EVENT Event);
void                     USBH_PNP_ProcessNotification            (USBH_NOTIFICATION * pPnpNotification);
void                     USBH_PNP_NotifyWrapperCallbackRoutine   (void * pContext);
void                     USBH_ProcessDevicePnpNotifications      (const USB_DEVICE * pDevice, USBH_PNP_EVENT Event);
void                     USBH_SetEnumErrorNotification           (unsigned Flags, USBH_STATUS Status, int ExtInfo, unsigned PortNumber);
void                     USBH_UnregisterAllEnumErrorNotifications(void);

/*********************************************************************
*
*       Memory management functions.
*
**********************************************************************
*/
#if USBH_MEM_DEBUG
void             USBH_Free                           (void * pMemBlock, const char * sFunc, const char * sFile, int Line);
void           * USBH_Malloc                         (U32    Size,      const char * sFunc, const char * sFile, int Line);
void           * USBH_MallocZeroed                   (U32    Size,      const char * sFunc, const char * sFile, int Line);
void           * USBH_TryMalloc                      (U32    Size,      const char * sFunc, const char * sFile, int Line);
void           * USBH_TryMallocZeroed                (U32    Size,      const char * sFunc, const char * sFile, int Line);
void           * USBH_AllocTransferMemory            (U32    NumBytes, unsigned Alignment, const char * sFunc, const char * sFile, int Line);
void           * USBH_TryAllocTransferMemory         (U32    NumBytes, unsigned Alignment, const char * sFunc, const char * sFile, int Line);
#else
void             USBH_Free                           (void * pMemBlock);
void           * USBH_Malloc                         (U32    Size);
void           * USBH_MallocZeroed                   (U32    Size);
void           * USBH_TryMalloc                      (U32    Size);
void           * USBH_TryMallocZeroed                (U32    Size);
void           * USBH_AllocTransferMemory            (U32    NumBytes, unsigned Alignment);
void           * USBH_TryAllocTransferMemory         (U32    NumBytes, unsigned Alignment);
#endif
void             USBH_MEM_ReoFree                    (int    Idx);
void             USBH_MEM_ScheduleReo                (void);
USBH_STATUS      USBH_HCM_AllocContiguousMemory      (U32 NumBytes, U32 Alignment, void ** ppVirtAddr, U32 * pPhyAddr);

//lint --emacro((9087,9079),USBH_MALLOC)             D:100[d]
//lint --emacro((9087,9079),USBH_MALLOC_ZEROED)      D:100[d]
//lint --emacro((9087,9079),USBH_TRY_MALLOC)         D:100[d]
//lint --emacro((9087,9079),USBH_TRY_MALLOC_ZEROED)  D:100[d]
//lint --emacro((9087,9079),USBH_MALLOC_XFERMEM)     D:100[d]
//lint --emacro((9087,9079),USBH_TRY_MALLOC_XFERMEM) D:100[d]
#if USBH_MEM_DEBUG
  #define USBH_FREE(pMemBlock)                 USBH_Free(pMemBlock, __func__, __FILE__, __LINE__)
  #define USBH_MALLOC(Size)                    USBH_Malloc(Size, __func__, __FILE__, __LINE__)
  #define USBH_MALLOC_ZEROED(Size)             USBH_MallocZeroed(Size, __func__, __FILE__, __LINE__)
  #define USBH_TRY_MALLOC(Size)                USBH_TryMalloc(Size, __func__, __FILE__, __LINE__)
  #define USBH_TRY_MALLOC_ZEROED(Size)         USBH_TryMallocZeroed(Size, __func__, __FILE__, __LINE__)
  #define USBH_MALLOC_XFERMEM(Size, Align)     USBH_AllocTransferMemory(Size, Align, __func__, __FILE__, __LINE__)
  #define USBH_TRY_MALLOC_XFERMEM(Size, Align) USBH_TryAllocTransferMemory(Size, Align, __func__, __FILE__, __LINE__)
#else
  #define USBH_FREE(pMemBlock)                 USBH_Free(pMemBlock)
  #define USBH_MALLOC(Size)                    USBH_Malloc(Size)
  #define USBH_MALLOC_ZEROED(Size)             USBH_MallocZeroed(Size)
  #define USBH_TRY_MALLOC(Size)                USBH_TryMalloc(Size)
  #define USBH_TRY_MALLOC_ZEROED(Size)         USBH_TryMallocZeroed(Size)
  #define USBH_MALLOC_XFERMEM(Size, Align)     USBH_AllocTransferMemory(Size, Align)
  #define USBH_TRY_MALLOC_XFERMEM(Size, Align) USBH_TryAllocTransferMemory(Size, Align)
#endif

/*********************************************************************
*
*       Helper functions.
*
**********************************************************************
*/
void     USBH__ConvSetupPacketToBuffer(const USBH_SETUP_PACKET * pSetup, U8 * pBuffer);
void     USBH_EnumPrepareGetDescReq   (USBH_URB * pUrb, U8 DescType, U8 DescIndex, U16 LanguageID, U16 RequestLength, void * pBuffer);

int      USBH_WaitEventTimed(USBH_OS_EVENT_OBJ * pEvent, U32 Timeout);

USBH_STATUS   USBH__AddNotification(USBH_NOTIFICATION_HOOK * pHook, USBH_NOTIFICATION_FUNC * pfNotification, void * pContext, USBH_NOTIFICATION_HOOK ** pFirst, USBH_NOTIFICATION_HANDLE Handle);
USBH_STATUS   USBH__RemoveNotification(const USBH_NOTIFICATION_HOOK * pHook, USBH_NOTIFICATION_HOOK ** pFirst);

USBH_HUB_PORT * USBH_HUB_GetHighSpeedHub(USBH_HUB_PORT * pHubPort);

const char * USBH_PortSpeed2Str         (USBH_SPEED    x);
const char * USBH_EPType2Str            (U8 x);
const char * USBH_UrbFunction2Str       (USBH_FUNCTION x);
const char * USBH_HubPortResetState2Str (USBH_HUB_PORTRESET_STATE x);
const char * USBH_HubEnumState2Str      (USBH_HUB_ENUM_STATE      x);
const char * USBH_HubAction2Str         (USBH_HUB_ACTION          x);
const char * USBH_PortStatus2Str        (U32 x);
const char * USBH_PortToDo2Str          (U8 x);
const char * USBH_EnumState2Str         (DEV_ENUM_STATE        x);
const char * USBH_HcState2Str           (HOST_CONTROLLER_STATE x);
const char * USBH_Ep0State2Str          (USBH_EP0_PHASE x);

/*********************************************************************
*
*       Trace API.
*
**********************************************************************
*/
enum {
  USBH_TRACE_ID_USBH_SUBMITURB = 0,
  // Make sure this is always the last entry.
  //
  USBH_TRACE_NUM_API_FUNCTIONS
};
#define USBH_TRACE_RESSOURCE_ID_OFFSET 0x55534248

#define USBH_TRACE_RESSOURCE_NAME(e) (U32)(e + USBH_TRACE_RESSOURCE_ID_OFFSET), #e      //lint !e9024  N:102
#define USBH_TRACE_API_DESCRIPTION  "M=emUSB-Host, T=USBH, 0 USBH_SubmitUrb InterfaceId=%d Function=%I"

#if USBH_SUPPORT_TRACE
  #define USBH_TRACE_RECORD_API_VOID(Id)                               if (USBH_Global.Trace.pAPI) {USBH_Global.Trace.pAPI->pfRecordVoid ((Id) + USBH_Global.Trace.APIOffset); }
  #define USBH_TRACE_RECORD_API_U32(Id, Para0)                         if (USBH_Global.Trace.pAPI) {USBH_Global.Trace.pAPI->pfRecordU32  ((Id) + USBH_Global.Trace.APIOffset, (U32)(Para0)); }
  #define USBH_TRACE_RECORD_API_U32x2(Id, Para0, Para1)                if (USBH_Global.Trace.pAPI) {USBH_Global.Trace.pAPI->pfRecordU32x2((Id) + USBH_Global.Trace.APIOffset, (U32)(Para0), (U32)(Para1)); }
  #define USBH_TRACE_RECORD_API_U32x3(Id, Para0, Para1, Para2)         if (USBH_Global.Trace.pAPI) {USBH_Global.Trace.pAPI->pfRecordU32x3((Id) + USBH_Global.Trace.APIOffset, (U32)(Para0), (U32)(Para1), (U32)(Para2)); }
  #define USBH_TRACE_RECORD_API_U32x4(Id, Para0, Para1, Para2, Para3)  if (USBH_Global.Trace.pAPI) {USBH_Global.Trace.pAPI->pfRecordU32x4((Id) + USBH_Global.Trace.APIOffset, (U32)(Para0), (U32)(Para1), (U32)(Para2), (U32)(Para3)); }
#else
  #define USBH_TRACE_RECORD_API_VOID(Id)
  #define USBH_TRACE_RECORD_API_U32(Id, Para0)
  #define USBH_TRACE_RECORD_API_U32x2(Id, Para0, Para1)
  #define USBH_TRACE_RECORD_API_U32x3(Id, Para0, Para1, Para2)
  #define USBH_TRACE_RECORD_API_U32x4(Id, Para0, Para1, Para2, Para3)
#endif

#if defined(__cplusplus)
  }
#endif

#endif

/*************************** End of file ****************************/
