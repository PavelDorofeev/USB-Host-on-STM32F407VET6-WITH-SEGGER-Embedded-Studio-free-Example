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
File        : USBH.h
Purpose     : API of the USB host stack
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USBH_H_
#define USBH_H_

#include "SEGGER.h"
#include "USBH_ConfDefaults.h"

#if defined(__cplusplus)
  extern "C" {                 // Make sure we have C-declarations in C++ programs
#endif


/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define USBH_VERSION   21500 // Format: Mmmrr. Example: 21400 is 2.14

/*********************************************************************
*
*       Status Codes
*
*  Description
*    Status codes returned by most of the API functions.
*/
typedef enum {
  USBH_STATUS_SUCCESS                          = 0x00,     // Operation successfully completed.
  USBH_STATUS_CRC                              = 0x01,     // Data packet received from device contained a CRC error.
  USBH_STATUS_BITSTUFFING                      = 0x02,     // Data packet received from device contained a bit stuffing violation.
  USBH_STATUS_DATATOGGLE                       = 0x03,     // Data packet received from device had data toggle PID that did
                                                           // not match the expected value.
  USBH_STATUS_STALL                            = 0x04,     // Endpoint was stalled by the device.
  USBH_STATUS_NOTRESPONDING                    = 0x05,     // USB device did not respond to the request
                                                           // (did not respond to IN token or did not provide a handshake to the OUT token.
  USBH_STATUS_PID_CHECK                        = 0x06,     // Check bits on PID from endpoint failed on data PID (IN)
                                                           // or handshake (OUT).
  USBH_STATUS_UNEXPECTED_PID                   = 0x07,     // Receive PID was not valid when encountered or PID
                                                           // value is not defined.
  USBH_STATUS_DATA_OVERRUN                     = 0x08,     // The amount of data returned by the device exceeded
                                                           // either the size of the maximum data packet allowed from
                                                           // the endpoint or the remaining buffer size (Babble error).
  USBH_STATUS_DATA_UNDERRUN                    = 0x09,     // The endpoint returned less than maximum packet size
                                                           // and that amount was not sufficient to fill the specified buffer.
  USBH_STATUS_XFER_SIZE                        = 0x0A,     // Size exceeded the maximum transfer size supported by the driver.
  USBH_STATUS_DMA_ERROR                        = 0x0B,     // Direct memory access error.
  USBH_STATUS_BUFFER_OVERRUN                   = 0x0C,     // During an IN transfer, the host controller received data from
                                                           // the device faster than it could be written to system memory.
  USBH_STATUS_BUFFER_UNDERRUN                  = 0x0D,     // During an OUT transfer, the host controller could not retrieve data from system
                                                           // memory fast enough to keep up with data USB data rate.
  USBH_STATUS_OHCI_NOT_ACCESSED                = 0x0F,     // Exclusive to OHCI. This code is set before the TD is placed on a list to be processed by the HC.
  USBH_STATUS_FRAME_ERROR                      = 0x12,     // An interrupt transfer could not be scheduled within a micro frame.
  USBH_STATUS_SPLIT_ERROR                      = 0x13,     // Error while using split transactions.
  USBH_STATUS_NEED_MORE_DATA                   = 0x20,     // The transfer could not be started yet. More data is required.
  USBH_STATUS_CHANNEL_NAK                      = 0x21,
  USBH_STATUS_ERROR                            = 0x22,     // Unspecified error occurred.
  USBH_STATUS_INVALID_PARAM                    = 0x23,     // An invalid parameter was provided.
  USBH_STATUS_PENDING                          = 0x24,     // The operation was started asynchronously.
  USBH_STATUS_DEVICE_REMOVED                   = 0x25,     // The device was detached from the host.
  USBH_STATUS_CANCELED                         = 0x26,     // The operation was canceled by user request.
  USBH_STATUS_HC_STOPPED                       = 0x27,     // Host controller stopped.
  USBH_STATUS_BUSY                             = 0x29,     // The endpoint, interface or device has pending requests and
                                                           // therefore the operation can not be executed.

  USBH_STATUS_INVALID_DESCRIPTOR               = 0x30,     // A device provided an invalid descriptor.

  USBH_STATUS_ENDPOINT_HALTED                  = 0x31,     // The endpoint has been halted.
                                                           // A pipe will be halted when a data transmission error (CRC, bit stuff, DATA toggle) occurs.
  USBH_STATUS_TIMEOUT                          = 0x32,     // The operation was aborted due to a timeout.
  USBH_STATUS_PORT                             = 0x33,     // Operation on a USB port failed.

  USBH_STATUS_INVALID_HANDLE                   = 0x34,     // An invalid handle was provided to the function.
  USBH_STATUS_NOT_OPENED                       = 0x35,     // The device or interface was not opened.
  USBH_STATUS_ALREADY_ADDED                    = 0x36,     // Item was already been added.
  USBH_STATUS_ENDPOINT_INVALID                 = 0x37,     // Invalid endpoint for the requested operation.

  USBH_STATUS_LENGTH                           = 0x40,     // The operation detected a length error.
  USBH_STATUS_COMMAND_FAILED                   = 0x42,     // This error is reported if the MSD command
                                                           // code was sent successfully but the
                                                           // status returned from the device indicates a command error.
  USBH_STATUS_INTERFACE_PROTOCOL               = 0x43,     // The used MSD interface protocol is not supported.
                                                           // The interface protocol is defined by the interface descriptor.
  USBH_STATUS_INTERFACE_SUB_CLASS              = 0x44,     // The used MSD interface sub class is not
                                                           // supported. The interface sub class is
                                                           // defined by the interface descriptor.
  USBH_STATUS_WRITE_PROTECT                    = 0x47,     // The MSD medium is write protected.
  USBH_STATUS_INTERNAL_BUFFER_NOT_EMPTY        = 0x51,
  USBH_STATUS_BAD_RESPONSE                     = 0x52,     // Device responded unexpectedly.
  USBH_STATUS_DEVICE_ERROR                     = 0x53,     // Device indicated a failure.

  USBH_STATUS_MTP_OPERATION_NOT_SUPPORTED      = 0x60,     // The requested MTP operation is not supported by the connected device.

  USBH_STATUS_MEMORY                           = 0x88,     // Memory could not been allocated.
  USBH_STATUS_RESOURCES                        = 0x89      // Not enough resources (e.g endpoints, events, handles, ...)
} USBH_STATUS;

/*********************************************************************
*
*  Endpoint characteristics.
*/
// Direction types
#define USB_IN_DIRECTION      0x80u
#define USB_OUT_DIRECTION     0x00u

// End Point types
#define USB_EP_TYPE_CONTROL   0x00u
#define USB_EP_TYPE_ISO       0x01u
#define USB_EP_TYPE_BULK      0x02u
#define USB_EP_TYPE_INT       0x03u

// SETUP bRequest Type Direction
#define USB_TO_DEVICE         0x00u
#define USB_TO_HOST           0x80u

// Setup bRequest Types
#define USB_REQTYPE_MASK                              0x60u // Used to mask off request type
#define USB_REQTYPE_STANDARD                          0x00u // Standard Request
#define USB_REQTYPE_CLASS                             0x20u // Class Request
#define USB_REQTYPE_VENDOR                            0x40u // Vendor Request
#define USB_REQTYPE_RESERVED                          0x60u // Reserved or illegal request

// Request Type Recipient
#define USB_RECIPIENT_MASK                            0x1Fu // Bitsd D0..D4
#define USB_DEVICE_RECIPIENT                          0u
#define USB_INTERFACE_RECIPIENT                       1u
#define USB_ENDPOINT_RECIPIENT                        2u
#define USB_OTHER_RECIPIENT                           3u
#define USB_RESERVED_RECIPIENT                        4u


/*********************************************************************
*
*       USBH_MTYPE
*
*  IDs to distinguish different message types
*/
#define USBH_MTYPE_INIT          (1UL <<  0)
#define USBH_MTYPE_CORE          (1UL <<  1)
#define USBH_MTYPE_TIMER         (1UL <<  2)
#define USBH_MTYPE_DRIVER        (1UL <<  3)
#define USBH_MTYPE_MEM           (1UL <<  4)
#define USBH_MTYPE_URB           (1UL <<  5)
#define USBH_MTYPE_OHCI          (1UL <<  6)
#define USBH_MTYPE_UBD           (1UL <<  7)
#define USBH_MTYPE_PNP           (1UL <<  8)
#define USBH_MTYPE_DEVICE        (1UL <<  9)
#define USBH_MTYPE_RHUB          (1UL << 10)
#define USBH_MTYPE_HUB           (1UL << 11)
#define USBH_MTYPE_MSD           (1UL << 12)
#define USBH_MTYPE_MSD_INTERN    (1UL << 13)
#define USBH_MTYPE_MSD_PHYS      (1UL << 14)
#define USBH_MTYPE_HID           (1UL << 15)
#define USBH_MTYPE_PRINTER_CLASS (1UL << 16)
#define USBH_MTYPE_CDC           (1UL << 17)
#define USBH_MTYPE_FT232         (1UL << 18)
#define USBH_MTYPE_APPLICATION   (1UL << 19)
#define USBH_MTYPE_CORE_NOTI     (1UL << 20)
#define USBH_MTYPE_MTP           (1UL << 21)
#define USBH_MTYPE_CORE_HC       (1UL << 22)
#define USBH_MTYPE_BULK          (1UL << 23)
#define USBH_MTYPE_LAN           (1UL << 24)
#define USBH_MTYPE_CCID          (1UL << 25)

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct _USB_DEVICE            USB_DEVICE;
typedef unsigned int                  USBH_INTERFACE_ID; // This ID identifies an interface in a unique way, a value of zero represents an invalid ID!
typedef unsigned int                  USBH_DEVICE_ID;    // This ID identifies a device in a unique way
typedef struct _USB_INTERFACE       * USBH_INTERFACE_HANDLE;
typedef struct _USB_ENDPOINT          USB_ENDPOINT;

//lint -esym(9058, _USB_DEVICE, _USB_INTERFACE, _USB_ENDPOINT)  N:100

/*********************************************************************
*
*       USBH_SPEED
*
*  Description
*    Enum containing operation speed values of a device.
*    Is used as a member in the USBH_INTERFACE_INFO data structure
*    and to get the operation speed of a device.
*/
typedef enum {
  USBH_SPEED_UNKNOWN, // The speed is unknown.
  USBH_LOW_SPEED,     // The device operates in low-speed mode.
  USBH_FULL_SPEED,    // The device operates in full-speed mode.
  USBH_HIGH_SPEED     // The device operates in high-speed mode.
} USBH_SPEED;

/*********************************************************************
*
*       USBH_DEVICE_EVENT
*
*  Description
*    Enum containing the device events.
*    Enumerates the types of device events. It is used by the
*    USBH_NOTIFICATION_FUNC callback to indicate which type of event occurred.
*/
typedef enum {
  USBH_DEVICE_EVENT_ADD,   // Indicates that a device was connected to the host and new interface is available.
  USBH_DEVICE_EVENT_REMOVE // Indicates that a device has been removed.
} USBH_DEVICE_EVENT;

typedef struct { // Device descriptor
  U8  bLength;
  U8  bDescriptorType;
  U16 bcdUSB;
  U8  bDeviceClass;
  U8  bDeviceSubClass;
  U8  bDeviceProtocol;
  U8  bMaxPacketSize0;
  U16 idVendor;
  U16 idProduct;
  U16 bcdDevice;
  U8  iManufacturer;
  U8  iProduct;
  U8  iSerialNumber;
  U8  bNumConfigurations;
} USBH_DEVICE_DESCRIPTOR;

/*********************************************************************
*
*       USBH_NOTIFICATION_FUNC
*
*  Description
*    Type of user callback set in USBH_PRINTER_RegisterNotification(),
*    USBH_HID_RegisterNotification(), USBH_CDC_AddNotification(),
*    USBH_FT232_RegisterNotification() and USBH_MTP_RegisterNotification().
*
*  Parameters
*    pContext : Pointer to a context passed by the user in the call to one
*               of the register functions.
*    DevIndex : Zero based index of the device that was added or removed.
*               First device has index 0, second one has index 1, etc
*    Event    : Enum USBH_DEVICE_EVENT which gives information about
*               the event that occurred.
*/
typedef void (USBH_NOTIFICATION_FUNC)       (void * pContext, U8 DevIndex, USBH_DEVICE_EVENT Event);

/*********************************************************************
*
*       USBH_ONSETCONFIGURATION_FUNC
*
*  Description
*    This call is used to determine which configuration shall be used when
*    the device has multiple configuration.
*
*
*  Parameters
*    pContext          : Pointer to a context passed by the user in the call to one
*                        of the AddOnSetConfiguration function.
*    pDeviceDesc       : Pointer to the Device Descriptor of the device.
*    ppConfigDesc      : Pointer to an array of the available ConfigDescriptors.
*    NumConfigurations : Number of available configurations.
*    pConfigValue      : Pointer to a variable to store the configuration value to set.
*
*  Return value
*    USBH_STATUS_SUCCESS on success or error code on failure.
*
*  Additional information
*    If the function returns an error code (including USBH_STATUS_TIMEOUT) it already may
*    have written part of the data. The number of bytes written successfully is always
*    stored in the variable pointed to by pNumBytesWritten.
*/
typedef USBH_STATUS (USBH_ONSETCONFIGURATION_FUNC)(void * pContext, const USBH_DEVICE_DESCRIPTOR * pDeviceDesc, const U8 * const * ppConfigDesc, unsigned NumConfigurations, U8 * pConfigValue);

/*********************************************************************
*
*       USBH_ON_SETPORTPOWER_FUNC
*
*  Description
*    Callback set by USBH_SetOnSetPortPower(). Is called when port power changes.
*
*  Parameters
*    HostControllerIndex  : Index of the host controller. This corresponds
*                           to the return value of the respective USBH_<DriverName>_Add call.
*    Port                 : 1-based port index.
*    PowerOn              : * 0 - power off
*                           * 1 - power on
*/
typedef void (USBH_ON_SETPORTPOWER_FUNC)    (U32 HostControllerIndex, U8 Port, U8 PowerOn);

// Mask bits for device mask
#define USBH_INFO_MASK_VID       0x0001u
#define USBH_INFO_MASK_PID       0x0002u
#define USBH_INFO_MASK_DEVICE    0x0004u
#define USBH_INFO_MASK_INTERFACE 0x0008u
#define USBH_INFO_MASK_CLASS     0x0010u
#define USBH_INFO_MASK_SUBCLASS  0x0020u
#define USBH_INFO_MASK_PROTOCOL  0x0040u
#define USBH_INFO_MASK_REMOVED   0x0080u
#define USBH_INFO_MASK_HUBS      0x0100u
#define USBH_INFO_MASK_VID_ARRAY 0x0200u
#define USBH_INFO_MASK_PID_ARRAY 0x0400u

/*********************************************************************
*
*       USBH_INTERFACE_MASK
*
*  Description
*    Data structure that defines conditions to select USB interfaces.
*    Can be used to register notifications. Members that are not selected
*    with Mask need not be initialized.
*/
typedef struct {
  U16   Mask;              // Contains an OR combination of the following flags. If the flag is set
                           // the related member of this structure is compared to the properties
                           // of the USB interface.
                           // + USBH_INFO_MASK_VID Compare the Vendor ID (VID) of the device.
                           // + USBH_INFO_MASK_PID Compare the Product ID (PID) of the device.
                           // + USBH_INFO_MASK_DEVICE Compare the bcdDevice value of the device.
                           // + USBH_INFO_MASK_INTERFACE Compare the interface number.
                           // + USBH_INFO_MASK_CLASS Compare the class of the interface.
                           // + USBH_INFO_MASK_SUBCLASS Compare the sub class of the interface.
                           // + USBH_INFO_MASK_PROTOCOL Compare the protocol of the interface.
                           // + USBH_INFO_MASK_VID_ARRAY Compare the Vendor ID (VID) of the device to a list if ids.
                           // + USBH_INFO_MASK_PID_ARRAY Compare the Product ID (PID) of the device to a list if ids.
                           // If both USBH_INFO_MASK_VID_ARRAY and USBH_INFO_MASK_PID_ARRAY are selected, then the
                           // VendorId/ProductId of the device is compared to pairs pVendorIds[i]/pProductIds[i].
  U16   VendorId;          // Vendor ID to compare with.
  U16   ProductId;         // Product ID to compare with.
  U16   bcdDevice;         // BCD coded device version to compare with.
  U8    Interface;         // Interface number to compare with.
  U8    Class;             // Class code to compare with.
  U8    SubClass;          // Sub class code to compare with.
  U8    Protocol;          // Protocol stored in the interface to compare with.
  const U16 * pVendorIds;  // Points to an array of Vendor IDs.
  const U16 * pProductIds; // Points to an array of Product IDs.
  U16   NumIds;            // Number of ids in *pVendorIds and *pProductIds.
                           // When only USBH_INFO_MASK_VID_ARRAY is set this is the size of the pVendorIds array.
                           // When only USBH_INFO_MASK_PID_ARRAY is set this is the size of the pProductIds array.
                           // When both are set this is the size for both arrays (the arrays have to be the same size
                           // when both flags are set).
} USBH_INTERFACE_MASK;

typedef struct _USBH_INTERFACE_LIST * USBH_INTERFACE_LIST_HANDLE; // Handle to the interface list

//lint -esym(9058, _USBH_INTERFACE_LIST)        N:100

/*********************************************************************
*
*       USBH_INTERFACE_INFO
*
*  Description
*    This structure contains information about a USB interface and the related device
*    and is returned by the function USBH_GetInterfaceInfo().
*/
typedef struct {
  USBH_INTERFACE_ID InterfaceId;          // The unique interface Id. This Id is assigned if the
                                          // USB device was successful enumerated. It is valid until the
                                          // device is removed for the host. If the device is reconnected
                                          // a different interface Id is assigned to each interface.
  USBH_DEVICE_ID    DeviceId;             // The unique device Id. This Id is assigned if the USB
                                          // device was successfully enumerated. It is valid until the
                                          // device is removed from the host. If the device is reconnected
                                          // a different device Id is assigned. The relation
                                          // between the device Id and the interface Id can be used by
                                          // an application to detect which USB interfaces belong to a device.
  U16               VendorId;             // The Vendor ID of the device.
  U16               ProductId;            // The Product ID of the device.
  U16               bcdDevice;            // The BCD coded device version.
  U8                Interface;            // The USB interface number.
  U8                Class;                // The interface class.
  U8                SubClass;             // The interface sub class.
  U8                Protocol;             // The interface protocol.
  unsigned int      OpenCount;            // Number of open handles for this interface.
  U8                ExclusiveUsed;        // If not 0, this interface is used exclusively.
  USBH_SPEED        Speed;                // Operation speed of the device.
  U8                SerialNumberSize;     // The size of the serial number in bytes, 0 means not available or error during request.
                                          // The serial number itself can be retrieved using USBH_GetInterfaceSerial().
  U8                NumConfigurations;    // Number of different configuration of the device.
  U8                CurrentConfiguration; // Currently selected configuration, zero-based: 0...(NumConfigurations-1)
  U8                HCIndex;              // Index of the host controller the device is connected to.
  U8                AlternateSetting;     // The current alternate setting for this interface.
} USBH_INTERFACE_INFO;

/*********************************************************************
*
*       USBH_SET_CONF_HOOK
*
*  Description
*    This structure is used to store the hook function and its parameters
*    which will be called consecutively before the SET_CONFIGURATION request
*    during the enumeration.
*/
typedef  struct _USBH_SET_CONF_HOOK {
  struct _USBH_SET_CONF_HOOK   * pNext;
  USBH_ONSETCONFIGURATION_FUNC * pfOnSetConfig;
  void                         * pContext;
} USBH_SET_CONF_HOOK;

/*********************************************************************
*
*       USBH_PORT_INFO
*
*  Description
*    Information about a connected USB device returned by USBH_GetPortInfo().
*/
typedef struct {
  U8                IsHighSpeedCapable;   // * 1: Port supports high-speed, full-speed and low-speed communication.
                                          // * 0: Port supports only full-speed and low-speed communication.
  U8                IsRootHub;            // * 1: RootHub, device is directly connected to the host.
                                          // * 0: Device is connected via an external hub to the host.
  U8                IsSelfPowered;        // * 1: Device is externally powered
                                          // * 0: Device is powered by USB host controller.
  U8                HCIndex;              // Index of the host controller the device is connected to.
  U16               MaxPower;             // Max power the USB device consumes from USB host controller / USB hub in mA.
  U16               PortNumber;           // Port number of the hub or roothub. Ports are counted starting with 1.
  USBH_SPEED        PortSpeed;            // The port speed is the speed with which the device is connected.
                                          // Can be either USBH_LOW_SPEED or USBH_FULL_SPEED or USBH_HIGH_SPEED.
  USBH_DEVICE_ID    DeviceId;             // The unique device Id. This Id is assigned if the USB
                                          // device was successfully enumerated. It is valid until the
                                          // device is removed from the host. If the device is reconnected
                                          // a different device Id is assigned. The relation
                                          // between the device Id and the interface Id can be used by
                                          // an application to detect which USB interfaces belong to a device.
  USBH_DEVICE_ID    HubDeviceId;          // The unique device Id of the HUB, if the device is connected via an external HUB.
                                          // If IsRootHub == 1, then HubDeviceId is zero.
  USBH_INTERFACE_ID HubInterfaceId;       // Interface Id of the HUB, if the device is connected via an external HUB.
                                          // If IsRootHub == 1, then HubInterfaceId is zero.
} USBH_PORT_INFO;

/*********************************************************************
*
*       USBH_SETUP_PACKET
*
*  Description
*    Structure to handle the control setup request
*/
typedef struct {
  U8  Type;
  U8  Request;
  U16 Value;
  U16 Index;
  U16 Length;
} USBH_SETUP_PACKET;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
void        USBH_Task                     (void);
void        USBH_ISRTask                  (void);
void        USBH_ConfigTransferBufferSize (U32 HCIndex, U32 Size);
void        USBH_ConfigRootHub            (U8 SupportOvercurrent, U8 PortsAlwaysPowered, U8 PerPortPowered);
void        USBH_ConfigMaxUSBDevices      (U8 NumDevices);
void        USBH_ConfigMaxNumEndpoints    (U8 MaxNumBulkEndpoints, U8 MaxNumIntEndpoints, U8 MaxNumIsoEndpoints);
void        USBH_ConfigSupportExternalHubs(U8 OnOff);
void        USBH_ConfigPortPowerPin       (U8 SetHighIsPowerOn);
USBH_STATUS USBH_ConfigPortPowerPinEx     (U32 HCIndex, U8 SetHighIsPowerOn);
void        USBH_ConfigPowerOnGoodTime    (unsigned PowerGoodTime);
void        USBH_Exit                     (void);
void        USBH_Init                     (void);
U32         USBH_GetVersion               (void);
void        USBH_AssignMemory             (void * pMem, U32 NumBytes);
void        USBH_AssignTransferMemory     (void * pMem, U32 NumBytes);
void        USBH_X_Config                 (void);
void        USBH_ServiceISR               (unsigned Index);
void        USBH_SetOnSetPortPower        (USBH_ON_SETPORTPOWER_FUNC * pfOnSetPortPower);
void        USBH_SetCacheConfig           (const SEGGER_CACHE_CONFIG *pConfig, unsigned ConfSize);
USBH_STATUS USBH_AddOnSetConfigurationHook(USBH_SET_CONF_HOOK * pHook, USBH_ONSETCONFIGURATION_FUNC * pfOnSetConfiguration, void  * pContext);


/*********************************************************************
*
*       Information retrieving functions
*/
USBH_INTERFACE_LIST_HANDLE USBH_CreateInterfaceList (const USBH_INTERFACE_MASK  * pInterfaceMask, unsigned int * pInterfaceCount);
void                       USBH_DestroyInterfaceList(USBH_INTERFACE_LIST_HANDLE   hInterfaceList);
USBH_INTERFACE_ID          USBH_GetInterfaceId      (USBH_INTERFACE_LIST_HANDLE   hInterfaceList, unsigned int Index);
USBH_STATUS                USBH_GetInterfaceInfo    (USBH_INTERFACE_ID            InterfaceID, USBH_INTERFACE_INFO * pInterfaceInfo);
USBH_STATUS                USBH_GetPortInfo         (USBH_INTERFACE_ID            InterfaceID, USBH_PORT_INFO * pPortInfo);
USBH_STATUS                USBH_GetInterfaceSerial  (USBH_INTERFACE_ID            InterfaceID, U32 BuffSize, U8 *pSerialNumber, U32 *pSerialNumberSize);
int                        USBH_IsRunning           (void);
int                        USBH_GetNumDevicesConnected(U32 HCIndex);
const char               * USBH_GetStatusStr        (USBH_STATUS x);

// Mask bits for device mask
#define USBH_EP_MASK_INDEX     0x0001u
#define USBH_EP_MASK_ADDRESS   0x0002u
#define USBH_EP_MASK_TYPE      0x0004u
#define USBH_EP_MASK_DIRECTION 0x0008u

/*********************************************************************
*
*       USBH_EP_MASK
*
*  Description
*    Is used as an input parameter to get an endpoint descriptor. The comparison with the
*    mask is true if each member that is marked as valid by a flag in the mask member is
*    equal to the value stored in the endpoint. E.g. if the mask is 0 the first endpoint is
*    returned. If  Mask  is set to  USBH_EP_MASK_INDEX  the zero based index can be used to
*    address all endpoints.
*/
typedef struct {
  U32 Mask;              // This member contains the information which fields are valid. It is an
                         // OR combination of the following flags:
                         // * USBH_EP_MASK_INDEX  The Index is used for comparison.
                         // * USBH_EP_MASK_ADDRESS  The  Address  field  is  used  for  comparison.
                         // * USBH_EP_MASK_TYPE  The Type field is used for comparison.
                         // * USBH_EP_MASK_DIRECTION   The  Direction  field  is  used  for comparison.
  U8  Index;             // If valid, this member contains the zero based index of the endpoint in
                         // the interface.
  U8  Address;           // If valid, this member contains an endpoint address with direction bit.
  U8  Type;              // If valid, this member contains the type of the endpoint:
                         // * USB_EP_TYPE_CONTROL
                         // * USB_EP_TYPE_BULK
                         // * USB_EP_TYPE_ISO
                         // * USB_EP_TYPE_INT
  U8  Direction;         // If valid, this member specifies a direction. It is one of the following values:
                         // * USB_IN_DIRECTION  From device to host
                         // * USB_OUT_DIRECTION  From host to device
} USBH_EP_MASK;

USBH_STATUS USBH_GetDeviceDescriptor                (USBH_INTERFACE_HANDLE hInterface, U8 * pDescriptor, unsigned * pBufferSize);
USBH_STATUS USBH_GetCurrentConfigurationDescriptor  (USBH_INTERFACE_HANDLE hInterface, U8 * pDescriptor, unsigned * pBufferSize);
USBH_STATUS USBH_GetSerialNumber                    (USBH_INTERFACE_HANDLE hInterface, U8 * pBuffer, unsigned * pBufferSize);
USBH_STATUS USBH_GetInterfaceDescriptor             (USBH_INTERFACE_HANDLE hInterface, U8 AlternateSetting, U8 * pBuffer, unsigned * pBufferSize);
USBH_STATUS USBH_GetMaxTransferSize                 (USBH_INTERFACE_HANDLE hInterface, U8 Endpoint, U32 *pMaxTransferSize);
USBH_STATUS USBH_GetEndpointDescriptor              (USBH_INTERFACE_HANDLE hInterface, U8 AlternateSetting, const USBH_EP_MASK * pMask, U8 * pBuffer, unsigned * pBufferSize);
USBH_STATUS USBH_GetDescriptor                      (USBH_INTERFACE_HANDLE hInterface, U8 AlternateSetting,   U8 Type, U8 * pBuffer, unsigned * pBufferSize);
USBH_STATUS USBH_GetDescriptorEx                    (USBH_INTERFACE_HANDLE hInterface, U8 Type, U8 DescIndex, U16 LangID, U8 * pBuffer, unsigned * pBufferSize);
USBH_STATUS USBH_GetStringDescriptor                (USBH_INTERFACE_HANDLE hInterface, U8 StringIndex, U16 LangID, U8 * pBuffer, unsigned * pNumBytes);
USBH_STATUS USBH_GetSpeed                           (USBH_INTERFACE_HANDLE hInterface, USBH_SPEED * pSpeed);
USBH_STATUS USBH_GetFrameNumber                     (USBH_INTERFACE_HANDLE hInterface, U32 * pFrameNumber);
USBH_STATUS USBH_GetInterfaceIdByHandle             (USBH_INTERFACE_HANDLE hInterface, USBH_INTERFACE_ID * pInterfaceId);
unsigned    USBH_GetNumAlternateSettings            (USBH_INTERFACE_HANDLE hInterface);
USBH_STATUS USBH_GetInterfaceCurrAltSetting         (USBH_INTERFACE_HANDLE hInterface, unsigned * pCurAltSetting);

U32         USBH_MEM_GetFree                        (int Idx);
U32         USBH_MEM_GetUsed                        (int Idx);
U32         USBH_MEM_GetMaxUsed                     (int Idx);
void        USBH_MEM_Panic                          (void);           //lint -function(exit,USBH_MEM_Panic) USBH_MEM_Panic does not return  N:100

/*********************************************************************
*
*       PnP and enumeration
*/

// Error type
#define USBH_ENUM_ERROR_EXTHUBPORT_FLAG 0x01u // The device is connected to an external hub
#define USBH_ENUM_ERROR_RETRY_FLAG      0x02u // The enumeration is retried
#define USBH_ENUM_ERROR_STOP_ENUM_FLAG  0x04u // The enumeration is stopped after retries
#define USBH_ENUM_ERROR_DISCONNECT_FLAG 0x08u // The parent port status is disconnected, this means the USB device is not connected
                                              // or it is connected and has an error. USBH_RestartEnumError() does nothing if
                                              // the  port status is disconnected.
#define USBH_ENUM_ERROR_ROOT_PORT_RESET 0x10u // Error during reset of a USB device on an root hub port
#define USBH_ENUM_ERROR_HUB_PORT_RESET  0x20u // Error during reset of a USB device on an external hub port
#define USBH_ENUM_ERROR_INIT_DEVICE     0x30u // Error during initialization of an device until it is in the configured state
#define USBH_ENUM_ERROR_INIT_HUB        0x40u // Error during initialization of an configured external hub device until the installation of an interrupt IN status pipe
#define USBH_ENUM_ERROR_LOCATION_MASK  (USBH_ENUM_ERROR_ROOT_PORT_RESET | \
                                        USBH_ENUM_ERROR_HUB_PORT_RESET  | \
                                        USBH_ENUM_ERROR_INIT_DEVICE     | \
                                        USBH_ENUM_ERROR_INIT_HUB)

