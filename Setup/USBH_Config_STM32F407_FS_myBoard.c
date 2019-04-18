
/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/
#include <stdlib.h>
#include "USBH.h"
#include "BSP_USB.h"
#include "USBH_HW_STM32F2xxFS.h" //// ok
#include "stm32f4xx.h"
#include "gpio.h"
//#include "usbh_core.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define USB_ISR_ID    (67)
#define USB_ISR_PRIO  254
#define ALLOC_SIZE             0xf000      // Size of memory dedicated to the stack in bytes
#define STM32_OTG_BASE_ADDRESS 0x50000000UL ////!!!! OK

//
// RCC
//
#define RCC_BASE_ADDR             ((unsigned int)(0x40023800))
#define RCC_CR                    (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x00))
#define RCC_AHB1RSTR              (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x10))
#define RCC_AHB2RSTR              (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x14))
#define RCC_AHB1ENR               (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x30))
#define RCC_AHB2ENR               (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x34))
#define RCC_PLLSAICFGR            (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x88))
#define RCC_DCKCFGR2              (*(volatile unsigned int*)(RCC_BASE_ADDR + 0x90))
//
// GPIO
//
#define GPIOA_BASE_ADDR           ((unsigned int)0x40020000)
#define GPIOA_MODER               (*(volatile unsigned int*)(GPIOA_BASE_ADDR + 0x00))
#define GPIOA_AFRL                (*(volatile unsigned int*)(GPIOA_BASE_ADDR + 0x20))
#define GPIOA_AFRH                (*(volatile unsigned int*)(GPIOA_BASE_ADDR + 0x24))

#define GPIOD_BASE_ADDR           ((unsigned int)0x40020C00)
#define GPIOD_MODER               (*(volatile unsigned int*)(GPIOD_BASE_ADDR + 0x00))
#define GPIOD_BSRR                (*(volatile unsigned int*)(GPIOD_BASE_ADDR + 0x18))
#define GPIOD_AFRL                (*(volatile unsigned int*)(GPIOD_BASE_ADDR + 0x20))
#define GPIOD_AFRH                (*(volatile unsigned int*)(GPIOD_BASE_ADDR + 0x24))

#define OTG_FS_GOTGCTL            (*(volatile unsigned int*)(0x50000000))

#define OTG_FS_GOTTGCTL_AVALOVAL  (1UL << 5)  // A-peripheral session valid override value
#define OTG_FS_GOTTGCTL_AVALOEN   (1UL << 4)  // A-peripheral session valid override enable
#define OTG_FS_GOTTGCTL_VBVALOVAL (1UL << 3)  // VBUS valid override value.
#define OTG_FS_GOTTGCTL_VBVALOEN  (1UL << 2)  // VBUS valid override enable.

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U32 _aPool [((ALLOC_SIZE) / 4)];

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       _InitUSBHw
*
*/
static void _InitUSBHw(void) 
{
  U32 v;

    /* Peripheral clock enable */
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();

    /* Peripheral interrupt init */
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);


/**
  You may make as below

    USB_OTG_FS GPIO Configuration    
    PA9     ------> USB_OTG_FS_VBUS
    PA11     ------> USB_OTG_FS_DM
    PA12     ------> USB_OTG_FS_DP 
   
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    
*/
 
  // OR you can make as below

  // OTGFSEN  USB OTG FS clock enable !!!!!!
  RCC_AHB2ENR |= 0
              | (1 <<  7)  // OTGFSEN: Enable USB OTG FS clock enable
              ;
  
  // Set PA10 (OTG_FS_ID) as alternate function
  v           = GPIOA_MODER;
  v          &= ~(0x3uL << (2 * 10));
  v          |=  (0x2uL << (2 * 10));  // A10 = режим 10 
  GPIOA_MODER = v;

  v           = GPIOA_AFRH;
  v          &= ~(0xFuL << (4 * 2)); // 1111 << 8
  v          |=  (0xAuL << (4 * 2)); // 1010 = 10 =  AF10 = OTG_FS_ID
  GPIOA_AFRH  = v;

  
  // Set PA11 (OTG_FS_DM) as alternate function
  v           = GPIOA_MODER;
  v          &= ~(0x3uL << (2 * 11)); 
  v          |=  (0x2uL << (2 * 11)); // A11 = mode 10 = 
  GPIOA_MODER = v;

  v           = GPIOA_AFRH;
  v          &= ~(0xFuL << (4 * 3));
  v          |=  (0xAuL << (4 * 3)); // 1010 = 10 = AF10 = OTG_FS_DM 
  GPIOA_AFRH  = v;

  
  // Set PA12 (OTG_FS_DP) as alternate function
  v           = GPIOA_MODER;
  v          &= ~(0x3uL << (2 * 12));
  v          |=  (0x2uL << (2 * 12)); // A12 = mode 10 = 
  GPIOA_MODER = v;
  v           = GPIOA_AFRH;
  v          &= ~(0xFuL << (4 * 4));
  v          |=  (0xAuL << (4 * 4)); // 1010 = 10 = AF10 = OTG_FS_DP 
  GPIOA_AFRH  = v;
  

  OTG_FS_GOTGCTL |= 0
                 |  OTG_FS_GOTTGCTL_AVALOVAL
                 |  OTG_FS_GOTTGCTL_AVALOEN
                 |  OTG_FS_GOTTGCTL_VBVALOVAL
                 |  OTG_FS_GOTTGCTL_VBVALOEN;

}

