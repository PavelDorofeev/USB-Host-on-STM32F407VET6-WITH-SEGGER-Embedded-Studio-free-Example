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
File    : USBH_ConfDefaults.h
Purpose : This file contains the default values for USB-Host
          configuration paramteres. To allow easy updates please
          do not change the parameters here, but add them in the
          USBH_Conf.h file, the defines there will replace the default
          values.
--------------------------  END-OF-HEADER  ---------------------------
*/

#ifndef   USBH_CONFDEFAULTS_H
#define   USBH_CONFDEFAULTS_H

#include "USBH_Conf.h"
#include <string.h>             // for memcpy() etc.

/*********************************************************************
*
*       Basic types
*/
#ifndef     USBH_DEBUG
  #define   USBH_DEBUG   (0)
#endif
#ifndef     USBH_MEMCPY
  #define   USBH_MEMCPY  memcpy     /*lint -e{9087,9005} D:105[b] */
#endif
#ifndef     USBH_MEMSET
  #define   USBH_MEMSET  memset     /*lint -e{9087,9005} D:105[b] */
#endif
#ifndef     USBH_MEMMOVE
  #define   USBH_MEMMOVE memmove    /*lint -e{9087,9005} D:105[b] */
#endif
#ifndef     USBH_MEMCMP
  #define   USBH_MEMCMP  memcmp     /*lint -e{9087,9005} D:105[b] */
#endif
#ifndef     USBH_IS_BIG_ENDIAN
  #define   USBH_IS_BIG_ENDIAN 0      // Little endian is default
#endif

#ifndef     USBH_PANIC
  #if       USBH_DEBUG
    #define USBH_PANIC(s) USBH_Panic(s)
  #else
    #ifdef _lint
      //lint -emacro({717,9036}, USBH_PANIC)  N:100
      #define USBH_PANIC(p)  do {} while(0)
    #else
      #define USBH_PANIC(p)
    #endif
  #endif
#endif
#ifndef     USBH_SUPPORT_LOG
  #if       USBH_DEBUG > 1
    #define USBH_SUPPORT_LOG   1
  #else
    #define USBH_SUPPORT_LOG   0
  #endif
#endif
#ifndef     USBH_SUPPORT_WARN
  #if       USBH_DEBUG > 1
    #define USBH_SUPPORT_WARN  1
  #else
    #define USBH_SUPPORT_WARN  0
  #endif
#endif
//
// Buffer size for USBH_Logf_Application and USBH_Warnf_Application
//
#ifndef   USBH_LOG_APP_BUFFER_SIZE
  #define USBH_LOG_APP_BUFFER_SIZE 200
#endif
//
// Buffer size for USBH_Logf
//
#ifndef   USBH_LOG_BUFFER_SIZE
  #define USBH_LOG_BUFFER_SIZE     200
#endif
//
// Buffer size for USBH_Warnf
//
#ifndef USBH_WARN_BUFFER_SIZE
  #define USBH_WARN_BUFFER_SIZE  200
#endif

//
// Trace/SystemView related configuration defaults.
//
#ifndef   USBH_SUPPORT_TRACE
  #define USBH_SUPPORT_TRACE            0
#endif

#ifndef    USBH_SUPPORT_ISO_TRANSFER
  #define  USBH_SUPPORT_ISO_TRANSFER    0
#endif

#ifndef   USBH_SUPPORT_VIRTUALMEM
  #define USBH_SUPPORT_VIRTUALMEM       1
#endif

#ifndef   USBH_SUPPORT_HUB_CLEAR_TT_BUFFER
  #define USBH_SUPPORT_HUB_CLEAR_TT_BUFFER   0
#endif

#ifndef   USBH_REO_FREE_MEM_LIST
  #define USBH_REO_FREE_MEM_LIST        0
#endif

//
// Specifies the default timeout, in milliseconds, to be used for synchronous operations on the control endpoint.
//
#ifndef USBH_MSD_EP0_TIMEOUT
  #define USBH_MSD_EP0_TIMEOUT          5000u
