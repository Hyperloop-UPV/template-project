/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32h7xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32h7xx_it.h"
#include "stm32h7xx_hal.h"
#include "HALAL/HardFault/HardfaultTrace.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern ETH_HandleTypeDef heth;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;
extern DMA_HandleTypeDef hdma_adc3;
extern DMA_HandleTypeDef hdma_i2c2_rx;
extern DMA_HandleTypeDef hdma_i2c2_tx;
extern I2C_HandleTypeDef hi2c2;
extern DMA_HandleTypeDef hdma_fmac_preload;
extern DMA_HandleTypeDef hdma_fmac_read;
extern DMA_HandleTypeDef hdma_fmac_write;
extern FMAC_HandleTypeDef hfmac;
extern LPTIM_HandleTypeDef hlptim1;
extern LPTIM_HandleTypeDef hlptim2;
extern LPTIM_HandleTypeDef hlptim3;
extern FDCAN_HandleTypeDef hfdcan1;
/*
Externs for calltrace
*/
extern uint32_t _stext;
extern uint32_t _etext;
extern uint32_t _sstack;
extern uint32_t _estack;
extern uint32_t _hf_stack_start;
extern uint32_t _hf_stack_end;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */

// calls my_fault_handler with the MSP(main stack pointer)
#define HARDFAULT_HANDLING_ASM()                                                                   \
    __asm__ __volatile__(/* Detect which stack was in use */                                       \
                         "tst lr, #4                \n"                                            \
                         "ite eq                    \n"                                            \
                         "mrseq r0, msp             \n"                                            \
                         "mrsne r0, psp             \n"                                            \
                                                                                                   \
                         /* Switch to dedicated HardFault stack */                                 \
                         "ldr r1, =_hf_stack_end    \n"                                            \
                         "msr msp, r1               \n"                                            \
                         "isb                       \n"                                            \
                                                                                                   \
                         /* Call C handler with original frame */                                  \
                         "b my_fault_handler_c      \n"                                            \
    )

// create the space for the hardfault section in the flash
__attribute__((section(".hardfault_log"))) volatile uint32_t hard_fault[128];

