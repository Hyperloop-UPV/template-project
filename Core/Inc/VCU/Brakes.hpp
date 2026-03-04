#pragma once
#include "ST-LIB.hpp"

/* using namespace ST_LIB;
constexpr auto led = ST_LIB::DigitalOutputDomain::DigitalOutput(ST_LIB::PB0);
 */

class Brakes{

    private:
    using DigitalOutputInstance = ST_LIB::DigitalOutputDomain::Instance;

    static inline DigitalOutputInstance* Actuator{};
    static inline DigitalOutputInstance* NucleoGreenLed{};
    static inline DigitalOutputInstance* TapeEnable{};
    static inline DigitalOutputInstance* Regulator{};


    static inline bool POD_braked{true};


    public:

    static void init(DigitalOutputInstance* actuator, DigitalOutputInstance* led, DigitalOutputInstance* tape_enable, DigitalOutputInstance* reg);
    static void brake(void);
    static void unbrake(void);
    static bool is_POD_braked(void);
    static void blink_nucleo_led(void);
    static void update(void);

};