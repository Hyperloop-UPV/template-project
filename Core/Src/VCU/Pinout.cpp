#include "VCU/Pinout.hpp"

void init_board(){
    MyBoard::init();
}

ST_LIB::EthernetDomain::Instance& get_ethernet(){
    return MyBoard::instance_of<eth>();
}

ST_LIB::DigitalOutputDomain::Instance& get_nucleo_led(){
    return MyBoard::instance_of<led>();
}

ST_LIB::DigitalOutputDomain::Instance& get_actuator(){
    return MyBoard::instance_of<actuator>();
}

ST_LIB::DigitalOutputDomain::Instance& get_tape_enable(){
    return MyBoard::instance_of<tape_enable>();
}

ST_LIB::DigitalOutputDomain::Instance& get_regulator_out(){
    return MyBoard::instance_of<regulator_out>();
}

