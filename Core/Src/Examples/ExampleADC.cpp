#ifdef EXAMPLE_ADC

#include "main.h"
#include "ST-LIB.hpp"

using namespace ST_LIB;

#ifdef TEST_0

constinit float adc_value = 0.0f;
constexpr auto adc = ADCDomain::ADC(
    ST_LIB::PA0,
    adc_value,
    ADCDomain::Resolution::BITS_12,
    ADCDomain::SampleTime::CYCLES_8_5
);

int main(void) {
    using ExampleADCBoard = ST_LIB::Board<adc>;
    ExampleADCBoard::init();

    auto& adc_instance = ExampleADCBoard::instance_of<adc>();

    // Ready to compile for Nucleo. Validate by wiring PA0 to 3.3V and then to GND,
    // and watch adc_value in the debugger to confirm the change.
    while (1) {
        adc_instance.read();
        HAL_Delay(100);
    }
}

#endif // TEST_0
#endif // EXAMPLE_ADC