/*********************************************************************
*
*       _OnPortPowerControl
*/
static void _OnPortPowerControl(U32 HostControllerIndex, U8 Port, U8 PowerOn) 
{
  USBH_USE_PARA(HostControllerIndex);
  USBH_USE_PARA(Port);

  if (PowerOn) 
  {
    GPIOD_BSRR  = ((1ul << 5) << 16); // Set pin low to enable VBUS.
  } 
  else 
  {
    GPIOD_BSRR  = ((1ul << 5) <<  0); // Set pin high to disable VBUS.
  }
}

/*********************************************************************
*
*       _ISR
*
*  Function description
*/
static void _ISR(void) 
{
  USBH_ServiceISR(0);
}

/*********************************************************************
*
*       USBH_X_Config
*
*  Function description
*/
void USBH_X_Config(void) 
{

 /* printf("  ---USBH_X_Config --\n"); 
  printf("  RCC_CFGR_SWS 0x%x\n",RCC_CFGR_SWS); 
  printf("  tmp 0x%x\n",RCC->CFGR & RCC_CFGR_SWS); 
  printf("  SystemCoreClock %d\n",SystemCoreClock); 
  printf("  RCC_PLLCFGR_PLLN %d\n",RCC_PLLCFGR_PLLN); 
  //printf("  HSE_VALUE %d\n",HSE_VALUE);*/


  USBH_AssignMemory(&_aPool[0], ALLOC_SIZE);    // Assigning memory should be the first thing

  USBH_ConfigSupportExternalHubs (1);           // Default values: The hub module is disabled, this is done to save memory.
  USBH_ConfigPowerOnGoodTime     (300);         // Default values: 300 ms wait time before the host starts communicating with a device.
  //
  // Define log and warn filter
  // Note: The terminal I/O emulation affects the timing
  // of your communication, since the debugger stops the target
  // for every terminal I/O unless you use RTT!
  //
  USBH_SetWarnFilter(0xFFFFFFFF);               // 0xFFFFFFFF: Do not filter: Output all warnings.
  USBH_SetLogFilter(0
                    | USBH_MTYPE_INIT
                    | USBH_MTYPE_APPLICATION
                    );
  _InitUSBHw();

  ///USBH_STM32F7_FS_Add((void*)STM32_OTG_BASE_ADDRESS);
  USBH_STM32F2_FS_Add((void*)STM32_OTG_BASE_ADDRESS);
  //
  //  Please uncomment this function when using OTG functionality.
  //  Otherwise the VBUS power-on will be permanently on and will cause
  //  OTG to detect a session where no session is available.
  //
   USBH_SetOnSetPortPower(_OnPortPowerControl);                 // This function sets a callback which allows to control VBUS-Power of a USB port.
  _OnPortPowerControl(0, 0, 1);                                // Enable power on USB port
  BSP_USBH_InstallISR_Ex(USB_ISR_ID, _ISR, USB_ISR_PRIO);
  ///BSP_USBH_InstallISR_Ex(OTG_FS_IRQn, _ISR, USB_ISR_PRIO);
  
}
/*************************** End of file ****************************/