/*********************************************************************
*
*       USBH_ENUM_ERROR
*
*  Description
*    Is used as a notification parameter for the  USBH_ON_ENUM_ERROR_FUNC  callback function.
*    This data structure does not contain detailed information about the device that
*    fails at enumeration because this information is not available in all phases of the enumeration.
*/
typedef struct {
  unsigned    Flags;                    // Additional flags to determine the location and the
                                        // type of the error.
                                        //
                                        // * USBH_ENUM_ERROR_EXTHUBPORT_FLAG
                                        // means  the  device  is  connected  to  an external hub.
                                        //
                                        // * USBH_ENUM_ERROR_RETRY_FLAG  the  bus
                                        // driver  retries  the  enumeration  of  this
                                        // device automatically.
                                        //
                                        // * USBH_ENUM_ERROR_STOP_ENUM_FLAG  the
                                        // bus  driver  does  not  restart  the  enumeration for
                                        // this device because all retries have
                                        // failed.  The  application  can  force  the  bus
                                        // driver  to  restart  the  enumeration  by  calling
                                        // the function USBH_RestartEnumError.
                                        //
                                        // * USBH_ENUM_ERROR_DISCONNECT_FLAG
                                        // means  the  device  has  been  disconnected
                                        // during  the  enumeration.  If  the  hub  port
                                        // reports a disconnect state the device cannot  be
                                        // re-enumerated  by  the  bus  driver
                                        // automatically.  Also  the  function
                                        // USBH_RestartEnumError  cannot  re-enumerate the device.
                                        //
                                        // * USBH_ENUM_ERROR_ROOT_PORT_RESET
                                        // means an error during the USB reset of a
                                        // root hub port occurs.
                                        //
                                        // * USBH_ENUM_ERROR_HUB_PORT_RESET
                                        // means an error during a reset of an external hub port occurs.

  int         PortNumber;               // Port number of the parent port where the USB
                                        // device is connected. A flag in the PortFlags field
                                        // determines if this is an external hub port.
  USBH_STATUS Status;                   // Status of the failed operation.
  int         ExtendedErrorInformation; // Internal information used for debugging.
} USBH_ENUM_ERROR;