void hardfault_flash_write(
    uint32_t addr_hard_fault,
    const void* data_hard_fault,
    size_t len_hard_fault,
    uint32_t addr_metadata,
    const void* data_metadata,
    size_t len_metadata
) {
    HAL_FLASH_Unlock();

    // Erase sector
    FLASH_EraseInitTypeDef erase;
    uint32_t sector_error = 0;
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Banks = FLASH_BANK_1;
    erase.Sector = FLASH_SECTOR_6;
    erase.NbSectors = 1;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(&erase, &sector_error) != HAL_OK) {
        HAL_FLASH_Lock();
        return;
    }

    size_t offset, copy_len;
    uint8_t block[32];

    offset = 0;
    while (offset < len_hard_fault) {
        memset(block, 0xFF, sizeof(block));
        copy_len = (len_hard_fault - offset) > 32 ? 32 : (len_hard_fault - offset);
        memcpy(block, (uint8_t*)data_hard_fault + offset, copy_len);

        if (HAL_FLASH_Program(
                FLASH_TYPEPROGRAM_FLASHWORD,
                addr_hard_fault + offset,
                (uintptr_t)block
            ) != HAL_OK) {
            HAL_FLASH_Lock();
            return;
        }
        offset += 32;
    }

    offset = 0;
    while (offset < len_metadata) {
        memset(block, 0xFF, sizeof(block));
        copy_len = (len_metadata - offset) > 32 ? 32 : (len_metadata - offset);
        memcpy(block, (uint8_t*)data_metadata + offset, copy_len);

        if (HAL_FLASH_Program(
                FLASH_TYPEPROGRAM_FLASHWORD,
                addr_metadata + offset,
                (uintptr_t)block
            ) != HAL_OK) {
            HAL_FLASH_Lock();
            return;
        }
        offset += 32;
    }

    SCB_InvalidateICache();
    SCB_InvalidateDCache();

    HAL_FLASH_Lock();
}
static uint8_t is_valid_pc(uint32_t pc) {
    pc &= ~1U; // Thumb
    return (pc >= (uint32_t)&_stext && pc < (uint32_t)&_etext);
}
__attribute__((optimize("O0"))) static void
scan_call_stack(sContextStateFrame* frame, HardFaultLog* log_hard_fault) {
    uint32_t* stack_start = (uint32_t*)&_sstack;
    uint32_t* stack_end = (uint32_t*)&_estack;

    log_hard_fault->CallTrace.depth = 0;
    uint32_t* sp = (uint32_t*)(frame + 1);
    while (sp < stack_end && sp >= stack_start) {
        uint32_t val = *sp++;
        if (log_hard_fault->CallTrace.depth >= CALL_TRACE_MAX_DEPTH)
            break;
        if ((val & 1U) == 0)
            continue;
        if (!is_valid_pc(val))
            continue;
        log_hard_fault->CallTrace.pcs[log_hard_fault->CallTrace.depth++] = val & ~1U;
    }
}
__attribute__((noreturn, optimize("O0"))) void my_fault_handler_c(sContextStateFrame* frame) {
    volatile uint32_t real_fault_pc = frame->return_address & ~1;
    volatile HardFaultLog log_hard_fault;

    volatile uint32_t* cfsr = (volatile uint32_t*)0xE000ED28;
    // keep the log in the estructure
    log_hard_fault.HF_flag = HF_FLAG_VALUE;
    log_hard_fault.frame = *frame;
    log_hard_fault.frame.return_address = real_fault_pc;
    log_hard_fault.CfsrDecode.cfsr = *cfsr;
    log_hard_fault.fault_address.Nothing_Valid = 0;

    const uint8_t memory_fault = *cfsr & 0x000000ff;
    if (memory_fault) {
        const uint8_t MMARVALID =
            memory_fault & 0b10000000; // We can find the exact place were occured the memory fault
        const uint8_t MLSPERR = memory_fault & 0b00100000; // MemManage fault FPU stack
        const uint8_t MSTKERR =
            memory_fault & 0b00010000; // Stack overflow while entring an exception
        const uint8_t MUNSTKERR =
            memory_fault &
            0b00001000; // Stack error while exiting from an exception (Corrupted stack)
        const uint8_t DACCVIOL =
            memory_fault & 0b00000010; // Data access violation (acceded to pointer NULL, to a
                                       // protected memory region, overflow in arrays ...)
        const uint8_t IACCVIOL = memory_fault & 0b00000001; // Instruction access violation
        if (MMARVALID) {
            uint32_t memory_fault_address = *(volatile uint32_t*)0xE000ED34;
            log_hard_fault.fault_address.MMAR_VALID = memory_fault_address;
        }
    }
    const uint8_t bus_fault = (*cfsr & 0x0000ff00) >> 8;
    if (bus_fault) {
        const uint8_t BFARVALID =
            bus_fault &
            0b10000000; // BFAR is valid we can know the address which triggered the fault
        const uint8_t LSPERR = bus_fault & 0b00100000;   // Fault stack FPU
        const uint8_t STKERR = bus_fault & 0b00010000;   // Fault stack while entring an exception
        const uint8_t UNSTKERR = bus_fault & 0b00001000; // Stack error while exiting an exception
        const uint8_t IMPRECISERR =
            bus_fault &
            0b00000010; // Bus fault, but the instruction that caused the error can be uncertain
        const uint8_t PRECISERR =
            bus_fault &
            0b00000001; // You can read Bfar to find the eact direction of the instruction
        if (BFARVALID) {
            volatile uint32_t bus_fault_address = *(volatile uint32_t*)0xE000ED38;
            log_hard_fault.fault_address.BFAR_VALID = bus_fault_address;
            // Don't trust in case IMPRECISERR == 1;
        }
    }
    const uint16_t usage_fault = (*cfsr & 0xffff0000) >> 16;
    if (usage_fault) {
        const uint16_t DIVBYZERO = usage_fault & 0x0200;  // Div by ZERO hardfault;
        const uint16_t UNALIGNED = usage_fault & 0x0100;  // Unaligned access operation occured
        const uint16_t NOCP = usage_fault & 0x0008;       // Access to FPU when is not present
        const uint16_t INVPC = usage_fault & 0x0004;      // Invalid program counter load
        const uint16_t INVSTATE = usage_fault & 0x0002;   // Invalid processor state
        const uint16_t UNDEFINSTR = usage_fault & 0x0001; // Undefined instruction.
    }
    if (usage_fault | bus_fault) {
        scan_call_stack(frame, &log_hard_fault);
    }
    volatile uint8_t metadata_buffer[0x100];
    memcpy(metadata_buffer, (void*)METADATA_FLASH_ADDR, 0x100);
    // write log hard fault
    hardfault_flash_write(
        HF_FLASH_ADDR,
        (uint8_t*)&log_hard_fault,
        sizeof(log_hard_fault),
        METADATA_FLASH_ADDR,
        &metadata_buffer,
        sizeof(metadata_buffer)
    );

    // halt only when a debugger is attached, otherwise reboot.
    volatile uint32_t* dhcsr = (volatile uint32_t*)0xE000EDF0;
    if ((*dhcsr & 0x1U) != 0U) {
        __BKPT(0);
        while (1) {
        }
    }

    // Reboot the system in non-debug runs.
    volatile uint32_t* aircr = (volatile uint32_t*)0xE000ED0C;
    __asm volatile("dsb");
    *aircr = (0x05FA << 16) | 0x1 << 2;
    __asm volatile("dsb");
    while (1) {
    } // should be unreachable
}

