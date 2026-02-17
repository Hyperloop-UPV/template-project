#ifdef EXAMPLE_EXTI

#include "main.h"
#include "ST-LIB.hpp"

#ifdef TEST_0
// Press the nucleo user button (PC13) to toggle the LED (PB0)

using namespace ST_LIB;
constexpr auto led = ST_LIB::DigitalOutputDomain::DigitalOutput(ST_LIB::PB0);
ST_LIB::DigitalOutputDomain::Instance* g_led;

void toggle_led() { g_led->toggle(); }

constexpr auto exti_req =
    ST_LIB::EXTIDomain::Device(ST_LIB::PC13, ST_LIB::EXTIDomain::Trigger::BOTH_EDGES, toggle_led);
using MainBoard = ST_LIB::Board<led, exti_req>;

int main(void) {
    MainBoard::init();

    static auto& led_instance = MainBoard::instance_of<led>();
    g_led = &led_instance;

    while (1) {
        // led_instance.toggle();
        // HAL_Delay(200);
    }
}
#endif

#endif