/*********************************************************************
*
*       USBH_PNP_EVENT
*
*  Description
*     Is used as a parameter for the PnP notification.
*/
typedef enum {
  USBH_ADD_DEVICE,     // Indicates that a device was connected to the host and a new interface is available.
  USBH_REMOVE_DEVICE   // Indicates that a device has been removed.
} USBH_PNP_EVENT;

typedef struct _ENUM_ERROR_NOTIFICATION * USBH_ENUM_ERROR_HANDLE;
typedef struct _USBH_NOTIFICATION       * USBH_NOTIFICATION_HANDLE;

//lint -esym(9058, _ENUM_ERROR_NOTIFICATION, _USBH_NOTIFICATION)  N:100

/*********************************************************************
*
*       USBH_NOTIFICATION_HOOK
*
*  Description
*    This structure is used to store the hook function and its parameters
*    which will be called consecutively during the registered notification callback
*    in the class modules.
*/
typedef struct _USBH_NOTIFICATION_HOOK {
  struct _USBH_NOTIFICATION_HOOK * pNext;
  USBH_NOTIFICATION_FUNC         * pfNotification;
  void                           * pContext;
  USBH_NOTIFICATION_HANDLE         Handle;
} USBH_NOTIFICATION_HOOK;

/*********************************************************************
*
*       USBH_ON_ENUM_ERROR_FUNC
*
*  Description
*    Is called by the library if an error occurs at enumeration stage.
*
*  Parameters
*    pContext   : Is a user defined pointer that was passed to USBH_RegisterEnumErrorNotification().
*    pEnumError : Pointer to a structure containing information about the error occurred.
*                 This structure is temporary and must not be accessed after the functions returns.
*
*  Additional information
*    Is called in the context of USBH_Task() function or of a ProcessInterrupt function of a
*    host controller. Before this function is called it must be registered with
*    USBH_RegisterEnumErrorNotification(). If a device is not successfully enumerated
*    the function USBH_RestartEnumError() can be called to re-start a new enumeration
*    in the context of this function. This callback mechanism is part of the enhanced
*    error recovery.
*/
typedef void (USBH_ON_ENUM_ERROR_FUNC) (void * pContext, const USBH_ENUM_ERROR * pEnumError);

