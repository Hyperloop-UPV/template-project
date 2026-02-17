/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file         stm32h7xx_hal_msp.c
 * @brief        This file provides code for the MSP Initialization
 *               and de-Initialization codes.
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
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
extern DMA_HandleTypeDef hdma_i2c2_rx;

extern DMA_HandleTypeDef hdma_i2c2_tx;

extern DMA_HandleTypeDef hdma_fmac_preload;
extern DMA_HandleTypeDef hdma_fmac_read;
extern DMA_HandleTypeDef hdma_fmac_write;

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim);
/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void) {
    /* USER CODE BEGIN MspInit 0 */

    /* USER CODE END MspInit 0 */

    __HAL_RCC_SYSCFG_CLK_ENABLE();

    /* System interrupt init*/

    /* USER CODE BEGIN MspInit 1 */

    /* USER CODE END MspInit 1 */
}

/**
 * @brief ADC MSP Initialization
 * This function configures the hardware resources used in this
 * example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc) {
    (void)hadc;
    /* ADC MSP is handled by HALAL/Services/ADC/NewADC.hpp */
}

/**
 * @brief ADC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc) {
    (void)hadc;
    /* ADC MSP is handled by HALAL/Services/ADC/NewADC.hpp */
}

/**
 * @brief CORDIC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hcordic: CORDIC handle pointer
 * @retval None
 */
void HAL_CORDIC_MspInit(CORDIC_HandleTypeDef* hcordic) {
    if (hcordic->Instance == CORDIC) {
        /* USER CODE BEGIN CORDIC_MspInit 0 */

        /* USER CODE END CORDIC_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_CORDIC_CLK_ENABLE();
        /* USER CODE BEGIN CORDIC_MspInit 1 */

        /* USER CODE END CORDIC_MspInit 1 */
    }
}

/**
 * @brief CORDIC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hcordic: CORDIC handle pointer
 * @retval None
 */
void HAL_CORDIC_MspDeInit(CORDIC_HandleTypeDef* hcordic) {
    if (hcordic->Instance == CORDIC) {
        /* USER CODE BEGIN CORDIC_MspDeInit 0 */

        /* USER CODE END CORDIC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_CORDIC_CLK_DISABLE();
        /* USER CODE BEGIN CORDIC_MspDeInit 1 */

        /* USER CODE END CORDIC_MspDeInit 1 */
    }
}

static uint32_t HAL_RCC_FDCAN_CLK_ENABLED = 0;

/**
 * @brief FDCAN MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hfdcan: FDCAN handle pointer
 * @retval None
 */
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hfdcan->Instance == FDCAN1) {
        /* USER CODE BEGIN FDCAN1_MspInit 0 */

        /* USER CODE END FDCAN1_MspInit 0 */
        /* Peripheral clock enable */
        HAL_RCC_FDCAN_CLK_ENABLED++;
        if (HAL_RCC_FDCAN_CLK_ENABLED == 1) {
            __HAL_RCC_FDCAN_CLK_ENABLE();
        }

        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**FDCAN1 GPIO Configuration
        PD0     ------> FDCAN1_RX
        PD1     ------> FDCAN1_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* USER CODE BEGIN FDCAN1_MspInit 1 */
        HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
        HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
        /* USER CODE END FDCAN1_MspInit 1 */
    } else if (hfdcan->Instance == FDCAN3) {
        /* USER CODE BEGIN FDCAN3_MspInit 0 */

        /* USER CODE END FDCAN3_MspInit 0 */
        /* Peripheral clock enable */
        HAL_RCC_FDCAN_CLK_ENABLED++;
        if (HAL_RCC_FDCAN_CLK_ENABLED == 1) {
            __HAL_RCC_FDCAN_CLK_ENABLE();
        }

        __HAL_RCC_GPIOG_CLK_ENABLE();
        /**FDCAN3 GPIO Configuration
        PG9     ------> FDCAN3_TX
        PG10     ------> FDCAN3_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_FDCAN3;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

        /* USER CODE BEGIN FDCAN3_MspInit 1 */
        HAL_NVIC_SetPriority(FDCAN3_IT0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(FDCAN3_IT0_IRQn);
        HAL_NVIC_SetPriority(FDCAN3_IT1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(FDCAN3_IT1_IRQn);
        /* USER CODE END FDCAN3_MspInit 1 */
    }
}

/**
 * @brief FDCAN MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hfdcan: FDCAN handle pointer
 * @retval None
 */
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan) {
    if (hfdcan->Instance == FDCAN1) {
        /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

        /* USER CODE END FDCAN1_MspDeInit 0 */
        /* Peripheral clock disable */
        HAL_RCC_FDCAN_CLK_ENABLED--;
        if (HAL_RCC_FDCAN_CLK_ENABLED == 0) {
            __HAL_RCC_FDCAN_CLK_DISABLE();
        }

        /**FDCAN1 GPIO Configuration
        PD0     ------> FDCAN1_RX
        PD1     ------> FDCAN1_TX
        */
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0 | GPIO_PIN_1);

        /* USER CODE BEGIN FDCAN1_MspDeInit 1 */
        HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
        HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);
        /* USER CODE END FDCAN1_MspDeInit 1 */
    } else if (hfdcan->Instance == FDCAN3) {
        /* USER CODE BEGIN FDCAN3_MspDeInit 0 */

        /* USER CODE END FDCAN3_MspDeInit 0 */
        /* Peripheral clock disable */
        HAL_RCC_FDCAN_CLK_ENABLED--;
        if (HAL_RCC_FDCAN_CLK_ENABLED == 0) {
            __HAL_RCC_FDCAN_CLK_DISABLE();
        }

        /**FDCAN3 GPIO Configuration
        PG9     ------> FDCAN3_TX
        PG10     ------> FDCAN3_RX
        */
        HAL_GPIO_DeInit(GPIOG, GPIO_PIN_9 | GPIO_PIN_10);

        /* USER CODE BEGIN FDCAN3_MspDeInit 1 */
        HAL_NVIC_DisableIRQ(FDCAN3_IT0_IRQn);
        HAL_NVIC_DisableIRQ(FDCAN3_IT1_IRQn);
        /* USER CODE END FDCAN3_MspDeInit 1 */
    }
}

