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
 *					   ADC
 ***********************************************/
#if 0 // Legacy ADC (replaced by NewADC). Kept here only as reference.
#if defined(HAL_ADC_MODULE_ENABLED) && defined(HAL_LPTIM_MODULE_ENABLED)

LowPowerTimer lptim1(*LPTIM1, hlptim1, LPTIM1_PERIOD, "LPTIM 1");
LowPowerTimer lptim2(*LPTIM2, hlptim2, LPTIM2_PERIOD, "LPTIM 2");
LowPowerTimer lptim3(*LPTIM3, hlptim3, LPTIM3_PERIOD, "LPTIM 3");

vector<uint32_t> channels1 = {};
vector<uint32_t> channels2 = {};
vector<uint32_t> channels3 = {};

ST_LIB::DMA_Domain::Instance dma_adc1 = {hdma_adc1};
ST_LIB::DMA_Domain::Instance dma_adc2 = {hdma_adc2};
ST_LIB::DMA_Domain::Instance dma_adc3 = {hdma_adc3};

ADC::InitData init_data1(ADC1, ADC_RESOLUTION_16B, ADC_EXTERNALTRIG_LPTIM1_OUT,
                         channels1, &dma_adc1, "ADC 1");
ADC::InitData init_data2(ADC2, ADC_RESOLUTION_16B, ADC_EXTERNALTRIG_LPTIM2_OUT,
                         channels2, &dma_adc2, "ADC 2");
ADC::InitData init_data3(ADC3, ADC_RESOLUTION_12B, ADC_EXTERNALTRIG_LPTIM3_OUT,
                         channels3, &dma_adc3, "ADC 3");

ADC::Peripheral ADC::peripherals[3] = {
    ADC::Peripheral(&hadc1, lptim1, init_data1),
    ADC::Peripheral(&hadc2, lptim2, init_data2),
    ADC::Peripheral(&hadc3, lptim3, init_data3)
};

map<Pin, ADC::Instance> ADC::available_instances = {
    {PF11, Instance(&peripherals[0], ADC_CHANNEL_2)},
    {PF12, Instance(&peripherals[0], ADC_CHANNEL_6)},
    {PF13, Instance(&peripherals[1], ADC_CHANNEL_2)},
    {PF14, Instance(&peripherals[1], ADC_CHANNEL_6)},
    {PF5, Instance(&peripherals[2], ADC_CHANNEL_4)},
    {PF6, Instance(&peripherals[2], ADC_CHANNEL_8)},
    {PF7, Instance(&peripherals[2], ADC_CHANNEL_3)},
    {PF8, Instance(&peripherals[2], ADC_CHANNEL_7)},
    {PF9, Instance(&peripherals[2], ADC_CHANNEL_2)},
    {PF10, Instance(&peripherals[2], ADC_CHANNEL_6)},
    {PC2, Instance(&peripherals[2], ADC_CHANNEL_0)},
    {PC3, Instance(&peripherals[2], ADC_CHANNEL_1)},
    {PF10, Instance(&peripherals[2], ADC_CHANNEL_6)},
    {PC0, Instance(&peripherals[0], ADC_CHANNEL_10)},
    {PA0, Instance(&peripherals[0], ADC_CHANNEL_16)},
    {PA3, Instance(&peripherals[0], ADC_CHANNEL_15)},
    {PA4, Instance(&peripherals[0], ADC_CHANNEL_18)},
    {PA5, Instance(&peripherals[0], ADC_CHANNEL_19)},
    {PA6, Instance(&peripherals[0], ADC_CHANNEL_3)},
    {PB0, Instance(&peripherals[0], ADC_CHANNEL_9)},
    {PB1, Instance(&peripherals[0], ADC_CHANNEL_5)}
};

uint32_t ADC::ranks[16] = {
    ADC_REGULAR_RANK_1,
    ADC_REGULAR_RANK_2,
    ADC_REGULAR_RANK_3,
    ADC_REGULAR_RANK_4,
    ADC_REGULAR_RANK_5,
    ADC_REGULAR_RANK_6,
    ADC_REGULAR_RANK_7,
    ADC_REGULAR_RANK_8,
    ADC_REGULAR_RANK_9,
    ADC_REGULAR_RANK_10,
    ADC_REGULAR_RANK_11,
    ADC_REGULAR_RANK_12,
    ADC_REGULAR_RANK_13,
    ADC_REGULAR_RANK_14,
    ADC_REGULAR_RANK_15,
    ADC_REGULAR_RANK_16
};

#endif
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