/*********************************************************************
*
*       USBH_ON_PNP_EVENT_FUNC
*
*  Description
*    Is called by the library if a PnP event occurs and if a PnP notification was registered.
*
*  Parameters
*    pContext     : Is the user defined pointer that was passed to USBH_RegisterEnumErrorNotification().
*                   The library does not dereference this pointer.
*    Event        : Enum USBH_DEVICE_EVENT specifies the PnP event.
*    InterfaceId  : Contains the interface Id of the removed or added interface.
*
*  Additional information
*    Is called in the context of USBH_Task() function. In the context of this function all
*    other API functions of the USB host stack can be called. The removed or added interface
*    can be identified by the interface Id. The client can use this information to find the
*    related USB Interface and close all handles if it was in use, to open it or to collect
*    information about the interface.
*/
typedef void (USBH_ON_PNP_EVENT_FUNC) (void * pContext, USBH_PNP_EVENT Event, USBH_INTERFACE_ID InterfaceId);

/*********************************************************************
*
*       USBH_PNP_NOTIFICATION
*
*  Description
*    Is used as an input parameter for the USBH_RegisterEnumErrorNotification()
*    function.
*/
typedef struct {
  USBH_ON_PNP_EVENT_FUNC * pfPnpNotification; // The notification function that is called from the USB stack
                                              // if a PnP event occurs.
  void                   * pContext;          // Pointer to a context, that is passed to the notification function.
  USBH_INTERFACE_MASK      InterfaceMask;     // Mask for the interfaces for which the PnP notifiation should be called.
} USBH_PNP_NOTIFICATION;