/**
 * @brief FMAC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hfmac: FMAC handle pointer
 * @retval None
 */
void HAL_FMAC_MspInit(FMAC_HandleTypeDef* hfmac) {
    if (hfmac->Instance == FMAC) {
        __HAL_RCC_FMAC_CLK_ENABLE();

        /* FMAC DMA Init */
        /* FMAC_PRELOAD Init */
        hdma_fmac_preload.Instance = DMA2_Stream0;
        hdma_fmac_preload.Init.Request = DMA_REQUEST_MEM2MEM;
        hdma_fmac_preload.Init.Direction = DMA_MEMORY_TO_MEMORY;
        hdma_fmac_preload.Init.PeriphInc = DMA_PINC_ENABLE;
        hdma_fmac_preload.Init.MemInc = DMA_MINC_DISABLE;
        hdma_fmac_preload.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_fmac_preload.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
        hdma_fmac_preload.Init.Mode = DMA_NORMAL;
        hdma_fmac_preload.Init.Priority = DMA_PRIORITY_HIGH;
        if (HAL_DMA_Init(&hdma_fmac_preload) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(hfmac, hdmaPreload, hdma_fmac_preload);

        /* FMAC_WRITE Init */
        hdma_fmac_write.Instance = DMA2_Stream1;
        hdma_fmac_write.Init.Request = DMA_REQUEST_FMAC_WRITE;
        hdma_fmac_write.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_fmac_write.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_fmac_write.Init.MemInc = DMA_MINC_ENABLE;
        hdma_fmac_write.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_fmac_write.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
        hdma_fmac_write.Init.Mode = DMA_NORMAL;
        hdma_fmac_write.Init.Priority = DMA_PRIORITY_HIGH;
        if (HAL_DMA_Init(&hdma_fmac_write) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(hfmac, hdmaIn, hdma_fmac_write);

        /* FMAC interrupt Init */

        /* FMAC DMA Init */
        /* FMAC_READ Init */
        hdma_fmac_read.Instance = DMA2_Stream2;
        hdma_fmac_read.Init.Request = DMA_REQUEST_FMAC_READ;
        hdma_fmac_read.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_fmac_read.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_fmac_read.Init.MemInc = DMA_MINC_ENABLE;
        hdma_fmac_read.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_fmac_read.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
        hdma_fmac_read.Init.Mode = DMA_NORMAL;
        hdma_fmac_read.Init.Priority = DMA_PRIORITY_HIGH;
        if (HAL_DMA_Init(&hdma_fmac_read) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(hfmac, hdmaOut, hdma_fmac_read);

        HAL_NVIC_SetPriority(FMAC_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(FMAC_IRQn);
    }
}

/**
 * @brief FMAC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hfmac: FMAC handle pointer
 * @retval None
 */
void HAL_FMAC_MspDeInit(FMAC_HandleTypeDef* hfmac) {
    if (hfmac->Instance == FMAC) {
        /* USER CODE BEGIN FMAC_MspDeInit 0 */

        /* USER CODE END FMAC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_FMAC_CLK_DISABLE();
        /* USER CODE BEGIN FMAC_MspDeInit 1 */

        /* USER CODE END FMAC_MspDeInit 1 */
    }
}

/**
 * @brief I2C MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (hi2c->Instance == I2C2) {
        /* USER CODE BEGIN I2C2_MspInit 0 */

        /* USER CODE END I2C2_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
        PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C1235CLKSOURCE_D2PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**I2C2 GPIO Configuration
        PF1     ------> I2C2_SCL
        PB11     ------> I2C2_SDA
        */
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* Peripheral clock enable */
        __HAL_RCC_I2C2_CLK_ENABLE();

        /* I2C2 DMA Init */
        /* I2C2_RX Init */
        hdma_i2c2_rx.Instance = DMA1_Stream3;
        hdma_i2c2_rx.Init.Request = DMA_REQUEST_I2C2_RX;
        hdma_i2c2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_i2c2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_i2c2_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_i2c2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_i2c2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        hdma_i2c2_rx.Init.Mode = DMA_CIRCULAR;
        hdma_i2c2_rx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_i2c2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hdma_i2c2_rx) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(hi2c, hdmarx, hdma_i2c2_rx);

        /* I2C2_TX Init */
        hdma_i2c2_tx.Instance = DMA1_Stream4;
        hdma_i2c2_tx.Init.Request = DMA_REQUEST_I2C2_TX;
        hdma_i2c2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_i2c2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_i2c2_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_i2c2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_i2c2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        hdma_i2c2_tx.Init.Mode = DMA_CIRCULAR;
        hdma_i2c2_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_i2c2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hdma_i2c2_tx) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(hi2c, hdmatx, hdma_i2c2_tx);

        /* I2C2 interrupt Init */
        HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
        /* USER CODE BEGIN I2C2_MspInit 1 */

        /* USER CODE END I2C2_MspInit 1 */
    }
}