__attribute__((naked)) void HardFault_Handler(void) {
    HARDFAULT_HANDLING_ASM();
    while (1) {
    }
}

void NMI_Handler(void) {
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */
    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
    while (1) {
    }
    /* USER CODE END NonMaskableInt_IRQn 1 */
}
/**
 * @brief This function handles Memory management fault.
 */
__attribute__((naked)) void MemManage_Handler(void) { HARDFAULT_HANDLING_ASM(); }
/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
__attribute__((naked)) void BusFault_Handler(void) { HARDFAULT_HANDLING_ASM(); }

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
__attribute__((naked)) void UsageFault_Handler(void) { HARDFAULT_HANDLING_ASM(); }

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void) {
    /* USER CODE BEGIN SVCall_IRQn 0 */

    /* USER CODE END SVCall_IRQn 0 */
    /* USER CODE BEGIN SVCall_IRQn 1 */

    /* USER CODE END SVCall_IRQn 1 */
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void) {
    /* USER CODE BEGIN DebugMonitor_IRQn 0 */

    /* USER CODE END DebugMonitor_IRQn 0 */
    /* USER CODE BEGIN DebugMonitor_IRQn 1 */

    /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void) {
    /* USER CODE BEGIN PendSV_IRQn 0 */

    /* USER CODE END PendSV_IRQn 0 */
    /* USER CODE BEGIN PendSV_IRQn 1 */

    /* USER CODE END PendSV_IRQn 1 */
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void) {
    /* USER CODE BEGIN SysTick_IRQn 0 */

    /* USER CODE END SysTick_IRQn 0 */
    HAL_IncTick();
    /* USER CODE BEGIN SysTick_IRQn 1 */

    /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles FDCAN 1 Line 0 interrupt
 */
void FDCAN1_IT0_IRQHandler(void) { HAL_FDCAN_IRQHandler(&hfdcan1); }

/**
 * @brief This function handles FDCAN 1 Line 1 interrupt
 */
void FDCAN1_IT1_IRQHandler(void) { HAL_FDCAN_IRQHandler(&hfdcan1); }

/**
 * @brief This function handles FDCAN 3 Line 0 interrupt
 */
void FDCAN3_IT0_IRQHandler(void) { HAL_FDCAN_IRQHandler(&hfdcan1); }

/**
 * @brief This function handles FDCAN 3 Line 1 interrupt
 */
void FDCAN3_IT1_IRQHandler(void) { HAL_FDCAN_IRQHandler(&hfdcan1); }

void FMAC_IRQHandler(void) { HAL_FMAC_IRQHandler(&hfmac); }

/**
 * @brief This function handles Ethernet global interrupt.
 */
void ETH_IRQHandler(void) {
    /* USER CODE BEGIN ETH_IRQn 0 */

    /* USER CODE END ETH_IRQn 0 */
    HAL_ETH_IRQHandler(&heth);
    /* USER CODE BEGIN ETH_IRQn 1 */

    /* USER CODE END ETH_IRQn 1 */
}

/**
 * @brief This function handles LPTIM1 global interrupt.
 */
void LPTIM1_IRQHandler(void) {
    /* USER CODE BEGIN LPTIM1_IRQn 0 */

    /* USER CODE END LPTIM1_IRQn 0 */
    HAL_LPTIM_IRQHandler(&hlptim1);
    /* USER CODE BEGIN LPTIM1_IRQn 1 */

    /* USER CODE END LPTIM1_IRQn 1 */
}

/**
 * @brief This function handles LPTIM2 global interrupt.
 */
void LPTIM2_IRQHandler(void) {
    /* USER CODE BEGIN LPTIM2_IRQn 0 */

    /* USER CODE END LPTIM2_IRQn 0 */
    HAL_LPTIM_IRQHandler(&hlptim2);
    /* USER CODE BEGIN LPTIM2_IRQn 1 */

    /* USER CODE END LPTIM2_IRQn 1 */
}

/**
 * @brief This function handles LPTIM3 global interrupt.
 */
void LPTIM3_IRQHandler(void) {
    /* USER CODE BEGIN LPTIM3_IRQn 0 */

    /* USER CODE END LPTIM3_IRQn 0 */
    HAL_LPTIM_IRQHandler(&hlptim3);
    /* USER CODE BEGIN LPTIM3_IRQn 1 */

    /* USER CODE END LPTIM3_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