USBH_ENUM_ERROR_HANDLE   USBH_RegisterEnumErrorNotification  (void * pContext, USBH_ON_ENUM_ERROR_FUNC * pfEnumErrorCallback);
void                     USBH_UnregisterEnumErrorNotification(USBH_ENUM_ERROR_HANDLE hEnumError);
void                     USBH_RestartEnumError               (void);
USBH_NOTIFICATION_HANDLE USBH_RegisterPnPNotification        (const USBH_PNP_NOTIFICATION    * pPnPNotification);
void                     USBH_UnregisterPnPNotification      (USBH_NOTIFICATION_HANDLE   hNotification);

/*********************************************************************
*
*       USBH_FUNCTION
*
*  Description
*    Is used as a member for the USBH_HEADER data structure.
*    All function codes use the API function USBH_SubmitUrb() and are handled asynchronously.
*/
typedef enum {
  USBH_FUNCTION_CONTROL_REQUEST,   // Is used to send an URB with a control request.
                                   // It uses the data structure USBH_CONTROL_REQUEST.
                                   // A control request includes standard, class and
                                   // vendor defines requests. The standard requests
                                   // SetAddress and SetInterface
                                   // can not be submitted by this request. These
                                   // requests require a special handling in the driver.
                                   // See USBH_FUNCTION_SET_INTERFACE for details.
  USBH_FUNCTION_BULK_REQUEST,      // Is used to transfer data to or from a bulk endpoint.
                                   // It uses the data structure USBH_BULK_INT_REQUEST.
  USBH_FUNCTION_INT_REQUEST,       // Is used to transfer data to or from an interrupt endpoint.
                                   // It uses the data structure USBH_BULK_INT_REQUEST.
                                   // The interval is defined by the endpoint descriptor.
  USBH_FUNCTION_ISO_REQUEST,       // Is used to transfer data to or from an ISO endpoint.
                                   // It uses the data structure USBH_ISO_REQUEST.
                                   // ISO transfer may not be supported by all host controllers.
  USBH_FUNCTION_RESET_DEVICE,      // Sends a USB reset to the device. This removes the device
                                   // and all its interfaces from the USB stack.
                                   // The application should abort all pending requests
                                   // and close all handles to this device.
                                   // All handles become invalid.
                                   // The USB stack then starts a new enumeration of the device.
                                   // All interfaces will get new interface Ids.
                                   // This request can be part of an error recovery or
                                   // part of special class protocols like DFU.
                                   // This function uses only the URB header.
  USBH_FUNCTION_RESET_ENDPOINT,    // Clears an error condition on a special endpoint.
                                   // If a data transfer error occurs that cannot be handled
                                   // in hardware the driver stops the endpoint and does
                                   // not allow further data transfers before the endpoint
                                   // is reset with this function. On a bulk or interrupt
                                   // endpoint the host driver sends a Clear Feature
                                   // Endpoint Halt request. This informs the device
                                   // about the hardware error. The driver resets the data
                                   // toggle bit for this endpoint. This request expects
                                   // that no pending URBs are scheduled on this endpoint.
                                   // Pending URBs must be aborted with the URB based
                                   // function USBH_FUNCTION_ABORT_ENDPOINT.
                                   // This function uses the data structure USBH_ENDPOINT_REQUEST.
  USBH_FUNCTION_ABORT_ENDPOINT,    // Aborts all pending requests on an endpoint.
                                   // The host controller calls the completion function
                                   // with a status code USBH_STATUS_CANCELED.
                                   // The completion of the URBs may be delayed.
                                   // The application should wait until all pending
                                   // requests have been returned by the driver before
                                   // the handle is closed or USBH_FUNCTION_RESET_ENDPOINT
                                   // is called.
  USBH_FUNCTION_SET_INTERFACE,     // Selects a new alternate setting for the interface.
                                   // There must be no pending requests on any endpoint to
                                   // this interface. The interface handle does not
                                   // become invalid during this operation.
                                   // The number of endpoints may be changed.
                                   // This request uses the data structure USBH_SET_INTERFACE.
  USBH_FUNCTION_SET_POWER_STATE    // Is used to set the power state for a device.
                                   // There must be no pending requests for this device
                                   // if the device is set to the suspend state.
                                   // The request uses the data structure
                                   // USBH_SET_POWER_STATE. After the enumeration
                                   // the device is in normal power state.
} USBH_FUNCTION;

