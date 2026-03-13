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

constexpr ExampleInput kSingleChannelInput{ST_LIB::PA0, "PA0"};
constexpr ExampleInput kDualChannelInput0{ST_LIB::PA0, "PA0"};
constexpr ExampleInput kDualChannelInput1{ST_LIB::PC0, "PC0"};

constexpr const char* kTerminalHint = "Terminal: ST-LINK VCP over USB (USART3, 115200 8N1)";
constexpr const char* kSingleChannelWiringHint = "Connect PA0 to GND, 3V3 or a potentiometer.";
constexpr const char* kDualChannelWiringHint = "Connect PA0 and PC0 to two analog sources.";

void start_terminal() {
#ifdef HAL_UART_MODULE_ENABLED
    if (!UART::set_up_printf(UART::uart3)) {
        ErrorHandler("Unable to set up UART printf for ADC example");
    }
    UART::start();
#endif
}

void print_banner(const char* title, const char* wiring_hint, const char* columns_hint) {
    printf("\n\r=== %s ===\n\r", title);
    printf("%s\n\r", kTerminalHint);
    printf("%s\n\r", wiring_hint);
    printf("Columns: %s\n\r\n\r", columns_hint);
}

} // namespace

#ifdef TEST_0

constinit float adc_value = 0.0f;
constexpr auto adc_input = kSingleChannelInput;
constexpr auto adc = ADCDomain::ADC(
    adc_input.pin,
    adc_value,
    ADCDomain::Resolution::BITS_12,
    ADCDomain::SampleTime::CYCLES_8_5
);

int main(void) {
    using ExampleADCBoard = ST_LIB::Board<adc>;
    ExampleADCBoard::init();
    start_terminal();

    auto& adc_instance = ExampleADCBoard::instance_of<adc>();

    print_banner("ADC single-channel example", kSingleChannelWiringHint, "t_ms raw voltage[V]");
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

constexpr auto adc_input_0_cfg = kDualChannelInput0;
constexpr auto adc_input_1_cfg = kDualChannelInput1;

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
    start_terminal();

    auto& adc_input_0_instance = ExampleADCBoard::instance_of<adc_input_0>();
    auto& adc_input_1_instance = ExampleADCBoard::instance_of<adc_input_1>();

    print_banner(
        "ADC dual-channel example",
        kDualChannelWiringHint,
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