/**
 * @brief I2C MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c) {
    if (hi2c->Instance == I2C2) {
        /* USER CODE BEGIN I2C2_MspDeInit 0 */

        /* USER CODE END I2C2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_I2C2_CLK_DISABLE();

        /**I2C2 GPIO Configuration
        PF1     ------> I2C2_SCL
        PB11     ------> I2C2_SDA
        */
        HAL_GPIO_DeInit(GPIOF, GPIO_PIN_1);

        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11);

        /* I2C2 DMA DeInit */
        HAL_DMA_DeInit(hi2c->hdmarx);
        HAL_DMA_DeInit(hi2c->hdmatx);

        /* I2C2 interrupt DeInit */
        HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
        /* USER CODE BEGIN I2C2_MspDeInit 1 */

        /* USER CODE END I2C2_MspDeInit 1 */
    }
}

/**
 * @brief LPTIM MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hlptim: LPTIM handle pointer
 * @retval None
 */
void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* hlptim) {
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (hlptim->Instance == LPTIM1) {
        /* USER CODE BEGIN LPTIM1_MspInit 0 */

        /* USER CODE END LPTIM1_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
        PeriphClkInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_D2PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_LPTIM1_CLK_ENABLE();
        /* LPTIM1 interrupt Init */
        HAL_NVIC_SetPriority(LPTIM1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
        /* USER CODE BEGIN LPTIM1_MspInit 1 */

        /* USER CODE END LPTIM1_MspInit 1 */
    } else if (hlptim->Instance == LPTIM2) {
        /* USER CODE BEGIN LPTIM2_MspInit 0 */

        /* USER CODE END LPTIM2_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM2;
        PeriphClkInitStruct.Lptim2ClockSelection = RCC_LPTIM2CLKSOURCE_D3PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_LPTIM2_CLK_ENABLE();
        /* LPTIM2 interrupt Init */
        HAL_NVIC_SetPriority(LPTIM2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(LPTIM2_IRQn);
        /* USER CODE BEGIN LPTIM2_MspInit 1 */

        /* USER CODE END LPTIM2_MspInit 1 */
    } else if (hlptim->Instance == LPTIM3) {
        /* USER CODE BEGIN LPTIM3_MspInit 0 */

        /* USER CODE END LPTIM3_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM3;
        PeriphClkInitStruct.Lptim345ClockSelection = RCC_LPTIM345CLKSOURCE_D3PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_LPTIM3_CLK_ENABLE();
        /* LPTIM3 interrupt Init */
        HAL_NVIC_SetPriority(LPTIM3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(LPTIM3_IRQn);
        /* USER CODE BEGIN LPTIM3_MspInit 1 */

        /* USER CODE END LPTIM3_MspInit 1 */
    }
}

/**
 * @brief LPTIM MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hlptim: LPTIM handle pointer
 * @retval None
 */
void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* hlptim) {
    if (hlptim->Instance == LPTIM1) {
        /* USER CODE BEGIN LPTIM1_MspDeInit 0 */

        /* USER CODE END LPTIM1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_LPTIM1_CLK_DISABLE();

        /* LPTIM1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(LPTIM1_IRQn);
        /* USER CODE BEGIN LPTIM1_MspDeInit 1 */

        /* USER CODE END LPTIM1_MspDeInit 1 */
    } else if (hlptim->Instance == LPTIM2) {
        /* USER CODE BEGIN LPTIM2_MspDeInit 0 */

        /* USER CODE END LPTIM2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_LPTIM2_CLK_DISABLE();

        /* LPTIM2 interrupt DeInit */
        HAL_NVIC_DisableIRQ(LPTIM2_IRQn);
        /* USER CODE BEGIN LPTIM2_MspDeInit 1 */

        /* USER CODE END LPTIM2_MspDeInit 1 */
    } else if (hlptim->Instance == LPTIM3) {
        /* USER CODE BEGIN LPTIM3_MspDeInit 0 */

        /* USER CODE END LPTIM3_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_LPTIM3_CLK_DISABLE();

        /* LPTIM3 interrupt DeInit */
        HAL_NVIC_DisableIRQ(LPTIM3_IRQn);
        /* USER CODE BEGIN LPTIM3_MspDeInit 1 */

        /* USER CODE END LPTIM3_MspDeInit 1 */
    }
}

/**
 * @brief RTC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc) {
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (hrtc->Instance == RTC) {
        /* USER CODE BEGIN RTC_MspInit 0 */

        /* USER CODE END RTC_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_RTC_ENABLE();
        /* USER CODE BEGIN RTC_MspInit 1 */

        /* USER CODE END RTC_MspInit 1 */
    }
}

