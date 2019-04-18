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
File    : OS_Config.h
Purpose : Configuration settings for the OS build and embOSView
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef OS_CONFIG_H                     /* Avoid multiple inclusion */
#define OS_CONFIG_H

/*********************************************************************
*
*       Configuration for RTOS build and embOSView communication
*
*  One of the following builds needs to be selected for both Debug and Release configuration:
*
*  OS_LIBMODE_XR    Extremely small release build without Round robin
*  OS_LIBMODE_R     Release build
*  OS_LIBMODE_S     Release build with stack check
*  OS_LIBMODE_SP    Release build with stack check and profiling
*  OS_LIBMODE_D     Debug build
*  OS_LIBMODE_DP    Debug build with profiling
*  OS_LIBMODE_DT    Debug build with trace
*/

#if (defined(DEBUG) && (DEBUG == 1))
  #define OS_LIBMODE_DP
#else
  #define OS_LIBMODE_R
  #define OS_VIEW_IFSELECT  OS_VIEW_DISABLED  // embOSView communication is disabled per default in release configuration
#endif

/*********************************************************************
*
*       Additional embOS compile time configuration defines when using
*       embOS sources in your project or rebuilding the embOS libraries
*       can be added here, e.g.:
*       #define OS_SUPPORT_TICKLESS  0  // Disable tickless support
*/

#endif                                  /* Avoid multiple inclusion */

/****** End Of File *************************************************/
