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
File    : Main.c
Purpose : Generic SEGGER application start
--------  END-OF-HEADER  ---------------------------------------------
*/
#include "stm32f4xx_hal.h"
#include "gpio.h"


#ifndef USE_SYSVIEW
  #define USE_SYSVIEW 0
#endif

#ifndef USE_RTT
  #define USE_RTT 0
#endif

#ifndef DEBUG
  #define DEBUG 0
#endif

#include "RTOS.h"
#include "BSP.h"
#if USE_SYSVIEW > 0
  #include "SEGGER_SYSVIEW.h"
#endif

#ifndef MAIN_TASK_STACK_SIZE
  #define MAIN_TASK_STACK_SIZE 512 // in words
#endif

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
#ifdef __cplusplus
extern "C" {    /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
#ifdef __cplusplus
}
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static OS_STACKPTR int Stack0[MAIN_TASK_STACK_SIZE];   /* Task stack */
static OS_TASK         TCB0;                  /* Task-control-block */

/*********************************************************************
*
*       main()
*
* Function description
*   Application entry point
*/
#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

void SystemClock_Config(void)
{
  printf("SystemClock_Config\n");
    
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

int main(void) 
{
  //SystemInit(); ///////!!!!!!!
  SystemClock_Config();
  
  OS_IncDI();                      /* Initially disable interrupts  */
  
  OS_InitKern();                   /* Initialize OS                 */
  OS_InitHW();                     /* Initialize Hardware for OS    */

  BSP_Init();                      /* Initialize BSP module         */
  //SystemClock_Config();
  BSP_SetLED(0);                     /* Initially set LED             */
  MX_GPIO_Init();
  /* You need to create at least one task before calling OS_Start() */
  OS_CREATETASK(&TCB0, "MainTask", MainTask, 100, Stack0);
  OS_Start();                      /* Start multitasking            */

  return 0;
}

/*********************************************************************
*
*       USBH_Panic
*
*  Function description
*    Is called if the stack encounters a critical situation.
*    In a release build, this function is not be linked in.
*
*  Parameters
*    s - Pointer to a string holding the error message.
*/
#if USE_RTT == 0 && DEBUG > 0
void USBH_Panic(const char * s);
void USBH_Panic(const char * s) {
  while (s);
}
#endif

/*********************************************************************
*
*       SEGGER_SYSVIEW_Conf
*
*  Function description
*    Dummy function, if not using SYSVIEW
*/
#if USE_SYSVIEW == 0
void SEGGER_SYSVIEW_Conf(void);
void SEGGER_SYSVIEW_Conf(void) {
}
#endif

/****** End Of File *************************************************/
