#include "main.h"

#include "ST-LIB.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

using namespace ST_LIB;

#ifndef EXAMPLE_SELECTED

constexpr auto led_req = ST_LIB::DigitalOutputDomain::DigitalOutput(ST_LIB::PB0);

using MainBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy, led_req>;
auto& led_instance = MainBoard::instance_of<led_req>();

extern "C" void BoardInit() { MainBoard::init(); }

int main(void) {
    while (1) {
        led_instance.toggle();
        HAL_Delay(200);
    }
}
#endif

extern "C" void Error_Handler(void) {
    PANIC("HAL error handler triggered");
    while (1) {
    }
}
