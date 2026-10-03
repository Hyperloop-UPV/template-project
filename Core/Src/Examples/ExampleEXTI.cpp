#ifdef EXAMPLE_EXTI

#include "main.h"
#include "ST-LIB.hpp"

#ifdef TEST_0
// Press the nucleo user button (PC13) to toggle the LED (PB0)

using namespace ST_LIB;
constexpr auto led_req = ST_LIB::DigitalOutputDomain::DigitalOutput(ST_LIB::PB0);

void toggle_led();

constexpr auto exti_req =
    ST_LIB::EXTIDomain::Device(ST_LIB::PC13, ST_LIB::EXTIDomain::Trigger::BOTH_EDGES, toggle_led);
using MainBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy, led_req, exti_req>;

auto led_instance = MainBoard::instance_of<led_req>();

void toggle_led() { led_instance.toggle(); }

extern "C" void BoardInit() { MainBoard::init(); }

int main(void) {
    while (1)
        ;
}
#endif

#endif
