#include "main.h"

#include "ST-LIB.hpp"

using namespace ST_LIB;

constexpr auto led = ST_LIB::DigitalOutputDomain::DigitalOutput(ST_LIB::PF13);

using MainBoard = ST_LIB::Board<led>;

#if !defined(EXAMPLE_ADC) && !defined(EXAMPLE_ETHERNET) && !defined(EXAMPLE_MPU) &&                \
    !defined(EXAMPLE_HARDFAULT) && !defined(EXAMPLE_EXTI)
int main(void) {
    MainBoard::init();

    auto& led_instance = MainBoard::instance_of<led>();

    while (1) {
        led_instance.toggle();
        HAL_Delay(200);
    }
}
#endif

extern "C" void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
