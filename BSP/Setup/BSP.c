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
Purpose : BSP for STM32746G Discovery eval board
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "BSP.h"
//#include "stm32f7xx.h"  // Device specific header file, contains CMSIS
#include "stm32f4xx.h"

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       BSP_Init()
*/
void BSP_Init(void) 
{
  
  RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOIEN);
}

/*********************************************************************
*
*       BSP_SetLED()
*/
void BSP_SetLED(int Index) 
{
  if (Index == 0) 
  {
    GPIOI->BSRR = (0x00001u << 1u);  // Switch on LED
  }
}

/*********************************************************************
*
*       BSP_ClrLED()
*/
void BSP_ClrLED(int Index) 
{
  if (Index == 0) {
    GPIOI->BSRR = (0x10000u << 1u);  // Switch off LED
  }
}

/*********************************************************************
*
*       BSP_ToggleLED()
*/
void BSP_ToggleLED(int Index) 
{
  if (Index == 0) 
  {
    if (GPIOI->ODR & (1uL << 1)) {     // LED is switched off
      GPIOI->BSRR = (0x10000u << 1u);  // Switch off LED
    } else {
      GPIOI->BSRR = (0x00001u << 1u);  // Switch on LED
    }
  }
}

/****** End Of File *************************************************/
