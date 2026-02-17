#ifdef EXAMPLE_HARDFAULT

#include "main.h"
#include "ST-LIB.hpp"

#ifdef TEST_MEMORY_FAULT
constexpr auto my_uint32_t = MPUDomain::Buffer<uint32_t>();

int main(void) {

    Hard_fault_check();
    STLIB::start();

    using myBoard = ST_LIB::Board<my_uint32_t>;
    myBoard::init();

    [[maybe_unused]] auto my_buffer =
        myBoard::instance_of<my_uint32_t>().template as<my_uint32_t>();
    my_buffer[1000000000] = 5;
    while (1) {
        STLIB::update();
    }
}

#endif

#ifdef TEST_BUS_FAULT

int main(void) {
    Hard_fault_check();
    *(uint32_t*)0xdead0000 = 0x20;
    STLIB::start();

    using myBoard = ST_LIB::Board<>;
    myBoard::init();

    while (1) {
        STLIB::update();
    }
}

#endif

#ifdef TEST_USAGE_FAULT

int main(void) {
    Hard_fault_check();
    __builtin_trap();
    STLIB::start();
    using myBoard = ST_LIB::Board<>;
    myBoard::init();

    while (1) {
        STLIB::update();
    }
}

#endif
#endif