/**
 * @brief RTC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc) {
    if (hrtc->Instance == RTC) {
        /* USER CODE BEGIN RTC_MspDeInit 0 */

        /* USER CODE END RTC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_RTC_DISABLE();
        /* USER CODE BEGIN RTC_MspDeInit 1 */

        /* USER CODE END RTC_MspDeInit 1 */
    }
}

/**
 * @brief UART MSP Initialization
 * This function configures the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (huart->Instance == USART1) {
        /* USER CODE BEGIN USART1_MspInit 0 */

        /* USER CODE END USART1_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
        PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16910CLKSOURCE_D2PCLK2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USER CODE BEGIN USART1_MspInit 1 */

        /* USER CODE END USART1_MspInit 1 */
    } else if (huart->Instance == USART2) {
        /* USER CODE BEGIN USART2_MspInit 0 */

        /* USER CODE END USART2_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2;
        PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**USART2 GPIO Configuration
        PD5     ------> USART2_TX
        PD6     ------> USART2_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* USER CODE BEGIN USART2_MspInit 1 */

        /* USER CODE END USART2_MspInit 1 */
    }
}

/**
 * @brief UART MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart) {
    if (huart->Instance == USART1) {
        /* USER CODE BEGIN USART1_MspDeInit 0 */

        /* USER CODE END USART1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();

        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

        /* USER CODE BEGIN USART1_MspDeInit 1 */

        /* USER CODE END USART1_MspDeInit 1 */
    } else if (huart->Instance == USART2) {
        /* USER CODE BEGIN USART2_MspDeInit 0 */

        /* USER CODE END USART2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /**USART2 GPIO Configuration
        PD5     ------> USART2_TX
        PD6     ------> USART2_RX
        */
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5 | GPIO_PIN_6);

        /* USER CODE BEGIN USART2_MspDeInit 1 */

        /* USER CODE END USART2_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
