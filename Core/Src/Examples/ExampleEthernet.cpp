#ifdef EXAMPLE_ETHERNET

#include "main.h"
#include "ST-LIB.hpp"

using namespace ST_LIB;

#ifdef TEST_0

constexpr auto led = ST_LIB::DigitalOutputDomain::DigitalOutput(ST_LIB::PB0);

#ifdef STLIB_ETH
#if defined(USE_PHY_LAN8742)
constexpr auto eth = EthernetDomain::Ethernet(
    EthernetDomain::PINSET_H10,
    "00:80:e1:00:01:07",
    "192.168.1.7",
    "255.255.0.0"
);
#elif defined(USE_PHY_LAN8700)
constexpr auto eth = EthernetDomain::Ethernet(
    EthernetDomain::PINSET_H10,
    "00:80:e1:00:01:07",
    "192.168.1.7",
    "255.255.0.0"
);
#elif defined(USE_PHY_KSZ8041)
constexpr auto eth = EthernetDomain::Ethernet(
    EthernetDomain::PINSET_H11,
    "00:80:e1:00:01:07",
    "192.168.1.7",
    "255.255.0.0"
);
#else
#error "No PHY selected for Ethernet pinset selection"
#endif
using ExampleEthernetBoard = ST_LIB::Board<eth, led>;
#else
using ExampleEthernetBoard = ST_LIB::Board<led>;
#endif

int main(void) {
    Hard_fault_check();

    ExampleEthernetBoard::init();
#ifdef STLIB_ETH
    auto& eth_instance = ExampleEthernetBoard::instance_of<eth>();
#endif
    auto& led_instance = ExampleEthernetBoard::instance_of<led>();

    led_instance.turn_on();
    while (1) {
#ifdef STLIB_ETH
        eth_instance.update();
#endif
    }
}

#endif // TEST_0
#endif // EXAMPLE_ETHERNET
