#define EXAMPLE_BASE
#define TEST_0 // Test to be run

// Include all examples, run the one defined above
#include "Examples/ExampleMPU.cpp"

#ifdef EXAMPLE_BASE

#include "main.h"
#include "ST-LIB.hpp"

int main(void) {
#ifdef SIM_ON
    SharedMemory::start();
#endif

    DigitalOutput led_on(PB0);
    STLIB::start();
    HAL_Delay(200);  // Flash 2: Increased delay

    Time::register_low_precision_alarm(200, [&]() { led_on.toggle();  // Flash 3: Changed alarm interval 
    });

    while (1) {
        STLIB::update();
    }
}

void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}

#endif
