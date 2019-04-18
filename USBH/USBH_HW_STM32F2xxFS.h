/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2019     SEGGER Microcontroller GmbH              *
*                                                                    *
*       www.segger.com     Support: www.segger.com/ticket            *
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
*       emUSB-Host version: V2.20                                    *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File        : USBH_HW_STM32F2xxFS.h
Purpose     : Header for the STM32F2xx/F4xx FullSpeed emUSB Host driver
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef USBH_HW_STM32F2XX_FS_H_
#define USBH_HW_STM32F2XX_FS_H_

#include "SEGGER.h"

#if defined(__cplusplus)
  extern "C" {                 // Make sure we have C-declarations in C++ programs
#endif

#define USBH_STM32F4_FS_Add(pBase)                              USBH_STM32F2_FS_Add(pBase)

U32            USBH_STM32F2_FS_Add(void * pBase);

#if defined(__cplusplus)
  }
#endif

#endif // USBH_HW_STM32F2XX_FS_H_

/*************************** End of file ****************************/
