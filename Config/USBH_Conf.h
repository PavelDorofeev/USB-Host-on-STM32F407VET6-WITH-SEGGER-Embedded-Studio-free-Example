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
File    : USBH_Conf.h
Purpose : Config file. Modify to reflect your configuration
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef USBH_CONF_H      // Avoid multiple inclusion

#define USBH_CONF_H

#if defined(__cplusplus) // Make sure we have C-declarations in C++ programs
  extern "C" {
#endif

#ifdef DEBUG
#if DEBUG
  #define USBH_DEBUG   2// Debug level: 1: Support "Panic" checks, 2: Support warn & log
#endif
#endif


// Make sure we have C-declarations in C++ programs
#if defined(__cplusplus)

}

#endif

#endif // Avoid multiple inclusion

/*************************** End of file ****************************/