typedef struct _USBH_URB USBH_URB;

//lint -esym(9058, _USBH_URB)  N:100

/*********************************************************************
*
*       USBH_ON_COMPLETION_FUNC
*
*  Description
*    Is called by the library when an URB request completes.
*
*  Parameters
*    pUrb : Contains the URB that was completed.
*
*  Additional information
*    Is called in the context of the USBH_Task() or USBH_ISRTask().
*/
typedef void (USBH_ON_COMPLETION_FUNC)(USBH_URB * pUrb);

/*********************************************************************
*
*       USBH_ON_COMPLETION_USER_FUNC
*
*  Description
*    Is called by the class implementation (CDC, HID) when a transfer request completes.
*
*  Parameters
*    pContextData : Pointer to context structure. Depends on the class, where the
*                   function is used. See API description of the appropriate class.
*
*  Additional information
*    Is called in the context of the USBH_Task() or USBH_ISRTask().
*/
typedef void (USBH_ON_COMPLETION_USER_FUNC)(void * pContextData);

/*********************************************************************
*
*       USBH_HEADER
*
*  Description
*    Common parameters for all URB based requests.
*/
typedef struct {
  USBH_FUNCTION                  Function;            // Function code defines the operation of the URB.
  USBH_STATUS                    Status;              // After completion this member contains the status for the request.
  USBH_ON_COMPLETION_FUNC      * pfOnCompletion;      // Caller provided pointer to the completion function. This completion
                                                      // function is called if the function  USBH_SubmitUrb()  returns
                                                      // USBH_STATUS_PENDING. If a different status code is returned the
                                                      // completion function is never called.
  void                         * pContext;            // This member can be used by the caller to store a context passed to
                                                      // the completion routine.
  //
  // The following members are for internal use only.
  //
  USBH_ON_COMPLETION_FUNC      * pfOnInternalCompletion;
  void                         * pInternalContext;
  U32                            HcFlags;
  USBH_ON_COMPLETION_USER_FUNC * pfOnUserCompletion;
  void                         * pUserContext;
  USB_DEVICE                   * pDevice;
  union {
    USBH_URB                   * pUrb;                // Used by _SubmitSetInterface and _SubmitClearFeatureEndpointStall
    USB_ENDPOINT               * pEndpoint;           // Used by _SubmitClearFeatureEndpointStall and SubmitISO
  } IntContext;
} USBH_HEADER;