#endif
//
// Specifies the maximum time in milliseconds, for reading all bytes with the bulk read operation.
//
#ifndef USBH_MSD_COMMAND_TIMEOUT
  #define USBH_MSD_COMMAND_TIMEOUT      3000u
#endif
//
// Specifies the maximum time in milliseconds, for reading all bytes with the bulk read operation.
//
#ifndef USBH_MSD_READ_TIMEOUT
  #define USBH_MSD_READ_TIMEOUT         10000u
#endif
//
// Specifies the maximum time, in milliseconds, for writing all bytes with the bulk write operation.
//
#ifndef USBH_MSD_WRITE_TIMEOUT
  #define USBH_MSD_WRITE_TIMEOUT        10000u
#endif
//
// Must be a multiple of the maximum packet length for bulk data endpoints.
// That are 64 bytes for a USB 1.1 device and 512 bytes for a USB 2.0 high speed device.
//
#ifndef USBH_MSD_MAX_TRANSFER_SIZE
  #define USBH_MSD_MAX_TRANSFER_SIZE    (64u * 1024u) // [bytes]
#endif
//
// Specifies the default sector size in bytes to be used for reading and writing.
//
#ifndef USBH_MSD_DEFAULT_SECTOR_SIZE
  #define USBH_MSD_DEFAULT_SECTOR_SIZE  512u
#endif
//
// The host controller waits this time after reset of a root hub port, before the device descriptor is requested or
// the Set Address command is sent.
//
#ifndef USBH_WAIT_AFTER_RESET
  #define USBH_WAIT_AFTER_RESET        180
#endif
//
// The host controller waits this time after reset of a external hub port, before the device descriptor is requested or
// the Set Address command is sent.
//
#ifndef USBH_HUB_WAIT_AFTER_RESET
  #define USBH_HUB_WAIT_AFTER_RESET    180u
#endif
//
// The USB stack waits this time before the next command is sent after Set Address. The device must answer to SetAddress on USB address 0 with the
// handshake and than set the new address. This is a potential racing condition if this step is performed in the firmware.
// Give the device this time to set the new address.
//
#ifndef WAIT_AFTER_SETADDRESS
  #define WAIT_AFTER_SETADDRESS         30u
#endif
//
// If an error is encountered during USB reset, set address or enumeration the process is repeated USBH_RESET_RETRY_COUNTER times
// before the port is finally disabled.
//
#ifndef USBH_RESET_RETRY_COUNTER
  #define USBH_RESET_RETRY_COUNTER       5u
#endif
//
// Describes the time before a USB reset is restarted,
// after the enumeration of the device (get descriptors, set configuration) has failed.
//
#ifndef USBH_DELAY_FOR_REENUM
  #define USBH_DELAY_FOR_REENUM       1000u
#endif
//
// The default size of the buffer to get descriptors from the device. If the buffer is too small for the configuration descriptor,
// a new buffer is dynamically allocated.
//
#define DEFAULT_TRANSFERBUFFER_SIZE     64u
//
// Default timeout for all setup requests. After this time a not completed setup request is terminated.
// Windows gives 2 sec to answer to a setup request. Less than that some devices behave quite strange.
// So we will also use such a "big" time-out.
//
#define DEFAULT_SETUP_TIMEOUT         2000
//
// On default, enumeration for multiple devices may be processed in parallel.
// Setting USBH_DELAY_BETWEEN_ENUMERATIONS > 0 will serialize all enumerations using a delay
// before a new enumeration is performed.
//
#ifndef USBH_DELAY_BETWEEN_ENUMERATIONS
  #define USBH_DELAY_BETWEEN_ENUMERATIONS   0u
#endif

//
// In order to avoid warnings for undefined parameters
//
#ifndef USBH_USE_PARA
  #if defined(NC30) || defined(NC308)
    #define USBH_USE_PARA(para)
  #else
    #define USBH_USE_PARA(para) (void)para;
  #endif
#endif

#endif

/*************************** End of file ****************************/
