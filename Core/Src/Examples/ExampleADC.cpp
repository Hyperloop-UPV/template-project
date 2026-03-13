#ifdef EXAMPLE_ADC

#include <cstdio>
#include <cstdint>

#include "main.h"
#include "ST-LIB.hpp"

using namespace ST_LIB;

namespace {

struct ExampleInput {
    GPIODomain::Pin pin;
    const char* label;
};

UART::Peripheral& default_terminal_uart() {
#ifdef NUCLEO
    return UART::uart3;
#else
    return UART::uart2;
#endif
}

const char* terminal_hint() {
#ifdef NUCLEO
    return "Terminal: ST-LINK VCP over USB (USART3, 115200 8N1)";
#else
    return "Terminal: USART2 on PD5/PD6 (115200 8N1)";
#endif
}

[[maybe_unused]] const char* single_channel_wiring_hint() {
#ifdef NUCLEO
    return "Connect PA0 to GND, 3V3 or a potentiometer. On NUCLEO-H723ZG this is CN10 pin 29 / "
           "D32, not Arduino A0.";
#else
    return "Connect PA0 to GND, 3V3 or a potentiometer and watch the terminal output.";
#endif
}

[[maybe_unused]] const char* dual_channel_wiring_hint() {
#ifdef NUCLEO
    return "Connect PA0 and PC0 to two analog sources. PA1 is tied to the Ethernet PHY on "
           "NUCLEO-H723ZG via SB57 and is not a safe ADC test pin.";
#else
    return "Connect PA0 and PA1 to two analog sources to validate multichannel DMA.";
#endif
}

void start_terminal(UART::Peripheral& uart) {
#ifdef HAL_UART_MODULE_ENABLED
    if (!UART::set_up_printf(uart)) {
        ErrorHandler("Unable to set up UART printf for ADC example");
    }
    UART::start();
#else
    (void)uart;
#endif
}

void print_banner(const char* title, const char* wiring_hint, const char* columns_hint) {
    printf("\n\r=== %s ===\n\r", title);
    printf("%s\n\r", terminal_hint());
    printf("%s\n\r", wiring_hint);
    printf("Columns: %s\n\r\n\r", columns_hint);
}

consteval ExampleInput single_channel_input() { return {ST_LIB::PA0, "PA0"}; }

consteval ExampleInput dual_channel_input_0() { return {ST_LIB::PA0, "PA0"}; }

consteval ExampleInput dual_channel_input_1() {
#ifdef NUCLEO
    return {ST_LIB::PC0, "PC0"};
#else
    return {ST_LIB::PA1, "PA1"};
#endif
}

} // namespace

#ifdef TEST_0

constinit float adc_value = 0.0f;
constexpr auto adc_input = single_channel_input();
constexpr auto adc = ADCDomain::ADC(
    adc_input.pin,
    adc_value,
    ADCDomain::Resolution::BITS_12,
    ADCDomain::SampleTime::CYCLES_8_5
);

int main(void) {
    using ExampleADCBoard = ST_LIB::Board<adc>;
    ExampleADCBoard::init();
    start_terminal(default_terminal_uart());

    auto& adc_instance = ExampleADCBoard::instance_of<adc>();

    print_banner("ADC single-channel example", single_channel_wiring_hint(), "t_ms raw voltage[V]");
    printf("Reading input: %s\n\r\n\r", adc_input.label);

    uint32_t sample_index = 0;
    while (1) {
        adc_instance.read();
        const float raw = adc_instance.get_raw();
        const float voltage = adc_instance.get_value();

        printf("%10lu %8.0f %10.4f\n\r", HAL_GetTick(), raw, voltage);

        ++sample_index;
        if ((sample_index % 20U) == 0U) {
            printf("Current mirrored output buffer value: %.4f V\n\r\n\r", adc_value);
        }

        HAL_Delay(100);
    }
}

#endif // TEST_0

#ifdef TEST_1

constinit float adc_input_0_value = 0.0f;
constinit float adc_input_1_value = 0.0f;

constexpr auto adc_input_0_cfg = dual_channel_input_0();
constexpr auto adc_input_1_cfg = dual_channel_input_1();

constexpr auto adc_input_0 = ADCDomain::ADC(
    adc_input_0_cfg.pin,
    adc_input_0_value,
    ADCDomain::Resolution::BITS_12,
    ADCDomain::SampleTime::CYCLES_8_5
);

constexpr auto adc_input_1 = ADCDomain::ADC(
    adc_input_1_cfg.pin,
    adc_input_1_value,
    ADCDomain::Resolution::BITS_12,
    ADCDomain::SampleTime::CYCLES_8_5
);

int main(void) {
    using ExampleADCBoard = ST_LIB::Board<adc_input_0, adc_input_1>;
    ExampleADCBoard::init();
    start_terminal(default_terminal_uart());

    auto& adc_input_0_instance = ExampleADCBoard::instance_of<adc_input_0>();
    auto& adc_input_1_instance = ExampleADCBoard::instance_of<adc_input_1>();

    print_banner(
        "ADC dual-channel example",
        dual_channel_wiring_hint(),
        "t_ms raw_0 raw_1 v_0[V] v_1[V]"
    );
    printf("Reading inputs: %s and %s\n\r\n\r", adc_input_0_cfg.label, adc_input_1_cfg.label);

    while (1) {
        adc_input_0_instance.read();
        adc_input_1_instance.read();

        const float raw_0 = adc_input_0_instance.get_raw();
        const float raw_1 = adc_input_1_instance.get_raw();
        const float voltage_0 = adc_input_0_instance.get_value();
        const float voltage_1 = adc_input_1_instance.get_value();

        printf(
            "%10lu %8.0f %8.0f %10.4f %10.4f\n\r",
            HAL_GetTick(),
            raw_0,
            raw_1,
            voltage_0,
            voltage_1
        );

        HAL_Delay(100);
    }
}

#endif // TEST_1
#endif // EXAMPLE_ADC