/*********************************************************************
*
*       USBH_CONTROL_REQUEST
*
*  Description
*    Defines parameters for a CONTROL transfer request.
*    Used with USBH_FUNCTION_CONTROL_REQUEST.
*
*  Additional information
*     A control request consists of a setup phase, an
*     optional data phase, and a handshake phase. The data phase is limited to a length of
*     4096 bytes. The  Setup  data structure must be filled in properly. The length field in
*     the Setup must contain the size of the Buffer. The caller must provide the storage
*     for the Buffer.
*
*     With this request any setup packet can be submitted. Some standard requests, like
*     SetAddress can be sent but would lead to a breakdown of the communication. It is not
*     allowed to set the following standard requests:
*
*     \bi{SetAddress}:
*     It is assigned by the USB stack during enumeration or USB reset.
*
*     \bi{Clear Feature Endpoint Halt}:
*     Use USBH_FUNCTION_RESET_ENDPOINT instead. The function
*     USBH_FUNCTION_RESET_ENDPOINT resets the data toggle bit in the host controller
*     structures.
*
*     \bi{SetConfiguration}
*/
typedef struct {
  USBH_SETUP_PACKET   Setup;    // The setup packet, direction of data phase, the length field must be valid!
  U8                  Endpoint; // The endpoint address with direction bit. Use 0 for default endpoint.
  void              * pBuffer;  // Pointer to the caller provided storage, can be NULL.
                                // This buffer is used in the data phase to transfer the data. The direction of the
                                // data transfer depends from the Type field in the Setup. See the
                                // USB specification for details.
  U32                 Length;   // Returns the number of bytes transferred in the data phase.
} USBH_CONTROL_REQUEST;

/*********************************************************************
*
*       USBH_BULK_INT_REQUEST
*
*  Description
*    Defines parameters for a BULK or INT transfer request.
*    Used with USBH_FUNCTION_BULK_REQUEST and USBH_FUNCTION_INT_REQUEST.
*/
typedef struct {
  U8     Endpoint;              // Specifies the endpoint address with direction bit.
  void * pBuffer;               // Pointer to a caller provided buffer.
  U32    Length;                // * [IN] length of data / size of buffer (in bytes).
                                // * [OUT] Bytes transferred.
} USBH_BULK_INT_REQUEST;

/*********************************************************************
*
*       USBH_ENDPOINT_REQUEST
*
*  Description
*    Defines parameter for an endpoint operation.
*    Used with USBH_FUNCTION_ABORT_ENDPOINT and USBH_FUNCTION_RESET_ENDPOINT.
*/
typedef struct {
  U8 Endpoint;                  // Specifies the endpoint address with direction bit.
} USBH_ENDPOINT_REQUEST;

/*********************************************************************
*
*       USBH_SET_INTERFACE
*
*  Description
*    Defines parameters for a control request to set an alternate interface setting.
*    Used with USBH_FUNCTION_SET_INTERFACE.
*/
typedef struct {
  U8 AlternateSetting;                   // Number of alternate interface setting (zero based).
} USBH_SET_INTERFACE;

/*********************************************************************
*
*       USBH_POWER_STATE
*
*  Description
*    Enumerates the power states of a device.
*    Is used as a member in the USBH_SET_POWER_STATE data structure.
*/
typedef enum {
  USBH_NORMAL_POWER, // The device is switched to normal operation.
  USBH_SUSPEND,      // The device is switched to USB suspend mode.
  USBH_POWER_OFF     // The device is powered off.
} USBH_POWER_STATE;

