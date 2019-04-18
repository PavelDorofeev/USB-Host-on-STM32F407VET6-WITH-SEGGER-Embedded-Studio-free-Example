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
File    : BSP.c
Purpose : BSP for SEGGER emPower V2 with Freescale K66 MCU
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "BSP_USB.h"
#include "SEGGER.h"
#include "RTOS.h"
#include "stm32f4xx.h"
//#include "stm32f7xx.h"     // Device specific header file, contains CMSIS

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

/*********************************************************************
*
*       Typedefs
*
**********************************************************************
*/
typedef void USB_ISR_HANDLER  (void);

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static USB_ISR_HANDLER * _pfOTG_FSHandler;
static USB_ISR_HANDLER * _pfOTG_HSHandler;

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
/*********************************************************************
*
*       USB
*
*  Functions for USB controllers (as far as present)
*/

/****** Declare ISR handler here to avoid "no prototype" warning. They are not declared in any CMSIS header */

#ifdef __cplusplus
extern "C" {
#endif
void OTG_FS_IRQHandler(void);
void OTG_HS_IRQHandler(void);
#ifdef __cplusplus
}
#endif

/*********************************************************************
*
*       OTG_FS_IRQHandler
*/
void OTG_FS_IRQHandler(void) 
{
  OS_EnterInterrupt(); // Inform embOS that interrupt code is running
  if (_pfOTG_FSHandler) {
    (_pfOTG_FSHandler)();
  }
  OS_LeaveInterrupt(); // Inform embOS that interrupt code is left
}

/*********************************************************************
*
*       OTG_HS_IRQHandler
*/
void OTG_HS_IRQHandler(void) 
{
  OS_EnterInterrupt(); // Inform embOS that interrupt code is running
  if (_pfOTG_HSHandler) {
    (_pfOTG_HSHandler)();
  }
  OS_LeaveInterrupt(); // Inform embOS that interrupt code is left
}

/*********************************************************************
*
*       BSP_USB_InstallISR_Ex()
*/
void BSP_USB_InstallISR_Ex(int ISRIndex, void (*pfISR)(void), int Prio)
{
  (void)Prio;
  if (ISRIndex == OTG_FS_IRQn) {
    _pfOTG_FSHandler = pfISR;
  }
  if (ISRIndex == OTG_HS_IRQn) {
    _pfOTG_HSHandler = pfISR;
  }
  NVIC_SetPriority((IRQn_Type)ISRIndex, (1u << __NVIC_PRIO_BITS) - 2u);
  NVIC_EnableIRQ((IRQn_Type)ISRIndex);
}

/*********************************************************************
*
*       BSP_USBH_InstallISR_Ex()
*/
void BSP_USBH_InstallISR_Ex(int ISRIndex, void (*pfISR)(void), int Prio){
  (void)Prio;
  if (ISRIndex == OTG_FS_IRQn) {
    _pfOTG_FSHandler = pfISR;
  }
  if (ISRIndex == OTG_HS_IRQn) {
    _pfOTG_HSHandler = pfISR;
  }
  NVIC_SetPriority((IRQn_Type)ISRIndex, (1u << __NVIC_PRIO_BITS) - 2u);
  NVIC_EnableIRQ((IRQn_Type)ISRIndex);
}

/****** End Of File *************************************************/
