#include "VCU/Brakes.hpp"

#include "Communications/Packets/OrderPackets.hpp"

void Brakes::init(DigitalOutputInstance* actuator, DigitalOutputInstance* led, DigitalOutputInstance* tape_enable, DigitalOutputInstance* reg) {
    Actuator = actuator;
    NucleoGreenLed = led;
    TapeEnable = tape_enable;
    Regulator = reg;

    tape_enable->turn_on();
    actuator->turn_off();
    Regulator->turn_off();
}

void Brakes::brake(void) {
    if (!POD_braked) {
        Actuator->turn_off();
        Regulator->turn_off();
        POD_braked = true;
    }
}

void Brakes::unbrake(void) {
    if (POD_braked) {
        Scheduler::set_timeout(2000000,[](){Regulator->turn_on();});

        Actuator->turn_on();
        POD_braked = false;
    }
}

bool Brakes::is_POD_braked(void) { return POD_braked; }

void Brakes::blink_nucleo_led() { NucleoGreenLed->toggle(); }

void Brakes::update(void) {
    if (OrderPackets::Brake_flag) {
        OrderPackets::Brake_flag = false;
        brake();
    }

    if (OrderPackets::Unbrake_flag) {
        OrderPackets::Unbrake_flag = false;
        unbrake();
    }
}