/*********************************************************************
*
*       USBH_SET_POWER_STATE
*
*  Description
*    Defines parameters to set or reset suspend mode for a device.
*    Used with USBH_FUNCTION_SET_POWER_STATE.
*
*  Additional information
*    If the device is switched to suspend, there must be no pending requests on the device.
*/
typedef struct {
  USBH_POWER_STATE PowerState;     // New power state of the device.
} USBH_SET_POWER_STATE;

/*********************************************************************
*
*       USBH_ISO_REQUEST
*
*  Description
*    Defines parameters for a ISO transfer request.
*    Used with USBH_FUNCTION_ISO_REQUEST.
*
*    Only Endpoint must be set to submit an ISO URB.
*    All other members are set by the driver, before the completion routine
*    is called.
*
*    For every packet send or received, the members of this structure are filled,
*    Header.Status is set to USBH_STATUS_SUCCESS and the callback function is called.
*    This is repeated until the URB is explicitly canceled.
*    The URB is finally terminated, if Header.Status != USBH_STATUS_SUCCESS.
*/
typedef struct {
  U8           Endpoint;              // [IN]  Specifies the endpoint address with direction bit.
  U8           NBuffers;              // [OUT] Number of buffers used by the driver.
  U16          Length;                // [OUT] Length of the data packet received (IN EPs only).
  const void * pData;                 // [OUT] Pointer to the data packet received (IN EPs only).
  USBH_STATUS  Status;                // [OUT] Status of the transaction.
} USBH_ISO_REQUEST;

/*********************************************************************
*
*       USBH_URB
*
*  Description
*    This data structure is used to submit an URB.
*    The URB is the basic structure for all asynchronous operations on the USB stack. All
*    requests that exchange data with the device are using this data structure. The caller
*    has to provide the memory for this structure. The memory must be permanent until
*    the completion function is called.
*/
struct _USBH_URB {
  USBH_HEADER Header;                         // Contains the URB header of type USBH_HEADER. The most important
                                              // parameters are the function code and the callback function.
  union {
    USBH_CONTROL_REQUEST   ControlRequest;
    USBH_BULK_INT_REQUEST  BulkIntRequest;
    USBH_ISO_REQUEST       IsoRequest;
    USBH_ENDPOINT_REQUEST  EndpointRequest;
    USBH_SET_INTERFACE     SetInterface;
    USBH_SET_POWER_STATE   SetPowerState;
  } Request;                                  // A union that contains information depending on the specific
                                              // request of the USBH_HEADER.
#if (USBH_DEBUG > 1)
  U32 UID;
#endif
};

/*********************************************************************
*
*       USBH_ISO_DATA_CTRL
*
*  Description
*    This data structure is used to provide or acknowledge ISO data.
*    Used with function USBH_IsoDataCtrl().
*/
typedef struct {
  U32          Length;        // [IN]  Length of data to be transferred via ISO OUT EP in bytes.
  const U8   * pData;         // [IN]  Pointer to data to be transferred via ISO OUT EP.
  const U8   * pBuffer;       // [OUT] Buffer used by the driver.
} USBH_ISO_DATA_CTRL;

// Used to access an interface
USBH_STATUS   USBH_OpenInterface  (USBH_INTERFACE_ID InterfaceID, U8 Exclusive, USBH_INTERFACE_HANDLE * pInterfaceHandle);
void          USBH_CloseInterface (USBH_INTERFACE_HANDLE hInterface);

// Interface function for all asynchronous requests. If the function returns USBH_STATUS_PENDING the completion routine is called.
// On each other status code the completion routine is never called. The storage of the URB must be provided by the caller and
// must be permanent until the URB is returned by the completion routine.
USBH_STATUS   USBH_SubmitUrb(USBH_INTERFACE_HANDLE hInterface, USBH_URB * pUrb);
USBH_STATUS   USBH_IsoDataCtrl(const USBH_URB * pUrb, USBH_ISO_DATA_CTRL *pIsoData);

void          USBH_SetRootPortPower(U32 HCIndex, U8 Port, USBH_POWER_STATE State);
void          USBH_HUB_SuspendResume(U32 HCIndex, U8 Port, U8 State);

/*********************************************************************
*
*       Public OS functions
*/
void USBH_OS_Delay           (unsigned ms);
U32  USBH_OS_GetTime32       (void);
void USBH_X_DisableInterrupt (void); // Function necessary for CMSIS RTX/Keil RTX, should be implemented in the USBH_Config_*.c file.
                                     // Should disable the interrupt for the used USB controller.
void USBH_X_EnableInterrupt  (void); // Function necessary for CMSIS RTX/Keil RTX, should be implemented in the USBH_Config_*.c file.
                                     // Should enable the interrupt for the used USB controller.

//lint -sem(USBH_OS_GetTime32, pure)    N:100

/*********************************************************************
*
*       Virtual memory support
*/
#if USBH_SUPPORT_VIRTUALMEM
/*********************************************************************
*
*       USBH_V2P_FUNC
*
*  Description
*    Function that translates a virtual address to a physical address.
*
*  Parameters
*    pVirtAddr : Pointer to virtual memory.
*
*  Return value
*    Physical memory address.
*/
typedef PTR_ADDR (USBH_V2P_FUNC)(void * pVirtAddr);

void                   USBH_Config_SetV2PHandler      (USBH_V2P_FUNC * pfV2PHandler);
PTR_ADDR               USBH_v2p                       (void * pVirtAddr);
#endif

typedef int  USBH_CHECK_ADDRESS_FUNC    (void * pMem);

/*********************************************************************
*
*       Log/Warn functions
*/
void USBH_Logf_Application   (const char * sFormat, ...);
void USBH_Warnf_Application  (const char * sFormat, ...);
void USBH_sprintf_Application(      char * pBuffer, unsigned BufferSize, const char * sFormat, ...);
void USBH_SetLogFilter       (U32 FilterMask);
void USBH_SetWarnFilter      (U32 FilterMask);
void USBH_AddLogFilter       (U32 FilterMask);
void USBH_AddWarnFilter      (U32 FilterMask);

/*********************************************************************
*
*       USBH Trace API
*/
typedef struct {
  //
  // Generic Trace Event logging.
  //
  void (*pfRecordVoid) (unsigned Id);
  void (*pfRecordU32)  (unsigned Id, U32 Para0);
  void (*pfRecordU32x2)(unsigned Id, U32 Para0, U32 Para1);
  void (*pfRecordU32x3)(unsigned Id, U32 Para0, U32 Para1, U32 Para2);
  void (*pfRecordU32x4)(unsigned Id, U32 Para0, U32 Para1, U32 Para2, U32 Para3);
} USBH_TRACE_API;

U32  USBH_TRACE_GetAPIDesc(const char** psDesc);
void USBH_TRACE_SetAPI    (const USBH_TRACE_API* pAPI, U32 APIOffset);


#if defined(__cplusplus)
  }
#endif

#endif    /* USBH_H_ */

/*************************** End of file ****************************/
