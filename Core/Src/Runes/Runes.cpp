#include "HALAL/HALAL.hpp"

DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;
DMA_HandleTypeDef hdma_adc3;
DMA_HandleTypeDef hdma_i2c2_rx;
DMA_HandleTypeDef hdma_i2c2_tx;
DMA_HandleTypeDef hdma_fmac_preload;
DMA_HandleTypeDef hdma_fmac_read;
DMA_HandleTypeDef hdma_fmac_write;
I2C_HandleTypeDef hi2c2;
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;
LPTIM_HandleTypeDef hlptim1;
LPTIM_HandleTypeDef hlptim2;
LPTIM_HandleTypeDef hlptim3;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim12;
extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;
extern TIM_HandleTypeDef htim15;
extern TIM_HandleTypeDef htim23;
extern TIM_HandleTypeDef htim24;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
FDCAN_HandleTypeDef hfdcan1;
FMAC_HandleTypeDef hfmac;

/************************************************
 *              Communication-FDCAN
 ***********************************************/

#ifdef HAL_FDCAN_MODULE_ENABLED

extern FDCAN_HandleTypeDef hfdcan1;

FDCAN::Instance FDCAN::instance1 = {
    .TX = PD1,
    .RX = PD0,
    .hfdcan = &hfdcan1,
    .instance = FDCAN1,
    .dlc = DLC::BYTES_64,
    .rx_location = FDCAN_RX_FIFO0,
    .fdcan_number = 1
};

FDCAN::Peripheral FDCAN::fdcan1 = FDCAN::Peripheral::peripheral1;

unordered_map<FDCAN::Peripheral, FDCAN::Instance*> FDCAN::available_fdcans = {
    {FDCAN::fdcan1, &FDCAN::instance1}
};

unordered_map<FDCAN_HandleTypeDef*, FDCAN::Instance*> FDCAN::handle_to_fdcan = {
    {FDCAN::instance1.hfdcan, &FDCAN::instance1}
};

#endif

/************************************************
 *              Communication-UART
 ***********************************************/
#ifdef HAL_UART_MODULE_ENABLED

UART::Instance UART::instance1 = {
    .TX = PA9,
    .RX = PA10,
    .huart = &huart1,
    .instance = USART1,
    .baud_rate = 115200,
    .word_length = UART_WORDLENGTH_8B,
};

UART::Instance UART::instance2 = {
    .TX = PD5,
    .RX = PD6,
    .huart = &huart2,
    .instance = USART2,
    .baud_rate = 115200,
    .word_length = UART_WORDLENGTH_8B,
};

UART::Peripheral UART::uart1 = UART::Peripheral::peripheral1;
UART::Peripheral UART::uart2 = UART::Peripheral::peripheral2;

unordered_map<UART::Peripheral, UART::Instance*> UART::available_uarts = {
    {UART::uart1, &UART::instance1},
    {UART::uart2, &UART::instance2},
};

uint8_t UART::printf_uart = 0;
bool UART::printf_ready = false;

#endif

/************************************************
 *					   I2C
 ***********************************************/

#ifdef HAL_I2C_MODULE_ENABLED
extern I2C_HandleTypeDef hi2c2;
I2C::Instance I2C::instance2 = {
    .SCL = PF1,
    .SDA = PB11,
    .hi2c = &hi2c2,
    .instance = I2C2,
    .RX_DMA = DMA::Stream::DMA1Stream3,
    .TX_DMA = DMA::Stream::DMA1Stream4
};
I2C::Peripheral I2C::i2c2 = I2C::Peripheral::peripheral2;
unordered_map<I2C::Peripheral, I2C::Instance*> I2C::available_i2cs = {{I2C::i2c2, &I2C::instance2}};
unordered_map<uint32_t, uint32_t> I2C::available_speed_frequencies = {{100, 0x60404E72}};
#endif

/************************************************
 *					   FMAC
 ***********************************************/

#ifdef HAL_FMAC_MODULE_ENABLED

MultiplierAccelerator::FMACInstance MultiplierAccelerator::Instance = {
    .hfmac = &hfmac,
    .dma_preload = DMA::Stream::DMA2Stream0,
    .dma_read = DMA::Stream::DMA2Stream1,
    .dma_write = DMA::Stream::DMA2Stream2,
};
#endif
