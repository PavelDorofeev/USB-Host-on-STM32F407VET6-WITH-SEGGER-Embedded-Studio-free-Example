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
File    : BSP.h
Purpose : BSP (Board support package)
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef BSP_H                           /* Avoid multiple inclusion */
#define BSP_H

/*********************************************************************
*
*       Defines, non-configurable
*
**********************************************************************
*/

/* In order to avoid warnings for undefined parameters */
#ifndef BSP_USE_PARA
  #if defined(NC30) || defined(NC308)
    #define BSP_USE_PARA(para)
  #else
    #define BSP_USE_PARA(para) para=para
  #endif
#endif

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       General
*/
void BSP_Init      (void);
void BSP_SetLED    (int Index);
void BSP_ClrLED    (int Index);
void BSP_ToggleLED (int Index);
void BSP_SDRAM_Init(void);

#ifdef __cplusplus
}
#endif

#endif                                  /* avoid multiple inclusion */

/****** End Of File *************************************************/
