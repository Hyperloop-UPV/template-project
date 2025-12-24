#include "main.h"
#include "stm32h7xx_it.h"

/* External variables --------------------------------------------------------*/
// SOLO mantenemos la de Ethernet si la hubiera, pero normalmente HAL la busca sola.
// Borramos todas las referencias externas a hdma_adc, hi2c, hspi, etc.

/******************************************************************************/
/* Cortex-M7 Processor Interruption and Exception Handlers          */
/******************************************************************************/

void NMI_Handler(void)
{
  while (1) {}
}

void HardFault_Handler(void)
{
  while (1) {}
}

void MemManage_Handler(void)
{
  while (1) {}
}

void BusFault_Handler(void)
{
  while (1) {}
}

void UsageFault_Handler(void)
{
  while (1) {}
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/******************************************************************************/

/* MANTENER SOLO ESTA (ETHERNET) */
extern ETH_HandleTypeDef heth;

void ETH_IRQHandler(void)
{
  HAL_ETH_IRQHandler(&heth);
}

/* ¡IMPORTANTE!
   Hemos borrado todas las funciones DMAx_Streamx_IRQHandler y TIMx_IRQHandler 
   porque daban error al no existir ya sus handles.
*/